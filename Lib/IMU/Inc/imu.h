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

/* Registers */
#define DEVICE_CONFIG_REG       0x11
#define DRIVE_CONFIG_REG        0x13
#define INT_CONFIG_REG          0x14
#define FIFO_CONFIG_REG         0x16
#define INT_STATUS_REG          0x2D
#define FIFO_COUNTH_REG         0x2E
#define FIFO_COUNTL_REG         0x2F
#define FIFO_DATA_REG           0x30
#define SIGNAL_PATH_RESET_REG   0x4B
#define PWR_MGMT0_REG           0x4E
#define GYRO_CONFIG0_REG        0x4F
#define ACCEL_CONFIG0_REG       0x50
#define FIFO_CONFIG1_REG        0x5F
#define FIFO_CONFIG2_REG        0x60
#define FIFO_CONFIG3_REG        0x61
#define INT_CONFIG0_REG         0x63
#define INT_CONFIG1_REG         0x64
#define INT_SOURCE0_REG         0x65
#define WHO_AM_I_REG            0x75
#define REG_BANK_SEL_REG        0x76

/* Register Masks */
#define DEVICE_CONFIG_MASK      0x11
#define DRIVE_CONFIG_MASK       0x3F
#define INT_CONFIG_MASK         0x3F
#define FIFO_CONFIG_MASK        0xC0
#define FIFO_CONFIG1_MASK       0x6F
#define FIFO_CONFIG2_MASK       0xFF
#define INT_CONFIG0_MASK        0x3F
#define INT_CONFIG1_MASK        0x30
#define INT_SOURCE0_MASK        0x7F

/* ODR Configurations */
#define IMU_ODR_8kHz            0b0011
#define IMU_ODR_4kHz            0b0100
#define IMU_ODR_2kHz            0b0101
#define IMU_ODR_1kHz            0b0110
#define IMU_ODR_500Hz           0b1111
#define IMU_ODR_200Hz           0b0111
#define IMU_ODR_100Hz           0b1000
#define IMU_ODR_50Hz            0b1001
#define IMU_ODR_25Hz            0b1010
#define IMU_ODR_12p5Hz          0b1011

/* Gyro FSR Configuration */
#define GYRO_FSR_2000dps        0b000
#define GYRO_FSR_1000dps        0b001
#define GYRO_FSR_500dps         0b010
#define GYRO_FSR_250dps         0b011
#define GYRO_FSR_125dps         0b100
#define GYRO_FSR_62p5dps        0b101
#define GYRO_FSR_31p25dps       0b110
#define GYRO_FSR_15p625dps      0b111

/* Accel FSR Configuration */
#define ACCEL_FSR_16g           0b000
#define ACCEL_FSR_8g            0b001
#define ACCEL_FSR_4g            0b010
#define ACCEL_FSR_2g            0b011

/***********************************************************************
-- FUNCTIONS --
***********************************************************************/

/* Initialize the ICM-42605 IMU */
int imu_init(void);

/* Single byte register write */
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