/*!
  @file		receiver.h
  @brief	Receive and decode serial data from the connected receiver over UART

  --OVERVIEW--
  The data can be decoded in a number of ways depending on the protocol specified
  These protocols include:
    - IBUS
    - CRSF

  When the receiver is initialized with a particular protocol, the decoder function
  pointer is assigned and the USART is initialized with a configuration given by the
  protocol.

  The data from the USART RX FIFO is sent to the decoder function and when the data
  is successfully parsed, the decoder function updates the channel data array and sets
  the valid flag

  The receiver can also be reconfigured during operation to select a different protocol

  --USAGE--
  The receiver_init function should be called with one of the given protocols enums during
  the program setup

  The process_receiver function should be called in the main program loop on every iteration
  of the loop
*/
#ifndef RECEIVER_H
#define RECEIVER_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CHANNELS 16

// Receiver protocols
typedef enum {
  IBUS,
  CRSF
} rx_protocols;

// Struct to hold receiver data
typedef struct {
  uint16_t channel_data[MAX_CHANNELS];
  uint8_t channel_data_valid;
} receiver;

// Global instance of receiver
extern receiver rx;

// Functions
void receiver_init(rx_protocols rx_protocol);
void receiver_reconfig(rx_protocols rx_protocol);

void process_receiver(void);

#ifdef __cplusplus
}
#endif

#endif