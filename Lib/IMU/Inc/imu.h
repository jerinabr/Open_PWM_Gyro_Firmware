/*!
    @file   imu.h
    @brief  Initialize and read the IMU (ICM-42605) when data is available
*/
#ifndef IMU_H
#define IMU_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
-- DEFINITIONS --
***********************************************************************/

/*
    Registers
*/
#define WHO_AM_I_REG    0x75

/***********************************************************************
-- FUNCTIONS --
***********************************************************************/

int imu_init(void);
void imu_write(uint8_t addr, uint8_t data, uint8_t *error);
void imu_read(uint8_t addr, volatile uint8_t rx_buf[], uint32_t num_bytes, uint8_t *error);

#ifdef __cplusplus
}
#endif

#endif