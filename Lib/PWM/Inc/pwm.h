/*!
    @file   pwm.h
    @brief  Convert channel outputs to timer PWM outputs

    -- CHANNEL MAPPING --
    TIM4 CH2 -> CH1 (PB7)
    TIM4 CH1 -> CH2 (PB6)
    TIM2 CH1 -> CH3 (PA0)
    TIM2 CH2 -> CH4 (PA1)
    TIM2 CH3 -> CH5 (PA2)
    TIM2 CH4 -> CH6 (PA3)
*/
#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define PULSE_WIDTH_MIN 1000
#define PULSE_WIDTH_MAX 2000
#define PULSE_WIDTH_CENTER 1500

/***********************************************************************
-- FUNCTIONS --
***********************************************************************/

void pwm_init(void);
void pwm_update_pw(uint16_t pulse_widths[]);
void pwm_channel_reverse_config(uint8_t channel, uint8_t reversed);

#ifdef __cplusplus
}
#endif

#endif