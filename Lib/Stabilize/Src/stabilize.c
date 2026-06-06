#include "stabilize.h"
#include "hw_config.h"
#include "math3d.h"
#include <stdint.h>

/* Clamp number between -1 and 1 */
#define CLAMP(VAL) (VAL > 1 ? 1 : (VAL < -1 ? -1 : VAL))

/* Return absolute value of number */
#define ABS(VAL) (VAL < 0 ? -VAL : VAL)

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
    context->control_output = context->input.control_input;
}

/*!
    @brief Apply basic gyro stabilization to output
    @param context Stabilization context
*/
void gyro_stabilize(struct Stabilization_Context *context) {
    /* Rotate the gyro vector to account for the device orientation in the
        airframe */
    struct Vector3 w_rotated = matrix_rotate_vector(
        context->input.w,
        context->config.device_orientation.r_matrix
    );

    /* Calculate control adjustments */
    float roll_adj = -(context->config.gains.roll * w_rotated.x);
    float pitch_adj = -(context->config.gains.pitch * w_rotated.y);
    float yaw_adj = -(context->config.gains.yaw * w_rotated.z);

    /* Reverse correction if specified */
    roll_adj = (context->config.reverse.roll ? -roll_adj : roll_adj);
    pitch_adj = (context->config.reverse.pitch ? -pitch_adj : pitch_adj);
    yaw_adj = (context->config.reverse.yaw ? -yaw_adj : yaw_adj);

    /* Scale adjustments based on control input. This makes the gyro correction
        weaker when the control input is greater. */
    roll_adj *= (1.0f - ABS(context->input.control_input.roll));
    pitch_adj *= (1.0f - ABS(context->input.control_input.pitch));
    yaw_adj *= (1.0f - ABS(context->input.control_input.yaw));

    /* Scale all adjustments by the global gain */
    roll_adj *= context->input.global_gain;
    pitch_adj *= context->input.global_gain;
    yaw_adj *= context->input.global_gain;

    /* Combine control input and control adjustments */
    float roll_out = context->input.control_input.roll + roll_adj;
    float pitch_out = context->input.control_input.pitch + pitch_adj;
    float yaw_out = context->input.control_input.yaw + yaw_adj;

    /* Apply stabilized control to control output */
    context->control_output.roll = CLAMP(roll_out);
    context->control_output.pitch = CLAMP(pitch_out);
    context->control_output.yaw = CLAMP(yaw_out);
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