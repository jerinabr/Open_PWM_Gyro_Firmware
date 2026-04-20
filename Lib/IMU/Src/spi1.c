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
static void configure_spi1(uint8_t polarity, uint8_t phase, uint8_t baud_rate) {
    /* Determine CR1 register bits */
    const uint32_t CPOL = (polarity & 0x1) << SPI_CR1_CPOL_Pos;
    const uint32_t CPHA = (phase & 0x1) << SPI_CR1_CPHA_Pos;
    const uint32_t BR_VAL = (baud_rate & 0x7) << SPI_CR1_BR_Pos;

    /* Configure SPI_CR1 register */
    SET_BIT(
        SPI1->CR1,
        SPI_CR1_SSM |       /* Chip select is controlled by software */
        SPI_CR1_SSI |       /* If this isn't set, the RX FIFO empty flag will
                                never go low for some reason */
        BR_VAL |            /* Set baud rate */
        SPI_CR1_MSTR |      /* Set SPI in master mode */
        CPOL |              /* Configure polarity */
        CPHA                /* Configure phase */
    );

    /* Configure SPI_CR2 register */
    SET_BIT(
        SPI1->CR2,
        SPI_CR2_FRXTH /* Set FIFO not empty event to happen when 1/4 full */
    );

    /* Enable the SPI peripheral */
    SET_BIT(SPI1->CR1, SPI_CR1_SPE);
}

/*!
    @brief Configure the DMAMUX and DMA1 peripherals to enable SPI transactions
    through DMA
*/
static void configure_dma(void) {
    /* 
        Configure SPI1 TX/RX DMA channels

        - Set the peripheral address register to the SPI1 data register address
        - Configure the channel configuration register with the desired
            parameters
        
        The DMA channels won't be enabled yet because the channel memory address
        and data transfer count will be updated at the start of every SPI
        transaction.

        The reference manual recommends writing the memory address register and
        data transfer count register as part of the DMA channel configuration
        sequence, but since the channel doesn't get enabled until the start of a
        SPI transaction, it's probably fine...
    */
    WRITE_REG(SPI1_TX_DMA_CHANNEL->CPAR, (uint32_t) &SPI1->DR);
    WRITE_REG(
        SPI1_TX_DMA_CHANNEL->CCR,
        DMA_PL_SPI1_TX |    /* Set the DMA channel priority level */
        DMA_CCR_MINC |      /* Enable memory increment mode */
        DMA_CCR_DIR |       /* Set the direction as memory-to-peripheral */
        DMA_CCR_TCIE        /* Enable the transfer complete interrupt */
    );

    WRITE_REG(SPI1_RX_DMA_CHANNEL->CPAR, (uint32_t) &SPI1->DR);
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
void spi1_init(uint8_t polarity, uint8_t phase, uint8_t baud_rate) {
    configure_pins();
    configure_spi1(polarity, phase, baud_rate);
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
    @param tx_buf Transmit data buffer
    @param rx_buf Receive data buffer
    @param num_bytes Number of bytes to be sent/received
    @details The DMA is used to transmit/receive data to/from the SPI1
    peripheral. This ensures that the data will be a continuous stream and the
    transaction will complete as fast as possible.

    The SPI is kept enabled during the entire transaction and only the DMA and
    chip-select are enabled/disabled.
    
    This function will block the processor until the transaction is
    completed.
*/
void spi1_transact_data(
    uint8_t tx_buf[],
    volatile uint8_t rx_buf[],
    uint32_t num_bytes
) {
    /*
        Configure and enable DMA

        Before enabling the DMA, we need to write the data transfer count and
        memory address registers of the TX and RX DMA channels to reflect the
        function arguments.

        The reference manual gives an order of operations for starting SPI
        communication using DMA:
            1. Set the RXDMAEN bit in the SPI_CR2 register
            2. Enable the SPI1 TX and RX DMA channels
            3. Set the TXDMAEN bit in the SPI_CR2 register
    */
    WRITE_REG(SPI1_TX_DMA_CHANNEL->CMAR, (uint32_t) tx_buf);
    WRITE_REG(SPI1_TX_DMA_CHANNEL->CNDTR, num_bytes);

    WRITE_REG(SPI1_RX_DMA_CHANNEL->CMAR, (uint32_t) rx_buf);
    WRITE_REG(SPI1_RX_DMA_CHANNEL->CNDTR, num_bytes);

    /* Enable chip-select */
    SET_BIT(GPIOA->BSRR, SPI_NSS_ENABLE);

    /* Reset DMA transfer status flags */
    tx_dma_transfer_complete = 0;
    rx_dma_transfer_complete = 0;

    /* Enable SPI DMA */
    SET_BIT(SPI1->CR2, SPI_CR2_RXDMAEN);
    SET_BIT(SPI1_TX_DMA_CHANNEL->CCR, DMA_CCR_EN);
    SET_BIT(SPI1_RX_DMA_CHANNEL->CCR, DMA_CCR_EN);
    SET_BIT(SPI1->CR2, SPI_CR2_TXDMAEN);

    /* Disable DMA channels after transfers are completed */
    while (!tx_dma_transfer_complete || !rx_dma_transfer_complete);
    CLEAR_BIT(SPI1_TX_DMA_CHANNEL->CCR, DMA_CCR_EN);
    CLEAR_BIT(SPI1_RX_DMA_CHANNEL->CCR, DMA_CCR_EN);

    /* Disable SPI DMA requests */
    CLEAR_BIT(
        SPI1->CR2,
        SPI_CR2_TXDMAEN |
        SPI_CR2_RXDMAEN
    );

    /* Disable chip-select after busy flag goes low and TX FIFO is empty */
    while (SPI1->SR & (SPI_SR_BSY_Msk | SPI_SR_FTLVL_Msk));
    SET_BIT(GPIOA->BSRR, SPI_NSS_DISABLE);
}

/***********************************************************************
-- INTERRUPT HANDLERS --
***********************************************************************/

/*!
    @brief Set TX buffer transfer complete flag when the SPI1 TX DMA channel
    completes the transfer
*/
void SPI1_TX_DMA_IRQ(void) {
    if (DMA1->ISR & SPI1_TX_DMA_TCIF) {
        tx_dma_transfer_complete = 1;
        SET_BIT(DMA1->IFCR, SPI1_TX_DMA_CTCIF);
    }
}

/*!
    @brief Set RX buffer transfer complete flag when the SPI1 RX DMA channel
    completes the transfer
*/
void SPI1_RX_DMA_IRQ(void) {
    if (DMA1->ISR & SPI1_RX_DMA_TCIF) {
        rx_dma_transfer_complete = 1;
        SET_BIT(DMA1->IFCR, SPI1_RX_DMA_CTCIF);
    }
}