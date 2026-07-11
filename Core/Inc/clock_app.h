/**
  ******************************************************************************
  * @file    clock_app.h
  * @brief   RTC real-time clock task and Home-page LVGL binding
  ******************************************************************************
  */

#ifndef CLOCK_APP_H
#define CLOCK_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_os2.h"
#include <stdint.h>

/**
 * @brief Create the private RTC-to-LVGL message queue.
 * @note  Call before creating/starting the RTOS tasks.
 */
void ClockApp_CreateObjects(void);

/**
 * @brief RTC worker loop. This function does not return.
 * @note  Call only from SystemTask.
 */
void ClockTask_Run(void);

/**
 * @brief Initialize the existing SquareLine clock objects.
 * @note  Call only in LVGLTask after ui_init().
 */
void ClockUI_Init(void);

/**
 * @brief Drain RTC messages and update labels/hands.
 * @note  Call only from LVGLTask.
 */
void ClockUI_ProcessQueue(void);

/**
 * @brief Set RTC calendar manually.
 * @param year       Full year, for example 2026.
 * @param month      1..12.
 * @param day        1..31.
 * @param hour       0..23.
 * @param minute     0..59.
 * @param second     0..59.
 * @return HAL_OK on success.
 */
int ClockApp_SetDateTime(
    uint16_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second
);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_APP_H */
