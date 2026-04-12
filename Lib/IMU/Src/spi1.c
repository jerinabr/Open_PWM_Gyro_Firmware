#include "spi1.h"
#include "hw_config.h"
#include "stm32g431xx.h"
#include "stm32g4xx.h"
#include <stdint.h>

/*
    GPIO configuration
*/
#define GPIO_PA5_AF5_SPI1_SCK   (0x5UL << GPIO_AFRL_AFSEL5_Pos)
#define GPIO_PA6_AF5_SPI1_MISO  (0x5UL << GPIO_AFRL_AFSEL6_Pos)
#define GPIO_PA7_AF5_SPI1_MOSI  (0x5UL << GPIO_AFRL_AFSEL7_Pos)

#define GPIO_PA4_GPO_MODE       (0x1UL << GPIO_MODER_MODE4_Pos)
#define GPIO_PA5_AF_MODE        (0x2UL << GPIO_MODER_MODE5_Pos)
#define GPIO_PA6_AF_MODE        (0x2UL << GPIO_MODER_MODE6_Pos)
#define GPIO_PA7_AF_MODE        (0x2UL << GPIO_MODER_MODE7_Pos)

#define SPI_NSS_ENABLE          GPIO_BSRR_BR4
#define SPI_NSS_DISABLE         GPIO_BSRR_BS4
/*
    SPI configuration
*/
#define SPI_BR_CLK_DIV_16       (0x3UL << SPI_CR1_BR_Pos)
/*
    DMA configuration
*/
#define DMA_PL_SPI1_TX          (SPI1_TX_DMA_PRIORITY << DMA_CCR_PL_Pos)
#define DMA_PL_SPI1_RX          (SPI1_RX_DMA_PRIORITY << DMA_CCR_PL_Pos)

#define DMAMUX_INPUT_SPI1_TX    (11 << DMAMUX_CxCR_DMAREQ_ID_Pos)
#define DMAMUX_INPUT_SPI1_RX    (10 << DMAMUX_CxCR_DMAREQ_ID_Pos)

/* Initialize the spi1 instance */
spi1_s spi1_buf = {
    .tx_buf = {0},
    .rx_buf = {0}
};

volatile uint8_t tx_dma_transfer_complete = 0;
volatile uint8_t rx_dma_transfer_complete = 0;

/***********************************************************************
-- PRIVATE FUNCTIONS --
***********************************************************************/

/*!
    @brief Configure GPIO pins PA4-PA7 for use by the SPI1 peripheral
    @details Pins PA5-PA7 are controlled directly by the hardware but pin PA4
    (NSS) is controlled by the software because the hardware control is finicky
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
    SET_BIT(GPIOA->BSRR, SPI_NSS_DISABLE);
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

/*!
    @brief Configure the DMAMUX and DMA1 peripherals to enable SPI transactions
    through DMA
*/
static void configure_dma(void) {
    /* 
        Configure SPI1 TX/RX DMA channels

        - Set the peripheral address register to the SPI1 data register address
        - Set the memory address register to the spi1_buf TX/RX buffer address
        - Set the total number of data to transfer to the max buffer length
            (this will be overwritten at the start of every SPI transaction)
        - Configure the channel configuration register with the desired
            parameters
        
        The DMA channels won't be enabled yet because the data transfer count
        can't be updated when the DMA is enabled. This count is dynamic so the
        DMA channels are only enabled at the start of a SPI transaction and are
        disabled when it's completed.
    */
    WRITE_REG(SPI1_TX_DMA_CHANNEL->CPAR, (uint32_t) &SPI1->DR);
    WRITE_REG(SPI1_TX_DMA_CHANNEL->CMAR, (uint32_t) &spi1_buf.tx_buf);
    WRITE_REG(SPI1_TX_DMA_CHANNEL->CNDTR, (uint32_t) BUF_LEN);
    WRITE_REG(
        SPI1_TX_DMA_CHANNEL->CCR,
        DMA_PL_SPI1_TX |    /* Set the DMA channel priority level */
        DMA_CCR_MINC |      /* Enable memory increment mode */
        DMA_CCR_DIR |       /* Set the direction as memory-to-peripheral */
        DMA_CCR_TCIE        /* Enable the transfer complete interrupt */
    );

    WRITE_REG(SPI1_RX_DMA_CHANNEL->CPAR, (uint32_t) &SPI1->DR);
    WRITE_REG(SPI1_RX_DMA_CHANNEL->CMAR, (uint32_t) &spi1_buf.rx_buf);
    WRITE_REG(SPI1_RX_DMA_CHANNEL->CNDTR, (uint32_t) BUF_LEN);
    WRITE_REG(
        SPI1_RX_DMA_CHANNEL->CCR,
        DMA_PL_SPI1_RX |    /* Set the DMA channel priority level */
        DMA_CCR_MINC |      /* Enable memory increment mode */
        DMA_CCR_TCIE        /* Enable the transfer complete interrupt */
    );

    /* Route the SPI1 TX/RX DMA requests to the DMAMUX */
    WRITE_REG(SPI1_TX_DMAMUX_CHANNEL->CCR, DMAMUX_INPUT_SPI1_TX);
    WRITE_REG(SPI1_RX_DMAMUX_CHANNEL->CCR, DMAMUX_INPUT_SPI1_RX);
}

/***********************************************************************
-- PUBLIC FUNCTIONS --
***********************************************************************/

/*!
    @brief Initialize the SPI1 peripheral and DMA interrupts
*/
void spi1_init(void) {
    configure_pins();
    configure_spi1();
    configure_dma();
    
    NVIC_SetPriority(
        SPI1_TX_DMA_IRQn,
        SPI1_TX_DMA_IRQ_PRIORITY)
    ;
    NVIC_EnableIRQ(SPI1_TX_DMA_IRQn);

    NVIC_SetPriority(
        SPI1_RX_DMA_IRQn,
        SPI1_RX_DMA_IRQ_PRIORITY
    );
    NVIC_EnableIRQ(SPI1_RX_DMA_IRQn);
}

/*!
    @brief Send data in tx_buf and receive data in rx_buf using DMA
    @param num_bytes Number of bytes to be sent/received
    @details This function will block the processor until the transaction is
    completed.
    
    The data to be transmitted must be loaded into spi1_buf.tx_buf
    before this function is called.

    Once the transaction completes, the received data will be in
    spi1_buf.rx_buf.
*/
void spi1_transact_data(uint32_t num_bytes) {
    /*
        Configure and enable DMA

        The reference manual gives an order of operations for starting SPI
        communication using DMA

        1. Modify the data transfer count registers for the TX and RX DMA
            channels to be num_words
        2. Set the RXDMAEN bit in the SPI_CR2 register
        3. Enable the SPI1 TX and RX DMA channels
        4. Set the TXDMAEN bit in the SPI_CR2 register
    */
    WRITE_REG(SPI1_TX_DMA_CHANNEL->CNDTR, num_bytes);
    WRITE_REG(SPI1_RX_DMA_CHANNEL->CNDTR, num_bytes);

    SET_BIT(SPI1->CR2, SPI_CR2_RXDMAEN);
    SET_BIT(SPI1_TX_DMA_CHANNEL->CCR, DMA_CCR_EN);
    SET_BIT(SPI1_RX_DMA_CHANNEL->CCR, DMA_CCR_EN);
    SET_BIT(SPI1->CR2, SPI_CR2_TXDMAEN);

    /* Enable SPI transaction */
    SET_BIT(GPIOA->BSRR, SPI_NSS_ENABLE);
    SET_BIT(SPI1->CR1, SPI_CR1_SPE);

    /* Disable the DMA channels after the transfers are completed */
    while (!tx_dma_transfer_complete || !rx_dma_transfer_complete);
    CLEAR_BIT(SPI1_TX_DMA_CHANNEL->CCR, DMA_CCR_EN);
    CLEAR_BIT(SPI1_RX_DMA_CHANNEL->CCR, DMA_CCR_EN);

    /* Disable SPI after the transaction is completed */
    while (SPI1->SR & (SPI_SR_BSY_Msk | SPI_SR_FTLVL_Msk));
    SET_BIT(GPIOA->BSRR, SPI_NSS_DISABLE);
    CLEAR_BIT(SPI1->CR1, SPI_CR1_SPE);

    /* Disable SPI DMA */
    CLEAR_BIT(
        SPI1->CR2,
        SPI_CR2_TXDMAEN |
        SPI_CR2_RXDMAEN
    );
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

/***********************************************************************
-- INTERRUPT HANDLERS --
***********************************************************************/

/*!
    @brief Set TX buffer transfer complete flag
*/
void SPI1_TX_DMA_IRQ(void) {
    if (DMA1->ISR & SPI1_TX_DMA_TCIF) {
        tx_dma_transfer_complete = 1;
        SET_BIT(DMA1->IFCR, SPI1_TX_DMA_CTCIF);
    }
}

/*!
    @brief Set RX buffer transfer complete flag
*/
void SPI1_RX_DMA_IRQ(void) {
    if (DMA1->ISR & SPI1_RX_DMA_TCIF) {
        rx_dma_transfer_complete = 1;
        SET_BIT(DMA1->IFCR, SPI1_RX_DMA_CTCIF);
    }
}