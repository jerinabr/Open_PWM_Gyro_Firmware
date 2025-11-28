#include "usart1.h"
#include "stm32g431xx.h"
#include "stm32g4xx.h"
#include <math.h>
#include <stdint.h>

#define USART1_KER_CK_PRES    (160E6f)

#define GPIO_PA9_AF7_USART1   (0x7UL << GPIO_AFRH_AFSEL9_Pos)
#define GPIO_PA9_AF_MODE      (0x2UL << GPIO_MODER_MODE9_Pos)

#define GPIO_PA10_AF7_USART1  (0x7UL << GPIO_AFRH_AFSEL10_Pos)
#define GPIO_PA10_AF_MODE     (0x2UL << GPIO_MODER_MODE10_Pos)

// ----------------------------------------------------------------------
// CONSTANTS
// ----------------------------------------------------------------------

const usart1_config_t usart1_default_config = {
  .baud_rate = 115200,
  .idle_level = 1,
  .parity = 0,
  .stop_bits = 1
};

// ----------------------------------------------------------------------
// PRIVATE FUNCTIONS
// ----------------------------------------------------------------------

/*!
  @brief  Calculate the USART_BRR register value needed for a given baud rate
  @return 16-bit value to use in BRR register for the given baud rate

  This baud rate calculation is only applicable when OVER8=0
*/
static inline uint32_t calculate_brr(uint32_t baud) {
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
static void configure_usart1(const usart1_config_t *config_data) {
  // Enable clock for USART1
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);

  // Configure USART1 with desired configuration
  usart1_reconfig(config_data);
}
// ----------------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------------

/*!
  @brief Initialize the USART1 peripheral with specified configuration
*/
void usart1_init(const usart1_config_t *config_data) {
  configure_pins();
  configure_usart1(config_data);
}

/*!
  @brief Reconfigure USART1 with the specified configuration
*/
void usart1_reconfig(const usart1_config_t *config_data) {
  // Disable USART1 before reconfiguration
  CLEAR_BIT(USART1->CR1, USART_CR1_UE);

  /*
    Validate specified configuration
  */
  const usart1_config_t *selected_config;
  // USART1 is configured to oversample by 16 so max baud rate is USART1_KER_CK_PRES / 16
  if (
    (config_data->baud_rate > 0 && config_data->baud_rate <= (USART1_KER_CK_PRES / 16)) &&
    (config_data->idle_level <= 1) &&
    (config_data->parity <= 2) &&
    (config_data->stop_bits > 0 && config_data->stop_bits <= 2)
  ) {
    selected_config = config_data;
  }
  // If given configuration is invalid then use default configuration
  else {
    selected_config = &usart1_default_config;
  }

  /*
    Select bit fields based on selected configuration
  */
  // Calculate brr value (register BRR)
  uint32_t brr_val = calculate_brr(selected_config->baud_rate);
  // Select rxinv bit field (register CR2)
  uint8_t rxinv_val = selected_config->idle_level == 0 ? 1 : 0;
  // Select parity bit fields (register CR1)
  uint8_t pce_val;
  uint8_t ps_val;
  switch (config_data->parity) {
    case 0:
      pce_val = 0;
      ps_val = 0;
      break;
    case 1:
      pce_val = 1;
      ps_val = 1;
      break;
    case 2:
      pce_val = 1;
      ps_val = 0;
      break;
    default:
      pce_val = 0;
      ps_val = 0;
      break;
  }
  // Select stop bit fields (register CR2)
  uint8_t stop_val;
  switch (config_data->stop_bits) {
    case 1:
      stop_val = 0b00;
      break;
    case 2:
      stop_val = 0b10;
      break;
    default:
      stop_val = 0b00;
      break;
  }

  /*
    Configure USART1 registers
  */
  MODIFY_REG( // Set baud rate
    USART1->BRR,
    USART_BRR_BRR_Msk,
    brr_val
  );
  MODIFY_REG( // Set rx inverted control and stop bits
    USART1->CR2,
    USART_CR2_RXINV_Msk | USART_CR2_STOP_Msk,
    (rxinv_val << USART_CR2_RXINV_Pos) | (stop_val << USART_CR2_STOP_Pos)
  );
  MODIFY_REG( // Set parity control bits
    USART1->CR1,
    USART_CR1_PCE_Msk | USART_CR1_PS_Msk,
    (pce_val << USART_CR1_PCE_Pos) | (ps_val << USART_CR1_PS_Pos)
  );
  SET_BIT(USART1->CR1,  USART_CR1_FIFOEN |  // Enable FIFOs
                        USART_CR1_RE |      // Enable receiver
                        USART_CR1_UE);      // Enable USART
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