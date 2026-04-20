/*!
    @file   hw_config.h
    @brief  Definitions and constants for the hardware/peripherals
*/
#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include "stm32g431xx.h"
#include "stm32g4xx.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/*
    DMA Configuration

    DMA channels are 1-indexed but DMAMUX channels are 0-indexed...
    Don't ask me why lol
*/
#define SPI1_RX_DMA_CHANNEL         DMA1_Channel1
#define SPI1_RX_DMAMUX_CHANNEL      DMAMUX1_Channel0
#define SPI1_RX_DMA_PRIORITY        0b11 /* Very high */
#define SPI1_RX_DMA_TCIF            DMA_ISR_TCIF1
#define SPI1_RX_DMA_CTCIF           DMA_IFCR_CTCIF1

#define SPI1_TX_DMA_CHANNEL         DMA1_Channel2
#define SPI1_TX_DMAMUX_CHANNEL      DMAMUX1_Channel1
#define SPI1_TX_DMA_PRIORITY        0b01 /* Medium */
#define SPI1_TX_DMA_TCIF            DMA_ISR_TCIF2
#define SPI1_TX_DMA_CTCIF           DMA_IFCR_CTCIF2

#define USART1_RX_DMA_CHANNEL       DMA1_Channel3
#define USART1_RX_DMAMUX_CHANNEL    DMAMUX1_Channel2
#define USART1_RX_DMA_PRIORITY      0b10 /* High */
#define USART1_RX_DMA_TCIF          DMA_ISR_TCIF3
#define USART1_RX_DMA_CTCIF         DMA_IFCR_CTCIF3

/*
    Interrupt Configuration
*/
#define SPI1_RX_DMA_IRQ             DMA1_Channel1_IRQHandler
#define SPI1_RX_DMA_IRQn            DMA1_Channel1_IRQn
#define SPI1_RX_DMA_IRQ_PRIORITY    15 /* Lowest priority */

#define SPI1_TX_DMA_IRQ             DMA1_Channel2_IRQHandler
#define SPI1_TX_DMA_IRQn            DMA1_Channel2_IRQn
#define SPI1_TX_DMA_IRQ_PRIORITY    15 /* Lowest priority */

#define USART1_RX_DMA_IRQ           DMA1_Channel3_IRQHandler
#define USART1_RX_DMA_IRQn          DMA1_Channel3_IRQn
#define USART1_RX_DMA_IRQ_PRIORITY  15 /* Lowest priority */

#ifdef __cplusplus
}
#endif

#endif