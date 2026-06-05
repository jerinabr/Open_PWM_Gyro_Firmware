#include "stabilize.h"
#include "hw_config.h"
#include "math3d.h"
#include <stdint.h>

/* Stabilization mode can be 0, 1, 2, 3, 4 */
uint8_t stabilization_mode = 0;

/* Function pointer for stabilization function */
static void (*st_func)(struct Stabilization_Context *context) = gyro_off;

/***********************************************************************
-- PRIVATE FUNCTIONS --
***********************************************************************/

/*!
    @brief Pass control input straight to control output with no modifications
    @param context Stabilization context
*/
void gyro_off(struct Stabilization_Context *context) {
    context->control_output.pitch = context->input.control_input.pitch;
}

/*!
    @brief Apply basic gyro stabilization to output
    @param context Stabilization context
*/
void gyro_stabilize(struct Stabilization_Context *context) {
    
}

/***********************************************************************
-- PUBLIC FUNCTIONS --
***********************************************************************/

/*!
    @brief Change the function called by the stabilization function based on the
    stabilization mode
    @param context Stabilization context
    @details This function should be called any time the stabilization mode is
    updated
*/
void set_stabilization_mode(struct Stabilization_Context *context) {
    switch (context->input.mode) {
        case GYRO_MODE_OFF: {
            st_func = gyro_off;
            break;
        }
        case GYRO_MODE_STABILIZE: {
            st_func = gyro_stabilize;
            break;
        }
        default: {
            st_func = gyro_off;
            break;
        }
    }
}

/*!
    @brief Apply stabilization to PWM outputs
    @param context Stabilization context
    @details The control_output member of the stabilization context struct will
    contain normalized and stabilized control outputs based on the control
    limits given.

    This function should be called at a frequency no faster than the IMU
    sampling frequency and no slower than the servo update rate.

    This function simply calls the ACTUAL stabilization function because the
    stabilization mode won't change frequently
*/
void apply_stabilization(struct Stabilization_Context *context) {
    st_func(context);
}