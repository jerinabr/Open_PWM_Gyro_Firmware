#include "pwm.h"
#include "timers.h"
#include "hw_config.h"
#include <stdint.h>

uint8_t ch_reversed[NUM_OUTPUT_CHANNELS] = {0};

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
    @param pulse_widths array of pulse widths in 1 us resolution. Values should
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

    /* Timer 4 compare registers */
    timers.tim4_cc[1] = pulse_widths[0];
    timers.tim4_cc[0] = pulse_widths[1];

    /* Timer 2 compare registers */
    timers.tim2_cc[0] = pulse_widths[2];
    timers.tim2_cc[1] = pulse_widths[3];
    timers.tim2_cc[2] = pulse_widths[4];
    timers.tim2_cc[3] = pulse_widths[5];
    timers_update_cc();
}

/*!
    @brief Configure a channel as reversed or normal
    @param channel channel number (must be less than NUM_OUTPUT_CHANNELS)
    @param reversed 0 for normal, 1 for reversed
*/
void pwm_channel_reverse_config(uint8_t channel, uint8_t reversed) {
    if (channel < NUM_OUTPUT_CHANNELS) {
        ch_reversed[channel] = reversed;
    }
}