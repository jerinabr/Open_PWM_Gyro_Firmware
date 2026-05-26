#include "receiver.h"
#include "ibus.h"
#include "usart1.h"
#include <stdint.h>

/* Function pointer to the selected protocol decoder function */
static uint8_t (*decode_rx_data)(uint8_t data, uint16_t channel_data[]);

/***********************************************************************
-- PRIVATE FUNCTIONS --
***********************************************************************/

/*!
    @brief Dummy function that returns RX data invalid
    @details This is just here till I implement all the receiver protocols
*/
static uint8_t dummy_decode_rx_data(uint8_t data, uint16_t channel_data[]) {
    return 0;
}

/*!
    @brief Configure the USART and assign the decoder function pointer
    @param rx_protocol Enum that defines the RX protocol to be used
    @param usart_config Pointer to USART config struct that will be updated with
    the new configuration
    @details This function will select the appropriate USART configuration and
    reassign the rx decoder function pointer based on the desired protocol
*/
static void select_rx_config(
    RX_PROTOCOLS rx_protocol,
    struct USART1_Config *usart_config
) {
    switch (rx_protocol) {
        case IBUS: {
            usart_config->baud_rate = IBUS_BAUD_RATE;
            usart_config->idle_level = IBUS_IDLE_LEVEL;
            decode_rx_data = parse_ibus_data;
            reset_ibus();
            break;
        }
        case CRSF: {
            /* TODO: Implement this eventually */
            usart_config->baud_rate = USART1_DEFAULT_BAUD_RATE;
            usart_config->idle_level = USART1_DEFAULT_IDLE_LEVEL;
            decode_rx_data = dummy_decode_rx_data;
            break;
        }
        default: {
            usart_config->baud_rate = USART1_DEFAULT_BAUD_RATE;
            usart_config->idle_level = USART1_DEFAULT_IDLE_LEVEL;
            decode_rx_data = dummy_decode_rx_data;
            break;
        }
    }
}

/***********************************************************************
-- PUBLIC FUNCTIONS --
***********************************************************************/

/*!
    @brief Initialize the reciever with a specific protocol
*/
void receiver_init(RX_PROTOCOLS rx_protocol) {
    struct USART1_Config usart_config;
    select_rx_config(rx_protocol, &usart_config);
    usart1_init(&usart_config);
}

/*!
    @brief Reconfigure the receiver with a specific protocol
*/
void receiver_reconfig(RX_PROTOCOLS rx_protocol) {
    struct USART1_Config usart_config;
    select_rx_config(rx_protocol, &usart_config);
    usart1_reconfig(&usart_config);
}

/*!
    @brief Read the RX FIFO and parse the data
    @param channel_data Array that will contain the decoded channel data from
    the receiver
    @return 1 if data valid, 0 if not
    @details This function should be called in the main program loop
*/
uint8_t read_receiver(uint16_t channel_data[]) {
    if (!usart1_rx_fifo_empty()) {
        uint8_t data = usart1_read_rx_fifo();
        return decode_rx_data(data, channel_data);
    }
    return 0;
}