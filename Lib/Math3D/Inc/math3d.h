/*!
    @file:	math3d.h
    @brief:	Implementation of a quaternion object, 3D vector object, and
    associated arithmetic functions
*/
#ifndef MATH3D_H
#define MATH3D_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
-- STRUCTS --
***********************************************************************/

struct Quaternion {
    float w;
    float x;
    float y;
    float z;
};

struct Vector3 {
    float x;
    float y;
    float z;
};

/***********************************************************************
-- FUNCTIONS --
***********************************************************************/

struct Quaternion quat_norm(struct Quaternion q0);
struct Quaternion quat_conj(struct Quaternion q0);
struct Quaternion quat_inv(struct Quaternion q0);
struct Quaternion quat_mult(struct Quaternion q0, struct Quaternion q1);

float vector_dot(struct Vector3 v0, struct Vector3 v1);
struct Vector3 vector_cross(struct Vector3 v0, struct Vector3 v1);

struct Vector3 rotate_vector(struct Quaternion q0, struct Vector3 v0);

#ifdef __cplusplus
}
#endif

#endif