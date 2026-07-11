/**
  ******************************************************************************
  * @file    sensor_app.h
  * @brief   SHT20, demo steps and weather-state task
  ******************************************************************************
  */

#ifndef SENSOR_APP_H
#define SENSOR_APP_H

#ifdef __cplusplus
extern "C" {
#endif

void SensorApp_CreateObjects(void);
void SensorTask_Run(void);

/* These functions contain LVGL calls and must run only in LVGLTask. */
void SensorUI_Init(void);
void SensorUI_ProcessQueue(void);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_APP_H */
