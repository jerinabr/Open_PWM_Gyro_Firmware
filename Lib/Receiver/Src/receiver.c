#include "receiver.h"
#include "usart1.h"
#include <stdint.h>

// Initialize the reciever instance
receiver_t rx = {
  .rx_data = {0},
  .rx_data_valid = 0
};

uint8_t rx_buffer[64];

// ----------------------------------------------------------------------
// PRIVATE FUNCTIONS
// ----------------------------------------------------------------------

static inline void parse_ibus_data(uint8_t data) {
  static enum {
    WAIT_FOR_LENGTH,
    WAIT_FOR_DATA,
    WAIT_FOR_CHECKSUM_1,
    WAIT_FOR_CHECKSUM_2
  } state = WAIT_FOR_LENGTH;

  static uint8_t payload_len = 0;
  static uint16_t checksum_calc = 0;
  static uint16_t checksum_rx = 0;
  static uint8_t buf_i = 0;

  switch (state) {
    case WAIT_FOR_LENGTH: {
      if (data > 0x03 && data <= 0x20) {
        buf_i = 0;
        payload_len = data + 1; // n bytes of data + 1 cmd byte
        checksum_calc = 0xFFFF - data;
        state = WAIT_FOR_DATA;
      }
      break;
    }

    case WAIT_FOR_DATA: {
      rx_buffer[buf_i++] = data;
      checksum_calc -= data;
      if (buf_i == payload_len) {
        state = WAIT_FOR_CHECKSUM_1;
      }
      break;
    }

    case WAIT_FOR_CHECKSUM_1: {
      checksum_rx |= (uint16_t) data;
      state = WAIT_FOR_CHECKSUM_2;
      break;
    }

    case WAIT_FOR_CHECKSUM_2: {
      checksum_rx |= (uint16_t) data << 8;
      if (checksum_calc == checksum_rx) {
        uint8_t cmd = rx_buffer[0];
        if (cmd == 0x40) {
          for (int i = 1; i < 16; i++) {
            rx.rx_data[i] = rx_buffer[2 * i] | (rx_buffer[2 * i + 1] << 8);
          }
          rx.rx_data_valid = 1;
        }
      }
      break;
    }
  }
}

static inline void process_received_byte(uint8_t data) {
  parse_ibus_data(data);
}

// ----------------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------------

/*!
  @brief Initialize the reciever
*/
void receiver_init() {
  usart1_init();
}

/*!
  @brief Read bytes
*/
void process_receiver() {
  uint8_t rx_buf[128];
  uint8_t rx_buf_i = 0;
  // Preload data into buffer
  while (rx_buf_i < 128) {
    if (!usart1_rx_fifo_empty()) {
      uint8_t data = usart1_read_rx_fifo();
      rx_buf[rx_buf_i++] = data;
    }
  }
  // Process rx data
  for (int i = 0; i < 128; i++) {
    process_received_byte(rx_buf[i]);
  }
}