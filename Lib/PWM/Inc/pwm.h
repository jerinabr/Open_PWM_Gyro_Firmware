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

#define NUM_CHANNELS 6

// ----------------------------------------------------------------------
// TYPES
// ----------------------------------------------------------------------

// Struct to hold channel PWM outputs
typedef struct {
    /*!< Integers between -500 and 500
        These values are mapped to a 1000us-2000us pulse width */
    int16_t ch_outputs[NUM_CHANNELS];
    
    /*!< Indicates if the channel output is reversed
        0 - Channel outputs aren't reversed
        1 - Channel outputs are reversed */
    uint8_t ch_reversed[NUM_CHANNELS];
} pwm_s;

// Global instance of pwm
extern pwm_s pwm;

// ----------------------------------------------------------------------
// FUNCTIONS
// ----------------------------------------------------------------------

void pwm_init(void);
void pwm_update(void);

#ifdef __cplusplus
}
#endif

#endif