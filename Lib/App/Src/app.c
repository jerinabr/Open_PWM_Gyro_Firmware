#include "ahrs.h"
#include "app.h"
#include "hw_config.h"
#include "sys_config.h"
#include "imu.h"
#include "led.h"
#include "math3d.h"
#include "pwm.h"
#include "receiver.h"
#include "stabilize.h"
#include "systick.h"
#include "stm32g431xx.h"
#include "stm32g4xx.h"
#include "usbd_cdc_if.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>

/***********************************************************************
-- MACROS --
***********************************************************************/

/* Mode select switch position boundaries */
#define SWITCH_POS_1_2_BOUNDARY 1333
#define SWITCH_POS_2_3_BOUNDARY 1666

/***********************************************************************
-- CONFIGURATION VARIABLES --
***********************************************************************/

/* Rotation of gyro with respect to aircraft */
float x_ang_init = 0;
float y_ang_init = 0;
float z_ang_init = 0;

/* Channel to control mapping */
uint8_t roll_channel = ROLL_CHANNEL;
uint8_t pitch_channel = PITCH_CHANNEL;
uint8_t yaw_channel = YAW_CHANNEL;
uint8_t gain_channel = GAIN_CHANNEL;
uint8_t mode_select_channel = MODE_CHANNEL;
uint16_t switch_pos_1_mode = SWITCH_POS_1_MODE;
uint16_t switch_pos_2_mode = SWITCH_POS_2_MODE;
uint16_t switch_pos_3_mode = SWITCH_POS_3_MODE;

/* Channel characteristics */
uint16_t channel_center = CHANNEL_CENTER;
uint16_t channel_max_throw = CHANNEL_MAX_THROW;
uint16_t gain_channel_min = GAIN_CHANNEL_MIN;
uint16_t gain_channel_width = GAIN_CHANNEL_WIDTH;

/***********************************************************************
-- RUNTIME VARIABLES --
***********************************************************************/

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
struct Vector3 w_imu = {0};

/* Stabilization context */
struct Stabilization_Context st_ctx;

/* Channel data in microseconds from receiver serial */
uint16_t rx_channel_data[MAX_RX_CHANNELS];

/* PWM outputs */
uint16_t pwm_outputs[NUM_OUTPUT_CHANNELS];

/* Last control update */
uint32_t last_update = 0;

/* Previous stabilization mode */
uint8_t prev_stabilization_mode = GYRO_MODE_OFF;

/* FOR DEBUG */
uint32_t t0 = 0;

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
    @brief Calculate and update the orientation of the IMU
*/
static void calc_orientation(void) {
    /*
        Update the angular velocity vector

        The y-axis is negated here because for some reason the ICM-42605
        coordinate and rotation frame doesn't match the standard ones used for
        aircraft.

        From this       To this
            +Z            +Z
            |      ->     |
           / \           / \
         +Y  +X        +X  +Y
    */
    w_imu.x = imu_data.gx;
    w_imu.y = -imu_data.gy;
    w_imu.z = imu_data.gz;
    
    /* Update the orientation quaternion using the Madgwick filter */
    madgwick_imu(
        imu_data.ax, imu_data.ay, imu_data.az,
        imu_data.gx, imu_data.gy, imu_data.gz,
        imu_data.ts_delta,
        &q_imu
    );

    /* Assign attitude to input */
    st_ctx.input.q = q_imu;
    st_ctx.input.w = w_imu;
}

/*!
    @brief Normalize and map the channel data from the receiver to control
    inputs
*/
static void map_rx_to_control_inputs(void) {
    /* Remove control input offset */
    float roll = (float) (rx_channel_data[roll_channel] - channel_center);
    float pitch = (float) (rx_channel_data[pitch_channel] - channel_center);
    float yaw = (float) (rx_channel_data[yaw_channel] - channel_center);
    float gain = (float) (rx_channel_data[gain_channel] - gain_channel_min);

    /* Normalize the control inputs */
    roll /= (float) channel_max_throw;
    pitch /= (float) channel_max_throw;
    yaw /= (float) channel_max_throw;
    gain /= (float) gain_channel_width;

    /* Assign control inputs to the stabilization context */
    st_ctx.input.control_input.roll = roll;
    st_ctx.input.control_input.pitch = pitch;
    st_ctx.input.control_input.yaw = yaw;
    st_ctx.input.global_gain = gain;

    /* Select mode based on switch position */
    if (rx_channel_data[mode_select_channel] < SWITCH_POS_1_2_BOUNDARY) {
        st_ctx.input.mode = switch_pos_1_mode;
    }
    else if (rx_channel_data[mode_select_channel] < SWITCH_POS_2_3_BOUNDARY) {
        st_ctx.input.mode = switch_pos_2_mode;
    }
    else {
        st_ctx.input.mode = switch_pos_3_mode;
    }
}

/*!
    @brief Convert and map normalized control outputs to respective output PWM
    channels
*/
static void map_control_outputs_to_pwm(void) {
    /* Map control outputs to respective channels */
    float roll = (float) channel_max_throw * st_ctx.control_output.roll;
    float pitch = (float) channel_max_throw * st_ctx.control_output.pitch;
    float yaw = (float) channel_max_throw * st_ctx.control_output.yaw;

    roll += (float) channel_center;
    pitch += (float) channel_center;
    yaw += (float) channel_center;

    /* Map rx inputs to pwm outputs initially */
    for (int i = 0; i < NUM_OUTPUT_CHANNELS; i++) {
        pwm_outputs[i] = rx_channel_data[i];
    }

    /* Override control pwm outputs with stabilized values */
    pwm_outputs[AILERON1_CHANNEL] = (uint16_t) roll;
    pwm_outputs[ELEVATOR1_CHANNEL] = (uint16_t) pitch;
    pwm_outputs[RUDDER1_CHANNEL] = (uint16_t) yaw;
}

/*!
    @brief Print 4 uint16 values over USB
*/
static void print_4_uint16(uint16_t arr[4]) {
    uint8_t tx_buf[64];
    uint8_t tx_buf_len = snprintf(
        (char*) tx_buf, 64,
        "Ch1: %hu\tCh2: %hu\tCh3: %hu\tCh4: %hu\r\n",
        arr[0], arr[1], arr[2], arr[3]
    );
    CDC_Transmit_FS(tx_buf, tx_buf_len);
}

/*!
    @brief Print 3 floats scaled by 1000 over USB
*/
static void print_3_floats(float f_list[3]) {
    int16_t scaled_a = f_list[0] * 1000;
    int16_t scaled_b = f_list[1] * 1000;
    int16_t scaled_c = f_list[2] * 1000;

    uint8_t tx_buf[256];
    uint8_t tx_buf_len = snprintf(
        (char*) tx_buf, 256,
        "%hd,%hd,%hd\r\n",
        scaled_a, scaled_b, scaled_c
    );
    CDC_Transmit_FS(tx_buf, tx_buf_len);
}

/*!
    @brief Print 4 floats scaled by 1000 over USB
*/
static void print_4_floats(float f_list[4]) {
    int16_t scaled_a = f_list[0] * 1000;
    int16_t scaled_b = f_list[1] * 1000;
    int16_t scaled_c = f_list[2] * 1000;
    int16_t scaled_d = f_list[3] * 1000;

    uint8_t tx_buf[256];
    uint8_t tx_buf_len = snprintf(
        (char*) tx_buf, 256,
        "%hd,%hd,%hd,%hd\r\n",
        scaled_a, scaled_b, scaled_c, scaled_d
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
void app_setup(void) {
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
        st_ctx.config.device_orientation.r_matrix
    );

    /* Configure stabilization context */
    st_ctx.config.control_limits.roll = ROLL_CONTROL_LIMIT;
    st_ctx.config.control_limits.pitch = PITCH_CONTROL_LIMIT;
    st_ctx.config.control_limits.yaw = YAW_CONTROL_LIMIT;
    st_ctx.config.ang_limits.roll = ROLL_ANG_LIMIT;
    st_ctx.config.ang_limits.pitch = PITCH_ANG_LIMIT;
    st_ctx.config.gains.roll = ROLL_GAIN;
    st_ctx.config.gains.pitch = PITCH_GAIN;
    st_ctx.config.gains.yaw = YAW_GAIN;
    st_ctx.config.reverse.roll = ROLL_ADJ_REVERSE;
    st_ctx.config.reverse.pitch = PITCH_ADJ_REVERSE;
    st_ctx.config.reverse.yaw = YAW_ADJ_REVERSE;

    /* Set the stabilization mode to off by default */
    st_ctx.input.mode = prev_stabilization_mode;
    set_stabilization_mode(&st_ctx);

    /* Set the last update time */
    last_update = get_ms();
}

/*!
    @brief Main loop for the application
    @details Any application code in this function will loop infinitely
*/
void app_loop(void) {

    /* Read IMU */
    uint8_t imu_data_valid = read_imu_data(&imu_data);
    if (imu_data_valid) {
        calc_orientation();
    }

    /* Read receiver */
    uint8_t rx_data_valid = read_receiver(rx_channel_data);
    if (rx_data_valid) {
        map_rx_to_control_inputs();
    }

    /* Update the stabilization mode if it changed */
    if (st_ctx.input.mode != prev_stabilization_mode) {
        prev_stabilization_mode = st_ctx.input.mode;
        set_stabilization_mode(&st_ctx);
    }

    /* Update stabilization and outputs at a rate of 50 Hz */
    uint32_t current_ms = get_ms();
    if (current_ms - last_update >= OUTPUT_UPDATE_PERIOD) {
        last_update = current_ms;
        apply_stabilization(&st_ctx);
        map_control_outputs_to_pwm();
        update_pwm_pw(pwm_outputs);
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