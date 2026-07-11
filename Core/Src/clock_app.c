/**
  ******************************************************************************
  * @file    clock_app.c
  * @brief   STM32 RTC real-time clock and existing Home-page binding
  ******************************************************************************
  *
  * Architecture:
  *   RTC -> SystemTask -> clockUiQueue -> LVGLTask -> existing SquareLine objects
  *
  * No LVGL API is called from SystemTask.
  ******************************************************************************
  */

#include "clock_app.h"

#include "rtc.h"
#include "ui.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CLOCK_UI_QUEUE_LENGTH       2U
#define CLOCK_UPDATE_PERIOD_MS      1000U
#define CLOCK_MIN_VALID_YEAR        26U

typedef struct
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    uint8_t weekday;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} clock_ui_message_t;

static osMessageQueueId_t g_clock_ui_queue = NULL;

static uint8_t g_last_hour = 0xFFU;
static uint8_t g_last_minute = 0xFFU;
static uint8_t g_last_second = 0xFFU;
static uint8_t g_last_date = 0xFFU;
static uint8_t g_last_month = 0xFFU;
static uint8_t g_last_year = 0xFFU;

static const char *const g_month_names[12] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/* HAL RTC weekday values are Monday=1 ... Sunday=7. */
static const char *const g_weekday_names[7] =
{
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

static uint8_t Clock_ParseTwoDigits(const char *text)
{
    uint8_t tens;
    uint8_t ones;

    if (text == NULL)
    {
        return 0U;
    }

    tens = (text[0] >= '0' && text[0] <= '9')
         ? (uint8_t)(text[0] - '0')
         : 0U;

    ones = (text[1] >= '0' && text[1] <= '9')
         ? (uint8_t)(text[1] - '0')
         : 0U;

    return (uint8_t)((tens * 10U) + ones);
}

static uint8_t Clock_ParseBuildMonth(const char *date_text)
{
    uint8_t index;

    if (date_text == NULL)
    {
        return 1U;
    }

    for (index = 0U; index < 12U; index++)
    {
        if ((date_text[0] == g_month_names[index][0]) &&
            (date_text[1] == g_month_names[index][1]) &&
            (date_text[2] == g_month_names[index][2]))
        {
            return (uint8_t)(index + 1U);
        }
    }

    return 1U;
}

static uint8_t Clock_ParseBuildDay(const char *date_text)
{
    uint8_t tens;
    uint8_t ones;

    if (date_text == NULL)
    {
        return 1U;
    }

    tens = (date_text[4] >= '0' && date_text[4] <= '9')
         ? (uint8_t)(date_text[4] - '0')
         : 0U;

    ones = (date_text[5] >= '0' && date_text[5] <= '9')
         ? (uint8_t)(date_text[5] - '0')
         : 1U;

    return (uint8_t)((tens * 10U) + ones);
}

static uint16_t Clock_ParseBuildYear(const char *date_text)
{
    uint16_t year;

    if (date_text == NULL)
    {
        return 2026U;
    }

    year = 0U;

    year += (uint16_t)(date_text[7] - '0') * 1000U;
    year += (uint16_t)(date_text[8] - '0') * 100U;
    year += (uint16_t)(date_text[9] - '0') * 10U;
    year += (uint16_t)(date_text[10] - '0');

    return year;
}

/**
 * @brief Calculate HAL weekday format: Monday=1 ... Sunday=7.
 */
static uint8_t Clock_CalculateWeekday(
    uint16_t year,
    uint8_t month,
    uint8_t day)
{
    static const uint8_t month_table[12] =
    {
        0U, 3U, 2U, 5U, 0U, 3U,
        5U, 1U, 4U, 6U, 2U, 4U
    };

    uint32_t weekday;

    if (month < 3U)
    {
        year--;
    }

    weekday =
        ((uint32_t)year +
         ((uint32_t)year / 4U) -
         ((uint32_t)year / 100U) +
         ((uint32_t)year / 400U) +
         month_table[month - 1U] +
         day) % 7U;

    /* Formula returns Sunday=0, Monday=1 ... Saturday=6. */
    return (weekday == 0U) ? 7U : (uint8_t)weekday;
}

static bool Clock_IsCalendarValid(
    const RTC_TimeTypeDef *time,
    const RTC_DateTypeDef *date)
{
    if ((time == NULL) || (date == NULL))
    {
        return false;
    }

    if ((time->Hours > 23U) ||
        (time->Minutes > 59U) ||
        (time->Seconds > 59U))
    {
        return false;
    }

    if ((date->Year < CLOCK_MIN_VALID_YEAR) ||
        (date->Month < 1U) ||
        (date->Month > 12U) ||
        (date->Date < 1U) ||
        (date->Date > 31U) ||
        (date->WeekDay < 1U) ||
        (date->WeekDay > 7U))
    {
        return false;
    }

    return true;
}

int ClockApp_SetDateTime(
    uint16_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second)
{
    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};

    if ((year < 2000U) ||
        (year > 2099U) ||
        (month < 1U) ||
        (month > 12U) ||
        (day < 1U) ||
        (day > 31U) ||
        (hour > 23U) ||
        (minute > 59U) ||
        (second > 59U))
    {
        return (int)HAL_ERROR;
    }

    time.Hours = hour;
    time.Minutes = minute;
    time.Seconds = second;
    time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    time.StoreOperation = RTC_STOREOPERATION_RESET;

    date.Year = (uint8_t)(year - 2000U);
    date.Month = month;
    date.Date = day;
    date.WeekDay = Clock_CalculateWeekday(year, month, day);

    if (HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN) != HAL_OK)
    {
        return (int)HAL_ERROR;
    }

    if (HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN) != HAL_OK)
    {
        return (int)HAL_ERROR;
    }

    return (int)HAL_OK;
}

static void Clock_InitializeFromBuildTimeIfNeeded(void)
{
    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};

    uint16_t build_year;
    uint8_t build_month;
    uint8_t build_day;
    uint8_t build_hour;
    uint8_t build_minute;
    uint8_t build_second;

    if ((HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) == HAL_OK) &&
        (HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN) == HAL_OK) &&
        Clock_IsCalendarValid(&time, &date))
    {
        return;
    }

    build_year = Clock_ParseBuildYear(__DATE__);
    build_month = Clock_ParseBuildMonth(__DATE__);
    build_day = Clock_ParseBuildDay(__DATE__);

    build_hour = Clock_ParseTwoDigits(&__TIME__[0]);
    build_minute = Clock_ParseTwoDigits(&__TIME__[3]);
    build_second = Clock_ParseTwoDigits(&__TIME__[6]);

    (void)ClockApp_SetDateTime(
        build_year,
        build_month,
        build_day,
        build_hour,
        build_minute,
        build_second
    );
}

static void Clock_PostLatest(const clock_ui_message_t *message)
{
    if ((g_clock_ui_queue == NULL) || (message == NULL))
    {
        return;
    }

    if (osMessageQueuePut(
            g_clock_ui_queue,
            message,
            0U,
            0U) != osOK)
    {
        /* Keep only the newest RTC snapshot. */
        (void)osMessageQueueReset(g_clock_ui_queue);

        (void)osMessageQueuePut(
            g_clock_ui_queue,
            message,
            0U,
            0U
        );
    }
}

void ClockApp_CreateObjects(void)
{
    static const osMessageQueueAttr_t queue_attributes =
    {
        .name = "clockUiQueue"
    };

    if (g_clock_ui_queue == NULL)
    {
        g_clock_ui_queue = osMessageQueueNew(
            CLOCK_UI_QUEUE_LENGTH,
            sizeof(clock_ui_message_t),
            &queue_attributes
        );
    }
}

void ClockTask_Run(void)
{
    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};
    clock_ui_message_t message;
    uint32_t next_wake_tick;
    uint32_t period_ticks;

    Clock_InitializeFromBuildTimeIfNeeded();

    period_ticks =
        (uint32_t)(
            (((uint64_t)CLOCK_UPDATE_PERIOD_MS * osKernelGetTickFreq()) +
             999ULL) /
            1000ULL
        );

    if (period_ticks == 0U)
    {
        period_ticks = 1U;
    }

    next_wake_tick = osKernelGetTickCount();

    for (;;)
    {
        /* STM32 HAL requires reading time first and date immediately after. */
        if ((HAL_RTC_GetTime(
                &hrtc,
                &time,
                RTC_FORMAT_BIN) == HAL_OK) &&
            (HAL_RTC_GetDate(
                &hrtc,
                &date,
                RTC_FORMAT_BIN) == HAL_OK))
        {
            message.hours = time.Hours;
            message.minutes = time.Minutes;
            message.seconds = time.Seconds;

            message.weekday = date.WeekDay;
            message.date = date.Date;
            message.month = date.Month;
            message.year = date.Year;

            Clock_PostLatest(&message);
        }

        next_wake_tick += period_ticks;

        if (osDelayUntil(next_wake_tick) != osOK)
        {
            next_wake_tick = osKernelGetTickCount();
            osDelay(CLOCK_UPDATE_PERIOD_MS);
        }
    }
}

static void ClockUI_Apply(const clock_ui_message_t *message)
{
    char text[32];
    uint16_t second_angle;
    uint16_t minute_angle;
    uint16_t hour_angle;
    uint8_t weekday_index;

    if (message == NULL)
    {
        return;
    }

    if ((ui_HomeTimeLabel == NULL) ||
        (ui_HomeDateLabel == NULL) ||
        (ui_HomeSecondHand == NULL) ||
        (ui_HomeMinuteHand == NULL) ||
        (ui_HomeHourHand == NULL))
    {
        return;
    }

    if ((message->hours != g_last_hour) ||
        (message->minutes != g_last_minute))
    {
        /* The separate ui_dots label supplies the colon. */
        (void)snprintf(
            text,
            sizeof(text),
            "%02u %02u",
            (unsigned int)message->hours,
            (unsigned int)message->minutes
        );

        lv_label_set_text(ui_HomeTimeLabel, text);

        g_last_hour = message->hours;
        g_last_minute = message->minutes;
    }

    if ((message->date != g_last_date) ||
        (message->month != g_last_month) ||
        (message->year != g_last_year))
    {
        weekday_index =
            ((message->weekday >= 1U) &&
             (message->weekday <= 7U))
            ? (uint8_t)(message->weekday - 1U)
            : 0U;

        (void)snprintf(
            text,
            sizeof(text),
            "%s %02u 20%02u %s",
            g_month_names[
                (message->month >= 1U && message->month <= 12U)
                ? (message->month - 1U)
                : 0U
            ],
            (unsigned int)message->date,
            (unsigned int)message->year,
            g_weekday_names[weekday_index]
        );

        lv_label_set_text(ui_HomeDateLabel, text);

        g_last_date = message->date;
        g_last_month = message->month;
        g_last_year = message->year;
    }

    if (message->seconds != g_last_second)
    {
        /* LVGL image angle unit is 0.1 degree. */
        second_angle = (uint16_t)(message->seconds * 60U);

        minute_angle =
            (uint16_t)(
                (message->minutes * 60U) +
                message->seconds
            );

        hour_angle =
            (uint16_t)(
                ((message->hours % 12U) * 300U) +
                (message->minutes * 5U) +
                (message->seconds / 12U)
            );

        lv_img_set_angle(ui_HomeSecondHand, (int16_t)second_angle);
        lv_img_set_angle(ui_HomeMinuteHand, (int16_t)minute_angle);
        lv_img_set_angle(ui_HomeHourHand, (int16_t)hour_angle);

        if (ui_dots != NULL)
        {
            lv_obj_set_style_text_opa(
                ui_dots,
                ((message->seconds & 1U) != 0U)
                    ? LV_OPA_COVER
                    : LV_OPA_40,
                LV_PART_MAIN | LV_STATE_DEFAULT
            );
        }

        g_last_second = message->seconds;
    }
}

void ClockUI_Init(void)
{
    g_last_hour = 0xFFU;
    g_last_minute = 0xFFU;
    g_last_second = 0xFFU;
    g_last_date = 0xFFU;
    g_last_month = 0xFFU;
    g_last_year = 0xFFU;

    if (ui_dots != NULL)
    {
        lv_label_set_text(ui_dots, ":");
    }
}

void ClockUI_ProcessQueue(void)
{
    clock_ui_message_t message;
    clock_ui_message_t latest;
    bool received;

    if (g_clock_ui_queue == NULL)
    {
        return;
    }

    received = false;

    while (osMessageQueueGet(
               g_clock_ui_queue,
               &message,
               NULL,
               0U) == osOK)
    {
        latest = message;
        received = true;
    }

    if (received)
    {
        ClockUI_Apply(&latest);
    }
}
