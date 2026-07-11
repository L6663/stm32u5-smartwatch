/**
  ******************************************************************************
  * @file    health_app.c
  * @brief   MAX30102 driver, acquisition task, algorithm and Health page update
  ******************************************************************************
  *
  * Architecture:
  *
  * MAX30102/I2C1
  *      -> HealthTask
  *      -> private CMSIS-RTOS2 message queue
  *      -> HealthUI_ProcessQueue()
  *      -> existing SquareLine Health page objects
  *
  * HealthTask never calls LVGL. All LVGL operations remain in LVGLTask.
  *
  * The SpO2 result is an uncalibrated project-level estimate. It is not a
  * medical measurement and must not be used for diagnosis.
  ******************************************************************************
  */

#include "health_app.h"

#include "cmsis_os2.h"
#include "i2c.h"
#include "usart.h"
#include "ui.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/* Created by app_freertos.c. */
extern osMutexId_t i2c1MutexHandle;

/* -------------------------------------------------------------------------- */
/* Public diagnostics                                                         */
/* -------------------------------------------------------------------------- */

volatile uint8_t  g_max30102_part_id = 0U;
volatile uint32_t g_max30102_last_red = 0U;
volatile uint32_t g_max30102_last_ir = 0U;
volatile uint16_t g_health_last_bpm = 0U;
volatile uint8_t  g_health_last_spo2 = 0U;
volatile uint32_t g_health_i2c_error_count = 0U;
volatile uint32_t g_health_ui_drop_count = 0U;

/* -------------------------------------------------------------------------- */
/* MAX30102 definitions                                                       */
/* -------------------------------------------------------------------------- */

#define MAX30102_I2C_ADDRESS              (0x57U << 1)
#define MAX30102_I2C_TIMEOUT_MS           100U

#define MAX30102_REG_INT_STATUS_1         0x00U
#define MAX30102_REG_INT_STATUS_2         0x01U
#define MAX30102_REG_INT_ENABLE_1         0x02U
#define MAX30102_REG_INT_ENABLE_2         0x03U
#define MAX30102_REG_FIFO_WR_PTR          0x04U
#define MAX30102_REG_OVF_COUNTER          0x05U
#define MAX30102_REG_FIFO_RD_PTR          0x06U
#define MAX30102_REG_FIFO_DATA            0x07U
#define MAX30102_REG_FIFO_CONFIG          0x08U
#define MAX30102_REG_MODE_CONFIG          0x09U
#define MAX30102_REG_SPO2_CONFIG          0x0AU
#define MAX30102_REG_LED1_PA              0x0CU
#define MAX30102_REG_LED2_PA              0x0DU
#define MAX30102_REG_PART_ID              0xFFU

#define MAX30102_EXPECTED_PART_ID         0x15U

#define MAX30102_MODE_RESET               0x40U
#define MAX30102_MODE_SHUTDOWN            0x80U
#define MAX30102_MODE_SPO2                0x03U

/*
 * FIFO_CONFIG:
 *   SMP_AVE = 000: no averaging
 *   FIFO_ROLLOVER_EN = 1
 *   FIFO_A_FULL = 0x0F
 */
#define MAX30102_FIFO_CONFIG_VALUE        0x1FU

/*
 * SPO2_CONFIG:
 *   ADC range = 4096 nA
 *   sample rate = 100 samples/s
 *   pulse width = 411 us
 *   ADC resolution = 18 bit
 */
#define MAX30102_SPO2_CONFIG_VALUE        0x27U

/*
 * Initial LED currents. These can be tuned after observing the raw IR/RED
 * values in Keil Watch.
 */
#define MAX30102_RED_LED_CURRENT          0x24U
#define MAX30102_IR_LED_CURRENT           0x24U

typedef struct
{
    uint32_t red;
    uint32_t ir;
} max30102_sample_t;

/* -------------------------------------------------------------------------- */
/* Health messages and queues                                                 */
/* -------------------------------------------------------------------------- */

typedef enum
{
    HEALTH_COMMAND_START = 1,
    HEALTH_COMMAND_STOP
} health_command_t;

typedef enum
{
    HEALTH_STATE_IDLE = 0,
    HEALTH_STATE_STARTING,
    HEALTH_STATE_MEASURING,
    HEALTH_STATE_NO_FINGER,
    HEALTH_STATE_SENSOR_ERROR
} health_state_t;

typedef enum
{
    HEALTH_UI_MESSAGE_STATE = 1,
    HEALTH_UI_MESSAGE_RESULT
} health_ui_message_type_t;

typedef struct
{
    health_ui_message_type_t type;
    health_state_t state;

    uint16_t heart_rate;
    uint16_t min_heart_rate;
    uint16_t max_heart_rate;

    uint8_t spo2;
    uint8_t signal_quality;

    int16_t waveform;
    uint16_t reserved;
} health_ui_message_t;

static osMessageQueueId_t g_health_command_queue = NULL;
static osMessageQueueId_t g_health_ui_queue = NULL;

/* -------------------------------------------------------------------------- */
/* Heart-rate algorithm                                                       */
/* -------------------------------------------------------------------------- */

#define HEALTH_SAMPLE_RATE_HZ                 100U
#define HEALTH_FINGER_IR_THRESHOLD            10000U
#define HEALTH_NO_FINGER_CONFIRM_SAMPLES      20U

/* 30 samples = 300 ms, 200 samples = 2 s. */
#define HEALTH_MIN_BEAT_INTERVAL_SAMPLES      30U
#define HEALTH_MAX_BEAT_INTERVAL_SAMPLES      200U

#define HEALTH_SIGNAL_WINDOW_SAMPLES          100U
#define HEALTH_SPO2_WINDOW_SAMPLES            100U
#define HEALTH_BPM_AVERAGE_COUNT              4U

/*
 * Most reflective MAX30102 modules produce a downward IR pulse in raw data.
 * Keep this at 1 first. If finger detection works but BPM never appears, change
 * it to 0 and rebuild.
 */
#define HEALTH_PULSE_INVERTED                 1U

typedef struct
{
    bool dc_initialized;
    bool finger_present;
    bool threshold_ready;

    uint16_t no_finger_count;

    int32_t dc_red;
    int32_t dc_ir;

    int32_t smooth_buffer[4];
    int64_t smooth_sum;
    uint8_t smooth_index;

    int32_t previous_2;
    int32_t previous_1;

    int32_t signal_min;
    int32_t signal_max;
    int32_t peak_threshold;
    uint16_t signal_window_count;
    uint8_t signal_quality;

    uint32_t sample_index;
    uint32_t last_peak_sample;

    uint16_t bpm_history[HEALTH_BPM_AVERAGE_COUNT];
    uint8_t bpm_history_count;
    uint8_t bpm_history_index;

    uint16_t current_bpm;
    uint16_t min_bpm;
    uint16_t max_bpm;

    uint64_t red_ac_sum;
    uint64_t ir_ac_sum;
    uint64_t red_dc_sum;
    uint64_t ir_dc_sum;
    uint16_t spo2_window_count;
    uint8_t current_spo2;
} health_algorithm_t;

typedef struct
{
    bool finger_changed;
    bool finger_present;
    bool new_result;

    uint16_t bpm;
    uint16_t min_bpm;
    uint16_t max_bpm;

    uint8_t spo2;
    uint8_t signal_quality;
    int16_t waveform;
} health_algorithm_result_t;

/* -------------------------------------------------------------------------- */
/* LVGL-side state                                                            */
/* -------------------------------------------------------------------------- */

static lv_chart_series_t *g_health_chart_series = NULL;
static lv_obj_t *g_spo2_value_label = NULL;
static bool g_health_ui_initialized = false;

/* -------------------------------------------------------------------------- */
/* Local prototypes                                                           */
/* -------------------------------------------------------------------------- */

static void Health_DebugPrintf(const char *format, ...);

static HAL_StatusTypeDef MAX30102_WriteRegister(uint8_t reg, uint8_t value);
static HAL_StatusTypeDef MAX30102_ReadRegister(uint8_t reg, uint8_t *value);
static HAL_StatusTypeDef MAX30102_ReadRegisters(
    uint8_t reg,
    uint8_t *buffer,
    uint16_t length
);
static HAL_StatusTypeDef MAX30102_ClearFIFO(void);
static HAL_StatusTypeDef MAX30102_Reset(void);
static HAL_StatusTypeDef MAX30102_Initialize(void);
static HAL_StatusTypeDef MAX30102_Start(void);
static HAL_StatusTypeDef MAX30102_Stop(void);
static HAL_StatusTypeDef MAX30102_GetAvailableSamples(uint8_t *count);
static HAL_StatusTypeDef MAX30102_ReadFIFO(max30102_sample_t *sample);

static void Health_SendState(health_state_t state);
static void Health_SendResult(const health_algorithm_result_t *result);

static void HealthAlgorithm_Reset(health_algorithm_t *algorithm);
static void HealthAlgorithm_ResetSignal(health_algorithm_t *algorithm);
static uint16_t HealthAlgorithm_AverageBpm(
    health_algorithm_t *algorithm,
    uint16_t bpm
);
static uint8_t HealthAlgorithm_CalculateSpO2(
    health_algorithm_t *algorithm
);
static void HealthAlgorithm_Process(
    health_algorithm_t *algorithm,
    const max30102_sample_t *sample,
    health_algorithm_result_t *result
);

static void HealthUI_ButtonEvent(lv_event_t *event);
static void HealthUI_ClearResultDisplay(void);
static void HealthUI_SetState(health_state_t state);
static void HealthUI_ApplyResult(const health_ui_message_t *message);

/* -------------------------------------------------------------------------- */
/* Debug output                                                               */
/* -------------------------------------------------------------------------- */

static void Health_DebugPrintf(const char *format, ...)
{
    char buffer[160];
    va_list arguments;
    int length;

    if (format == NULL)
    {
        return;
    }

    va_start(arguments, format);
    length = vsnprintf(
        buffer,
        sizeof(buffer),
        format,
        arguments
    );
    va_end(arguments);

    if (length <= 0)
    {
        return;
    }

    if (length >= (int)sizeof(buffer))
    {
        length = (int)sizeof(buffer) - 1;
    }

    (void)HAL_UART_Transmit(
        &huart1,
        (uint8_t *)buffer,
        (uint16_t)length,
        100U
    );
}

/* -------------------------------------------------------------------------- */
/* MAX30102 low-level driver                                                  */
/* -------------------------------------------------------------------------- */

static HAL_StatusTypeDef MAX30102_WriteRegister(
    uint8_t reg,
    uint8_t value)
{
    return HAL_I2C_Mem_Write(
        &hi2c1,
        MAX30102_I2C_ADDRESS,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        &value,
        1U,
        MAX30102_I2C_TIMEOUT_MS
    );
}

static HAL_StatusTypeDef MAX30102_ReadRegister(
    uint8_t reg,
    uint8_t *value)
{
    if (value == NULL)
    {
        return HAL_ERROR;
    }

    return HAL_I2C_Mem_Read(
        &hi2c1,
        MAX30102_I2C_ADDRESS,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        value,
        1U,
        MAX30102_I2C_TIMEOUT_MS
    );
}

static HAL_StatusTypeDef MAX30102_ReadRegisters(
    uint8_t reg,
    uint8_t *buffer,
    uint16_t length)
{
    if ((buffer == NULL) || (length == 0U))
    {
        return HAL_ERROR;
    }

    return HAL_I2C_Mem_Read(
        &hi2c1,
        MAX30102_I2C_ADDRESS,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        buffer,
        length,
        MAX30102_I2C_TIMEOUT_MS
    );
}

static HAL_StatusTypeDef MAX30102_ClearFIFO(void)
{
    HAL_StatusTypeDef status;

    status = MAX30102_WriteRegister(
        MAX30102_REG_FIFO_WR_PTR,
        0U
    );

    if (status != HAL_OK)
    {
        return status;
    }

    status = MAX30102_WriteRegister(
        MAX30102_REG_OVF_COUNTER,
        0U
    );

    if (status != HAL_OK)
    {
        return status;
    }

    return MAX30102_WriteRegister(
        MAX30102_REG_FIFO_RD_PTR,
        0U
    );
}

static HAL_StatusTypeDef MAX30102_Reset(void)
{
    HAL_StatusTypeDef status;
    uint8_t mode;
    uint32_t timeout;

    status = MAX30102_WriteRegister(
        MAX30102_REG_MODE_CONFIG,
        MAX30102_MODE_RESET
    );

    if (status != HAL_OK)
    {
        return status;
    }

    for (timeout = 0U; timeout < 100U; timeout++)
    {
        osDelay(1U);

        status = MAX30102_ReadRegister(
            MAX30102_REG_MODE_CONFIG,
            &mode
        );

        if (status != HAL_OK)
        {
            return status;
        }

        if ((mode & MAX30102_MODE_RESET) == 0U)
        {
            return HAL_OK;
        }
    }

    return HAL_TIMEOUT;
}

static HAL_StatusTypeDef MAX30102_Initialize(void)
{
    HAL_StatusTypeDef status;
    uint8_t part_id;
    uint8_t dummy;

    status = HAL_I2C_IsDeviceReady(
        &hi2c1,
        MAX30102_I2C_ADDRESS,
        3U,
        MAX30102_I2C_TIMEOUT_MS
    );

    if (status != HAL_OK)
    {
        return status;
    }

    status = MAX30102_ReadRegister(
        MAX30102_REG_PART_ID,
        &part_id
    );

    if (status != HAL_OK)
    {
        return status;
    }

    g_max30102_part_id = part_id;

    if (part_id != MAX30102_EXPECTED_PART_ID)
    {
        return HAL_ERROR;
    }

    status = MAX30102_Reset();
    if (status != HAL_OK)
    {
        return status;
    }

    status = MAX30102_WriteRegister(
        MAX30102_REG_INT_ENABLE_1,
        0U
    );
    if (status != HAL_OK)
    {
        return status;
    }

    status = MAX30102_WriteRegister(
        MAX30102_REG_INT_ENABLE_2,
        0U
    );
    if (status != HAL_OK)
    {
        return status;
    }

    status = MAX30102_ClearFIFO();
    if (status != HAL_OK)
    {
        return status;
    }

    status = MAX30102_WriteRegister(
        MAX30102_REG_FIFO_CONFIG,
        MAX30102_FIFO_CONFIG_VALUE
    );
    if (status != HAL_OK)
    {
        return status;
    }

    status = MAX30102_WriteRegister(
        MAX30102_REG_SPO2_CONFIG,
        MAX30102_SPO2_CONFIG_VALUE
    );
    if (status != HAL_OK)
    {
        return status;
    }

    status = MAX30102_WriteRegister(
        MAX30102_REG_LED1_PA,
        MAX30102_RED_LED_CURRENT
    );
    if (status != HAL_OK)
    {
        return status;
    }

    status = MAX30102_WriteRegister(
        MAX30102_REG_LED2_PA,
        MAX30102_IR_LED_CURRENT
    );
    if (status != HAL_OK)
    {
        return status;
    }

    status = MAX30102_WriteRegister(
        MAX30102_REG_MODE_CONFIG,
        MAX30102_MODE_SPO2
    );
    if (status != HAL_OK)
    {
        return status;
    }

    /* Clear stale interrupt status bits. */
    (void)MAX30102_ReadRegister(
        MAX30102_REG_INT_STATUS_1,
        &dummy
    );

    (void)MAX30102_ReadRegister(
        MAX30102_REG_INT_STATUS_2,
        &dummy
    );

    /* Leave the optical sensor off until Start Measuring is pressed. */
    return MAX30102_Stop();
}

static HAL_StatusTypeDef MAX30102_Start(void)
{
    HAL_StatusTypeDef status;

    status = MAX30102_ClearFIFO();
    if (status != HAL_OK)
    {
        return status;
    }

    return MAX30102_WriteRegister(
        MAX30102_REG_MODE_CONFIG,
        MAX30102_MODE_SPO2
    );
}

static HAL_StatusTypeDef MAX30102_Stop(void)
{
    HAL_StatusTypeDef status;
    uint8_t mode;

    status = MAX30102_ReadRegister(
        MAX30102_REG_MODE_CONFIG,
        &mode
    );

    if (status != HAL_OK)
    {
        return status;
    }

    mode |= MAX30102_MODE_SHUTDOWN;

    return MAX30102_WriteRegister(
        MAX30102_REG_MODE_CONFIG,
        mode
    );
}

static HAL_StatusTypeDef MAX30102_GetAvailableSamples(
    uint8_t *count)
{
    HAL_StatusTypeDef status;
    uint8_t pointers[3];
    uint8_t write_pointer;
    uint8_t overflow_counter;
    uint8_t read_pointer;

    if (count == NULL)
    {
        return HAL_ERROR;
    }

    status = MAX30102_ReadRegisters(
        MAX30102_REG_FIFO_WR_PTR,
        pointers,
        sizeof(pointers)
    );

    if (status != HAL_OK)
    {
        return status;
    }

    write_pointer = pointers[0] & 0x1FU;
    overflow_counter = pointers[1] & 0x1FU;
    read_pointer = pointers[2] & 0x1FU;

    *count = (uint8_t)(
        (write_pointer - read_pointer) & 0x1FU
    );

    /*
     * With FIFO rollover enabled, write == read can also mean that data was
     * overwritten. Read a full batch in that case instead of reporting empty.
     */
    if ((*count == 0U) && (overflow_counter != 0U))
    {
        *count = 31U;
    }

    return HAL_OK;
}

static HAL_StatusTypeDef MAX30102_ReadFIFO(
    max30102_sample_t *sample)
{
    HAL_StatusTypeDef status;
    uint8_t data[6];

    if (sample == NULL)
    {
        return HAL_ERROR;
    }

    status = MAX30102_ReadRegisters(
        MAX30102_REG_FIFO_DATA,
        data,
        sizeof(data)
    );

    if (status != HAL_OK)
    {
        return status;
    }

    /* SpO2 mode FIFO order: RED[17:0], IR[17:0]. */
    sample->red =
        ((((uint32_t)data[0]) << 16) |
         (((uint32_t)data[1]) << 8)  |
         ((uint32_t)data[2])) &
        0x3FFFFU;

    sample->ir =
        ((((uint32_t)data[3]) << 16) |
         (((uint32_t)data[4]) << 8)  |
         ((uint32_t)data[5])) &
        0x3FFFFU;

    g_max30102_last_red = sample->red;
    g_max30102_last_ir = sample->ir;

    return HAL_OK;
}

/* -------------------------------------------------------------------------- */
/* Queue helpers                                                              */
/* -------------------------------------------------------------------------- */

void HealthApp_CreateObjects(void)
{
    static const osMessageQueueAttr_t command_attributes =
    {
        .name = "healthCommandQueue"
    };

    static const osMessageQueueAttr_t ui_attributes =
    {
        .name = "healthUiQueue"
    };

    if (g_health_command_queue == NULL)
    {
        g_health_command_queue = osMessageQueueNew(
            4U,
            sizeof(health_command_t),
            &command_attributes
        );
    }

    if (g_health_ui_queue == NULL)
    {
        g_health_ui_queue = osMessageQueueNew(
            12U,
            sizeof(health_ui_message_t),
            &ui_attributes
        );
    }
}

static void Health_PutCommand(health_command_t command)
{
    if (g_health_command_queue == NULL)
    {
        return;
    }

    if (osMessageQueuePut(
            g_health_command_queue,
            &command,
            0U,
            0U) != osOK)
    {
        /*
         * Only the newest button state matters. Drop stale commands if the
         * queue ever becomes full.
         */
        (void)osMessageQueueReset(
            g_health_command_queue
        );

        (void)osMessageQueuePut(
            g_health_command_queue,
            &command,
            0U,
            0U
        );
    }
}

void HealthApp_RequestStart(void)
{
    Health_PutCommand(HEALTH_COMMAND_START);
}

void HealthApp_RequestStop(void)
{
    Health_PutCommand(HEALTH_COMMAND_STOP);
}

static void Health_PutUiMessage(
    const health_ui_message_t *message)
{
    health_ui_message_t discarded_message;

    if ((g_health_ui_queue == NULL) ||
        (message == NULL))
    {
        return;
    }

    if (osMessageQueuePut(
            g_health_ui_queue,
            message,
            0U,
            0U) != osOK)
    {
        /*
         * Keep the display responsive: discard the oldest health update and
         * submit the newest state/result.
         */
        (void)osMessageQueueGet(
            g_health_ui_queue,
            &discarded_message,
            NULL,
            0U
        );

        if (osMessageQueuePut(
                g_health_ui_queue,
                message,
                0U,
                0U) != osOK)
        {
            g_health_ui_drop_count++;
        }
    }
}

static void Health_SendState(health_state_t state)
{
    health_ui_message_t message;

    memset(&message, 0, sizeof(message));

    message.type = HEALTH_UI_MESSAGE_STATE;
    message.state = state;

    Health_PutUiMessage(&message);
}

static void Health_SendResult(
    const health_algorithm_result_t *result)
{
    health_ui_message_t message;

    if (result == NULL)
    {
        return;
    }

    memset(&message, 0, sizeof(message));

    message.type = HEALTH_UI_MESSAGE_RESULT;
    message.state = HEALTH_STATE_MEASURING;

    message.heart_rate = result->bpm;
    message.min_heart_rate = result->min_bpm;
    message.max_heart_rate = result->max_bpm;

    message.spo2 = result->spo2;
    message.signal_quality = result->signal_quality;
    message.waveform = result->waveform;

    Health_PutUiMessage(&message);
}

/* -------------------------------------------------------------------------- */
/* Algorithm                                                                  */
/* -------------------------------------------------------------------------- */

static void HealthAlgorithm_Reset(
    health_algorithm_t *algorithm)
{
    if (algorithm == NULL)
    {
        return;
    }

    memset(algorithm, 0, sizeof(*algorithm));

    algorithm->signal_min = INT32_MAX;
    algorithm->signal_max = INT32_MIN;
}

static void HealthAlgorithm_ResetSignal(
    health_algorithm_t *algorithm)
{
    uint16_t session_min_bpm;
    uint16_t session_max_bpm;

    if (algorithm == NULL)
    {
        return;
    }

    session_min_bpm = algorithm->min_bpm;
    session_max_bpm = algorithm->max_bpm;

    memset(algorithm, 0, sizeof(*algorithm));

    algorithm->signal_min = INT32_MAX;
    algorithm->signal_max = INT32_MIN;
    algorithm->min_bpm = session_min_bpm;
    algorithm->max_bpm = session_max_bpm;
}

static uint16_t HealthAlgorithm_AverageBpm(
    health_algorithm_t *algorithm,
    uint16_t bpm)
{
    uint32_t total;
    uint8_t index;

    algorithm->bpm_history[
        algorithm->bpm_history_index
    ] = bpm;

    algorithm->bpm_history_index++;

    if (algorithm->bpm_history_index >=
        HEALTH_BPM_AVERAGE_COUNT)
    {
        algorithm->bpm_history_index = 0U;
    }

    if (algorithm->bpm_history_count <
        HEALTH_BPM_AVERAGE_COUNT)
    {
        algorithm->bpm_history_count++;
    }

    total = 0U;

    for (index = 0U;
         index < algorithm->bpm_history_count;
         index++)
    {
        total += algorithm->bpm_history[index];
    }

    return (uint16_t)(
        total / algorithm->bpm_history_count
    );
}

static uint8_t HealthAlgorithm_CalculateSpO2(
    health_algorithm_t *algorithm)
{
    uint64_t red_ac_average;
    uint64_t ir_ac_average;
    uint64_t red_dc_average;
    uint64_t ir_dc_average;

    uint64_t numerator;
    uint64_t denominator;

    uint32_t ratio_x1000;
    int32_t spo2;

    if ((algorithm == NULL) ||
        (algorithm->spo2_window_count == 0U))
    {
        return 0U;
    }

    red_ac_average =
        algorithm->red_ac_sum /
        algorithm->spo2_window_count;

    ir_ac_average =
        algorithm->ir_ac_sum /
        algorithm->spo2_window_count;

    red_dc_average =
        algorithm->red_dc_sum /
        algorithm->spo2_window_count;

    ir_dc_average =
        algorithm->ir_dc_sum /
        algorithm->spo2_window_count;

    if ((red_ac_average == 0U) ||
        (ir_ac_average == 0U) ||
        (red_dc_average == 0U) ||
        (ir_dc_average == 0U))
    {
        return 0U;
    }

    /*
     * Ratio of ratios:
     *
     * R = (ACred / DCred) / (ACir / DCir)
     */
    numerator =
        red_ac_average *
        ir_dc_average *
        1000ULL;

    denominator =
        ir_ac_average *
        red_dc_average;

    if (denominator == 0U)
    {
        return 0U;
    }

    ratio_x1000 =
        (uint32_t)(numerator / denominator);

    /*
     * Demonstration approximation:
     *
     * SpO2 ~= 110 - 25R
     *
     * A finished product requires optical/mechanical calibration against a
     * reference pulse oximeter.
     */
    spo2 =
        110 -
        (int32_t)(
            (25U * ratio_x1000) / 1000U
        );

    if (spo2 > 100)
    {
        spo2 = 100;
    }

    if (spo2 < 70)
    {
        spo2 = 70;
    }

    return (uint8_t)spo2;
}

static void HealthAlgorithm_Process(
    health_algorithm_t *algorithm,
    const max30102_sample_t *sample,
    health_algorithm_result_t *result)
{
    bool old_finger_present;

    int32_t ac_red;
    int32_t ac_ir;
    int32_t filtered;
    int32_t amplitude;

    int32_t absolute_red;
    int32_t absolute_ir;

    uint32_t beat_interval;
    uint16_t instantaneous_bpm;

    if ((algorithm == NULL) ||
        (sample == NULL) ||
        (result == NULL))
    {
        return;
    }

    memset(result, 0, sizeof(*result));

    old_finger_present = algorithm->finger_present;

    /* Debounced finger detection based on the IR DC level. */
    if (sample->ir < HEALTH_FINGER_IR_THRESHOLD)
    {
        if (algorithm->no_finger_count <
            HEALTH_NO_FINGER_CONFIRM_SAMPLES)
        {
            algorithm->no_finger_count++;
        }

        if (algorithm->no_finger_count >=
            HEALTH_NO_FINGER_CONFIRM_SAMPLES)
        {
            if (algorithm->finger_present)
            {
                HealthAlgorithm_ResetSignal(algorithm);
                algorithm->no_finger_count =
                    HEALTH_NO_FINGER_CONFIRM_SAMPLES;
            }

            algorithm->finger_present = false;
        }

        result->finger_present =
            algorithm->finger_present;

        result->finger_changed =
            (old_finger_present !=
             algorithm->finger_present);

        return;
    }

    algorithm->no_finger_count = 0U;
    algorithm->finger_present = true;

    result->finger_present = true;
    result->finger_changed =
        (old_finger_present !=
         algorithm->finger_present);

    /*
     * The first valid finger sample initializes the slowly varying DC level.
     */
    if (!algorithm->dc_initialized)
    {
        algorithm->dc_red = (int32_t)sample->red;
        algorithm->dc_ir = (int32_t)sample->ir;
        algorithm->dc_initialized = true;

        return;
    }

    /* First-order DC estimator, approximately 32-sample time constant. */
    algorithm->dc_red +=
        ((int32_t)sample->red -
         algorithm->dc_red) / 32;

    algorithm->dc_ir +=
        ((int32_t)sample->ir -
         algorithm->dc_ir) / 32;

#if HEALTH_PULSE_INVERTED
    ac_red =
        algorithm->dc_red -
        (int32_t)sample->red;

    ac_ir =
        algorithm->dc_ir -
        (int32_t)sample->ir;
#else
    ac_red =
        (int32_t)sample->red -
        algorithm->dc_red;

    ac_ir =
        (int32_t)sample->ir -
        algorithm->dc_ir;
#endif

    /* Four-point moving average. */
    algorithm->smooth_sum -=
        algorithm->smooth_buffer[
            algorithm->smooth_index
        ];

    algorithm->smooth_buffer[
        algorithm->smooth_index
    ] = ac_ir;

    algorithm->smooth_sum += ac_ir;

    algorithm->smooth_index =
        (uint8_t)(
            (algorithm->smooth_index + 1U) &
            0x03U
        );

    filtered =
        (int32_t)(algorithm->smooth_sum / 4);

    if (filtered > 32767)
    {
        result->waveform = 32767;
    }
    else if (filtered < -32768)
    {
        result->waveform = -32768;
    }
    else
    {
        result->waveform = (int16_t)filtered;
    }

    /*
     * A one-second min/max window builds an adaptive peak threshold.
     */
    if (filtered < algorithm->signal_min)
    {
        algorithm->signal_min = filtered;
    }

    if (filtered > algorithm->signal_max)
    {
        algorithm->signal_max = filtered;
    }

    algorithm->signal_window_count++;

    if (algorithm->signal_window_count >=
        HEALTH_SIGNAL_WINDOW_SAMPLES)
    {
        amplitude =
            algorithm->signal_max -
            algorithm->signal_min;

        algorithm->peak_threshold =
            algorithm->signal_min +
            ((amplitude * 55) / 100);

        algorithm->threshold_ready = true;

        if (amplitude <= 100)
        {
            algorithm->signal_quality = 0U;
        }
        else if (amplitude >= 5000)
        {
            algorithm->signal_quality = 100U;
        }
        else
        {
            algorithm->signal_quality =
                (uint8_t)(
                    (amplitude * 100) / 5000
                );
        }

        algorithm->signal_min = INT32_MAX;
        algorithm->signal_max = INT32_MIN;
        algorithm->signal_window_count = 0U;
    }

    algorithm->sample_index++;

    /*
     * Detect a local maximum and apply a 300 ms refractory period.
     */
    if (algorithm->threshold_ready &&
        (algorithm->previous_1 >
         algorithm->previous_2) &&
        (algorithm->previous_1 >= filtered) &&
        (algorithm->previous_1 >
         algorithm->peak_threshold) &&
        (algorithm->signal_quality >= 8U))
    {
        if (algorithm->last_peak_sample != 0U)
        {
            beat_interval =
                algorithm->sample_index -
                algorithm->last_peak_sample;

            if ((beat_interval >=
                 HEALTH_MIN_BEAT_INTERVAL_SAMPLES) &&
                (beat_interval <=
                 HEALTH_MAX_BEAT_INTERVAL_SAMPLES))
            {
                instantaneous_bpm =
                    (uint16_t)(
                        (60U * HEALTH_SAMPLE_RATE_HZ) /
                        beat_interval
                    );

                if ((instantaneous_bpm >= 35U) &&
                    (instantaneous_bpm <= 220U))
                {
                    algorithm->current_bpm =
                        HealthAlgorithm_AverageBpm(
                            algorithm,
                            instantaneous_bpm
                        );

                    if ((algorithm->min_bpm == 0U) ||
                        (algorithm->current_bpm <
                         algorithm->min_bpm))
                    {
                        algorithm->min_bpm =
                            algorithm->current_bpm;
                    }

                    if (algorithm->current_bpm >
                        algorithm->max_bpm)
                    {
                        algorithm->max_bpm =
                            algorithm->current_bpm;
                    }

                    result->new_result = true;
                }
            }
        }

        algorithm->last_peak_sample =
            algorithm->sample_index;
    }

    algorithm->previous_2 =
        algorithm->previous_1;

    algorithm->previous_1 =
        filtered;

    /*
     * Collect one-second RED/IR AC and DC averages for the demonstration SpO2
     * estimator.
     */
    absolute_red =
        (ac_red >= 0) ?
        ac_red :
        -ac_red;

    absolute_ir =
        (ac_ir >= 0) ?
        ac_ir :
        -ac_ir;

    algorithm->red_ac_sum +=
        (uint32_t)absolute_red;

    algorithm->ir_ac_sum +=
        (uint32_t)absolute_ir;

    algorithm->red_dc_sum +=
        (uint32_t)algorithm->dc_red;

    algorithm->ir_dc_sum +=
        (uint32_t)algorithm->dc_ir;

    algorithm->spo2_window_count++;

    if (algorithm->spo2_window_count >=
        HEALTH_SPO2_WINDOW_SAMPLES)
    {
        algorithm->current_spo2 =
            HealthAlgorithm_CalculateSpO2(
                algorithm
            );

        algorithm->red_ac_sum = 0U;
        algorithm->ir_ac_sum = 0U;
        algorithm->red_dc_sum = 0U;
        algorithm->ir_dc_sum = 0U;
        algorithm->spo2_window_count = 0U;

        if ((algorithm->current_spo2 != 0U) &&
            (algorithm->current_bpm != 0U))
        {
            result->new_result = true;
        }
    }

    result->bpm = algorithm->current_bpm;
    result->min_bpm = algorithm->min_bpm;
    result->max_bpm = algorithm->max_bpm;
    result->spo2 = algorithm->current_spo2;
    result->signal_quality = algorithm->signal_quality;
}

/* -------------------------------------------------------------------------- */
/* HealthTask                                                                 */
/* -------------------------------------------------------------------------- */

void HealthTask_Run(void)
{
    health_algorithm_t algorithm;
    health_algorithm_result_t result;
    max30102_sample_t sample;

    health_command_t command;
    health_state_t reported_state;

    HAL_StatusTypeDef status;
    bool sensor_ready;
    bool measuring;

    uint8_t available_samples;
    uint8_t samples_to_read;
    uint8_t index;

    HealthAlgorithm_Reset(&algorithm);

    sensor_ready = false;
    measuring = false;
    reported_state = HEALTH_STATE_IDLE;

    if ((g_health_command_queue == NULL) ||
        (g_health_ui_queue == NULL))
    {
        Health_DebugPrintf(
            "[HEALTH ERROR] queues not created\r\n"
        );

        for (;;)
        {
            osDelay(1000U);
        }
    }

    /*
     * Initial sensor probe. Measurement remains stopped after initialization.
     */
    if ((i2c1MutexHandle != NULL) &&
        (osMutexAcquire(
            i2c1MutexHandle,
            200U) == osOK))
    {
        status = MAX30102_Initialize();

        (void)osMutexRelease(
            i2c1MutexHandle
        );

        sensor_ready = (status == HAL_OK);
    }

    if (sensor_ready)
    {
        Health_DebugPrintf(
            "[HEALTH] MAX30102 ready, part=0x%02X\r\n",
            (unsigned int)g_max30102_part_id
        );

        Health_SendState(HEALTH_STATE_IDLE);
    }
    else
    {
        g_health_i2c_error_count++;

        Health_DebugPrintf(
            "[HEALTH ERROR] MAX30102 init failed\r\n"
        );

        reported_state =
            HEALTH_STATE_SENSOR_ERROR;

        Health_SendState(
            HEALTH_STATE_SENSOR_ERROR
        );
    }

    for (;;)
    {
        /*
         * Idle: block until Start/Stop.
         * Measuring: check command queue without blocking so FIFO service
         * continues.
         */
        if (osMessageQueueGet(
                g_health_command_queue,
                &command,
                NULL,
                measuring ?
                0U :
                osWaitForever) == osOK)
        {
            if (command == HEALTH_COMMAND_START)
            {
                Health_SendState(
                    HEALTH_STATE_STARTING
                );

                /*
                 * Retry full initialization when the previous sensor access
                 * failed or when the board was connected after boot.
                 */
                if (!sensor_ready)
                {
                    if ((i2c1MutexHandle != NULL) &&
                        (osMutexAcquire(
                            i2c1MutexHandle,
                            200U) == osOK))
                    {
                        status =
                            MAX30102_Initialize();

                        (void)osMutexRelease(
                            i2c1MutexHandle
                        );

                        sensor_ready =
                            (status == HAL_OK);
                    }
                }

                if (sensor_ready &&
                    (i2c1MutexHandle != NULL) &&
                    (osMutexAcquire(
                        i2c1MutexHandle,
                        200U) == osOK))
                {
                    status = MAX30102_Start();

                    (void)osMutexRelease(
                        i2c1MutexHandle
                    );

                    if (status == HAL_OK)
                    {
                        HealthAlgorithm_Reset(
                            &algorithm
                        );

                        measuring = true;
                        reported_state =
                            HEALTH_STATE_MEASURING;

                        g_health_last_bpm = 0U;
                        g_health_last_spo2 = 0U;

                        Health_DebugPrintf(
                            "[HEALTH] measurement started\r\n"
                        );

                        Health_SendState(
                            HEALTH_STATE_MEASURING
                        );
                    }
                    else
                    {
                        sensor_ready = false;
                        measuring = false;
                        g_health_i2c_error_count++;

                        reported_state =
                            HEALTH_STATE_SENSOR_ERROR;

                        Health_SendState(
                            HEALTH_STATE_SENSOR_ERROR
                        );
                    }
                }
                else
                {
                    measuring = false;
                    g_health_i2c_error_count++;

                    reported_state =
                        HEALTH_STATE_SENSOR_ERROR;

                    Health_SendState(
                        HEALTH_STATE_SENSOR_ERROR
                    );
                }
            }
            else if (command == HEALTH_COMMAND_STOP)
            {
                if (sensor_ready &&
                    (i2c1MutexHandle != NULL) &&
                    (osMutexAcquire(
                        i2c1MutexHandle,
                        100U) == osOK))
                {
                    (void)MAX30102_Stop();

                    (void)osMutexRelease(
                        i2c1MutexHandle
                    );
                }

                measuring = false;
                reported_state = HEALTH_STATE_IDLE;

                HealthAlgorithm_Reset(
                    &algorithm
                );

                Health_DebugPrintf(
                    "[HEALTH] measurement stopped\r\n"
                );

                Health_SendState(
                    HEALTH_STATE_IDLE
                );
            }
        }

        if (!measuring)
        {
            continue;
        }

        status = HAL_OK;
        available_samples = 0U;

        if ((i2c1MutexHandle != NULL) &&
            (osMutexAcquire(
                i2c1MutexHandle,
                20U) == osOK))
        {
            status =
                MAX30102_GetAvailableSamples(
                    &available_samples
                );

            if (status == HAL_OK)
            {
                samples_to_read =
                    available_samples;

                /*
                 * Limit one task pass to avoid monopolizing I2C1 and CPU time.
                 */
                if (samples_to_read > 8U)
                {
                    samples_to_read = 8U;
                }

                for (index = 0U;
                     index < samples_to_read;
                     index++)
                {
                    status = MAX30102_ReadFIFO(
                        &sample
                    );

                    if (status != HAL_OK)
                    {
                        break;
                    }

                    HealthAlgorithm_Process(
                        &algorithm,
                        &sample,
                        &result
                    );

                    if (result.finger_changed)
                    {
                        reported_state =
                            result.finger_present ?
                            HEALTH_STATE_MEASURING :
                            HEALTH_STATE_NO_FINGER;

                        Health_SendState(
                            reported_state
                        );
                    }

                    if ((!result.finger_present) &&
                        (reported_state !=
                         HEALTH_STATE_NO_FINGER))
                    {
                        reported_state =
                            HEALTH_STATE_NO_FINGER;

                        Health_SendState(
                            HEALTH_STATE_NO_FINGER
                        );
                    }
                    else if (result.finger_present &&
                             (reported_state ==
                              HEALTH_STATE_NO_FINGER))
                    {
                        reported_state =
                            HEALTH_STATE_MEASURING;

                        Health_SendState(
                            HEALTH_STATE_MEASURING
                        );
                    }

                    if (result.new_result)
                    {
                        g_health_last_bpm =
                            result.bpm;

                        g_health_last_spo2 =
                            result.spo2;

                        Health_SendResult(&result);
                    }
                }
            }

            (void)osMutexRelease(
                i2c1MutexHandle
            );
        }

        if (status != HAL_OK)
        {
            sensor_ready = false;
            measuring = false;
            g_health_i2c_error_count++;

            reported_state =
                HEALTH_STATE_SENSOR_ERROR;

            Health_DebugPrintf(
                "[HEALTH ERROR] I2C/FIFO access failed\r\n"
            );

            Health_SendState(
                HEALTH_STATE_SENSOR_ERROR
            );
        }

        osDelay(5U);
    }
}

/* -------------------------------------------------------------------------- */
/* LVGL-side Health page controller                                           */
/* -------------------------------------------------------------------------- */

static void HealthUI_ClearResultDisplay(void)
{
    if (ui_HeartValueLabel != NULL)
    {
        lv_label_set_text(
            ui_HeartValueLabel,
            "--"
        );
    }

    if (ui_MAXnumLabel != NULL)
    {
        lv_label_set_text(
            ui_MAXnumLabel,
            "--"
        );
    }

    if (ui_MINNUMLabel != NULL)
    {
        lv_label_set_text(
            ui_MINNUMLabel,
            "--"
        );
    }
		if (ui_HeartRangeLabel != NULL)
		{
				lv_label_set_text(
						ui_HeartRangeLabel,
						"--~--"
				);
		}	

    if (g_spo2_value_label != NULL)
    {
        lv_label_set_text(
            g_spo2_value_label,
            "--%"
        );

        lv_obj_set_style_text_color(
            g_spo2_value_label,
            lv_color_hex(0xFFFFFF),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    if ((ui_HeartChart != NULL) &&
        (g_health_chart_series != NULL))
    {
        lv_chart_set_all_value(
            ui_HeartChart,
            g_health_chart_series,
            LV_CHART_POINT_NONE
        );

        lv_chart_refresh(ui_HeartChart);
    }
}

static void HealthUI_ShowSpinner(bool visible)
{
    if (ui_HeartSpinner == NULL)
    {
        return;
    }

    if (visible)
    {
        lv_obj_clear_flag(
            ui_HeartSpinner,
            LV_OBJ_FLAG_HIDDEN
        );
    }
    else
    {
        lv_obj_add_flag(
            ui_HeartSpinner,
            LV_OBJ_FLAG_HIDDEN
        );
    }
}

static void HealthUI_SetState(health_state_t state)
{
    if ((ui_HeartValueLabel == NULL) ||
        (ui_HeartStateLabel == NULL))
    {
        return;
    }

    switch (state)
    {
        case HEALTH_STATE_IDLE:
            lv_label_set_text(
                ui_HeartValueLabel,
                "--"
            );

            lv_label_set_text(
                ui_HeartStateLabel,
                "Ready"
            );

            lv_obj_set_style_text_color(
                ui_HeartStateLabel,
                lv_color_hex(0xA9B6C4),
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );

            HealthUI_ShowSpinner(false);
            break;

        case HEALTH_STATE_STARTING:
            HealthUI_ClearResultDisplay();

            lv_label_set_text(
                ui_HeartStateLabel,
                "Starting..."
            );

            lv_obj_set_style_text_color(
                ui_HeartStateLabel,
                lv_color_hex(0x16D9E3),
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );

            HealthUI_ShowSpinner(true);
            break;

        case HEALTH_STATE_MEASURING:
            lv_label_set_text(
                ui_HeartStateLabel,
                "Measuring"
            );

            lv_obj_set_style_text_color(
                ui_HeartStateLabel,
                lv_color_hex(0x16D9E3),
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );

            HealthUI_ShowSpinner(true);
            break;

        case HEALTH_STATE_NO_FINGER:
            lv_label_set_text(
                ui_HeartValueLabel,
                "--"
            );

            lv_label_set_text(
                ui_HeartStateLabel,
                "Place finger"
            );

            lv_obj_set_style_text_color(
                ui_HeartStateLabel,
                lv_color_hex(0xFF9F43),
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );

            if (g_spo2_value_label != NULL)
            {
                lv_label_set_text(
                    g_spo2_value_label,
                    "--%"
                );
            }

            HealthUI_ShowSpinner(true);
            break;

        case HEALTH_STATE_SENSOR_ERROR:
        default:
            lv_label_set_text(
                ui_HeartValueLabel,
                "--"
            );

            lv_label_set_text(
                ui_HeartStateLabel,
                "Sensor error"
            );

            lv_obj_set_style_text_color(
                ui_HeartStateLabel,
                lv_color_hex(0xFF4567),
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );

            if (g_spo2_value_label != NULL)
            {
                lv_label_set_text(
                    g_spo2_value_label,
                    "ERR"
                );
            }

            HealthUI_ShowSpinner(false);
            break;
    }
}

static void HealthUI_ButtonEvent(lv_event_t *event)
{
    lv_obj_t *button;

    if ((event == NULL) ||
        (lv_event_get_code(event) !=
         LV_EVENT_VALUE_CHANGED))
    {
        return;
    }

    button = lv_event_get_target(event);

    if (button == NULL)
    {
        return;
    }

    if (lv_obj_has_state(
            button,
            LV_STATE_CHECKED))
    {
        if (ui_MeasureBtnLabel != NULL)
        {
            lv_label_set_text(
                ui_MeasureBtnLabel,
                "STOP MEASURING"
            );
        }

        HealthUI_ClearResultDisplay();
        HealthUI_SetState(
            HEALTH_STATE_STARTING
        );

        HealthApp_RequestStart();
    }
    else
    {
        if (ui_MeasureBtnLabel != NULL)
        {
            lv_label_set_text(
                ui_MeasureBtnLabel,
                "START MEASURING"
            );
        }

        HealthUI_SetState(
            HEALTH_STATE_IDLE
        );

        HealthApp_RequestStop();
    }
}

void HealthUI_Init(void)
{
    if (g_health_ui_initialized)
    {
        return;
    }

    if ((ui_ScreenHealth == NULL) ||
        (ui_HealthMeasureBtn == NULL))
    {
        return;
    }

    /*
     * Keep the SquareLine-generated button callback. It owns the existing
     * heart/blood animations and label behavior. This second callback only
     * sends the real sensor start/stop command.
     */
    (void)lv_obj_add_event_cb(
        ui_HealthMeasureBtn,
        HealthUI_ButtonEvent,
        LV_EVENT_VALUE_CHANGED,
        NULL
    );

    if (ui_HeartChart != NULL)
    {
        g_health_chart_series =
            lv_chart_get_series_next(
                ui_HeartChart,
                NULL
            );

        if (g_health_chart_series != NULL)
        {
            lv_chart_set_point_count(
                ui_HeartChart,
                25U
            );

            lv_chart_set_update_mode(
                ui_HeartChart,
                LV_CHART_UPDATE_MODE_SHIFT
            );

            lv_chart_set_all_value(
                ui_HeartChart,
                g_health_chart_series,
                LV_CHART_POINT_NONE
            );
        }
    }

    /*
     * The existing SquareLine BloodOxygenPanel contains the blood-wave
     * animation but no numeric SpO2 label. Add the number inside that existing
     * panel; no second page or duplicate card is created.
     */
    if (ui_BloodOxygenPanel != NULL)
    {
        g_spo2_value_label =
            lv_label_create(
                ui_BloodOxygenPanel
            );

        lv_label_set_text(
            g_spo2_value_label,
            "--%"
        );

        lv_obj_set_align(
            g_spo2_value_label,
            LV_ALIGN_CENTER
        );

        lv_obj_set_y(
            g_spo2_value_label,
            -3
        );

        lv_obj_set_style_text_font(
            g_spo2_value_label,
            &lv_font_montserrat_16,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        lv_obj_set_style_text_color(
            g_spo2_value_label,
            lv_color_hex(0xFFFFFF),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        /*
         * The panel is only 77 x 77 and already contains the blood-wave
         * animation.  Keep only one compact numeric label to avoid the
         * overlapping "--% / SpO2" layout.
         */
        lv_obj_set_y(
            g_spo2_value_label,
            -1
        );

        lv_obj_set_style_text_font(
            g_spo2_value_label,
            &lv_font_montserrat_14,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        lv_obj_move_foreground(
            g_spo2_value_label
        );
    }

    if (ui_HeartRangeLabel != NULL)
    {
        lv_label_set_text(
            ui_HeartRangeLabel,
            "60~100"
        );
    }

    /*
     * SquareLine may export the checkable button with a stale state after
     * regeneration. Force a known idle state at runtime.
     */
    lv_obj_clear_state(
        ui_HealthMeasureBtn,
        LV_STATE_CHECKED |
        LV_STATE_PRESSED
    );

    if (ui_MeasureBtnLabel != NULL)
    {
        lv_label_set_text(
            ui_MeasureBtnLabel,
            "START MEASURING"
        );
    }

    HealthUI_ClearResultDisplay();
    HealthUI_SetState(HEALTH_STATE_IDLE);

    g_health_ui_initialized = true;
}

static void HealthUI_ApplyResult(
    const health_ui_message_t *message)
{
    lv_color_t color;

    if (message == NULL)
    {
        return;
    }

    if ((message->heart_rate >= 35U) &&
        (message->heart_rate <= 220U))
    {
        lv_label_set_text_fmt(
            ui_HeartValueLabel,
            "%u",
            (unsigned int)
            message->heart_rate
        );

        if (message->heart_rate < 60U)
        {
            lv_label_set_text(
                ui_HeartStateLabel,
                "Low"
            );

            color = lv_color_hex(0x248CFF);
        }
        else if (message->heart_rate > 100U)
        {
            lv_label_set_text(
                ui_HeartStateLabel,
                "High"
            );

            color = lv_color_hex(0xFF4567);
        }
        else
        {
            lv_label_set_text(
                ui_HeartStateLabel,
                "Normal"
            );

            color = lv_color_hex(0x39E58C);
        }

        lv_obj_set_style_text_color(
            ui_HeartStateLabel,
            color,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        if ((ui_HeartChart != NULL) &&
            (g_health_chart_series != NULL))
        {
            lv_chart_set_next_value(
                ui_HeartChart,
                g_health_chart_series,
                (lv_coord_t)
                message->heart_rate
            );
        }

        if (ui_MAXnumLabel != NULL)
        {
            lv_label_set_text_fmt(
                ui_MAXnumLabel,
                "%u",
                (unsigned int)
                message->max_heart_rate
            );
        }

        if (ui_MINNUMLabel != NULL)
        {
            lv_label_set_text_fmt(
                ui_MINNUMLabel,
                "%u",
                (unsigned int)
                message->min_heart_rate
            );
        }

        if (ui_LabelSportHeart != NULL)
        {
            lv_label_set_text_fmt(
                ui_LabelSportHeart,
                "Heart %u bpm",
                (unsigned int)
                message->heart_rate
            );
        }
    }
		/* 动态更新测量范围 */
		if ((ui_HeartRangeLabel != NULL) &&
				(message->min_heart_rate > 0U) &&
				(message->max_heart_rate >=
				 message->min_heart_rate))
		{
				lv_label_set_text_fmt(
						ui_HeartRangeLabel,
						"%u~%u",
						(unsigned int)
						message->min_heart_rate,
						(unsigned int)
						message->max_heart_rate
				);
		}

    if ((message->spo2 >= 70U) &&
        (message->spo2 <= 100U) &&
        (g_spo2_value_label != NULL))
    {
        lv_label_set_text_fmt(
            g_spo2_value_label,
            "%u%%",
            (unsigned int)message->spo2
        );

        if (message->spo2 >= 95U)
        {
            color = lv_color_hex(0x39E58C);
        }
        else if (message->spo2 >= 90U)
        {
            color = lv_color_hex(0xFF9F43);
        }
        else
        {
            color = lv_color_hex(0xFF4567);
        }

        lv_obj_set_style_text_color(
            g_spo2_value_label,
            color,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    HealthUI_ShowSpinner(true);
}

void HealthUI_ProcessQueue(void)
{
    health_ui_message_t message;

    if ((!g_health_ui_initialized) ||
        (g_health_ui_queue == NULL))
    {
        return;
    }

    while (osMessageQueueGet(
               g_health_ui_queue,
               &message,
               NULL,
               0U) == osOK)
    {
        if (message.type ==
            HEALTH_UI_MESSAGE_STATE)
        {
            HealthUI_SetState(
                message.state
            );
        }
        else if (message.type ==
                 HEALTH_UI_MESSAGE_RESULT)
        {
            HealthUI_ApplyResult(
                &message
            );
        }
        else
        {
            /* Ignore unknown messages. */
        }
    }
}

void HealthUI_OnScreenUnloaded(void)
{
    HealthApp_RequestStop();

    if (ui_HealthMeasureBtn != NULL)
    {
        lv_obj_clear_state(
            ui_HealthMeasureBtn,
            LV_STATE_CHECKED |
            LV_STATE_PRESSED
        );
    }

    if (ui_MeasureBtnLabel != NULL)
    {
        lv_label_set_text(
            ui_MeasureBtnLabel,
            "START MEASURING"
        );
    }

    HealthUI_SetState(
        HEALTH_STATE_IDLE
    );
}
