#include "spi1.h"
#include "stm32g431xx.h"
#include "stm32g4xx.h"
#include <stdint.h>

/*
    GPIO configuration defines
*/
#define GPIO_PA5_AF5_SPI1_SCK   (0x5UL << GPIO_AFRL_AFSEL5_Pos)
#define GPIO_PA6_AF5_SPI1_MISO  (0x5UL << GPIO_AFRL_AFSEL6_Pos)
#define GPIO_PA7_AF5_SPI1_MOSI  (0x5UL << GPIO_AFRL_AFSEL7_Pos)

#define GPIO_PA4_GPO_MODE       (0x1UL << GPIO_MODER_MODE4_Pos)
#define GPIO_PA5_AF_MODE        (0x2UL << GPIO_MODER_MODE5_Pos)
#define GPIO_PA6_AF_MODE        (0x2UL << GPIO_MODER_MODE6_Pos)
#define GPIO_PA7_AF_MODE        (0x2UL << GPIO_MODER_MODE7_Pos)

#define SPI_BR_CLK_DIV_16       (0x3UL << SPI_CR1_BR_Pos)

// ----------------------------------------------------------------------
// PRIVATE FUNCTIONS
// ----------------------------------------------------------------------

/*!
    @brief Configure GPIO pins PA4-PA7 for use by the SPI1 peripheral
    @details Pins PA5-PA7 are controlled directly by the hardware but pin PA4
    is controlled by the software because the hardware control is finnicky
*/
static void configure_pins(void) {
    /* Set alternate function for PA5-PA7 as SPI1 pins */
    SET_BIT(
        GPIOA->AFR[0],
        GPIO_PA5_AF5_SPI1_SCK |
        GPIO_PA6_AF5_SPI1_MISO |
        GPIO_PA7_AF5_SPI1_MOSI
    );

    /* Set PA5-PA7 in alternate function mode */
    MODIFY_REG(
        GPIOA->MODER,
        GPIO_MODER_MODE4 |
        GPIO_MODER_MODE5 |
        GPIO_MODER_MODE6 |
        GPIO_MODER_MODE7,
        GPIO_PA4_GPO_MODE |
        GPIO_PA5_AF_MODE |
        GPIO_PA6_AF_MODE |
        GPIO_PA7_AF_MODE
    );

    /* SPI NSS pin is active-low so initialize it high */
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS4);
}

/*!
    @brief Configure the SPI1 peripheral in master mode at 10MHz
*/
static void configure_spi1(void) {
    /* Configure SPI_CR1 register */
    SET_BIT(
        SPI1->CR1,
        SPI_CR1_SSM |       /* Chip select is controlled by software */
        SPI_CR1_SSI |       /* If this isn't set, the RX FIFO empty flag will
                                never go low for some reason */
        SPI_BR_CLK_DIV_16 | /* Set baud rate to 10Mbps */
        SPI_CR1_MSTR        /* Set SPI in master mode */
    );

    /* Configure SPI_CR2 register */
    SET_BIT(
        SPI1->CR2,
        SPI_CR2_FRXTH /* Set FIFO not empty event to happen when 1/4 full */
    );
}

// ----------------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------------

/*!
    @brief Initialize the SPI1 peripheral and pins
*/
void spi1_init(void) {
    configure_pins();
    configure_spi1();
}

void test(void) {
    uint8_t tmp1[2];
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR4);
    SET_BIT(SPI1->CR1, SPI_CR1_SPE);

    while ((SPI1->SR & SPI_SR_TXE_Msk) == 0);
    const uint8_t addr = 0x80 | 0x75;
    *(volatile uint8_t*) &SPI1->DR = addr;

    while ((SPI1->SR & SPI_SR_RXNE_Msk) == 0);
    tmp1[0] = *(volatile uint8_t*) &SPI1->DR;

    while ((SPI1->SR & SPI_SR_TXE_Msk) == 0);
    *(volatile uint8_t*) &SPI1->DR = 0xFF;

    while ((SPI1->SR & SPI_SR_RXNE_Msk) == 0);
    tmp1[1] = *(volatile uint8_t*) &SPI1->DR;

    while ((SPI1->SR & SPI_SR_FTLVL_Msk) != 0);
    while ((SPI1->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY);
    CLEAR_BIT(SPI1->CR1, SPI_CR1_SPE);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS4);

    uint8_t nothing = tmp1[0];
    uint8_t data = tmp1[1];
}