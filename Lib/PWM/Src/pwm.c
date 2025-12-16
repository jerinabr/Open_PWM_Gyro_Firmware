#include "pwm.h"
#include "timers.h"
#include <stdint.h>

// Initialize the pwm instance
pwm_s pwm = {
    .ch_outputs = {0},
    .ch_reversed = {0}
};

// Array of converted channel outputs
uint32_t channel_output_cc[NUM_CHANNELS];

// ----------------------------------------------------------------------
// PRIVATE FUNCTIONS
// ----------------------------------------------------------------------

/*!
    @brief Convert channel outputs to timer compare register values
    @details The channel is mapped 1000-2000 if it isn't reversed

    If the channel is reversed, a value of -500 maps to 2000 and a value of
    500 maps to 1000
*/
static void convert_ch_outputs_to_cc(void) {
    for (int i = 0; i < NUM_CHANNELS; i++) {
        if (pwm.ch_reversed[i]) {
            channel_output_cc[i] = (uint32_t) (1500 - pwm.ch_outputs[i]);
        }
        else {
            channel_output_cc[i] = (uint32_t) (pwm.ch_outputs[i] + 1500);
        }
    }
}

// ----------------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------------

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
    @brief Update the PWM outputs
    @details This function converts the channel values to timer compare values
    and then maps them to the correct timers and channels
*/
void pwm_update(void) {
    convert_ch_outputs_to_cc();
    // Timer 4 compare registers
    timers.tim4_cc[1] = channel_output_cc[0];
    timers.tim4_cc[0] = channel_output_cc[1];
    // Timer 2 compare registers
    timers.tim2_cc[0] = channel_output_cc[2];
    timers.tim2_cc[1] = channel_output_cc[3];
    timers.tim2_cc[2] = channel_output_cc[4];
    timers.tim2_cc[3] = channel_output_cc[5];
    timers_update_cc();
}