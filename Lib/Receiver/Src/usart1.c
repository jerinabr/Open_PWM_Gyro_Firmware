#include "usart1.h"
#include "stm32g431xx.h"
#include "stm32g4xx.h"
#include <math.h>
#include <stdint.h>

#define USART1_KER_CK_PRES    (160E6f)
#define STARTUP_BAUD          (115200)

#define GPIO_PA9_AF7_USART1   (0x7UL << GPIO_AFRH_AFSEL9_Pos)
#define GPIO_PA9_AF_MODE      (0x2UL << GPIO_MODER_MODE9_Pos)

#define GPIO_PA10_AF7_USART1  (0x7UL << GPIO_AFRH_AFSEL10_Pos)
#define GPIO_PA10_AF_MODE     (0x2UL << GPIO_MODER_MODE10_Pos)

// ----------------------------------------------------------------------
// PRIVATE FUNCTIONS
// ----------------------------------------------------------------------

/*!
  @brief  Calculate the USART_BRR register value needed for a given baud rate
  @return 16-bit value to use in BRR register for the given baud rate

  This baud rate calculation is only applicable when OVER8=0
*/
static inline uint32_t calculate_brr(unsigned int baud) {
  return (uint32_t) roundf(USART1_KER_CK_PRES / baud) & 0xFFFF;
}

/*!
  @brief Configure GPIO pins PA9 and PA10 to be used by USART1
*/
static void configure_pins(void) {
  // Enable clock for GPIO Port B
  SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN);

  // Configure PA9 and PA10 for use by USART1
  SET_BIT(    // Set alternate function for PA9 and PA10 as USART1 pins
    GPIOA->AFR[1],
    GPIO_PA9_AF7_USART1 | GPIO_PA10_AF7_USART1
  );
  MODIFY_REG( // Set PA9 and PA10 into alternate function mode
    GPIOA->MODER,
    GPIO_MODER_MODE9 | GPIO_MODER_MODE10,
    GPIO_PA9_AF_MODE | GPIO_PA10_AF_MODE
  );
}

/*!
  @brief Configure USART1 to receive in FIFO mode
*/
static void configure_usart1(void) {
  // Enable clock for USART1
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);

  // Disable USART1 before configuration
  CLEAR_BIT(USART1->CR1, USART_CR1_UE);

  // Configure USART1
  SET_BIT(USART1->BRR,  calculate_brr(STARTUP_BAUD)); // Set baud rate to default baud rate
  SET_BIT(USART1->CR1,  USART_CR1_FIFOEN |            // Enable FIFOs
                        USART_CR1_RE |                // Enable receiver
                        USART_CR1_UE);                // Enable USART
}
// ----------------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------------

/*!
  @brief Initialize the USART1 peripheral
*/
void usart1_init() {
  configure_pins();
  configure_usart1();
}

/*!
  @brief  Checks if the RX FIFO is empty
  @return 1 if RX FIFO is empty, 0 otherwise
*/
uint8_t usart1_rx_fifo_empty() {
  return (READ_BIT(USART1->ISR, USART_ISR_RXNE_RXFNE) == 0x0);
}

/*!
  @brief  Reads a byte from the RX FIFO
  @return Byte from RX FIFO
*/
uint8_t usart1_read_rx_fifo() {
  return (uint8_t) (READ_REG(USART1->RDR) & 0xFF);
}