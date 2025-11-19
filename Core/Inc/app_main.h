/*!
  @file   app_main.h
  @brief  Main user code implementation to avoid the ST generated comments

  Instead of deleting all the USER CODE comments in the main.c provided by ST,
  the actual application code will be here and the app_config() and app_loop()
  functions will be called in the ST provided main.c file

  This preserves the main.c code and keeps the application code from being deleted
  if code generation with CubeMX is ever needed
*/
#ifndef APP_MAIN_H
#define APP_MAIN_H

#include "stm32g4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void app_config(void);
void app_loop(void);

#ifdef __cplusplus
}
#endif

#endif