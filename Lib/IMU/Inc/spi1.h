/*!
    @file   spi1.h
    @brief  Configure the SPI1 peripheral to run at 10MHz in master mode

    --SPI PINOUT--
    SPI1_NSS  -> PA4 (SW controlled)
    SPI1_SCK  -> PA5
    SPI1_MISO -> PA6
    SPI1_MOSI -> PA7

    The pins are controlled by the hardware except for NSS (pin PA4).

    This is because the hardware will keep the NSS pin held low as long as the
    SPI peripheral is enabled and in master mode, OR it will pulse NSS after
    every byte transaction.

    We want to be able to send variable-length data frames with the NSS only
    held low during the transaction, so we control the NSS pin via software.

    The SPI transactions are done using DMA to ensure a continuous data stream
    and the fastest transaction.

    NOTE: The SPI is kept enabled from initialization instead of being enabled
    at the start of a transaction and disabled at the end of it.
    
    This is because the STM32 releases the GPIO lines when the SPI is disabled
    so if the clock polarity is 1, the SCLK line will still be low until the
    interface is enabled.
    
    This is problematic because if the TX FIFO is written before the peripheral
    is enabled, then once it is enabled the clock lines goes high before going
    back low. This timing can cause unexpected behavior from the SPI device if
    the chip-select line isn't enabled right after the low-to-high transition.
*/
#ifndef SPI1_H
#define SPI1_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
-- FUNCTIONS --
***********************************************************************/

void spi1_init(
    uint8_t polarity,
    uint8_t phase,
    uint8_t baud_rate
);

void spi1_transact_data(
    uint8_t tx_buf[],
    volatile uint8_t rx_buf[],
    uint32_t num_bytes
);

#ifdef __cplusplus
}
#endif

#endif