#include "app.h"

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
  CDC_Transmit_FS("hello\n", 6);
  HAL_GPIO_TogglePin(LED_RED_PORT, LED_RED_PIN);
  HAL_Delay(1000);
}