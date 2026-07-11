/**
  ******************************************************************************
  * @file    sport_app.c
  * @brief   Walk/run/cycling demonstration session
  ******************************************************************************
  *
  * The current board has no GPS or motion sensor, therefore distance, calories
  * and sport heart rate are demonstration values. The code keeps the data flow
  * correct:
  *
  * LVGL button
  *   -> command queue
  *   -> SensorTask calls SportApp_Tick1s()
  *   -> private UI queue
  *   -> LVGLTask calls SportUI_ProcessQueue()
  *
  * No LVGL API is called outside LVGLTask.
  ******************************************************************************
  */

#include "sport_app.h"

#include "cmsis_os2.h"
#include "lvgl.h"
#include "ui.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef enum
{
    SPORT_MODE_WALK = 0,
    SPORT_MODE_RUN,
    SPORT_MODE_CYCLING,
    SPORT_MODE_COUNT
} sport_mode_t;

typedef enum
{
    SPORT_COMMAND_START = 1,
    SPORT_COMMAND_STOP,
    SPORT_COMMAND_PREVIOUS_MODE,
    SPORT_COMMAND_NEXT_MODE,
    SPORT_COMMAND_RESET
} sport_command_t;

typedef struct
{
    sport_mode_t mode;
    bool running;

    uint32_t elapsed_seconds;
    uint32_t calories_centi;
    uint32_t distance_centimeters;

    uint16_t heart_rate;
    uint8_t progress_percent;
} sport_ui_message_t;

typedef struct
{
    sport_mode_t mode;
    bool running;

    uint32_t elapsed_seconds;
    uint32_t calories_centi;
    uint32_t distance_centimeters;

    uint16_t heart_rate;
    uint32_t random_state;
} sport_state_t;

static osMessageQueueId_t g_sport_command_queue = NULL;
static osMessageQueueId_t g_sport_ui_queue = NULL;

static sport_state_t g_sport_state =
{
    .mode = SPORT_MODE_RUN,
    .running = false,
    .elapsed_seconds = 0U,
    .calories_centi = 0U,
    .distance_centimeters = 0U,
    .heart_rate = 78U,
    .random_state = 0x2468ACE1U
};

static const char *const g_sport_mode_names[SPORT_MODE_COUNT] =
{
    "Outdoor Walk",
    "Outdoor Run",
    "Cycling"
};

static void SportApp_SendCommand(
    sport_command_t command)
{
    if (g_sport_command_queue == NULL)
    {
        return;
    }

    if (osMessageQueuePut(
            g_sport_command_queue,
            &command,
            0U,
            0U) != osOK)
    {
        (void)osMessageQueueReset(
            g_sport_command_queue
        );

        (void)osMessageQueuePut(
            g_sport_command_queue,
            &command,
            0U,
            0U
        );
    }
}

static void SportApp_ResetSession(void)
{
    g_sport_state.running = false;
    g_sport_state.elapsed_seconds = 0U;
    g_sport_state.calories_centi = 0U;
    g_sport_state.distance_centimeters = 0U;

    switch (g_sport_state.mode)
    {
        case SPORT_MODE_WALK:
            g_sport_state.heart_rate = 82U;
            break;

        case SPORT_MODE_CYCLING:
            g_sport_state.heart_rate = 88U;
            break;

        case SPORT_MODE_RUN:
        default:
            g_sport_state.heart_rate = 90U;
            break;
    }
}

static uint32_t SportApp_NextRandom(void)
{
    g_sport_state.random_state =
        g_sport_state.random_state *
        1664525U +
        1013904223U;

    return g_sport_state.random_state;
}

static uint16_t SportApp_TargetHeartRate(void)
{
    int32_t base;
    int32_t variation;

    switch (g_sport_state.mode)
    {
        case SPORT_MODE_WALK:
            base = 96;
            variation =
                (int32_t)((SportApp_NextRandom() >> 24U) % 13U) - 6;
            break;

        case SPORT_MODE_CYCLING:
            base = 124;
            variation =
                (int32_t)((SportApp_NextRandom() >> 24U) % 17U) - 8;
            break;

        case SPORT_MODE_RUN:
        default:
            base = 138;
            variation =
                (int32_t)((SportApp_NextRandom() >> 24U) % 21U) - 10;
            break;
    }

    return (uint16_t)(base + variation);
}

static void SportApp_UpdateHeartRate(void)
{
    uint16_t target;

    target = SportApp_TargetHeartRate();

    if (g_sport_state.heart_rate < target)
    {
        g_sport_state.heart_rate +=
            ((target - g_sport_state.heart_rate) > 3U) ?
            3U :
            1U;
    }
    else if (g_sport_state.heart_rate > target)
    {
        g_sport_state.heart_rate -=
            ((g_sport_state.heart_rate - target) > 3U) ?
            3U :
            1U;
    }
}

static uint32_t SportApp_GoalCentimeters(void)
{
    switch (g_sport_state.mode)
    {
        case SPORT_MODE_WALK:
            return 200000U;   /* 2.00 km */

        case SPORT_MODE_CYCLING:
            return 500000U;   /* 5.00 km */

        case SPORT_MODE_RUN:
        default:
            return 300000U;   /* 3.00 km */
    }
}

static uint8_t SportApp_GetProgress(void)
{
    uint32_t goal;
    uint32_t progress;

    goal = SportApp_GoalCentimeters();

    if (goal == 0U)
    {
        return 0U;
    }

    progress =
        (g_sport_state.distance_centimeters * 100U) /
        goal;

    if (progress > 100U)
    {
        progress = 100U;
    }

    return (uint8_t)progress;
}

static void SportApp_SendUiState(void)
{
    sport_ui_message_t message;

    if (g_sport_ui_queue == NULL)
    {
        return;
    }

    memset(&message, 0, sizeof(message));

    message.mode = g_sport_state.mode;
    message.running = g_sport_state.running;
    message.elapsed_seconds =
        g_sport_state.elapsed_seconds;
    message.calories_centi =
        g_sport_state.calories_centi;
    message.distance_centimeters =
        g_sport_state.distance_centimeters;
    message.heart_rate =
        g_sport_state.heart_rate;
    message.progress_percent =
        SportApp_GetProgress();

    if (osMessageQueuePut(
            g_sport_ui_queue,
            &message,
            0U,
            0U) != osOK)
    {
        (void)osMessageQueueReset(
            g_sport_ui_queue
        );

        (void)osMessageQueuePut(
            g_sport_ui_queue,
            &message,
            0U,
            0U
        );
    }
}

static void SportApp_ProcessCommands(void)
{
    sport_command_t command;
    bool changed;

    changed = false;

    while (osMessageQueueGet(
               g_sport_command_queue,
               &command,
               NULL,
               0U) == osOK)
    {
        switch (command)
        {
            case SPORT_COMMAND_START:
                g_sport_state.running = true;
                changed = true;
                break;

            case SPORT_COMMAND_STOP:
                g_sport_state.running = false;
                changed = true;
                break;

            case SPORT_COMMAND_PREVIOUS_MODE:
                if (!g_sport_state.running)
                {
                    if (g_sport_state.mode == SPORT_MODE_WALK)
                    {
                        g_sport_state.mode =
                            SPORT_MODE_CYCLING;
                    }
                    else
                    {
                        g_sport_state.mode =
                            (sport_mode_t)(
                                (uint32_t)g_sport_state.mode - 1U
                            );
                    }

                    SportApp_ResetSession();
                    changed = true;
                }
                break;

            case SPORT_COMMAND_NEXT_MODE:
                if (!g_sport_state.running)
                {
                    g_sport_state.mode =
                        (sport_mode_t)(
                            ((uint32_t)g_sport_state.mode + 1U) %
                            (uint32_t)SPORT_MODE_COUNT
                        );

                    SportApp_ResetSession();
                    changed = true;
                }
                break;

            case SPORT_COMMAND_RESET:
                SportApp_ResetSession();
                changed = true;
                break;

            default:
                break;
        }
    }

    if (changed)
    {
        SportApp_SendUiState();
    }
}

void SportApp_CreateObjects(void)
{
    static const osMessageQueueAttr_t command_attributes =
    {
        .name = "sportCommandQueue"
    };

    static const osMessageQueueAttr_t ui_attributes =
    {
        .name = "sportUiQueue"
    };

    if (g_sport_command_queue == NULL)
    {
        g_sport_command_queue =
            osMessageQueueNew(
                8U,
                sizeof(sport_command_t),
                &command_attributes
            );
    }

    if (g_sport_ui_queue == NULL)
    {
        g_sport_ui_queue =
            osMessageQueueNew(
                4U,
                sizeof(sport_ui_message_t),
                &ui_attributes
            );
    }
}

void SportApp_Tick1s(void)
{
    uint32_t random_value;
    uint32_t simulated_steps;

    SportApp_ProcessCommands();

    if (!g_sport_state.running)
    {
        return;
    }

    g_sport_state.elapsed_seconds++;

    random_value =
        (SportApp_NextRandom() >> 24U) % 10U;

    switch (g_sport_state.mode)
    {
        case SPORT_MODE_WALK:
            simulated_steps =
                1U + (random_value % 2U);

            g_sport_state.distance_centimeters +=
                simulated_steps * 72U;

            /*
             * Approximately 4.2 kcal/minute.
             */
            g_sport_state.calories_centi += 7U;
            break;

        case SPORT_MODE_CYCLING:
            /*
             * 500 cm/s equals approximately 18 km/h.
             */
            g_sport_state.distance_centimeters += 500U;

            /*
             * Approximately 7.8 kcal/minute.
             */
            g_sport_state.calories_centi += 13U;
            break;

        case SPORT_MODE_RUN:
        default:
            simulated_steps =
                2U + (random_value % 3U);

            g_sport_state.distance_centimeters +=
                simulated_steps * 95U;

            /*
             * Approximately 10.2 kcal/minute.
             */
            g_sport_state.calories_centi += 17U;
            break;
    }

    SportApp_UpdateHeartRate();
    SportApp_SendUiState();
}

static void SportUI_StartButtonEvent(
    lv_event_t *event)
{
    lv_event_code_t code;
    lv_obj_t *button;

    if (event == NULL)
    {
        return;
    }

    code = lv_event_get_code(event);

    if (code != LV_EVENT_VALUE_CHANGED)
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
        SportApp_SendCommand(
            SPORT_COMMAND_START
        );
    }
    else
    {
        SportApp_SendCommand(
            SPORT_COMMAND_STOP
        );
    }
}

static void SportUI_PreviousEvent(
    lv_event_t *event)
{
    if ((event != NULL) &&
        (lv_event_get_code(event) ==
         LV_EVENT_CLICKED))
    {
        SportApp_SendCommand(
            SPORT_COMMAND_PREVIOUS_MODE
        );
    }
}

static void SportUI_NextEvent(
    lv_event_t *event)
{
    if ((event != NULL) &&
        (lv_event_get_code(event) ==
         LV_EVENT_CLICKED))
    {
        SportApp_SendCommand(
            SPORT_COMMAND_NEXT_MODE
        );
    }
}

void SportUI_Init(void)
{
    if (ui_ScreenSport == NULL)
    {
        return;
    }

    if (ui_BtnSportStart != NULL)
    {
        lv_obj_clear_state(
            ui_BtnSportStart,
            LV_STATE_CHECKED |
            LV_STATE_PRESSED
        );

        (void)lv_obj_add_event_cb(
            ui_BtnSportStart,
            SportUI_StartButtonEvent,
            LV_EVENT_VALUE_CHANGED,
            NULL
        );
    }

    if (ui_BtnPrev != NULL)
    {
        (void)lv_obj_add_event_cb(
            ui_BtnPrev,
            SportUI_PreviousEvent,
            LV_EVENT_CLICKED,
            NULL
        );
    }

    if (ui_BtnNext != NULL)
    {
        (void)lv_obj_add_event_cb(
            ui_BtnNext,
            SportUI_NextEvent,
            LV_EVENT_CLICKED,
            NULL
        );
    }

    if (ui_SportArc != NULL)
    {
        lv_arc_set_range(
            ui_SportArc,
            0,
            100
        );

        lv_arc_set_value(
            ui_SportArc,
            0
        );

        lv_obj_clear_flag(
            ui_SportArc,
            LV_OBJ_FLAG_CLICKABLE
        );
    }

    if (ui_LabelSportMode != NULL)
    {
        lv_label_set_text(
            ui_LabelSportMode,
            "Outdoor Run"
        );
    }

    if (ui_Label37 != NULL)
    {
        lv_label_set_text(
            ui_Label37,
            "Ready"
        );
    }

    if (ui_LabelSportHeart != NULL)
    {
        lv_label_set_text(
            ui_LabelSportHeart,
            "Heart -- bpm"
        );
    }

    if (ui_Label41 != NULL)
    {
        lv_label_set_text(
            ui_Label41,
            "0"
        );
    }

    if (ui_Label42 != NULL)
    {
        lv_label_set_text(
            ui_Label42,
            "kcal"
        );
    }

    if (ui_Duration != NULL)
    {
        lv_label_set_text(
            ui_Duration,
            "Duration"
        );
    }

    if (ui_Label6 != NULL)
    {
        lv_label_set_text(
            ui_Label6,
            "00:00"
        );
    }

    if (ui_min1 != NULL)
    {
        lv_label_set_text(
            ui_min1,
            "min:s"
        );
    }

    if (ui_Label9 != NULL)
    {
        lv_label_set_text(
            ui_Label9,
            "0.00"
        );
    }

    if (ui_Label10 != NULL)
    {
        lv_label_set_text(
            ui_Label10,
            "km"
        );
    }

    if (ui_LabelSportStart != NULL)
    {
        lv_label_set_text(
            ui_LabelSportStart,
            "START"
        );
    }

    if (ui_PlayCircle != NULL)
    {
        lv_obj_clear_state(
            ui_PlayCircle,
            LV_STATE_CHECKED |
            LV_STATE_PRESSED
        );
    }

    SportApp_SendUiState();
}

void SportUI_ProcessQueue(void)
{
    sport_ui_message_t message;
    sport_ui_message_t latest;
    bool received;

    uint32_t minutes;
    uint32_t seconds;

    uint32_t kilometers;
    uint32_t hundredths;

    if (g_sport_ui_queue == NULL)
    {
        return;
    }

    received = false;

    while (osMessageQueueGet(
               g_sport_ui_queue,
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

    if (ui_LabelSportMode != NULL)
    {
        lv_label_set_text(
            ui_LabelSportMode,
            g_sport_mode_names[latest.mode]
        );
    }

    if (ui_Label37 != NULL)
    {
        if (latest.running)
        {
            lv_label_set_text(
                ui_Label37,
                "Running"
            );

            lv_obj_set_style_text_color(
                ui_Label37,
                lv_color_hex(0x39E58C),
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );
        }
        else if (latest.elapsed_seconds == 0U)
        {
            lv_label_set_text(
                ui_Label37,
                "Ready"
            );

            lv_obj_set_style_text_color(
                ui_Label37,
                lv_color_hex(0xA9B6C4),
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );
        }
        else
        {
            lv_label_set_text(
                ui_Label37,
                "Paused"
            );

            lv_obj_set_style_text_color(
                ui_Label37,
                lv_color_hex(0xFF9F43),
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );
        }
    }

    if (ui_LabelSportHeart != NULL)
    {
        lv_label_set_text_fmt(
            ui_LabelSportHeart,
            "Heart %u bpm",
            (unsigned int)latest.heart_rate
        );
    }

    if (ui_Label41 != NULL)
    {
        lv_label_set_text_fmt(
            ui_Label41,
            "%lu",
            (unsigned long)(
                latest.calories_centi / 100U
            )
        );
    }

    minutes =
        latest.elapsed_seconds / 60U;

    seconds =
        latest.elapsed_seconds % 60U;

    if (ui_Label6 != NULL)
    {
        lv_label_set_text_fmt(
            ui_Label6,
            "%02lu:%02lu",
            (unsigned long)minutes,
            (unsigned long)seconds
        );
    }

    kilometers =
        latest.distance_centimeters /
        100000U;

    hundredths =
        (latest.distance_centimeters %
         100000U) /
        1000U;

    if (ui_Label9 != NULL)
    {
        lv_label_set_text_fmt(
            ui_Label9,
            "%lu.%02lu",
            (unsigned long)kilometers,
            (unsigned long)hundredths
        );
    }

    if (ui_SportArc != NULL)
    {
        lv_arc_set_value(
            ui_SportArc,
            latest.progress_percent
        );
    }

    if (ui_BtnSportStart != NULL)
    {
        if (latest.running)
        {
            lv_obj_add_state(
                ui_BtnSportStart,
                LV_STATE_CHECKED
            );
        }
        else
        {
            lv_obj_clear_state(
                ui_BtnSportStart,
                LV_STATE_CHECKED
            );
        }
    }

    if (ui_LabelSportStart != NULL)
    {
        lv_label_set_text(
            ui_LabelSportStart,
            latest.running ?
            "STOP" :
            "START"
        );
    }

    if (ui_PlayCircle != NULL)
    {
        if (latest.running)
        {
            lv_obj_add_state(
                ui_PlayCircle,
                LV_STATE_CHECKED
            );
        }
        else
        {
            lv_obj_clear_state(
                ui_PlayCircle,
                LV_STATE_CHECKED
            );
        }
    }
}
