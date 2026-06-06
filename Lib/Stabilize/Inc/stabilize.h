/*!
    @file:	stabilize.h
    @brief:	Apply stabilization depending on the stabilization mode

    The stabilization modes that are supported are:
        - Off
            - no stabilization
        - Stabilize
            - Apply stabilization based purely on gyro readings
        - Hold 
            -Hold aircraft attitude on all axis that don't have control inputs
        - Level
            - Return aircraft to level flight when control inputs are released
        - Limit
            - Limit the maximum pitch and roll angles of the aircraft

    A context struct is used to pass all the information needed to the
    stabilization algorithms. This struct should be declared and modified in the
    application code.
*/
#ifndef STABILIZE_H
#define STABILIZE_H

#include "math3d.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
-- GYRO MODES --
***********************************************************************/

#define GYRO_MODE_OFF 0
#define GYRO_MODE_STABILIZE 1
#define GYRO_MODE_HOLD 2
#define GYRO_MODE_LEVEL 3
#define GYRO_MODE_LIMIT 4

/***********************************************************************
-- STABILIZATION CONTEXT OBJECTS --
***********************************************************************/

/*!
    @brief Attitude control representation
    @details Each control value is and should be normalized to be between -100
    and 100. Extended range could be up to -150 to 150 range.
*/
struct Attitude {
    float roll;
    float pitch;
    float yaw;
};

/*!
    @brief Stabilizer context object
    @details This struct should be declared in the application code and passed
    as a pointer to the stabilization functions
*/
struct Stabilization_Context {
    /* Stabilization configuration settings */
    struct {
        /* Normalized control input/output limits. Each member represents an
            absolute value for the control limit. Default should be 1 but could
            go as high as 1.5 depending on TX settings */
        struct Attitude control_limits;

        /* Angle limits (in radians) for the Limit stabilization mode */
        struct {
            float roll;
            float pitch;
        } ang_limits;

        /* Correction gains for stabilization. Each gain should be a value > 0.
            Individual axis gains can be configured and all gains will be
            multiplied by a global gain. */
        struct Attitude gains;

        /* Indicates if the correction direction for the control output needs to
            be reversed. 0 for no reversal, 1 for reversal */
        struct {
            uint8_t roll;
            uint8_t pitch;
            uint8_t yaw;
        } reverse;

        /* Device orientation with respect to the airframe represented as a
            Quaternion and a rotation matrix */
        struct {
            struct Quaternion q;
            float r_matrix[3][3];
        } device_orientation;
    } config;

    /* Stabilization input */
    struct {
        /* Device orientation as a normalized quaternion */
        struct Quaternion q;

        /* IMU gyro angular velocities (rad/s) */
        struct Vector3 w;

        /* Normalized control input (-1 to 1) */
        struct Attitude control_input;

        /* Global gain (0 to 1) */
        float global_gain;

        /* Stabilization mode */
        uint8_t mode;
    } input;

    /* Normalized stabilization output (-1 to 1) */
    struct Attitude control_output;
};

/***********************************************************************
-- FUNCTIONS --
***********************************************************************/

/* Internal stabilization functions */
void gyro_off(struct Stabilization_Context *context);
void gyro_stabilize(struct Stabilization_Context *context);

/* Main stabilization functions */
void set_stabilization_mode(struct Stabilization_Context *context);
void apply_stabilization(struct Stabilization_Context *context);

#ifdef __cplusplus
}
#endif

#endif