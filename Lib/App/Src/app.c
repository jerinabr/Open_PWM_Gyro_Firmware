#include "app.h"
#include "receiver.h"
#include "pwm.h"
#include "stm32g4xx.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_gpio.h"
#include "usbd_cdc_if.h"
#include <stdint.h>
#include <stdio.h>

#define LED_RED_PORT  GPIOB
#define LED_RED_PIN   GPIO_PIN_5

// FOR DEBUG
uint32_t t0 = 0;

// ----------------------------------------------------------------------
// PRIVATE FUNCTIONS
// ----------------------------------------------------------------------

/*!
  @brief Enable clocks to the peripherals used in this design
  @details Section 7.2.17 of the reference manual states that there's a 2 clock
  delay between writing the enable bit and the peripheral clock being active, so
  we read the register after writing it to cause a bit of delay
*/
static void enable_peripheral_clocks() {
  // Enable clock for GPIO Ports A and B
  uint32_t AHB2ENR_Mask = RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;
  SET_BIT(RCC->AHB2ENR, AHB2ENR_Mask);
  while (!READ_BIT(RCC->AHB2ENR, AHB2ENR_Mask));

  // Enable clock for TIM2 and TIM4
  uint32_t APB1ENR1_Mask = RCC_APB1ENR1_TIM2EN | RCC_APB1ENR1_TIM4EN;
  SET_BIT(RCC->APB1ENR1, APB1ENR1_Mask);
  while (!READ_BIT(RCC->APB1ENR1, APB1ENR1_Mask));
  
  // Enable clock for USART1
  uint32_t APB2ENR_Mask = RCC_APB2ENR_USART1EN;
  SET_BIT(RCC->APB2ENR, APB2ENR_Mask);
  while (!READ_BIT(RCC->APB2ENR, APB2ENR_Mask));
}

// ----------------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------------

/*!
  @brief Configure the application
  @details Any application configuration that happens before the main loop should be
  placed here
*/
void config() {
  enable_peripheral_clocks();
  receiver_init(IBUS);
  pwm_init();
  
  // FOR DEBUG
  // Initialize the LED off (LED is active low)
  HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
  t0 = HAL_GetTick();
}

/*!
  @brief Main loop for the application
  @details Any application code in this function will loop infinitely
*/
void loop() {
  // Process peripheral data
  process_receiver();

  // FOR DEBUG
  if (rx.channel_data_valid) {
    rx.channel_data_valid = 0;

    // Print RX outputs to USB
    uint8_t tx_buf[64];
    uint8_t tx_buf_len = snprintf(
      (char *) tx_buf, 64,
      "Ch1: %hu\tCh2: %hu\tCh3: %hu\tCh4: %hu\r\n",
      rx.channel_data[0],
      rx.channel_data[1],
      rx.channel_data[2],
      rx.channel_data[3]
    );
    CDC_Transmit_FS(tx_buf, tx_buf_len);

    // Pass RX outputs straight to channel outputs
    for (int i = 0; i < NUM_CHANNELS; i++) {
      pwm.ch_outputs[i] = (int16_t) rx.channel_data[i] - 1500;
    }
    pwm_update();
  }

  // FOR DEBUG
  // The LED turns on after 1 second
  // This helps us tell if the device browns out because the LED would shut off
  if (HAL_GetTick() - t0 > 1000) {
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
  }
}