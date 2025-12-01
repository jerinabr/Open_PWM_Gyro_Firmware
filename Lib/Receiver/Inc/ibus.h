/*!
  @file		ibus.h
  @brief	Parse and decode IBus protocol from a receiver

  An IBus frame is structured as follows:

  Byte 1      - Total number of bytes in the frame
  Byte 2      - Command code
  Byte 3:n-2  - Payload
  Byte n-1    - Checksum LSB
  Byte n      - Checksum MSB

  This library parses the frame with every new byte that is received

  When the received frame is valid, the receiver channels are updated
  and the receiver channel valid flag is set
*/
#ifndef IBUS_H
#define IBUS_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define IBUS_BAUD_RATE  115200
#define IBUS_IDLE_LEVEL 1

void reset_ibus(void);
void parse_ibus_data(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif