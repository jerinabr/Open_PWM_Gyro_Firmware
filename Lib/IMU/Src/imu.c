#include "imu.h"
#include "spi1.h"
#include "systick.h"
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

/* Register configurations */
#define SOFT_RESET              0x01
#define RESET_DONE_MASK         0x10
#define SLEW_RATE_4ns_12ns      0x03
#define INT1_DRIVE_PUSH_PULL    (0x01 << 1)
#define STREAM_TO_FIFO_MODE     (0x01 << 6)
#define FLUSH_FIFO              (0x01 << 1)
#define SEND_ALL_DATA_TO_FIFO   0x07
#define FIFO_WM_LSB             0x01
#define THS_INT_CLEAR_ON_READ   (0x02 << 2)
#define DEASSERT_INT_RESET      0x00
#define INT1_SRC_FIFO_THS       (0x01 << 2)
#define GYRO_ACCEL_LN_MODE      0x0F
#define GYRO_ACCEL_OFF          0x00

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
        Copy every received byte except the first one because it's garbage data
        that's sent when back when the address is written
    */
    for (int i = 0; i < num_bytes; i++) {
        data[i] = rx_buf[i + 1];
    }
}

/*!
    @brief Write then read back an IMU register to verify it was written
    correctly
    @param addr Register address
    @param data Data to be written
    @param rd_mask Bitmask for the register read (to be used when register bits
    are reserved)
    @return EXIT_SUCCESS if the data read matches the data written, EXIT_FAILURE
    otherwise
*/
static int imu_write_verify(uint8_t addr, uint8_t data, uint8_t rd_mask) {
    imu_write_byte(addr, data);
    uint8_t read_byte;
    imu_read_byte(addr, &read_byte);
    if ((read_byte & rd_mask) != data) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*!
    @brief Configure the IMU registers
    @return EXIT_SUCCESS if the IMU configured successfully, EXIT_FAILURE
    otherwise
*/
static int imu_config(void) {
    uint8_t wr_valid;
    uint8_t rd_data;

    /* Soft reset the IMU */
    imu_write_byte(DEVICE_CONFIG_REG, SOFT_RESET);

    /*
        Datasheet says to wait 1 ms for soft reset to be effective, but we'll
        wait 2 ms to be EXTRA sure
    */
    delay_ms(2);

    /* Verify software reset completed */
    imu_read_byte(INT_STATUS_REG, &rd_data);
    if (!(rd_data & RESET_DONE_MASK)) {
        return rd_data;
    }

    /* Set SPI output slew rate to 4ns - 12ns */
    wr_valid = imu_write_verify(
        DRIVE_CONFIG_REG,
        SLEW_RATE_4ns_12ns,
        DRIVE_CONFIG_MASK
    );
    if (wr_valid != EXIT_SUCCESS) {
        return 1;
    }

    /* Set INT1 output driver to be push-pull */
    wr_valid = imu_write_verify(
        INT_CONFIG_REG,
        INT1_DRIVE_PUSH_PULL,
        INT_CONFIG_MASK
    );
    if (wr_valid != EXIT_SUCCESS) {
        return 2;
    }
    
    /* Enable Stream-to-FIFO mode */
    wr_valid = imu_write_verify(
        FIFO_CONFIG_REG,
        STREAM_TO_FIFO_MODE,
        FIFO_CONFIG_MASK
    );
    if (wr_valid != EXIT_SUCCESS) {
        return 3;
    }

    /* Flush the FIFO in case there's any data in it */
    imu_write_byte(SIGNAL_PATH_RESET_REG, FLUSH_FIFO);

    /* Send accel, gyro, temp, and timestamp data to FIFO */
    wr_valid = imu_write_verify(
        FIFO_CONFIG1_REG,
        SEND_ALL_DATA_TO_FIFO,
        FIFO_CONFIG1_MASK
    );
    if (wr_valid != EXIT_SUCCESS) {
        return 4;
    }

    /* Set FIFO interrupt threshold to 1 packet */
    wr_valid = imu_write_verify(
        FIFO_CONFIG2_REG,
        FIFO_WM_LSB,
        FIFO_CONFIG2_MASK
    );
    if (wr_valid != EXIT_SUCCESS) {
        return 5;
    }

    /* Clear FIFO threshold interrupt when the FIFO is read */
    wr_valid = imu_write_verify(
        INT_CONFIG0_REG,
        THS_INT_CLEAR_ON_READ,
        INT_CONFIG0_MASK
    );
    if (wr_valid != EXIT_SUCCESS) {
        return 6;
    }

    /* Deassert async reset for the interrupt */
    wr_valid = imu_write_verify(
        INT_CONFIG1_REG,
        DEASSERT_INT_RESET,
        INT_CONFIG1_MASK
    );
    if (wr_valid != EXIT_SUCCESS) {
        return 7;
    }

    /* Set FIFO threshold as the interrupt source for INT1 */
    wr_valid = imu_write_verify(
        INT_SOURCE0_REG,
        INT1_SRC_FIFO_THS,
        INT_SOURCE0_MASK
    );
    if (wr_valid != EXIT_SUCCESS) {
        return 8;
    }

    return EXIT_SUCCESS;
}

/***********************************************************************
-- PUBLIC FUNCTIONS --
***********************************************************************/

/*!
    @brief Initialize the IMU
    @return EXIT_SUCCESS if IMU initialized, otherwise EXIT_FAILURE
*/
int imu_init(void) {
    spi1_init(SPI_CPOL, SPI_CPHA, SPI_BR_10MHz);

    /* Verify SPI communication and sensor identity */
    uint8_t reg_data;
    imu_read_byte(WHO_AM_I_REG, &reg_data);
    if (reg_data != WHO_AM_I_VAL) {
        return EXIT_FAILURE;
    }

    /* Configure the IMU */
    return imu_config();
}

/*!
    @brief Turn on the gyro and accelerometer in low noise mode
    @return EXIT_SUCCESS if the IMU was enabled, EXIT_FAILURE otherwise
*/
int imu_enable(void) {
    /* Flush the FIFO */
    imu_write_byte(SIGNAL_PATH_RESET_REG, FLUSH_FIFO);

    /* Place gyro and accelerometer in low noise mode */
    return imu_write_verify(
        PWR_MGMT0_REG,
        GYRO_ACCEL_LN_MODE,
        PWR_MGMT0_MASK
    );
}

/*!
    @brief Turn off the gyro and accelerometer
    @return EXIT_SUCCESS if the IMU was disable, EXIT_FAILURE otherwise
*/
int imu_disable(void) {
    return imu_write_verify(
        PWR_MGMT0_REG,
        GYRO_ACCEL_OFF,
        PWR_MGMT0_MASK
    );
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
    transaction with a length of 1 so that's what this function does.
    
    I know this function COULD just return a single byte but I wanted to keep it
    consistent with the imu_read_bytes function.
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