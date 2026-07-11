/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : app_freertos.c
  * @brief          : FreeRTOS application file
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "ui.h"

#include "health_app.h"
#include "clock_app.h"
#include "sensor_app.h"
#include "sport_app.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
    UI_PAGE_HOME = 0,
    UI_PAGE_HEALTH,
    UI_PAGE_SPORT,
    UI_PAGE_WEATHER,
    UI_PAGE_CALL,
    UI_PAGE_COUNT
} ui_page_index_t;

typedef void (*ui_generated_event_cb_t)(
    lv_event_t *event
);

/* SquareLine-generated screen callbacks. */
void ui_event_ScreenHome(lv_event_t *event);
void ui_event_ScreenHealth(lv_event_t *event);
void ui_event_ScreenSport(lv_event_t *event);
void ui_event_ScreenWeather(lv_event_t *event);
void ui_event_ScreenCall(lv_event_t *event);

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define LVGL_TASK_PERIOD_MS          5U
#define UI_PAGE_ANIM_TIME_MS         180U
#define DEBUG_HEARTBEAT_ENABLE       0U
#define DEBUG_HEARTBEAT_TIME_MS      10000U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

volatile uint32_t g_lvgl_stage = 0U;
volatile uint32_t g_lvgl_loop_count = 0U;

static lv_obj_t *g_ui_pages[UI_PAGE_COUNT];

static const char *const g_ui_page_names[
    UI_PAGE_COUNT
] =
{
    "Home",
    "Health",
    "Sport",
    "Weather",
    "Call"
};

static const ui_generated_event_cb_t
g_ui_generated_screen_callbacks[
    UI_PAGE_COUNT
] =
{
    ui_event_ScreenHome,
    ui_event_ScreenHealth,
    ui_event_ScreenSport,
    ui_event_ScreenWeather,
    ui_event_ScreenCall
};

static ui_page_index_t g_ui_current_page =
    UI_PAGE_HOME;

static bool g_ui_transition_active = false;

/* USER CODE END Variables */

/* Definitions for LVGLTask */
osThreadId_t LVGLTaskHandle;
const osThreadAttr_t LVGLTask_attributes = {
  .name = "LVGLTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 2048 * 4
};

/* Definitions for HealthTask */
osThreadId_t HealthTaskHandle;
const osThreadAttr_t HealthTask_attributes = {
  .name = "HealthTask",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 2048 * 4
};

/* Definitions for SensorTask */
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
  .name = "SensorTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024 * 4
};

/* Definitions for SystemTask */
osThreadId_t SystemTaskHandle;
const osThreadAttr_t SystemTask_attributes = {
  .name = "SystemTask",
  .priority = (osPriority_t) osPriorityBelowNormal,
  .stack_size = 768 * 4
};

/* Definitions for i2c1Mutex */
osMutexId_t i2c1MutexHandle;
const osMutexAttr_t i2c1Mutex_attributes = {
  .name = "i2c1Mutex"
};

/* Definitions for uiQueue */
osMessageQueueId_t uiQueueHandle;
const osMessageQueueAttr_t uiQueue_attributes = {
  .name = "uiQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

static void Debug_Print(const char *text);
static void Debug_Printf(const char *format, ...);

static bool UI_Initialize_Five_Pages(void);
static void UI_Page_Event(lv_event_t *event);

static void UI_Apply_Message(
    const ui_app_message_t *message
);

static ui_page_index_t UI_Page_Index_From_Object(
    lv_obj_t *page
);

/* USER CODE END FunctionPrototypes */

/* USER CODE BEGIN 5 */

void vApplicationMallocFailedHook(void)
{
    Debug_Print(
        "\r\n[RTOS ERROR] pvPortMalloc failed\r\n"
    );

    taskDISABLE_INTERRUPTS();

    for (;;)
    {
    }
}

/* USER CODE END 5 */

/* USER CODE BEGIN 3 */

void vApplicationTickHook(void)
{
    /*
     * LVGL time is advanced only inside LVGLTask.
     */
}

/* USER CODE END 3 */

/* USER CODE BEGIN 4 */

void vApplicationStackOverflowHook(
    xTaskHandle task,
    char *task_name)
{
    (void)task;

    Debug_Print(
        "\r\n[RTOS ERROR] stack overflow: "
    );

    Debug_Print(
        (task_name != NULL) ?
        task_name :
        "unknown"
    );

    Debug_Print("\r\n");

    taskDISABLE_INTERRUPTS();

    for (;;)
    {
    }
}

/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */

    Debug_Print(
        "\r\n========================================\r\n"
    );

    Debug_Print(
        "[RTOS] MX_FREERTOS_Init entered\r\n"
    );

  /* USER CODE END Init */

  /* creation of i2c1Mutex */
  i2c1MutexHandle =
      osMutexNew(&i2c1Mutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */

    if (i2c1MutexHandle == NULL)
    {
        Debug_Print(
            "[RTOS ERROR] i2c1Mutex creation failed\r\n"
        );
    }

  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* USER CODE END RTOS_TIMERS */

  /* creation of uiQueue */
  uiQueueHandle =
      osMessageQueueNew(
          24U,
          sizeof(ui_app_message_t),
          &uiQueue_attributes
      );

  /* USER CODE BEGIN RTOS_QUEUES */

    HealthApp_CreateObjects();
    ClockApp_CreateObjects();
    SensorApp_CreateObjects();
    SportApp_CreateObjects();

    if (uiQueueHandle == NULL)
    {
        Debug_Print(
            "[RTOS ERROR] uiQueue creation failed\r\n"
        );
    }

  /* USER CODE END RTOS_QUEUES */

  /* creation of LVGLTask */
  LVGLTaskHandle =
      osThreadNew(
          LVGLTask,
          NULL,
          &LVGLTask_attributes
      );

  /* creation of HealthTask */
  HealthTaskHandle =
      osThreadNew(
          StartHealthTask,
          NULL,
          &HealthTask_attributes
      );

  /* creation of SensorTask */
  SensorTaskHandle =
      osThreadNew(
          StartSensorTask,
          NULL,
          &SensorTask_attributes
      );

  /* creation of SystemTask */
  SystemTaskHandle =
      osThreadNew(
          StartSystemTask,
          NULL,
          &SystemTask_attributes
      );

  /* USER CODE BEGIN RTOS_THREADS */

    if ((LVGLTaskHandle == NULL) ||
        (HealthTaskHandle == NULL) ||
        (SensorTaskHandle == NULL) ||
        (SystemTaskHandle == NULL))
    {
        Debug_Print(
            "[RTOS ERROR] task creation failed\r\n"
        );
    }
    else
    {
        Debug_Print(
            "[RTOS] all tasks created\r\n"
        );
    }

    Debug_Print("========================================\r\n");

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_LVGLTask */
/**
* @brief Function implementing the LVGLTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LVGLTask */

void LVGLTask(void *argument)
{
  /* USER CODE BEGIN LVGLTask */

    lv_disp_t *display;
    ui_app_message_t message;

    uint32_t lvgl_last_tick;
    uint32_t lvgl_current_tick;
    uint32_t lvgl_elapsed_ticks;
    uint32_t lvgl_tick_frequency;
    uint32_t lvgl_elapsed_ms;

#if DEBUG_HEARTBEAT_ENABLE
    uint32_t heartbeat_ms;
#endif

    (void)argument;

    g_lvgl_stage = 1U;

    Debug_Print("[LVGL] task entered\r\n");

    lv_init();
    g_lvgl_stage = 2U;

    lv_port_disp_init();
    g_lvgl_stage = 3U;

    display = lv_disp_get_default();

    if (display == NULL)
    {
        g_lvgl_stage = 101U;

        Debug_Print(
            "[LVGL ERROR] default display is NULL\r\n"
        );

        for (;;)
        {
            osDelay(1000U);
        }
    }

    lv_port_indev_init();

    if (!UI_Initialize_Five_Pages())
    {
        for (;;)
        {
            osDelay(1000U);
        }
    }

    lv_refr_now(display);
    g_lvgl_stage = 8U;

    if (lv_scr_act() != ui_ScreenHome)
    {
        g_lvgl_stage = 108U;

        Debug_Print(
            "[UI ERROR] active screen is not Home\r\n"
        );

        for (;;)
        {
            osDelay(1000U);
        }
    }

    g_lvgl_stage = 9U;

    Debug_Print(
        "[LVGL] five-page UI ready\r\n"
    );

    lvgl_tick_frequency = osKernelGetTickFreq();

    if (lvgl_tick_frequency == 0U)
    {
        lvgl_tick_frequency = 1000U;
    }

    lvgl_last_tick = osKernelGetTickCount();

#if DEBUG_HEARTBEAT_ENABLE
    heartbeat_ms = 0U;
#endif

    for (;;)
    {
        /*
         * Generic text updates from SensorTask and future network tasks.
         */
        while (osMessageQueueGet(
                   uiQueueHandle,
                   &message,
                   NULL,
                   0U) == osOK)
        {
            UI_Apply_Message(&message);
        }

        /*
         * Private typed queues. Every function below executes in LVGLTask.
         */
        HealthUI_ProcessQueue();
        ClockUI_ProcessQueue();
        SensorUI_ProcessQueue();
        SportUI_ProcessQueue();

        /*
         * Advance LVGL with real elapsed RTOS time rather than assuming every
         * loop took exactly 5 ms. DMA activity or a preemption can extend a
         * loop; fixed increments would otherwise make animations and timers
         * run progressively slow.
         */
        lvgl_current_tick = osKernelGetTickCount();
        lvgl_elapsed_ticks = lvgl_current_tick - lvgl_last_tick;

        lvgl_elapsed_ms =
            (uint32_t)(
                ((uint64_t)lvgl_elapsed_ticks * 1000ULL) /
                lvgl_tick_frequency
            );

        if (lvgl_elapsed_ms > 0U)
        {
            lv_tick_inc(lvgl_elapsed_ms);
            lvgl_last_tick = lvgl_current_tick;
        }

        (void)lv_timer_handler();

        g_lvgl_loop_count++;

#if DEBUG_HEARTBEAT_ENABLE
        heartbeat_ms += LVGL_TASK_PERIOD_MS;

        if (heartbeat_ms >=
            DEBUG_HEARTBEAT_TIME_MS)
        {
            heartbeat_ms = 0U;

            Debug_Printf(
                "[LVGL] loop=%lu page=%s heap=%lu\r\n",
                (unsigned long)g_lvgl_loop_count,
                g_ui_page_names[g_ui_current_page],
                (unsigned long)xPortGetFreeHeapSize()
            );
        }
#endif

        osDelay(LVGL_TASK_PERIOD_MS);
    }

  /* USER CODE END LVGLTask */
}

/* USER CODE BEGIN Header_StartHealthTask */
/**
* @brief Function implementing the HealthTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartHealthTask */

void StartHealthTask(void *argument)
{
  /* USER CODE BEGIN HealthTask */

    (void)argument;

    HealthTask_Run();

    for (;;)
    {
        osDelay(1000U);
    }

  /* USER CODE END HealthTask */
}

/* USER CODE BEGIN Header_StartSensorTask */
/**
* @brief Function implementing the SensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSensorTask */

void StartSensorTask(void *argument)
{
  /* USER CODE BEGIN SensorTask */

    (void)argument;

    SensorTask_Run();

    for (;;)
    {
        osDelay(1000U);
    }

  /* USER CODE END SensorTask */
}

/* USER CODE BEGIN Header_StartSystemTask */
/**
* @brief Function implementing the SystemTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSystemTask */

void StartSystemTask(void *argument)
{
  /* USER CODE BEGIN SystemTask */

    (void)argument;

    ClockTask_Run();

    for (;;)
    {
        osDelay(1000U);
    }

  /* USER CODE END SystemTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

static void Debug_Print(const char *text)
{
    size_t length;

    if (text == NULL)
    {
        return;
    }

    length = strlen(text);

    if (length == 0U)
    {
        return;
    }

    (void)HAL_UART_Transmit(
        &huart1,
        (uint8_t *)text,
        (uint16_t)length,
        1000U
    );
}

static void Debug_Printf(
    const char *format,
    ...)
{
    char buffer[192];
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
        1000U
    );
}

bool UI_PostText(
    ui_field_id_t field,
    const char *text,
    uint32_t timeout_ms)
{
    ui_app_message_t message;

    if ((uiQueueHandle == NULL) ||
        (text == NULL) ||
        (field >= UI_FIELD_COUNT))
    {
        return false;
    }

    message.field = field;

    (void)strncpy(
        message.text,
        text,
        UI_APP_TEXT_LENGTH - 1U
    );

    message.text[
        UI_APP_TEXT_LENGTH - 1U
    ] = '\0';

    return (
        osMessageQueuePut(
            uiQueueHandle,
            &message,
            0U,
            timeout_ms
        ) == osOK
    );
}

static void UI_Apply_Message(
    const ui_app_message_t *message)
{
    lv_obj_t *target;

    if (message == NULL)
    {
        return;
    }

    target = NULL;

    switch (message->field)
    {
        case UI_FIELD_HOME_TIME:
            target = ui_HomeTimeLabel;
            break;

        case UI_FIELD_HOME_DATE:
            target = ui_HomeDateLabel;
            break;

        case UI_FIELD_HOME_WEATHER_TEMP:
            target = ui_HomeWeatherTempLabel;
            break;

        case UI_FIELD_HOME_WEATHER_STATE:
            target = ui_HomeWeatherStateLabel;
            break;

        case UI_FIELD_HOME_BATTERY:
            target = ui_HomeBatteryLabel;
            break;

        case UI_FIELD_HOME_TEMPERATURE:
            target = ui_HomeTempLabel;
            break;

        case UI_FIELD_HOME_HUMIDITY:
            target = ui_HomeHumidityLabel;
            break;

        case UI_FIELD_HOME_STEPS:
            target = ui_HomeStepLabel;
            break;

        case UI_FIELD_HEALTH_HEART_RATE:
            target = ui_HeartValueLabel;
            break;

        case UI_FIELD_HEALTH_STATE:
            target = ui_HeartStateLabel;
            break;

        case UI_FIELD_HEALTH_RANGE:
            target = ui_HeartRangeLabel;
            break;

        case UI_FIELD_HEALTH_MAX:
            target = ui_MAXnumLabel;
            break;

        case UI_FIELD_HEALTH_MIN:
            target = ui_MINNUMLabel;
            break;

        case UI_FIELD_SPORT_MODE:
            target = ui_LabelSportMode;
            break;

        case UI_FIELD_SPORT_HEART_RATE:
            target = ui_LabelSportHeart;
            break;

        case UI_FIELD_SPORT_CALORIES:
            target = ui_Label41;
            break;

        case UI_FIELD_SPORT_DURATION:
            target = ui_Label6;
            break;

        case UI_FIELD_SPORT_DISTANCE:
            target = ui_Label9;
            break;

        case UI_FIELD_WEATHER_LOCATION:
            target = ui_LabelLocation;
            break;

        case UI_FIELD_WEATHER_DATE:
            target = ui_LabelDate;
            break;

        case UI_FIELD_WEATHER_TEMPERATURE:
            target = ui_LabelTemp;
            break;

        case UI_FIELD_WEATHER_CONDITION:
            target = ui_LabelCondition;
            break;

        case UI_FIELD_WEATHER_HIGH_LOW:
            target = ui_LabelHighLow;
            break;

        case UI_FIELD_CALLER_NAME:
            target = ui_LabelCallerName;
            break;

        case UI_FIELD_CALL_STATE:
            target = ui_LabelCallState;
            break;

        case UI_FIELD_CALL_DETAIL:
            target = ui_LabelCallSub;
            break;

        default:
            break;
    }

    if (target != NULL)
    {
        lv_label_set_text(
            target,
            message->text
        );
    }
}

static bool UI_Initialize_Five_Pages(void)
{
    uint32_t index;

    ui_init();

    g_ui_pages[UI_PAGE_HOME] =
        ui_ScreenHome;

    g_ui_pages[UI_PAGE_HEALTH] =
        ui_ScreenHealth;

    g_ui_pages[UI_PAGE_SPORT] =
        ui_ScreenSport;

    g_ui_pages[UI_PAGE_WEATHER] =
        ui_ScreenWeather;

    g_ui_pages[UI_PAGE_CALL] =
        ui_ScreenCall;

    for (index = 0U;
         index < (uint32_t)UI_PAGE_COUNT;
         index++)
    {
        if (g_ui_pages[index] == NULL)
        {
            g_lvgl_stage = 103U + index;

            Debug_Printf(
                "[UI ERROR] %s is NULL\r\n",
                g_ui_page_names[index]
            );

            return false;
        }
    }

    /*
     * Remove only SquareLine's screen-level callbacks and install the single
     * centralized gesture callback. Button callbacks remain untouched.
     */
    for (index = 0U;
         index < (uint32_t)UI_PAGE_COUNT;
         index++)
    {
        if (!lv_obj_remove_event_cb(
                g_ui_pages[index],
                g_ui_generated_screen_callbacks[index]))
        {
            g_lvgl_stage = 109U;

            Debug_Printf(
                "[UI ERROR] callback not found: %s\r\n",
                g_ui_page_names[index]
            );

            return false;
        }

        if (lv_obj_add_event_cb(
                g_ui_pages[index],
                UI_Page_Event,
                LV_EVENT_ALL,
                NULL) == NULL)
        {
            g_lvgl_stage = 110U;

            Debug_Printf(
                "[UI ERROR] callback bind failed: %s\r\n",
                g_ui_page_names[index]
            );

            return false;
        }
    }

    /*
     * Existing SquareLine objects are valid now. These functions only bind
     * runtime data; they do not create duplicate screens.
     */
    HealthUI_Init();
    ClockUI_Init();
    SensorUI_Init();
    SportUI_Init();

    g_ui_current_page = UI_PAGE_HOME;
    g_ui_transition_active = false;

    lv_disp_load_scr(
        g_ui_pages[UI_PAGE_HOME]
    );

    return (
        lv_scr_act() ==
        g_ui_pages[UI_PAGE_HOME]
    );
}

static ui_page_index_t UI_Page_Index_From_Object(
    lv_obj_t *page)
{
    ui_page_index_t index;

    for (index = UI_PAGE_HOME;
         index < UI_PAGE_COUNT;
         index++)
    {
        if (g_ui_pages[index] == page)
        {
            return index;
        }
    }

    return UI_PAGE_COUNT;
}

static void UI_Page_Event(lv_event_t *event)
{
    lv_event_code_t code;
    lv_obj_t *page;

    ui_page_index_t page_index;
    ui_page_index_t next_page;

    lv_indev_t *input;
    lv_dir_t direction;
    lv_scr_load_anim_t animation;

    if (event == NULL)
    {
        return;
    }

    code = lv_event_get_code(event);
    page = lv_event_get_current_target(event);

    page_index =
        UI_Page_Index_From_Object(page);

    if (page_index >= UI_PAGE_COUNT)
    {
        return;
    }

    if (code != LV_EVENT_GESTURE)
    {
        if ((page_index == UI_PAGE_HEALTH) &&
            (code == LV_EVENT_SCREEN_UNLOADED))
        {
            HealthUI_OnScreenUnloaded();
        }

        /*
         * Preserve generated page-loaded/page-unloaded animations and business
         * callbacks. Only generated gesture navigation is suppressed.
         */
        g_ui_generated_screen_callbacks[
            page_index
        ](event);

        if (code == LV_EVENT_SCREEN_LOADED)
        {
            g_ui_current_page = page_index;
            g_ui_transition_active = false;
        }

        return;
    }

    input = lv_indev_get_act();

    if (input == NULL)
    {
        return;
    }

    direction =
        lv_indev_get_gesture_dir(input);

    if ((direction != LV_DIR_LEFT) &&
        (direction != LV_DIR_RIGHT))
    {
        return;
    }

    lv_indev_wait_release(input);

    if (g_ui_transition_active)
    {
        return;
    }

    if (direction == LV_DIR_LEFT)
    {
        next_page =
            (ui_page_index_t)(
                ((uint32_t)page_index + 1U) %
                (uint32_t)UI_PAGE_COUNT
            );

        animation =
            LV_SCR_LOAD_ANIM_MOVE_LEFT;
    }
    else
    {
        next_page =
            (page_index == UI_PAGE_HOME) ?
            UI_PAGE_CALL :
            (ui_page_index_t)(
                (uint32_t)page_index - 1U
            );

        animation =
            LV_SCR_LOAD_ANIM_MOVE_RIGHT;
    }

    g_ui_transition_active = true;

    lv_scr_load_anim(
        g_ui_pages[next_page],
        animation,
        UI_PAGE_ANIM_TIME_MS,
        0U,
        false
    );
}

/* USER CODE END Application */
	