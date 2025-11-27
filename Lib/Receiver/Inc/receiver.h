/*!
  @file		receiver.h
  @brief	Receive and decode serial data from the connected receiver over UART

  This library initializes the UART interface to receive data from the SER header.
  
  Since the data from the receiver can NOT be missed, DMA is used to transfer the
  incoming data to a circular buffer.
  
  We only care about the most recent channel outputs, so instead of a traditional
  FIFO architecture, there's simply a pointer to the latest data.

  The data can be decoded in a number of ways depending on the protocol specified.
  These protocols include:
    - IBUS
    - SBUS
    - CRSF
  
  Along with this, the channel mappings are configurable. These mappings include:
    - Roll
    - Pitch
    - Yaw
    - Throttle
    - Gyro Mode
    - Gyro Gain
    - Aux Control 0..9
*/
#ifndef RECEIVER_H
#define RECEIVER_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// Struct to hold receiver data
typedef struct {
  uint16_t rx_data[16];
  uint8_t rx_data_valid;
} receiver_t;

// Global instance of receiver
extern receiver_t rx;

// Functions
void receiver_init(void);
void process_receiver(void);

#ifdef __cplusplus
}
#endif

#endif