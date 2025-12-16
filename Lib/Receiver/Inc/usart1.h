/*!
    @file   usart1.h
    @brief  Configure and initialize the USART1 peripheral

    This library initializes the USART1 peripheral with a specified
    configuration, no parity bit, and 1 stop bit

    If the given configuration doesn't pass a validation check, then the default
    configuration is used to initialize the USART

    The USART can also be reconfigured with a new configuration at any point

    The USART uses a FIFO to buffer RX data. To read the data from the FIFO, the
    FIFO has to first be checked to see if it has data in it. If it does, then
    the FIFO read function can be called

    If the FIFO buffers too much data, then it will error and will probably need
    to be flushed I think before it can be read again
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
  @brief USART configuration data
*/
typedef struct {
    /*!< Baud rate must be > 0 and <= 10,000,000 */
    uint32_t baud_rate;

    /*!< Indicates if the UART idle logic level is low or high
        0 - Idle logic level is low
        1 - Idle logic level is high */
    uint8_t idle_level;
} usart_config;

// ----------------------------------------------------------------------
// CONSTANTS
// ----------------------------------------------------------------------

extern const usart_config usart1_default_config;

// ----------------------------------------------------------------------
// FUNCTIONS
// ----------------------------------------------------------------------

void usart1_init(const usart_config *config_data);
void usart1_reconfig(const usart_config *config_data);

uint8_t usart1_rx_fifo_empty(void);
uint8_t usart1_read_rx_fifo(void);

#ifdef __cplusplus
}
#endif

#endif