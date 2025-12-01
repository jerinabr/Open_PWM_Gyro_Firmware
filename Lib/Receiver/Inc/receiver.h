/*!
  @file		receiver.h
  @brief	Receive and decode serial data from the connected receiver over UART

  This library initializes the UART interface to receive data from the SER header

  The data can be decoded in a number of ways depending on the protocol specified
  These protocols include:
    - IBUS
    - CRSF
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