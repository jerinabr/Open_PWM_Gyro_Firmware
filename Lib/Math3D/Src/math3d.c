#include "math3d.h"
#include <math.h>

const float DEG_TO_RAD = 3.141592653589793f / 180.0f;

/***********************************************************************
-- QUATERNION FUNCTIONS --
***********************************************************************/

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
    @param q0 Quaternion on the left of the multiplication
    @param q1 Quaternion on the right of the multiplication
    @return Product of q0 and q1
    @details Quaternion multiplication is NOT commutative so be careful about
    assigning q0 and q1
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

/***********************************************************************
-- VECTOR3 FUNCTIONS --
***********************************************************************/

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

/***********************************************************************
-- ROTATION MATRIX FUNCTIONS --
***********************************************************************/

/*!
    @brief Convert Euler angles to a 3x3 rotation matrix
    @param wx Rotation angle around world-space X-axis
    @param wy Rotation angle around world-space Y-axis
    @param wz Rotation angle around world-space Z-axis
    @param r0 3x3 array of floats that will contain the calculated rotation
    matrix
    @details The conversion done here is in the order XYZ extrinsic because that
    is what's typically used for aerospace. This is the same as ZYX intrinsic.

    This is quite an expensive operation and only really recommended for
    precomputing rotation matrices to be applied at runtime.
*/
void euler_to_rot_matrix(float wx, float wy, float wz, float r0[3][3]) {
    /* Convert degrees to radians for trig functions */
    float rad_x = wx * DEG_TO_RAD;
    float rad_y = wy * DEG_TO_RAD;
    float rad_z = wz * DEG_TO_RAD;

    /* Precompute trig values */
    float sx = sinf(rad_x);
    float sy = sinf(rad_y);
    float sz = sinf(rad_z);
    float cx = cosf(rad_x);
    float cy = cosf(rad_y);
    float cz = cosf(rad_z);

    /* Update rotation matrix */
    r0[0][0] = cz * cy;
    r0[0][1] = cz * sy * sx - cx * sz;
    r0[0][2] = sz * sx + cz * cx * sy;
    r0[1][0] = cy * sz;
    r0[1][1] = cz * cx + sz * sy * sx;
    r0[1][2] = cx * sz * sy - cz * sx;
    r0[2][0] = -sy;
    r0[2][1] = cy * sx;
    r0[2][2] = cy * cx;
}

/***********************************************************************
-- ROTATION FUNCTIONS --
***********************************************************************/

/*!
    @brief Rotate a vector3 by q0. Operation: v1 = q0 * v0 * q0*
    @param q0 Unit quaternion that will apply the rotation
    @param v0 Vector3 that will be rotated
    @return Rotated vector3
    @details The optimized math used here was taken from this blog:
    https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
*/
struct Vector3 quat_rotate_vector(struct Quaternion q0, struct Vector3 v0) {
    /* Extract vector component of q0 */
    struct Vector3 q_vec = {
        .x = q0.x,
        .y = q0.y,
        .z = q0.z
    };

    /* Calculate cross product of quaternion vector and v0 */
    struct Vector3 t = vector_cross(q_vec, v0);
    t.x *= 2.0f;
    t.y *= 2.0f;
    t.z *= 2.0f;

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

/*!
    @brief Apply a rotation matrix to a vector
    @param r0 3x3 rotation matrix
    @param v0 Vector to be rotated
    @return Rotated vector3
    @details Rotating a vector by a rotation matrix uses only 9 multiplications
    and 6 additions compared to the 18 multiplications and 6 additions needed
    for the optimized quaternion rotation.

    It's preferable to use a rotation matrix if the rotation being applied is
    precomputed otherwise converting from a quaternion or euler angles to a
    rotation matrix is quite expensive.
*/
struct Vector3 matrix_rotate_vector(float r0[3][3], struct Vector3 v0) {
    struct Vector3 result = {
        .x = r0[0][0] * v0.x + r0[0][1] * v0.y + r0[0][2] * v0.z,
        .y = r0[1][0] * v0.x + r0[1][1] * v0.y + r0[1][2] * v0.z,
        .z = r0[2][0] * v0.x + r0[2][1] * v0.y + r0[2][2] * v0.z
    };
    return result;
}