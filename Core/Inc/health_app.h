/**
  ******************************************************************************
  * @file    health_app.h
  * @brief   MAX30102 heart-rate/SpO2 acquisition and Health page controller
  ******************************************************************************
  */

#ifndef HEALTH_APP_H
#define HEALTH_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * Create the Health command queue and Health-to-LVGL queue.
 * Call this once in MX_FREERTOS_Init() after osKernelInitialize().
 */
void HealthApp_CreateObjects(void);

/*
 * Non-blocking measurement commands.
 * They may be called from LVGL event callbacks.
 */
void HealthApp_RequestStart(void);
void HealthApp_RequestStop(void);

/*
 * Permanent HealthTask entry.
 * This function never returns.
 */
void HealthTask_Run(void);

/*
 * The following functions contain LVGL operations and therefore must only be
 * called from LVGLTask.
 */
void HealthUI_Init(void);
void HealthUI_ProcessQueue(void);
void HealthUI_OnScreenUnloaded(void);

/* Runtime diagnostics for Keil Watch. */
extern volatile uint8_t  g_max30102_part_id;
extern volatile uint32_t g_max30102_last_red;
extern volatile uint32_t g_max30102_last_ir;
extern volatile uint16_t g_health_last_bpm;
extern volatile uint8_t  g_health_last_spo2;
extern volatile uint32_t g_health_i2c_error_count;
extern volatile uint32_t g_health_ui_drop_count;

#ifdef __cplusplus
}
#endif

#endif /* HEALTH_APP_H */
