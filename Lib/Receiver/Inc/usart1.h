/*!
  @file		usart1.h
  @brief	USART1 driver

  This library configures and initializes the USART1 peripheral
*/
#ifndef USART1_H
#define USART1_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

void usart1_init(void);
uint8_t usart1_rx_fifo_empty(void);
uint8_t usart1_read_rx_fifo(void);

#ifdef __cplusplus
}
#endif

#endif