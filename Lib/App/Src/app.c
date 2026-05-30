#include "ahrs.h"
#include "app.h"
#include "hw_config.h"
#include "imu.h"
#include "led.h"
#include "math3d.h"
#include "pwm.h"
#include "receiver.h"
#include "systick.h"
#include "stm32g431xx.h"
#include "stm32g4xx.h"
#include "usbd_cdc_if.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MADGWICK_LEARNING_RATE 0.1

/* FOR DEBUG */
uint32_t t0 = 0;

/* Data read from the onboard IMU */
struct IMU_Data imu_data;

/* Quaternion rotation from IMU data sensor fusion */
struct Quaternion q_imu = {
    .w = 1,
    .x = 0,
    .y = 0,
    .z = 0
};

/* Vector representation of angular velocity */
struct Vector3 w = {0};

/* Rotation of gyro with respect to aircraft */
float x_ang_init = 90;
float y_ang_init = 0;
float z_ang_init = 0;
float r_init[3][3];

/* Channel data in microseconds from receiver serial */
uint16_t rx_channel_data[MAX_RX_CHANNELS];

/***********************************************************************
-- PRIVATE FUNCTIONS --
***********************************************************************/

/*!
    @brief Enable clocks to the peripherals used in this design
    @details Section 7.2.17 of the reference manual states that there's a two
    clock delay between writing the enable bit and the peripheral clock being
    active, so we read the register after writing it to cause a bit of delay
*/
static void enable_peripheral_clocks(void) {
    /* Enable clocks for DMAMUX and DMA1 */
    uint32_t AHB1ENR_Mask = RCC_AHB1ENR_DMAMUX1EN | RCC_AHB1ENR_DMA1EN;
    SET_BIT(RCC->AHB1ENR, AHB1ENR_Mask);
    while (!READ_BIT(RCC->AHB1ENR, AHB1ENR_Mask));

    /* Enable clock for GPIO Ports A and B */
    uint32_t AHB2ENR_Mask = RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;
    SET_BIT(RCC->AHB2ENR, AHB2ENR_Mask);
    while (!READ_BIT(RCC->AHB2ENR, AHB2ENR_Mask));

    /* Enable clock for TIM2 and TIM4 */
    uint32_t APB1ENR1_Mask = RCC_APB1ENR1_TIM2EN | RCC_APB1ENR1_TIM4EN;
    SET_BIT(RCC->APB1ENR1, APB1ENR1_Mask);
    while (!READ_BIT(RCC->APB1ENR1, APB1ENR1_Mask));
    
    /* Enable clock for USART1, SPI1, and SYSCFG */
    uint32_t APB2ENR_Mask = RCC_APB2ENR_USART1EN |
                            RCC_APB2ENR_SPI1EN |
                            RCC_APB2ENR_SYSCFGEN;
    SET_BIT(RCC->APB2ENR, APB2ENR_Mask);
    while (!READ_BIT(RCC->APB2ENR, APB2ENR_Mask));
}

/*!
    @brief Calculate orientation of the IMU
    @returns 1 if orientation updated, 0 if not
*/
uint8_t calc_orientation(void) {
    /* Read IMU */
    uint8_t imu_data_valid = read_imu_data(&imu_data);
    if (!imu_data_valid) {
        return 0;
    }

    /* Update the angular velocity vector */
    w.x = imu_data.gx;
    w.y = imu_data.gy;
    w.z = imu_data.gz;
    
    /* Update the orientation quaternion using the Madgwick filter */
    madgwick_imu(
        MADGWICK_LEARNING_RATE,
        imu_data.ax, imu_data.ay, imu_data.az,
        imu_data.gx, imu_data.gy, imu_data.gz,
        imu_data.ts_delta,
        &q_imu
    );

    return 1;
}

/*!
    @brief Print receiver outputs over USB for debug
*/
void print_rx(void) {
    uint8_t tx_buf[64];
    uint8_t tx_buf_len = snprintf(
        (char*) tx_buf, 64,
        "Ch1: %hu\tCh2: %hu\tCh3: %hu\tCh4: %hu\r\n",
        rx_channel_data[0],
        rx_channel_data[1],
        rx_channel_data[2],
        rx_channel_data[3]
    );
    CDC_Transmit_FS(tx_buf, tx_buf_len);
}

/*!
    @brief Print gyro output over USB for debug
*/
void print_gyro(void) {
    struct Vector3 w_rotated = matrix_rotate_vector(r_init, w);
    int16_t wx = w_rotated.x * 1000;
    int16_t wy = w_rotated.y * 1000;
    int16_t wz = w_rotated.z * 1000;

    uint8_t tx_buf[256];
    uint8_t tx_buf_len = snprintf(
        (char*) tx_buf, 256,
        "%hd,%hd,%hd\r\n",
        wx, wy, wz
    );
    CDC_Transmit_FS(tx_buf, tx_buf_len);
}

/*!
    @brief Print quaternion output over USB for debug
*/
void print_quat(void) {
    int16_t qw = q_imu.w * 1000;
    int16_t qx = q_imu.x * 1000;
    int16_t qy = q_imu.y * 1000;
    int16_t qz = q_imu.z * 1000;

    uint8_t tx_buf[256];
    uint8_t tx_buf_len = snprintf(
        (char*) tx_buf, 256,
        "%hd,%hd,%hd,%hd\r\n",
        qw, qx, qy, qz
    );
    CDC_Transmit_FS(tx_buf, tx_buf_len);
}

/***********************************************************************
-- PUBLIC FUNCTIONS --
***********************************************************************/

/*!
    @brief Configure the application
    @details Any application configuration that happens before the main loop
    should be placed here
*/
void app_config(void) {
    enable_peripheral_clocks();

    /* Initialize peripherals */
    led_init();
    pwm_init();
    receiver_init(IBUS);

    /* Try to initialize the IMU */
    uint8_t imu_status = imu_init();
    if (imu_status != EXIT_SUCCESS) {
        while (1) {
            delay_ms(125);
            led_toggle();
        }
    }

    /* Enable the IMU */
    imu_enable();
    
    /* FOR DEBUG */
    t0 = get_ms();

    /* Convert gyro orientation to rotation matrix */
    euler_to_rot_matrix(
        x_ang_init,
        y_ang_init,
        z_ang_init,
        r_init
    );
}

/*!
    @brief Main loop for the application
    @details Any application code in this function will loop infinitely
*/
void app_loop(void) {
    /* Update orientation */
    uint8_t orientation_update = calc_orientation();
    if (orientation_update) {
        print_gyro();
    }

    /* Read receiver */
    uint8_t rx_data_valid = read_receiver(rx_channel_data);
    if (rx_data_valid) {
        update_pwm_pw(rx_channel_data);
    }

    /*
        FOR DEBUG

        The LED turns on after 1 second 
        This helps us tell if the device browns out because the LED would shut
        off when the device starts up
    */
    if (get_ms() - t0 > 1000) {
        led_on();
    }
}