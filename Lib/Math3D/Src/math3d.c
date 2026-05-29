#include "math3d.h"
#include <math.h>

/*!
    @brief Normalize a quaternion. Operation: q1 = q0 / |q0|
    @param q0 Input quaternion
    @return Normalized q0
*/
struct Quaternion quat_norm(struct Quaternion q0) {
    /* Calculate quaternion magnitude */
    float mag = sqrtf(q0.w * q0.w + q0.x * q0.x + q0.y * q0.y + q0.z * q0.z);

    /* Normalize */
    q0.w /= mag;
    q0.x /= mag;
    q0.y /= mag;
    q0.z /= mag;

    return q0;
}

/*!
    @brief Calculate the conjugate of a quaternion
    @param q0 Input quaternion
    @return Conjugate of q0
*/
struct Quaternion quat_conj(struct Quaternion q0) {
    q0.x = -q0.x;
    q0.y = -q0.y;
    q0.z = -q0.z;
    return q0;
}

/*!
    @brief Calculate the inverse of a quaternion. Operation: q1 = q0* / |q0|^2
    @param q0 Input quaternion
    @return Inverse of q0
*/
struct Quaternion quat_inv(struct Quaternion q0) {
    /* Calculate conjugate of q0 */
    q0 = quat_conj(q0);
    
    /* Calculate magnitude squared of q0* */
    float mag_squared = q0.w * q0.w + q0.x * q0.x + q0.y * q0.y + q0.z * q0.z;

    /* Conjugate gets divided by magnitude squared for the inverse */
    q0.w /= mag_squared;
    q0.x /= mag_squared;
    q0.y /= mag_squared;
    q0.z /= mag_squared;

    return q0;
}

/*!
    @brief Multiply two quaternions. Operation: q2 = q0 * q1
    @param q0 Quaternion that is being multiplied
    @param q1 Quaternion that is multiplying
    @return Product of q1 and q0
*/
struct Quaternion quat_mult(struct Quaternion q0, struct Quaternion q1) {
    /* Calculate q0 * q1 */
    float w = q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z;
    float x = q0.w * q1.x + q0.x * q1.w + q0.y * q1.z - q0.z * q1.y;
    float y = q0.w * q1.y - q0.x * q1.z + q0.y * q1.w + q0.z * q1.x;
    float z = q0.w * q1.z + q0.x * q1.y - q0.y * q1.x + q0.z * q1.w;

    /* Assign result of q0 * q1 to q0 */
    q0.w = w;
    q0.x = x;
    q0.y = y;
    q0.z = z;

    return q0;
}

/*!
    @brief Calculate the dot product of 2 vectors
    @param v0 Vector 0
    @param v1 Vector 1
    @return Dot product of v0 and v1 (v0 . v1)
*/
float vector_dot(struct Vector3 v0, struct Vector3 v1) {
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

/*!
    @brief Calculate the cross product of 2 vectors
    @param v0 Vector 0
    @param v1 Vector 1
    @return Cross product of v0 and v1 (v0 x v1)
*/
struct Vector3 vector_cross(struct Vector3 v0, struct Vector3 v1) {
    struct Vector3 cross = {
        .x = v0.y * v1.z - v0.z * v1.y,
        .y = v0.z * v1.x - v0.x * v1.z,
        .z = v0.x * v1.y - v0.y * v1.x
    };
    return cross;
}

/*!
    @brief Rotate a vector3 by q0. Operation: v1 = q0 * v0 * q0*
    @param q0 Unit quaternion that will apply the rotation
    @param v0 Vector3 that will be rotated
    @return Rotated vector3
    @details The optimized math used here was taken from this blog:
    https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
*/
struct Vector3 rotate_vector(struct Quaternion q0, struct Vector3 v0) {
    /* Extract vector component of q0 */
    struct Vector3 q_vec = {
        .x = q0.x,
        .y = q0.y,
        .z = q0.z
    };

    /* Calculate cross product of quaternion vector and v0 */
    struct Vector3 t = vector_cross(q_vec, v0);
    t.x *= 2;
    t.y *= 2;
    t.z *= 2;

    /* Calculate cross product of quaternion vector and t */
    struct Vector3 t2 = vector_cross(q_vec, t);

    /* Construct rotated vector */
    struct Vector3 v_prime = {
        .x = v0.x + q0.w * t.x + t2.x,
        .y = v0.y + q0.w * t.y + t2.y,
        .z = v0.z + q0.w * t.z + t2.z
    };
    return v_prime;
}