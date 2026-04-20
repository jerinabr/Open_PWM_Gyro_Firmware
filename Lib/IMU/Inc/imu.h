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

void imu_write_byte(
    uint8_t addr,
    uint8_t data
);

/* Read functions for single or multi-byte reads */
void imu_read_byte(
    uint8_t addr,
    uint8_t *data
);

void imu_read_bytes(
    uint8_t addr,
    uint8_t data[],
    uint32_t num_bytes
);

#ifdef __cplusplus
}
#endif

#endif