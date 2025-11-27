#include "app.h"
#include "receiver.h"
#include "stm32g4xx_hal.h"
#include "usbd_cdc_if.h"
#include <stdint.h>

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
  if (rx.rx_data_valid) {
    rx.rx_data_valid = 0;
    uint8_t tx_buf[33];
    for (int i = 0; i < 16; i++) {
      tx_buf[2 * i + 0] = (uint8_t) rx.rx_data[i] & 0xFF;
      tx_buf[2 * i + 1] = (uint8_t) (rx.rx_data[i] >> 8) & 0xFF;
    }
    tx_buf[32] = '\n';
    CDC_Transmit_FS(tx_buf, 33);
  }
  // HAL_GPIO_TogglePin(LED_RED_PORT, LED_RED_PIN);
  // HAL_Delay(100);
}