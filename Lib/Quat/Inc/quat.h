/*!
    @file:	quat.h
    @brief:	Implementation of a quaternion object and associated arithmetic
    operation functions
*/
#ifndef QUAT_H
#define QUAT_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
-- QUATERNION STRUCT --
***********************************************************************/

struct Quaternion {
    float w;
    float x;
    float y;
    float z;
};

/***********************************************************************
-- FUNCTIONS --
***********************************************************************/

void quat_norm(struct Quaternion *quat);

#ifdef __cplusplus
}
#endif

#endif