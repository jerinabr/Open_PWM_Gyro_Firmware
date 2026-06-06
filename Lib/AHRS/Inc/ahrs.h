/*!
    @file:	ahrs.h
    @brief:	Sensor fusion algorithm for IMU data

    At this time, the only algorithm present in here is the Madgwick filter. The
    C implementation of this was taken directly from his 2010 paper "An
    efficient orientation filter for inertial and inertial/magnetic sensor
    arrays."
*/
#ifndef AHRS_H
#define AHRS_H

#include "math3d.h"
#ifdef __cplusplus
extern "C" {
#endif

void madgwick_imu(
    float ax, float ay, float az,
    float gx, float gy, float gz,
    float t_delta,
    struct Quaternion *quat
);

#ifdef __cplusplus
}
#endif

#endif