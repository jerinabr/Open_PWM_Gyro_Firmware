#include "quat.h"
#include <math.h>

/*!
    @brief Normalise a quaternion
    @param quat Pointer to a Quaternion struct
*/
void quat_norm(struct Quaternion *quat) {
    /* Calculate quaternion magnitude */
    float mag = sqrtf(
        quat->w * quat->w +
        quat->x * quat->x +
        quat->y * quat->y +
        quat->z * quat->z
    );

    /* Normalise */
    quat->w /= mag;
    quat->x /= mag;
    quat->y /= mag;
    quat->z /= mag;
}