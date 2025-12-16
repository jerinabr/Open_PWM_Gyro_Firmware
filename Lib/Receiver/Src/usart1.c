#include "usart1.h"
#include "stm32g431xx.h"
#include "stm32g4xx.h"
#include <math.h>
#include <stdint.h>

#define GPIO_PA9_AF7_USART1     (0x7UL << GPIO_AFRH_AFSEL9_Pos)
#define GPIO_PA9_AF_MODE        (0x2UL << GPIO_MODER_MODE9_Pos)

#define GPIO_PA10_AF7_USART1    (0x7UL << GPIO_AFRH_AFSEL10_Pos)
#define GPIO_PA10_AF_MODE       (0x2UL << GPIO_MODER_MODE10_Pos)

// Prescaler=1 so USART1 clock = APB2 clock
#define USART1_KER_CK_PRES      (160E6f)

// ----------------------------------------------------------------------
// CONSTANTS
// ----------------------------------------------------------------------

const usart_config usart1_default_config = {
    .baud_rate = 115200,
    .idle_level = 1
};

/*
    MIN_BAUD_RATE is calculated by dividing the USART1 clock by the largest
    value possible for the BRR register which is 160e6 / 65535 ~= 2441.4
    We round this up to 2442 by adding 1 for int truncation
    This would give a BRR value of 65520

    MAX_BAUD_RATE is calculated by dividing the USART1 clock by the oversampling
    rate of the USART which is 160e6 / 16 = 10000000
*/
const uint32_t MIN_BAUD_RATE = USART1_KER_CK_PRES / 65535 + 1;
const uint32_t MAX_BAUD_RATE = USART1_KER_CK_PRES / 16;

// ----------------------------------------------------------------------
// PRIVATE FUNCTIONS
// ----------------------------------------------------------------------

/*!
    @brief Calculate the USART_BRR register value needed for a given baud rate
    @return 16-bit value to use in BRR register for the given baud rate

    This baud rate calculation is only applicable with a 16x oversampling rate
    (OVER8=0)
*/
static inline uint16_t calculate_brr(uint32_t baud) {
    return (uint16_t) roundf(USART1_KER_CK_PRES / baud) & 0xFFFF;
}

/*!
    @brief Configure GPIO pins PA9 and PA10 to be used by USART1
*/
static void configure_pins(void) {
    // Set alternate function for PA9 and PA10 as USART1 pins
    SET_BIT(
        GPIOA->AFR[1],
        GPIO_PA9_AF7_USART1 | GPIO_PA10_AF7_USART1
    );
    // Set PA9 and PA10 into alternate function mode
    MODIFY_REG(
        GPIOA->MODER,
        GPIO_MODER_MODE9 | GPIO_MODER_MODE10,
        GPIO_PA9_AF_MODE | GPIO_PA10_AF_MODE
    );
}

// ----------------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------------

/*!
    @brief Initialize the USART1 peripheral with specified configuration
*/
void usart1_init(const usart_config *config_data) {
    configure_pins();
    usart1_reconfig(config_data);
}

/*!
    @brief Reconfigure USART1 with the specified configuration
*/
void usart1_reconfig(const usart_config *config_data) {
    // Flush the receive FIFO before reconfiguration
    // Give tmp an "unused" attribute to avoid unused variable compiler warning
    uint8_t __attribute__((unused)) tmp;
    while (!usart1_rx_fifo_empty()) {
        tmp = usart1_read_rx_fifo();
    }

    // Disable USART1 before reconfiguration
    CLEAR_BIT(USART1->CR1, USART_CR1_UE);

    /*
        Validate specified configuration
    */
    const usart_config *selected_config;
    if (
        config_data->baud_rate >= MIN_BAUD_RATE &&
        config_data->baud_rate <= MAX_BAUD_RATE &&
        config_data->idle_level <= 1
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
    uint16_t brr_val = calculate_brr(selected_config->baud_rate);
    uint8_t rxinv_val = selected_config->idle_level == 0 ? 1 : 0;

    /*
        Configure USART1 registers
    */
    MODIFY_REG( // Set baud rate
        USART1->BRR,
        USART_BRR_BRR_Msk,
        brr_val
    );
    MODIFY_REG( // Set rx inverted control
        USART1->CR2,
        USART_CR2_RXINV_Msk,
        rxinv_val << USART_CR2_RXINV_Pos
    );
    SET_BIT(USART1->CR1,    USART_CR1_FIFOEN |  // Enable FIFOs
                            USART_CR1_RE |      // Enable receiver
                            USART_CR1_UE);      // Enable USART
}

/*!
    @brief Check if the RX FIFO is empty
    @return 1 if RX FIFO is empty, 0 otherwise
*/
uint8_t usart1_rx_fifo_empty() {
    return (READ_BIT(USART1->ISR, USART_ISR_RXNE_RXFNE) == 0x0);
}

/*!
    @brief Read a byte from the RX FIFO
    @return Byte from RX FIFO
*/
uint8_t usart1_read_rx_fifo() {
    return (uint8_t) (READ_REG(USART1->RDR) & 0xFF);
}