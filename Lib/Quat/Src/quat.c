#include "quat.h"
#include <math.h>

/*!
    @brief Normalise a quaternion
    @param quat Pointer to a Quaternion struct
*/
void quat_norm(struct Quaternion *quat) {
    /* Calculate quaternion magnitude */
    float norm = sqrtf(
        quat->w * quat->w +
        quat->x * quat->x +
        quat->y * quat->y +
        quat->z * quat->z
    );

    /* Normalise */
    quat->w /= norm;
    quat->x /= norm;
    quat->y /= norm;
    quat->z /= norm;
}