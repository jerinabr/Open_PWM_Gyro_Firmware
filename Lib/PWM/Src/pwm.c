#include "pwm.h"
#include "timers.h"
#include "hw_config.h"
#include <stdint.h>

/* Channel configuration */
uint8_t ch_reversed[NUM_OUTPUT_CHANNELS] = {0};

/* Timer data structure */
struct Timer_Data timer_data;

/***********************************************************************
-- PUBLIC FUNCTIONS --
***********************************************************************/

/*!
    @brief Initialize the PWM outputs
    @details The PWM outputs are held low on initialization and don't go high
    until the channel values are greater than 0 and the pwm update function has
    been called
*/
void pwm_init(void) {
    timers_init();
}

/*!
    @brief Update the PWM pulse widths
    @param pulse_widths Array of pulse widths in 1 us resolution. Values should
    be in the range [500, 2500]
    @details This function updates the pulse widths based on configuration and
    then maps them to the correct timers and channels
*/
void pwm_update_pw(uint16_t pulse_widths[]) {
    for (int i = 0; i < NUM_OUTPUT_CHANNELS; i++) {
        /* Clamp the pulse width */
        if (pulse_widths[i] < PULSE_WIDTH_MIN) {
            pulse_widths[i] = PULSE_WIDTH_MIN;
        } else if (pulse_widths[i] > PULSE_WIDTH_MAX) {
            pulse_widths[i] = PULSE_WIDTH_MAX;
        }

        /* Reverse the channel around the pulse width center */
        if (ch_reversed[i]) {
            pulse_widths[i] = 2 * PULSE_WIDTH_CENTER - pulse_widths[i];
        }
    }

    /* Update timer 4 compare registers */
    timer_data.tim4_cc[1] = (uint32_t) pulse_widths[0];
    timer_data.tim4_cc[0] = (uint32_t) pulse_widths[1];

    /* Update timer 2 compare registers */
    timer_data.tim2_cc[0] = (uint32_t) pulse_widths[2];
    timer_data.tim2_cc[1] = (uint32_t) pulse_widths[3];
    timer_data.tim2_cc[2] = (uint32_t) pulse_widths[4];
    timer_data.tim2_cc[3] = (uint32_t) pulse_widths[5];

    /* Update timer CC values */
    timers_update_cc(&timer_data);
}

/*!
    @brief Configure a channel as reversed or normal
    @param channel Channel number (must be less than NUM_OUTPUT_CHANNELS)
    @param reversed 0 for normal, 1 for reversed
*/
void pwm_channel_reverse_config(uint8_t channel, uint8_t reversed) {
    if (channel < NUM_OUTPUT_CHANNELS) {
        ch_reversed[channel] = reversed;
    }
}