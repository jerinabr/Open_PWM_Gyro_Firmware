#include "imu.h"
#include "spi1.h"
#include <stdint.h>
#include <stdlib.h>

/* SPI configuration */
#define SPI_CPOL 0x1
#define SPI_CPHA 0x1
#define SPI_BR_10MHz 0x3 /* I'll abstract this later lol */
#define SPI_BR_625kHz 0x7

/* Read operation is signified by bit 7 of the address byte being set */
#define READ_OP 0x80

/* IMU data FIFO is 16 bytes per packet */
#define FIFO_READ_LEN 16

/* Expected WHO_AM_I register value for ICM-42605 */
#define WHO_AM_I_VAL 0x42

/***********************************************************************
-- PRIVATE FUNCTIONS --
***********************************************************************/

/*!
    @brief Read 1 or more bytes from an IMU register
    @param addr Register address
    @param data Array that the data will be written into
    @param num_bytes Number of bytes to read from the register
    @details Most IMU registers are only a single byte, but the FIFO register
    can be many bytes deep. Any read operation will read num_bytes from the
    specified address without incrementing the address.
*/
static inline void imu_read(
    uint8_t addr,
    uint8_t data[],
    uint32_t num_bytes
) {
    /* Transaction length is 1 byte of address + num_bytes */
    uint32_t transaction_len = num_bytes + 1;

    uint8_t tx_buf[transaction_len];
    volatile uint8_t rx_buf[transaction_len];
    tx_buf[0] = addr | READ_OP;
    spi1_transact_data(
        tx_buf,
        rx_buf,
        transaction_len
    );

    /*
        Ignore the first received byte because it's received when the address
        byte is sent
    */
    for (int i = 0; i < num_bytes; i++) {
        data[i] = rx_buf[i + 1];
    }
}

/***********************************************************************
-- PUBLIC FUNCTIONS --
***********************************************************************/

/*!
    @brief Initialize the IMU
    @returns EXIT_SUCCESS if IMU initialized, otherwise EXIT_FAILURE
*/
int imu_init(void) {
    spi1_init(SPI_CPOL, SPI_CPHA, SPI_BR_10MHz);

    /* Verify SPI communication and sensor identity */
    uint8_t reg_data;
    imu_read_byte(WHO_AM_I_REG, &reg_data);
    if (reg_data != WHO_AM_I_VAL) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*!
    @brief Write a byte to a register in the IMU
    @param addr Register address
    @param data Data to be written
*/
void imu_write_byte(
    uint8_t addr,
    uint8_t data
) {
    uint8_t tx_buf[2] = {addr, data};
    volatile uint8_t rx_buf[2];
    spi1_transact_data(
        tx_buf,
        rx_buf,
        2
    );
}

/*!
    @brief Read a single byte from a register in the IMU
    @param addr Register address to read from
    @param data Pointer to data that is read back
    @details A single byte transaction can be treated as a multi-byte
    transaction with a length of 1 so that's what this function does
*/
void imu_read_byte(
    uint8_t addr,
    uint8_t *data
) {
    imu_read(addr, data, 1);
}

/*!
    @brief Read multiple bytes from a register in the IMU
    @param addr Register address to read from
    @param data Array of data that is read back
    @param num_bytes Number of bytes to read
*/
void imu_read_bytes(
    uint8_t addr,
    uint8_t data[],
    uint32_t num_bytes
) {
    imu_read(addr, data, num_bytes);
}