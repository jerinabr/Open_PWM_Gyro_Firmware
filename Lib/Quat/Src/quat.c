#include "quat.h"
#include <math.h>

/*!
    @brief Normalize a quaternion. Operation: q = q / |q|
    @param q Pointer to a quaternion that will be normalized. After the function
    call this quaternion will contain the normalized quaternion.
*/
void quat_norm(struct Quaternion *q) {
    /* Calculate quaternion magnitude */
    float mag = sqrtf(
        q->w * q->w +
        q->x * q->x +
        q->y * q->y +
        q->z * q->z
    );

    /* Normalize */
    q->w /= mag;
    q->x /= mag;
    q->y /= mag;
    q->z /= mag;
}

/*!
    @brief Multiply two quaternions. Operation: q0 = q1 * q0
    @param q0 Pointer to quaternion that is being multiplied. After the function
    call this quaternion will contain the result of the multiplication.
    @param q1 Pointer to quaternion that is multiplying
*/
void quat_mult(struct Quaternion *q0, struct Quaternion *q1) {
    /* Calculate q1 * q0 */
    float w = q1->w * q0->w - q1->x * q0->x - q1->y * q0->y - q1->z * q0->z;
    float x = q1->w * q0->x + q1->x * q0->w + q1->y * q0->z - q1->z * q0->y;
    float y = q1->w * q0->y - q1->x * q0->z + q1->y * q0->w + q1->z * q0->x;
    float z = q1->w * q0->z + q1->x * q0->y - q1->y * q0->x + q1->z * q0->w;

    /* Assign result of q1 * q0 to q0 */
    q0->w = w;
    q0->x = x;
    q0->y = y;
    q0->z = z;
}