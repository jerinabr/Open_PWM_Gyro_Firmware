#include "app.h"
#include "receiver.h"
#include "stm32g4xx_hal.h"
#include "usbd_cdc_if.h"
#include <stdint.h>
#include <stdio.h>

#define LED_RED_PORT  GPIOB
#define LED_RED_PIN   GPIO_PIN_5

/*!
  @brief Configure the application

  Any application configuration that happens before the main loop should be placed here
*/
void config() {
  // Initialize peripherals
  receiver_init();

  process_receiver();
  if (rx.rx_data_valid) {
    rx.rx_data_valid = 0;
    // HAL_GPIO_TogglePin(LED_RED_PORT, LED_RED_PIN);
    // CDC_Transmit_FS(rx.rx_data, 36);
  }
}

/*!
  @brief Main loop for the application

  Any application code in this function will loop infinitely
*/
void loop() {
  // Process peripheral data
  // process_receiver();
  // if (rx.rx_data_valid) {
  //   rx.rx_data_valid = 0;
  //   uint8_t tx_buf[64];
  //   uint8_t tx_buf_len = snprintf((char *) tx_buf, 64, "RX data valid %lu\r\n", HAL_GetTick());
  //   CDC_Transmit_FS(tx_buf, tx_buf_len);
  // }
  // uint8_t tx_buf[64];
  // uint8_t tx_buf_len = snprintf((char*) tx_buf, 64, "%lu\r\n", HAL_GetTick());
  // CDC_Transmit_FS(tx_buf, tx_buf_len);
  // HAL_GPIO_TogglePin(LED_RED_PORT, LED_RED_PIN);
  // HAL_Delay(500);
}