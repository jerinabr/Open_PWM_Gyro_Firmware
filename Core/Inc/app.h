/*!
  @file   app.h
  @brief  Main user code implementation to avoid the ST generated comments

  Instead of deleting all the USER CODE comments in the main.c provided by ST,
  the actual application code will be here and the config() and loop() functions
  will be called in the ST provided main.c file

  This preserves the main.c code and keeps the application code from being deleted
  if code generation with CubeMX is ever needed
*/
#ifndef APP_H
#define APP_H

// ST libraries
#include "stm32g4xx_hal.h"
#include "usbd_cdc_if.h"

// User libraries
#include "Receiver.h"

#define LED_RED_PORT  GPIOB
#define LED_RED_PIN   GPIO_PIN_5

#ifdef __cplusplus
extern "C" {
#endif

void config(void);
void loop(void);

#ifdef __cplusplus
}
#endif

#endif