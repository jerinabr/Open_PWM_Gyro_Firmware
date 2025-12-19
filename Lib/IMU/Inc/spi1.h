/*!
    @file   spi1.h
    @brief  Configure the SPI1 peripheral to run at 10MHz in master mode

    --SPI PINOUT--
    SPI1_NSS  -> PA4 (SW controlled)
    SPI1_SCK  -> PA5
    SPI1_MISO -> PA6
    SPI1_MOSI -> PA7

    The pins are controlled by the hardware except for pin PA4

    This is because the hardware would keep the NSS pin held low as long as
    the SPI peripheral is enabled and in master mode, OR it would pulse NSS
    after every byte transaction

    We want to be able to send variable-length data frames with the NSS only
    held low during the transaction, so we control the NSS pin via software
*/
#ifndef SPI1_H
#define SPI1_H

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------
// FUNCTIONS
// ----------------------------------------------------------------------

void spi1_init(void);

#ifdef __cplusplus
}
#endif

#endif