#include "led.h"
#include "hw_config.h"
#include "stm32g431xx.h"
#include "stm32g4xx.h"
#include <stdint.h>

static uint8_t led_status = 0;

/***********************************************************************
-- PUBLIC FUNCTIONS --
***********************************************************************/

/*!
    @brief Initialize the red status LED
*/
void led_init(void) {
    /* Configure PB5 as GPIO */
    MODIFY_REG(
        GPIOB->MODER,
        GPIO_MODER_MODE5_Msk,
        GPIO_PB5_GPO_MODE
    );

    /* LED is active-low so initialize the pin to logic level high */
    SET_BIT(GPIOB->BSRR, GPIO_PB5_BS);
}

/*!
    @brief Turn on the LED
*/
void led_on(void) {
    SET_BIT(GPIOB->BSRR, GPIO_PB5_BR);
    led_status = 1;
}

/*!
    @brief Turn off the LED
*/
void led_off(void) {
    SET_BIT(GPIOB->BSRR, GPIO_PB5_BS);
    led_status = 0;
}

/*!
    @brief Toggle the LED
*/
void led_toggle(void) {
    if (led_status) {
        led_off();
    }
    else {
        led_on();
    }
}