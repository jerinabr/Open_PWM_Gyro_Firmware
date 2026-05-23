/*!
    @file   imu.h
    @brief  Initialize and read the IMU (ICM-42605) when data is available

    The IMU is configured in Stream-to-FIFO mode so that way all the sensor data
    can be read from a single register instead of needing to do twelve separate
    SPI transactions.

    The INT1 pin is connected to PB0 on the MCU, and it will pulse active-low
    when the FIFO has more than 1 packet in it. This will keep the SPI interface
    idle until the interrupt occurs. Once the interrupt occurs, the FIFO count
    register is read to determine how many packets to read from the FIFO.

    The IMU data from the FIFO is always in the range -32768 to 32767. The FSR
    determines what the LSB is.
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
#define IMU_REG_DEVICE_CONFIG       0x11
#define IMU_REG_DRIVE_CONFIG        0x13
#define IMU_REG_INT_CONFIG          0x14
#define IMU_REG_FIFO_CONFIG         0x16
#define IMU_REG_INT_STATUS          0x2D
#define IMU_REG_FIFO_COUNTH         0x2E
#define IMU_REG_FIFO_COUNTL         0x2F
#define IMU_REG_FIFO_DATA           0x30
#define IMU_REG_SIGNAL_PATH_RESET   0x4B
#define IMU_REG_PWR_MGMT0           0x4E
#define IMU_REG_GYRO_CONFIG0        0x4F
#define IMU_REG_ACCEL_CONFIG0       0x50
#define IMU_REG_TMST_CONFIG         0x54
#define IMU_REG_FIFO_CONFIG1        0x5F
#define IMU_REG_FIFO_CONFIG2        0x60
#define IMU_REG_FIFO_CONFIG3        0x61
#define IMU_REG_INT_CONFIG0         0x63
#define IMU_REG_INT_CONFIG1         0x64
#define IMU_REG_INT_SOURCE0         0x65
#define IMU_REG_WHO_AM_I            0x75
#define IMU_REG_REG_BANK_SEL        0x76

/* Register Masks */
#define IMU_MASK_DEVICE_CONFIG      0x11
#define IMU_MASK_DRIVE_CONFIG       0x3F
#define IMU_MASK_INT_CONFIG         0x3F
#define IMU_MASK_FIFO_CONFIG        0xC0
#define IMU_MASK_PWR_MGMT0          0x3F
#define IMU_MASK_TMST_CONFIG        0x3F
#define IMU_MASK_FIFO_CONFIG1       0x6F
#define IMU_MASK_FIFO_CONFIG2       0xFF
#define IMU_MASK_INT_CONFIG0        0x3F
#define IMU_MASK_INT_CONFIG1        0x70
#define IMU_MASK_INT_SOURCE0        0x7F

/* ODR Configurations */
#define IMU_ODR_8kHz        0b0011
#define IMU_ODR_4kHz        0b0100
#define IMU_ODR_2kHz        0b0101
#define IMU_ODR_1kHz        0b0110
#define IMU_ODR_500Hz       0b1111
#define IMU_ODR_200Hz       0b0111
#define IMU_ODR_100Hz       0b1000
#define IMU_ODR_50Hz        0b1001
#define IMU_ODR_25Hz        0b1010
#define IMU_ODR_12p5Hz      0b1011

/* Gyro FSR Configuration */
#define GYRO_FSR_2000dps    0b000
#define GYRO_FSR_1000dps    0b001
#define GYRO_FSR_500dps     0b010
#define GYRO_FSR_250dps     0b011
#define GYRO_FSR_125dps     0b100
#define GYRO_FSR_62p5dps    0b101
#define GYRO_FSR_31p25dps   0b110
#define GYRO_FSR_15p625dps  0b111

/* Accel FSR Configuration */
#define ACCEL_FSR_16g       0b000
#define ACCEL_FSR_8g        0b001
#define ACCEL_FSR_4g        0b010
#define ACCEL_FSR_2g        0b011

/* IMU FIFO packet size in bytes */
#define IMU_FIFO_PACKET_LEN 16

/***********************************************************************
-- IMU DATA STRUCTURE --
***********************************************************************/

/*!
    @brief Accel, Gyro, and timestamp delta from IMU
    @details acceleration data is in g's and gyroscope data is in deg/s.
    ts_delta is the number of seconds elapsed since the last sample.
*/
struct IMU_Data {
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    float ts_delta;
};

/***********************************************************************
-- FUNCTIONS --
***********************************************************************/

/* Initialize the ICM-42605 IMU without enabling it */
int imu_init(void);

/* Enable or disable the ICM-42605 gyro and accelerometer */
int imu_enable(void);
int imu_disable(void);

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

/* IMU data processing loop */
uint8_t read_imu_data(struct IMU_Data *imu_data);

#ifdef __cplusplus
}
#endif

#endif