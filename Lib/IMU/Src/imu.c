#include "imu.h"
#include "imu_exti.h"
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

/* Expected WHO_AM_I register value for ICM-42605 */
#define WHO_AM_I_VAL 0x42

/* IMU FIFO packet size in bytes */
#define IMU_FIFO_PACKET_LEN 16

/* Register configurations */
#define SOFT_RESET              0x01
#define RESET_DONE_MASK         0x10
#define SLEW_RATE_4ns_12ns      0x03
#define INT1_DRIVE_PUSH_PULL    (0x01 << 1)
#define STREAM_TO_FIFO_MODE     (0x01 << 6)
#define FLUSH_FIFO              (0x01 << 1)
#define TMST_DELTA_EN           0x25
#define SEND_ALL_DATA_TO_FIFO   0x07
#define FIFO_WM_LSB             0x01
#define THS_INT_CLEAR_ON_READ   (0x02 << 2)
#define DEASSERT_INT_RESET      0x00
#define INT1_SRC_FIFO_THS       (0x01 << 2)
#define GYRO_ACCEL_LN_MODE      0x0F
#define GYRO_ACCEL_OFF          0x00

/*
    IMU data scaling constants 
    - 16g accel FSR
    - 2000 deg/s gyro FSR
*/
const float ACCEL_SCALE = 9.80665 / 2048.0;
const float GYRO_SCALE = (3.141592653589793 / 180.0) / 16.4;

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
    uint8_t config_step = 0;

    /* Soft reset the IMU */
    imu_write_byte(IMU_REG_DEVICE_CONFIG, SOFT_RESET);

    /*
        Datasheet says to wait 1 ms for soft reset to be effective, but we'll
        wait 2 ms to be EXTRA sure
    */
    delay_ms(2);

    /* Verify software reset completed */
    config_step++;
    imu_read_byte(IMU_REG_INT_STATUS, &rd_data);
    if (!(rd_data & RESET_DONE_MASK)) {
        return config_step;
    }

    /* Set SPI output slew rate to 4ns - 12ns */
    config_step++;
    wr_valid = imu_write_verify(
        IMU_REG_DRIVE_CONFIG,
        SLEW_RATE_4ns_12ns,
        IMU_MASK_DRIVE_CONFIG
    );
    if (wr_valid != EXIT_SUCCESS) {
        return config_step;
    }

    /* Set INT1 output driver to be push-pull */
    config_step++;
    wr_valid = imu_write_verify(
        IMU_REG_INT_CONFIG,
        INT1_DRIVE_PUSH_PULL,
        IMU_MASK_INT_CONFIG
    );
    if (wr_valid != EXIT_SUCCESS) {
        return config_step;
    }
    
    /* Enable Stream-to-FIFO mode */
    config_step++;
    wr_valid = imu_write_verify(
        IMU_REG_FIFO_CONFIG,
        STREAM_TO_FIFO_MODE,
        IMU_MASK_FIFO_CONFIG
    );
    if (wr_valid != EXIT_SUCCESS) {
        return config_step;
    }

    /* Flush the FIFO in case there's any data in it */
    config_step++;
    imu_write_byte(IMU_REG_SIGNAL_PATH_RESET, FLUSH_FIFO);

    /* Set timestamp in the FIFO to be a delta instead of absolute */
    config_step++;
    wr_valid = imu_write_verify(
        IMU_REG_TMST_CONFIG,
        TMST_DELTA_EN,
        IMU_MASK_TMST_CONFIG
    );
    if (wr_valid != EXIT_SUCCESS) {
        return config_step;
    }

    /* Send accel, gyro, temp, and timestamp data to FIFO */
    config_step++;
    wr_valid = imu_write_verify(
        IMU_REG_FIFO_CONFIG1,
        SEND_ALL_DATA_TO_FIFO,
        IMU_MASK_FIFO_CONFIG1
    );
    if (wr_valid != EXIT_SUCCESS) {
        return config_step;
    }

    /* Set FIFO interrupt threshold to 1 packet */
    config_step++;
    wr_valid = imu_write_verify(
        IMU_REG_FIFO_CONFIG2,
        FIFO_WM_LSB,
        IMU_MASK_FIFO_CONFIG2
    );
    if (wr_valid != EXIT_SUCCESS) {
        return config_step;
    }

    /* Set FIFO threshold as the interrupt source for INT1 */
    config_step++;
    wr_valid = imu_write_verify(
        IMU_REG_INT_SOURCE0,
        INT1_SRC_FIFO_THS,
        IMU_MASK_INT_SOURCE0
    );
    if (wr_valid != EXIT_SUCCESS) {
        return config_step;
    }

    /* Deassert async reset for the interrupt */
    config_step++;
    wr_valid = imu_write_verify(
        IMU_REG_INT_CONFIG1,
        DEASSERT_INT_RESET,
        IMU_MASK_INT_CONFIG1
    );
    if (wr_valid != EXIT_SUCCESS) {
        return config_step;
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
    imu_read_byte(IMU_REG_WHO_AM_I, &reg_data);
    if (reg_data != WHO_AM_I_VAL) {
        return EXIT_FAILURE;
    }

    /* Configure the IMU */
    uint8_t config_status = imu_config();
    if (config_status != EXIT_SUCCESS) {
        return config_status;
    }

    /* Configure the IMU INT1 pin interrupt */
    imu_exti_init();

    return EXIT_SUCCESS;
}

/*!
    @brief Turn on the gyro and accelerometer in low noise mode
    @return EXIT_SUCCESS if the IMU was enabled, EXIT_FAILURE otherwise
*/
int imu_enable(void) {
    /* Flush the FIFO */
    imu_write_byte(IMU_REG_SIGNAL_PATH_RESET, FLUSH_FIFO);

    /* Place gyro and accelerometer in low noise mode */
    uint8_t wr_valid = imu_write_verify(
        IMU_REG_PWR_MGMT0,
        GYRO_ACCEL_LN_MODE,
        IMU_MASK_PWR_MGMT0
    );
    if (wr_valid != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    /* CLear the INT1 flag if it's active */
    clear_int1();

    return EXIT_SUCCESS;
}

/*!
    @brief Turn off the gyro and accelerometer
    @return EXIT_SUCCESS if the IMU was disable, EXIT_FAILURE otherwise
*/
int imu_disable(void) {
    return imu_write_verify(
        IMU_REG_PWR_MGMT0,
        GYRO_ACCEL_OFF,
        IMU_MASK_PWR_MGMT0
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

/*!
    @brief Read sensor data from the IMU if it's available
    @param imu_data pointer to an IMU_Data struct that will be modified if data
    is available
    @return 1 if the data is valid. 0 if not.
    @details The INT1 flag indicates that data is available in the IMU FIFO. The
    INT_STATUS register is read first to clear the interrupt flags internal to
    the IMU. The FIFO_COUNTX registers are read to see how many bytes are in
    the FIFO. Ideally, this should never be more than 16 (1 packet). If for some
    reason this is greater than 1 packet, then the function will return only the
    most recent packet with a modified timestamp delta.
*/
uint8_t read_imu_data(struct IMU_Data *imu_data) {
    /* INT1 flag indicates data is available */
    if (!is_int1_active()) {
        return 0;
    }

    clear_int1();
    
    /* Read the INT_STATUS register to clear the IMU interrupt */
    uint8_t int_status;
    imu_read_byte(IMU_REG_INT_STATUS, &int_status);

    /* Get number of bytes in the FIFO */
    uint8_t fifo_count_upper;
    uint8_t fifo_count_lower;
    imu_read_byte(IMU_REG_FIFO_COUNTH, &fifo_count_upper);
    imu_read_byte(IMU_REG_FIFO_COUNTL, &fifo_count_lower);
    uint16_t fifo_count = (fifo_count_upper << 8) | fifo_count_lower;
    if (fifo_count == 0) {
        return 0;
    }

    /* Read all the data in the FIFO */
    uint8_t fifo_data[fifo_count];
    imu_read_bytes(
        IMU_REG_FIFO_DATA,
        fifo_data,
        fifo_count
    );

    /* Only process data from the last FIFO packet */
    uint8_t last_packet_idx = fifo_count - IMU_FIFO_PACKET_LEN;
    uint8_t *last_packet = (fifo_data + last_packet_idx);

    /* Calculate the actual timestamp delta based on the number of packets */
    uint16_t ts_delta = 0;
    if (fifo_count > IMU_FIFO_PACKET_LEN) {
        /* Combine timestamp delta from all packets in the FIFO */
        uint8_t num_packets = fifo_count / IMU_FIFO_PACKET_LEN;
        for (int i = 0; i < num_packets; i++) {
            uint8_t start_idx = IMU_FIFO_PACKET_LEN * i;
            uint16_t ts_delta_upper = fifo_data[start_idx + 14];
            uint16_t ts_delta_lower = fifo_data[start_idx + 15];
            ts_delta += (ts_delta_upper << 8) | ts_delta_lower;
        }
    } else {
        /* Use the timestamp delta from the one packet */
        ts_delta = (last_packet[14] << 8) | last_packet[15];
    }

    /* Construct IMU data from the FIFO data */
    int16_t ax = (last_packet[1] << 8) | last_packet[2];
    int16_t ay = (last_packet[3] << 8) | last_packet[4];
    int16_t az = (last_packet[5] << 8) | last_packet[6];
    int16_t gx = (last_packet[7] << 8) | last_packet[8];
    int16_t gy = (last_packet[9] << 8) | last_packet[10];
    int16_t gz = (last_packet[11] << 8) | last_packet[12];

    /* Scale the IMU data and update the IMU data structure */
    imu_data->ax = ax * ACCEL_SCALE;
    imu_data->ay = ay * ACCEL_SCALE;
    imu_data->az = az * ACCEL_SCALE;
    imu_data->gx = gx * GYRO_SCALE;
    imu_data->gy = gy * GYRO_SCALE;
    imu_data->gz = gz * GYRO_SCALE;
    imu_data->ts_delta = ts_delta;

    return 1;
}