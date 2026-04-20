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
#define FIFO_READ_LEN 16

/* Expected WHO_AM_I register value for ICM-42605 */
#define WHO_AM_I_VAL 0x42

/***********************************************************************
-- PRIVATE FUNCTIONS --
***********************************************************************/

/*!
    @brief Read 1 or more bytes from an IMU register
    @param addr Register address
    @param rx_buf Array that the data will be written into
    @param num_bytes Number of bytes to read from the register
    @param error Error bits from DMA
    @details Most IMU registers are only a single byte, but the FIFO register
    can be many bytes deep. Any read operation will read num_bytes from the
    specified address without incrementing the address.
*/
void imu_read(
    uint8_t addr,
    uint8_t rx_buf[],
    uint32_t num_bytes,
    uint8_t *error
) {
    uint32_t transaction_len = num_bytes + 1;
    uint8_t tx_buf_dma[transaction_len];
    volatile uint8_t rx_buf_dma[transaction_len];
    tx_buf_dma[0] = addr | READ_OP;
    spi1_transact_data(
        tx_buf_dma,
        rx_buf_dma,
        transaction_len,
        error
    );

    /* First byte of the received data is just 0x00 */
    for (int i = 0; i < num_bytes; i++) {
        rx_buf[i] = rx_buf_dma[i + 1];
    }
}

/***********************************************************************
-- PUBLIC FUNCTIONS --
***********************************************************************/

/*!
    @brief Initialize the IMU
    @returns EXIT_SUCCESS if IMU initialized, otherwise EXIT_FAILURE
*/
int imu_init(uint8_t *error) {
    spi1_init(SPI_CPOL, SPI_CPHA, SPI_BR_10MHz);

    /* Verify SPI communication and sensor identity */
    uint8_t reg_data;
    imu_read_byte(WHO_AM_I_REG, &reg_data, error);
    if (reg_data != WHO_AM_I_VAL) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*!
    @brief Write a byte to a register in the IMU
    @param addr Register address
    @param data Data to be written
    @param error Pointer to DMA error bits
*/
void imu_write_byte(
    uint8_t addr,
    uint8_t data,
    uint8_t *error
) {
    uint8_t tx_buf[ACTUAL_REG_WRITE_LEN] = {addr, data};
    volatile uint8_t rx_buf[ACTUAL_REG_WRITE_LEN];
    spi1_transact_data(
        tx_buf,
        rx_buf,
        ACTUAL_REG_WRITE_LEN,
        error
    );
}

/*!
    @brief Read a single byte from a register in the IMU
    @param addr Register address to read from
    @param data Pointer to data that is read back
    @param error Pointer to DMA error bits
    @details A single byte transaction can be treated as a multi-byte
    transaction with a length of 1 so that's what this function does
*/
void imu_read_byte(
    uint8_t addr,
    uint8_t *data,
    uint8_t *error
) {
    imu_read(addr, data, 1, error);
}

/*!
    @brief Read multiple bytes from a register in the IMU
    @param addr Register address to read from
    @param data Array of data that is read back
    @param num_bytes Number of bytes to read
    @param error Pointer to DMA error bits
*/
void imu_read_bytes(
    uint8_t addr,
    uint8_t data[],
    uint32_t num_bytes,
    uint8_t *error
) {
    imu_read(addr, data, num_bytes, error);
}