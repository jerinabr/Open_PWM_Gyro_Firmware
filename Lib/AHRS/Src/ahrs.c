#include "ahrs.h"
#include "math3d.h"
#include <math.h>

#define LEARNING_RATE 0.1

/*!
    @brief Use the madgwick filter to compute the quaternion orientation of the
    IMU
    @param ax X component of the acceleration data (g)
    @param ay Y component of the acceleration data (g)
    @param az Z component of the acceleration data (g)
    @param gx X component of the gyroscope data (rad/s)
    @param gy Y component of the gyroscope data (rad/s)
    @param gz Z component of the gyroscope data (rad/s)
    @param t_delta Time since last filter update (seconds)
    @param quat Pointer to struct of the current quaternion representation
    @details This code is the C implementation of Sebastian Madgwick's sensor
    fusion algorithm he introduced in his 2010 paper, "An efficient orientation
    filter for inertial and inertial/magnetic sensor arrays."
    
    This code is taken from section A of his paper. I've cleaned it up a tad but
    most of the original code is present.

    The link to the paper can be found here:
    https://web.enib.fr/~kerhoas/iot/reseau-de-capteurs/carte-imu-mpu9250/documents/INVENSENSE/madgwick_internal_report.pdf

    Floating-point math is used here instead of doubles or fixed point because
    the STM32G431 has a single-precision FPU.
*/
void madgwick_imu(
    float ax, float ay, float az,
    float gx, float gy, float gz,
    float deltat,
    struct Quaternion *quat
) {
    /* Vector norm */
    float norm;
    /* Quaternion derivative from gyroscope data */
    float SEqDot_omega_1, SEqDot_omega_2, SEqDot_omega_3, SEqDot_omega_4;
    /* Objective function elements */
    float f_1, f_2, f_3;
    /* Objective function Jacobian elements */
    float J_11or24, J_12or23, J_13or22, J_14or21, J_32, J_33;
    /* Estimated direction of the gyroscope error */
    float SEqHatDot_1, SEqHatDot_2, SEqHatDot_3, SEqHatDot_4;

    /* Auxiliary variables to avoid repeated calculations */
    float halfSEq_1 = 0.5f * quat->w;
    float halfSEq_2 = 0.5f * quat->x;
    float halfSEq_3 = 0.5f * quat->y;
    float halfSEq_4 = 0.5f * quat->z;
    float twoSEq_1 = 2.0f * quat->w;
    float twoSEq_2 = 2.0f * quat->x;
    float twoSEq_3 = 2.0f * quat->y;

    /* Normalise the accelerometer data */
    norm = sqrtf(ax * ax + ay * ay + az * az);
    ax /= norm;
    ay /= norm;
    az /= norm;

    /*
        Compute the objective function and Jacobian

        J_11, J_13, J32, and J_33 are negated during the matrix multiplication
    */
    f_1 = twoSEq_2 * quat->z - twoSEq_1 * quat->y - ax;
    f_2 = twoSEq_1 * quat->x + twoSEq_3 * quat->z - ay;
    f_3 = 1.0f - twoSEq_2 * quat->x - twoSEq_3 * quat->y - az;
    J_11or24 = twoSEq_3;
    J_12or23 = 2.0f * quat->z;
    J_13or22 = twoSEq_1;
    J_14or21 = twoSEq_2;
    J_32 = 2.0f * J_14or21;
    J_33 = 2.0f * J_11or24;

    /* Compute the gradient (matrix multiplication) */
    SEqHatDot_1 = J_14or21 * f_2 - J_11or24 * f_1;
    SEqHatDot_2 = J_12or23 * f_1 + J_13or22 * f_2 - J_32 * f_3;
    SEqHatDot_3 = J_12or23 * f_2 - J_33 * f_3 - J_13or22 * f_1;
    SEqHatDot_4 = J_14or21 * f_1 + J_11or24 * f_2;

    /* Normalise the gradient */
    norm = sqrtf(
        SEqHatDot_1 * SEqHatDot_1 +
        SEqHatDot_2 * SEqHatDot_2 +
        SEqHatDot_3 * SEqHatDot_3 +
        SEqHatDot_4 * SEqHatDot_4
    );
    SEqHatDot_1 /= norm;
    SEqHatDot_2 /= norm;
    SEqHatDot_3 /= norm;
    SEqHatDot_4 /= norm;

    /* Compute the quaternion derivative measured by the gyroscope */
    SEqDot_omega_1 = -halfSEq_2 * gx - halfSEq_3 * gy - halfSEq_4 * gz;
    SEqDot_omega_2 = halfSEq_1 * gx + halfSEq_3 * gz - halfSEq_4 * gy;
    SEqDot_omega_3 = halfSEq_1 * gy - halfSEq_2 * gz + halfSEq_4 * gx;
    SEqDot_omega_4 = halfSEq_1 * gz + halfSEq_2 * gy - halfSEq_3 * gx;

    /* Compute then integrate the estimated quaternion derivative */
    quat->w += (SEqDot_omega_1 - (LEARNING_RATE * SEqHatDot_1)) * deltat;
    quat->x += (SEqDot_omega_2 - (LEARNING_RATE * SEqHatDot_2)) * deltat;
    quat->y += (SEqDot_omega_3 - (LEARNING_RATE * SEqHatDot_3)) * deltat;
    quat->z += (SEqDot_omega_4 - (LEARNING_RATE * SEqHatDot_4)) * deltat;

    /* Normalise quaternion */
    *quat = quat_norm(*quat);
}