#include "spi1.h"
#include "stm32g431xx.h"
#include "stm32g4xx.h"
#include <stdint.h>

/*
    GPIO configuration defines
*/
#define GPIO_PA4_AF5_SPI1_NSS   (0x5UL << GPIO_AFRL_AFSEL4_Pos)
#define GPIO_PA5_AF5_SPI1_SCK   (0x5UL << GPIO_AFRL_AFSEL5_Pos)
#define GPIO_PA6_AF5_SPI1_MISO  (0x5UL << GPIO_AFRL_AFSEL6_Pos)
#define GPIO_PA7_AF5_SPI1_MOSI  (0x5UL << GPIO_AFRL_AFSEL7_Pos)

#define GPIO_PA4_GPO_MODE       (0x1UL << GPIO_MODER_MODE4_Pos)
#define GPIO_PA4_AF_MODE        (0x2UL << GPIO_MODER_MODE4_Pos)
#define GPIO_PA5_AF_MODE        (0x2UL << GPIO_MODER_MODE5_Pos)
#define GPIO_PA6_AF_MODE        (0x2UL << GPIO_MODER_MODE6_Pos)
#define GPIO_PA7_AF_MODE        (0x2UL << GPIO_MODER_MODE7_Pos)

// ----------------------------------------------------------------------
// PRIVATE FUNCTIONS
// ----------------------------------------------------------------------

/*!
    @brief Configure GPIO pins PA4-PA7 as SPI1 pins
    @details Pins PA5-PA7 are controlled directly by the hardware but pin PA4
    is controlled by the software to allow a finer control over the SPI frames
*/
static void configure_pins(void) {
    /*
        Configure pins PA5-PA7 for use by SPI1 peripheral
    */
    // Set alternate function for PA4-PA7 as SPI1 pins
    SET_BIT(
        GPIOA->AFR[0],
        GPIO_PA4_AF5_SPI1_NSS |
        GPIO_PA5_AF5_SPI1_SCK |
        GPIO_PA6_AF5_SPI1_MISO |
        GPIO_PA7_AF5_SPI1_MOSI
    );
    // Set PA4-PA7 in alternate function mode
    MODIFY_REG(
        GPIOA->MODER,
        GPIO_MODER_MODE4 |
        GPIO_MODER_MODE5 |
        GPIO_MODER_MODE6 |
        GPIO_MODER_MODE7,
        GPIO_PA4_AF_MODE |
        GPIO_PA5_AF_MODE |
        GPIO_PA6_AF_MODE |
        GPIO_PA7_AF_MODE
    );
}

static void configure_spi1(void) {
    /*
        Configure SPI_CR1 register
    */
    SET_BIT(
        SPI1->CR1,
        (0b011 << SPI_CR1_BR_Pos) | // Set baud rate to 10Mbps
        SPI_CR1_MSTR |              // Set SPI in master mode
        SPI_CR1_CPOL |              // Set clock level to '1' when idle
        SPI_CR1_CPHA                // Latch data on rising edge
    );

    /*
        Configure SPI_CR2 register
    */
    SET_BIT(
        SPI1->CR2,
        SPI_CR2_SSOE    // Enable slave select output
    );
}

// ----------------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------------

void spi1_init(void) {
    configure_pins();
    configure_spi1();
}