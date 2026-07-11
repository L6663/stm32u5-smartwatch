#ifndef UI_APP_DATA_H
#define UI_APP_DATA_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UI_APP_TEXT_LENGTH 28U

typedef enum
{
    UI_FIELD_HOME_TIME = 0,
    UI_FIELD_HOME_DATE,
    UI_FIELD_HOME_WEATHER_TEMP,
    UI_FIELD_HOME_WEATHER_STATE,
    UI_FIELD_HOME_BATTERY,
    UI_FIELD_HOME_TEMPERATURE,
    UI_FIELD_HOME_HUMIDITY,
    UI_FIELD_HOME_STEPS,
    UI_FIELD_HEALTH_HEART_RATE,
    UI_FIELD_HEALTH_STATE,
    UI_FIELD_HEALTH_RANGE,
    UI_FIELD_HEALTH_MAX,
    UI_FIELD_HEALTH_MIN,
    UI_FIELD_SPORT_MODE,
    UI_FIELD_SPORT_HEART_RATE,
    UI_FIELD_SPORT_CALORIES,
    UI_FIELD_SPORT_DURATION,
    UI_FIELD_SPORT_DISTANCE,
    UI_FIELD_WEATHER_LOCATION,
    UI_FIELD_WEATHER_DATE,
    UI_FIELD_WEATHER_TEMPERATURE,
    UI_FIELD_WEATHER_CONDITION,
    UI_FIELD_WEATHER_HIGH_LOW,
    UI_FIELD_CALLER_NAME,
    UI_FIELD_CALL_STATE,
    UI_FIELD_CALL_DETAIL,
    UI_FIELD_COUNT
} ui_field_id_t;

typedef struct
{
    ui_field_id_t field;
    char text[UI_APP_TEXT_LENGTH];
} ui_app_message_t;

/* Thread-context API. This function never calls LVGL. */
bool UI_PostText(ui_field_id_t field, const char *text, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* UI_APP_DATA_H */