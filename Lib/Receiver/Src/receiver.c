#include "receiver.h"
#include "ibus.h"
#include "usart1.h"
#include <stdint.h>

// Initialize the reciever instance
receiver rx = {
  .channel_data = {0},
  .channel_data_valid = 0
};

// Function pointer to the selected protocol decoder function
static void (*decode_rx_data)(uint8_t data);

// ----------------------------------------------------------------------
// PRIVATE FUNCTIONS
// ----------------------------------------------------------------------

/*!
  @brief Set the channels and valid flag to 0

  This is just here till I implement all the receiver protocols
*/
static void decode_rx_data_placeholder(uint8_t data) {
  for (int i = 0; i < MAX_CHANNELS; i++) {
    rx.channel_data[i] = 0;
  }
  rx.channel_data_valid = 0;
}

/*!
  @brief Configure the USART and assign the decoder function pointer
*/
static void select_rx_config(usart_config* usart_config, rx_protocols rx_protocol) {
  switch (rx_protocol) {
    case IBUS: {
      usart_config->baud_rate = IBUS_BAUD_RATE;
      usart_config->idle_level = IBUS_IDLE_LEVEL;
      decode_rx_data = parse_ibus_data;
      reset_ibus();
      break;
    }
    case CRSF: {
      // TODO: Implement this eventually
      decode_rx_data = decode_rx_data_placeholder;
      break;
    }
    default: {
      decode_rx_data = decode_rx_data_placeholder;
      break;
    }
  }
}

// ----------------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------------

/*!
  @brief Initialize the reciever with a specific protocol
*/
void receiver_init(rx_protocols rx_protocol) {
  usart_config usart_config = {
    .baud_rate = usart1_default_config.baud_rate,
    .idle_level = usart1_default_config.idle_level
  };
  select_rx_config(&usart_config, rx_protocol);
  usart1_init(&usart_config);
}

/*!
  @brief Reconfigure the receiver with a specific protocol
*/
void receiver_reconfig(rx_protocols rx_protocol) {
  usart_config usart_config = {
    .baud_rate = usart1_default_config.baud_rate,
    .idle_level = usart1_default_config.idle_level
  };
  select_rx_config(&usart_config, rx_protocol);
  usart1_reconfig(&usart_config);
}

/*!
  @brief Read the RX FIFO and parse the data

  This function should be called in the main program loop
*/
void process_receiver() {
  if (!usart1_rx_fifo_empty()) {
    uint8_t data = usart1_read_rx_fifo();
    decode_rx_data(data);
  }
}