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
    GPIO Configuration

    All the peripheral pins and their configurations are defined here
*/
/* Receiver UART */
#define GPIO_PA9_AF_USART1_TX   (0x7UL << GPIO_AFRH_AFSEL9_Pos)
#define GPIO_PA10_AF_USART1_RX  (0x7UL << GPIO_AFRH_AFSEL10_Pos)

#define GPIO_PA9_AF_MODE        (0x2UL << GPIO_MODER_MODE9_Pos)
#define GPIO_PA10_AF_MODE       (0x2UL << GPIO_MODER_MODE10_Pos)

/* IMU SPI */
#define GPIO_PA5_AF_SPI1_SCK    (0x5UL << GPIO_AFRL_AFSEL5_Pos)
#define GPIO_PA6_AF_SPI1_MISO   (0x5UL << GPIO_AFRL_AFSEL6_Pos)
#define GPIO_PA7_AF_SPI1_MOSI   (0x5UL << GPIO_AFRL_AFSEL7_Pos)

#define GPIO_PA4_GPO_MODE       (0x1UL << GPIO_MODER_MODE4_Pos)
#define GPIO_PA5_AF_MODE        (0x2UL << GPIO_MODER_MODE5_Pos)
#define GPIO_PA6_AF_MODE        (0x2UL << GPIO_MODER_MODE6_Pos)
#define GPIO_PA7_AF_MODE        (0x2UL << GPIO_MODER_MODE7_Pos)

#define GPIO_PA5_OSPEEDR_MS     (0x1UL << GPIO_OSPEEDR_OSPEED5_Pos)
#define GPIO_PA7_OSPEEDR_MS     (0x1UL << GPIO_OSPEEDR_OSPEED7_Pos)

/* IMU INT1 */
#define GPIO_PB0_INPUT_MODE     (0x0UL << GPIO_MODER_MODE0_Pos)

/* PWM outputs */
#define GPIO_PA0_AF_TIM2_CH1    (0x1UL << GPIO_AFRL_AFSEL0_Pos)
#define GPIO_PA1_AF_TIM2_CH2    (0x1UL << GPIO_AFRL_AFSEL1_Pos)
#define GPIO_PA2_AF_TIM2_CH3    (0x1UL << GPIO_AFRL_AFSEL2_Pos)
#define GPIO_PA3_AF_TIM2_CH4    (0x1UL << GPIO_AFRL_AFSEL3_Pos)
#define GPIO_PB6_AF_TIM4_CH1    (0x2UL << GPIO_AFRL_AFSEL6_Pos)
#define GPIO_PB7_AF_TIM4_CH2    (0x2UL << GPIO_AFRL_AFSEL7_Pos)

#define GPIO_PA0_AF_MODE        (0x2UL << GPIO_MODER_MODE0_Pos)
#define GPIO_PA1_AF_MODE        (0x2UL << GPIO_MODER_MODE1_Pos)
#define GPIO_PA2_AF_MODE        (0x2UL << GPIO_MODER_MODE2_Pos)
#define GPIO_PA3_AF_MODE        (0x2UL << GPIO_MODER_MODE3_Pos)
#define GPIO_PB6_AF_MODE        (0x2UL << GPIO_MODER_MODE6_Pos)
#define GPIO_PB7_AF_MODE        (0x2UL << GPIO_MODER_MODE7_Pos)

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