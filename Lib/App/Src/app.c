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
}

/*!
  @brief Main loop for the application

  Any application code in this function will loop infinitely
*/
void loop() {
  // Process peripheral data
  process_receiver();

  // FOR DEBUG
  if (rx.rx_data_valid) {
    rx.rx_data_valid = 0;
    uint8_t tx_buf[64];
    uint8_t tx_buf_len = snprintf(
      (char *) tx_buf, 64,
      "Ch1: %hu\tCh2: %hu\tCh3: %hu\tCh4: %hu\r\n",
      rx.rx_data[0],
      rx.rx_data[1],
      rx.rx_data[2],
      rx.rx_data[3]
    );
    CDC_Transmit_FS(tx_buf, tx_buf_len);
  }
  // HAL_GPIO_TogglePin(LED_RED_PORT, LED_RED_PIN);
  // HAL_Delay(500);
}