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

// ----------------------------------------------------------------------
// TYPES
// ----------------------------------------------------------------------

/*!
  @brief USART1 configuration data
*/
typedef struct {
  uint32_t baud_rate; /*!< Baud rate must be > 0 and <= 10,000,000 */
  uint8_t idle_level; /*!< Indicates if the UART idle logic level is low or high
                            0 - Idle logic level is low
                            1 - Idle logic level is high */
  uint8_t parity;     /*!< Odd, even, or no parity bit
                            0 - No parity bit
                            1 - Odd parity bit
                            2 - Even parity bit */
  uint8_t stop_bits;  /*!< 1 stop bit or 2 stop bits
                            1 - 1 stop bit
                            2 - 2 stop bits */
} usart1_config_t;

// ----------------------------------------------------------------------
// CONSTANTS
// ----------------------------------------------------------------------

extern const usart1_config_t usart1_default_config;

// ----------------------------------------------------------------------
// FUNCTIONS
// ----------------------------------------------------------------------

void usart1_init(const usart1_config_t *config_data);
void usart1_reconfig(const usart1_config_t *config_data);

uint8_t usart1_rx_fifo_empty(void);
uint8_t usart1_read_rx_fifo(void);

#ifdef __cplusplus
}
#endif

#endif