/*!
    @file   led.h
    @brief  Control the onboard red status LED
*/

#ifndef LED_H
#define LED_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Initialize LED GPIO */
void led_init(void);

/* LED on/off */
void led_on(void);
void led_off(void);

#ifdef __cplusplus
}
#endif

#endif