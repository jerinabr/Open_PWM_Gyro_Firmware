/*!
    @file   imu_exti.h
    @brief  Configure the IMU INT1 pin input to indicate when there's an IMU
    interrupt flag

    The EXTI is used to trigger an interrupt when the falling-edge of the IMU
    INT1 pin is detected.

    The IMU INT1 pin is connected to pin PB0 on the MCU.
*/
#ifndef IMU_EXTI_H
#define IMU_EXTI_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
-- FUNCTIONS --
***********************************************************************/

/* Enable the EXTI interrupt for the IMU INT1 pin */
void imu_exti_init(void);

/* Get and clear the IMU interrupt status */
uint8_t is_int1_active(void);

#ifdef __cplusplus
}
#endif

#endif