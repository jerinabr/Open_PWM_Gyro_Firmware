#include "imu.h"
#include "spi1.h"
#include "stm32g4xx_hal.h"
#include <stdint.h>
#include <stdlib.h>

/* SPI configuration */
#define SPI_CPOL 0x1
#define SPI_CPHA 0x1
#define SPI_BR_10MHz 0x3 /* I'll abstract this later lol */
#define SPI_BR_625kHz 0x7

/* Read operation is signified by bit 7 of the address byte being set */
#define READ_OP 0x80

/* Write operations are only 2 bytes for now (address and data byte) */
#define ACTUAL_REG_WRITE_LEN 2
#define ACTUAL_REG_READ_LEN 2
#define REG_READ_LEN 1
#define FIFO_READ_LEN 16

/* Expected WHO_AM_I register value for ICM-42605 */
#define WHO_AM_I_VAL 0x42

/***********************************************************************
-- PRIVATE FUNCTIONS --
***********************************************************************/

/***********************************************************************
-- PUBLIC FUNCTIONS --
***********************************************************************/

/*!
    @brief Initialize the IMU
    @returns EXIT_SUCCESS if IMU initialized, otherwise EXIT_FAILURE
*/
int imu_init(void) {
    spi1_init(SPI_CPOL, SPI_CPHA, SPI_BR_625kHz);

    /* Verify SPI communication and sensor identity */
    // uint8_t rx_buf[8];
    // uint8_t tx_buf[2] = {0x80 | 0x75, 0xFF};
    // uint8_t rx_buf[2];
    // HAL_Delay(7000);
    // spi1_transact_data(tx_buf, rx_buf, 2);
    // // imu_read(WHO_AM_I_REG, rx_buf, 1);
    // if (rx_buf[1] != WHO_AM_I_VAL) {
    //     return EXIT_FAILURE;
    // }
    return EXIT_SUCCESS;
}

/*!
    @brief Write a byte to an IMU register
    @param addr Register address
    @param data Data to be written
*/
void imu_write(uint8_t addr, uint8_t data, uint8_t *error) {
    uint8_t rx_buf[ACTUAL_REG_WRITE_LEN];
    uint8_t tx_buf[ACTUAL_REG_WRITE_LEN] = {addr, data};
    spi1_transact_data(tx_buf, rx_buf, ACTUAL_REG_WRITE_LEN, error);
}

/*!
    @brief Read 1 or more bytes from an IMU register
    @param addr Register address
    @param rx_buf Array that the data will be written into
    @param num_bytes Number of bytes to read from the register
    @details Most IMU registers are only a single byte, but the FIFO register
    can be many bytes deep. Any read operation will read num_bytes from the
    specified address without incrementing the address.
*/
void imu_read(uint8_t addr, volatile uint8_t rx_buf[], uint32_t num_bytes, uint8_t *error) {
    uint32_t transaction_len = num_bytes+1;
    uint8_t tx_buf[transaction_len];
    tx_buf[0] = addr | READ_OP;
    for (int i = 1; i < transaction_len; i++) {
        tx_buf[i] = 0x00;
    }
    spi1_transact_data(tx_buf, rx_buf, transaction_len, error);
}