/**
  ******************************************************************************
  * @file    sensor_app.c
  * @brief   SHT20 temperature/humidity, random steps and demo weather
  ******************************************************************************
  *
  * Architecture:
  *
  * SHT20 / demo generator
  *      -> SensorTask
  *      -> text queue / private icon queue
  *      -> LVGLTask
  *      -> existing SquareLine Home and Weather objects
  *
  * SensorTask never calls LVGL.
  ******************************************************************************
  */

#include "sensor_app.h"
#include "sport_app.h"

#include "app_freertos.h"
#include "cmsis_os2.h"
#include "i2c.h"
#include "ui.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern osMutexId_t i2c1MutexHandle;

/* -------------------------------------------------------------------------- */
/* SHT20                                                                       */
/* -------------------------------------------------------------------------- */

#define SHT20_I2C_ADDRESS                 (0x40U << 1)
#define SHT20_CMD_TEMP_NO_HOLD            0xF3U
#define SHT20_CMD_HUM_NO_HOLD             0xF5U
#define SHT20_I2C_TIMEOUT_MS              100U

#define SENSOR_PERIOD_MS                  1000U
#define SENSOR_SHT20_UPDATE_SECONDS       2U
#define SENSOR_WEATHER_UPDATE_SECONDS     30U

typedef enum
{
    WEATHER_ICON_SUNNY = 0,
    WEATHER_ICON_PARTLY_CLOUDY,
    WEATHER_ICON_CLOUDY,
    WEATHER_ICON_SUN_RAIN,
    WEATHER_ICON_RAIN,
    WEATHER_ICON_THUNDER
} sensor_weather_icon_t;

typedef struct
{
    sensor_weather_icon_t icon;
} sensor_ui_message_t;

typedef struct
{
    const char *state;
    int8_t temperature;
    int8_t high;
    int8_t low;
    sensor_weather_icon_t icon;
} sensor_weather_demo_t;

static const sensor_weather_demo_t g_weather_demo[] =
{
    { "Sunny",          28, 31, 23, WEATHER_ICON_SUNNY },
    { "Partly Cloudy",  26, 29, 22, WEATHER_ICON_PARTLY_CLOUDY },
    { "Cloudy",         24, 27, 21, WEATHER_ICON_CLOUDY },
    { "Sun to Rain",    23, 26, 19, WEATHER_ICON_SUN_RAIN },
    { "Light Rain",     21, 24, 18, WEATHER_ICON_RAIN },
    { "Thunderstorm",   20, 23, 17, WEATHER_ICON_THUNDER }
};

#define WEATHER_DEMO_COUNT \
    ((uint32_t)(sizeof(g_weather_demo) / sizeof(g_weather_demo[0])))

static osMessageQueueId_t g_sensor_ui_queue = NULL;

volatile uint32_t g_sensor_step_count = 3268U;
volatile int32_t g_sensor_temperature_centi = 2450;
volatile uint32_t g_sensor_humidity_centi = 6000U;
volatile uint32_t g_sensor_sht20_error_count = 0U;
volatile uint8_t g_sensor_using_demo_environment = 0U;

static uint8_t SHT20_CalculateCRC(
    const uint8_t *data,
    uint8_t length)
{
    uint8_t crc;
    uint8_t byte_index;
    uint8_t bit_index;

    crc = 0U;

    for (byte_index = 0U;
         byte_index < length;
         byte_index++)
    {
        crc ^= data[byte_index];

        for (bit_index = 0U;
             bit_index < 8U;
             bit_index++)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc =
                    (uint8_t)(
                        (crc << 1U) ^ 0x31U
                    );
            }
            else
            {
                crc <<= 1U;
            }
        }
    }

    return crc;
}

static HAL_StatusTypeDef SHT20_SendCommand(
    uint8_t command)
{
    HAL_StatusTypeDef status;

    if ((i2c1MutexHandle == NULL) ||
        (osMutexAcquire(
            i2c1MutexHandle,
            50U) != osOK))
    {
        return HAL_BUSY;
    }

    status = HAL_I2C_Master_Transmit(
        &hi2c1,
        SHT20_I2C_ADDRESS,
        &command,
        1U,
        SHT20_I2C_TIMEOUT_MS
    );

    (void)osMutexRelease(i2c1MutexHandle);

    return status;
}

static HAL_StatusTypeDef SHT20_ReadResult(
    uint8_t data[3])
{
    HAL_StatusTypeDef status;

    if ((data == NULL) ||
        (i2c1MutexHandle == NULL) ||
        (osMutexAcquire(
            i2c1MutexHandle,
            50U) != osOK))
    {
        return HAL_BUSY;
    }

    status = HAL_I2C_Master_Receive(
        &hi2c1,
        SHT20_I2C_ADDRESS,
        data,
        3U,
        SHT20_I2C_TIMEOUT_MS
    );

    (void)osMutexRelease(i2c1MutexHandle);

    return status;
}

static HAL_StatusTypeDef SHT20_ReadRaw(
    uint8_t command,
    uint32_t conversion_delay_ms,
    uint16_t *raw_value)
{
    uint8_t data[3];
    HAL_StatusTypeDef status;

    if (raw_value == NULL)
    {
        return HAL_ERROR;
    }

    status = SHT20_SendCommand(command);

    if (status != HAL_OK)
    {
        return status;
    }

    /*
     * No-hold mode releases I2C during sensor conversion, so MAX30102 can keep
     * servicing its FIFO on the same bus.
     */
    osDelay(conversion_delay_ms);

    status = SHT20_ReadResult(data);

    if (status != HAL_OK)
    {
        return status;
    }

    if (SHT20_CalculateCRC(data, 2U) != data[2])
    {
        return HAL_ERROR;
    }

    *raw_value =
        (uint16_t)(
            (((uint16_t)data[0]) << 8U) |
            ((uint16_t)data[1])
        );

    *raw_value &= 0xFFFCU;

    return HAL_OK;
}

static HAL_StatusTypeDef SHT20_ReadEnvironment(
    int32_t *temperature_centi,
    uint32_t *humidity_centi)
{
    uint16_t raw_temperature;
    uint16_t raw_humidity;
    int32_t temperature;
    int32_t humidity;
    HAL_StatusTypeDef status;

    if ((temperature_centi == NULL) ||
        (humidity_centi == NULL))
    {
        return HAL_ERROR;
    }

    status = SHT20_ReadRaw(
        SHT20_CMD_TEMP_NO_HOLD,
        90U,
        &raw_temperature
    );

    if (status != HAL_OK)
    {
        return status;
    }

    status = SHT20_ReadRaw(
        SHT20_CMD_HUM_NO_HOLD,
        35U,
        &raw_humidity
    );

    if (status != HAL_OK)
    {
        return status;
    }

    /*
     * Fixed-point forms of the SHT20 datasheet equations.
     * Values are returned in 0.01 degree C and 0.01 %RH.
     */
    temperature =
        -4685 +
        (int32_t)(
            (17572LL * raw_temperature) /
            65536LL
        );

    humidity =
        -600 +
        (int32_t)(
            (12500LL * raw_humidity) /
            65536LL
        );

    if (humidity < 0)
    {
        humidity = 0;
    }

    if (humidity > 10000)
    {
        humidity = 10000;
    }

    *temperature_centi = temperature;
    *humidity_centi = (uint32_t)humidity;

    return HAL_OK;
}

static void Sensor_PostWeatherIcon(
    sensor_weather_icon_t icon)
{
    sensor_ui_message_t message;

    if (g_sensor_ui_queue == NULL)
    {
        return;
    }

    message.icon = icon;

    if (osMessageQueuePut(
            g_sensor_ui_queue,
            &message,
            0U,
            0U) != osOK)
    {
        (void)osMessageQueueReset(
            g_sensor_ui_queue
        );

        (void)osMessageQueuePut(
            g_sensor_ui_queue,
            &message,
            0U,
            0U
        );
    }
}

static void Sensor_PostEnvironment(
    int32_t temperature_centi,
    uint32_t humidity_centi)
{
    char text[24];
    int32_t whole_temperature;
    int32_t temperature_decimal;

    whole_temperature =
        temperature_centi / 100;

    temperature_decimal =
        temperature_centi % 100;

    if (temperature_decimal < 0)
    {
        temperature_decimal =
            -temperature_decimal;
    }

    (void)snprintf(
        text,
        sizeof(text),
        "%ld.%01ld C",
        (long)whole_temperature,
        (long)(temperature_decimal / 10)
    );

    (void)UI_PostText(
        UI_FIELD_HOME_TEMPERATURE,
        text,
        0U
    );

    (void)snprintf(
        text,
        sizeof(text),
        "%lu.%01lu%%",
        (unsigned long)(humidity_centi / 100U),
        (unsigned long)((humidity_centi % 100U) / 10U)
    );

    (void)UI_PostText(
        UI_FIELD_HOME_HUMIDITY,
        text,
        0U
    );
}

static void Sensor_PostSteps(uint32_t steps)
{
    char text[20];

    (void)snprintf(
        text,
        sizeof(text),
        "%lu",
        (unsigned long)steps
    );

    (void)UI_PostText(
        UI_FIELD_HOME_STEPS,
        text,
        0U
    );
}

static void Sensor_PostWeather(
    const sensor_weather_demo_t *weather)
{
    char text[24];

    if (weather == NULL)
    {
        return;
    }

    (void)snprintf(
        text,
        sizeof(text),
        "%d C",
        (int)weather->temperature
    );

    (void)UI_PostText(
        UI_FIELD_HOME_WEATHER_TEMP,
        text,
        0U
    );

    (void)UI_PostText(
        UI_FIELD_HOME_WEATHER_STATE,
        weather->state,
        0U
    );

    (void)UI_PostText(
        UI_FIELD_WEATHER_LOCATION,
        "Local",
        0U
    );

    (void)snprintf(
        text,
        sizeof(text),
        "%d C",
        (int)weather->temperature
    );

    (void)UI_PostText(
        UI_FIELD_WEATHER_TEMPERATURE,
        text,
        0U
    );

    (void)UI_PostText(
        UI_FIELD_WEATHER_CONDITION,
        weather->state,
        0U
    );

    (void)snprintf(
        text,
        sizeof(text),
        "H:%d C  L:%d C",
        (int)weather->high,
        (int)weather->low
    );

    (void)UI_PostText(
        UI_FIELD_WEATHER_HIGH_LOW,
        text,
        0U
    );

    Sensor_PostWeatherIcon(weather->icon);
}

void SensorApp_CreateObjects(void)
{
    static const osMessageQueueAttr_t queue_attributes =
    {
        .name = "sensorUiQueue"
    };

    if (g_sensor_ui_queue == NULL)
    {
        g_sensor_ui_queue = osMessageQueueNew(
            2U,
            sizeof(sensor_ui_message_t),
            &queue_attributes
        );
    }
}

void SensorTask_Run(void)
{
    uint32_t random_state;
    uint32_t random_value;

    uint32_t elapsed_seconds;
    uint32_t weather_index;
    uint32_t next_wake_tick;
    uint32_t period_ticks;

    int32_t temperature_centi;
    uint32_t humidity_centi;

    HAL_StatusTypeDef status;

    random_state =
        0x13579BDFU ^
        HAL_GetTick();

    elapsed_seconds = 0U;
    weather_index = 1U;

    period_ticks =
        (uint32_t)(
            (((uint64_t)SENSOR_PERIOD_MS * osKernelGetTickFreq()) +
             999ULL) /
            1000ULL
        );

    if (period_ticks == 0U)
    {
        period_ticks = 1U;
    }

    next_wake_tick = osKernelGetTickCount();

    Sensor_PostSteps(g_sensor_step_count);
    Sensor_PostWeather(
        &g_weather_demo[weather_index]
    );

    (void)UI_PostText(
        UI_FIELD_HOME_BATTERY,
        "86%",
        0U
    );

    for (;;)
    {
        random_state =
            random_state * 1664525U +
            1013904223U;

        random_value =
            (random_state >> 24U) % 10U;

        /*
         * Demo pedometer:
         * 30% no step, 50% +1 step, 20% +2 steps.
         */
        if (random_value >= 3U)
        {
            g_sensor_step_count +=
                (random_value < 8U) ?
                1U :
                2U;
        }

        if (g_sensor_step_count > 99999U)
        {
            g_sensor_step_count = 0U;
        }

        Sensor_PostSteps(g_sensor_step_count);

        if ((elapsed_seconds %
             SENSOR_SHT20_UPDATE_SECONDS) == 0U)
        {
            status = SHT20_ReadEnvironment(
                &temperature_centi,
                &humidity_centi
            );

            if (status == HAL_OK)
            {
                g_sensor_using_demo_environment = 0U;
                g_sensor_temperature_centi =
                    temperature_centi;
                g_sensor_humidity_centi =
                    humidity_centi;
            }
            else
            {
                /*
                 * Keep the UI alive when the module is absent:
                 * use a slowly changing demonstration value.
                 */
                g_sensor_sht20_error_count++;
                g_sensor_using_demo_environment = 1U;

                random_state =
                    random_state * 1664525U +
                    1013904223U;

                g_sensor_temperature_centi =
                    2450 +
                    (int32_t)(
                        ((random_state >> 24U) % 81U)
                    ) -
                    40;

                random_state =
                    random_state * 1664525U +
                    1013904223U;

                g_sensor_humidity_centi =
                    6000U +
                    ((random_state >> 24U) % 401U) -
                    200U;
            }

            Sensor_PostEnvironment(
                g_sensor_temperature_centi,
                g_sensor_humidity_centi
            );
        }

        if ((elapsed_seconds > 0U) &&
            ((elapsed_seconds %
              SENSOR_WEATHER_UPDATE_SECONDS) == 0U))
        {
            weather_index++;

            if (weather_index >=
                WEATHER_DEMO_COUNT)
            {
                weather_index = 0U;
            }

            Sensor_PostWeather(
                &g_weather_demo[weather_index]
            );
        }

        /*
         * Sport session timing shares SensorTask's stable one-second cadence.
         * SportApp_Tick1s contains no LVGL calls.
         */
        SportApp_Tick1s();

        elapsed_seconds++;

        /* Keep the one-second cadence independent of SHT20 conversion time. */
        next_wake_tick += period_ticks;

        if (osDelayUntil(next_wake_tick) != osOK)
        {
            next_wake_tick = osKernelGetTickCount();
            osDelay(SENSOR_PERIOD_MS);
        }
    }
}

void SensorUI_Init(void)
{
    /*
     * The first task message supplies the actual state. Keep the existing
     * SquareLine objects and do not create a second weather card.
     */
}

void SensorUI_ProcessQueue(void)
{
    sensor_ui_message_t message;
    sensor_ui_message_t latest;
    const lv_img_dsc_t *image_source;
    bool received;

    if (g_sensor_ui_queue == NULL)
    {
        return;
    }

    received = false;

    while (osMessageQueueGet(
               g_sensor_ui_queue,
               &message,
               NULL,
               0U) == osOK)
    {
        latest = message;
        received = true;
    }

    if (!received)
    {
        return;
    }

    switch (latest.icon)
    {
        case WEATHER_ICON_SUNNY:
            image_source = &ui_img_942679780;
            break;

        case WEATHER_ICON_PARTLY_CLOUDY:
            image_source = &ui_img_1607374796;
            break;

        case WEATHER_ICON_CLOUDY:
            image_source = &ui_img_1772208564;
            break;

        case WEATHER_ICON_SUN_RAIN:
            image_source = &ui_img_2087202441;
            break;

        case WEATHER_ICON_RAIN:
            image_source = &ui_img_1225357782;
            break;

        case WEATHER_ICON_THUNDER:
        default:
            image_source = &ui_img_1840017345;
            break;
    }

    if (ui_Image2 != NULL)
    {
        lv_img_set_src(
            ui_Image2,
            image_source
        );
    }

    if (ui_ImgWeatherMain != NULL)
    {
        lv_img_set_src(
            ui_ImgWeatherMain,
            image_source
        );
    }
}
