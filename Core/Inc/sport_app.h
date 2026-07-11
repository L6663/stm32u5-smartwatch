/**
  ******************************************************************************
  * @file    sport_app.h
  * @brief   Demonstration sport-session controller
  ******************************************************************************
  */

#ifndef SPORT_APP_H
#define SPORT_APP_H

#ifdef __cplusplus
extern "C" {
#endif

void SportApp_CreateObjects(void);

/*
 * Called once per second by SensorTask.
 * This function contains no LVGL calls.
 */
void SportApp_Tick1s(void);

/*
 * These functions contain LVGL calls and must run only in LVGLTask.
 */
void SportUI_Init(void);
void SportUI_ProcessQueue(void);

#ifdef __cplusplus
}
#endif

#endif /* SPORT_APP_H */
