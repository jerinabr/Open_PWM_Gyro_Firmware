#include "ibus.h"
#include "receiver.h"
#include <stdint.h>

#define MIN_FRAME_LENGTH    0x04
#define MAX_FRAME_LENGTH    0x20
#define SERVO_CMD_CODE      0x40
#define NON_PAYLOAD_BYTES   4
#define IBUS_BUFFER_LEN     48

static enum {
    WAIT_FOR_LENGTH,
    WAIT_FOR_CMD,
    WAIT_FOR_DATA,
    WAIT_FOR_CHECKSUM_1,
    WAIT_FOR_CHECKSUM_2
} state = WAIT_FOR_LENGTH;

static uint8_t payload_len = 0;
static uint8_t cmd = 0;
static uint16_t checksum_calc = 0;
static uint16_t checksum_rx = 0;

static uint8_t ibus_buf[IBUS_BUFFER_LEN];
static uint8_t ibus_buf_i = 0;

/*!
    @brief Reset the IBus decoder state machine
    @details This is to be called when the RX protocol is changed to IBus
*/
void reset_ibus() {
  state = WAIT_FOR_LENGTH;
}

/*!
    @brief Parse the incoming IBus data and extract channel data
    @details Every IBus frame has a packet length byte, command byte, some
    amount of payload bytes, and 2 bytes for a 16-bit checksum, in that order

    The first state checks to see if the first byte (length byte) is at least 4
    bytes, since the frame has 4 bytes + n payload bytes, and at most 32 bytes,
    since the IBus protocol CURRENTLY sends 14 channels over (2 bytes per
    channel)

    If this is valid, then the state machine proceeds with reading the command
    byte, the payloads bytes, and the 2 checksum bytes

    If the command code is a channel data command (0x40), then convert the
    payload data into channel data and indicate the received data was valid
*/
void parse_ibus_data(uint8_t data) {
    switch (state) {
        case WAIT_FOR_LENGTH: {
            if (data > MIN_FRAME_LENGTH && data <= MAX_FRAME_LENGTH) {
                state = WAIT_FOR_CMD;
                ibus_buf_i = 0;
                payload_len = data - NON_PAYLOAD_BYTES;
                checksum_calc = 0xFFFF - data;
            }
            break;
        }

        case WAIT_FOR_CMD: {
            state = WAIT_FOR_DATA;
            cmd = data;
            checksum_calc -= data;
            break;
        }

        case WAIT_FOR_DATA: {
            ibus_buf[ibus_buf_i++] = data;
            checksum_calc -= data;
            if (ibus_buf_i == payload_len || ibus_buf_i == IBUS_BUFFER_LEN) {
                state = WAIT_FOR_CHECKSUM_1;
            }
            break;
        }

        case WAIT_FOR_CHECKSUM_1: {
            state = WAIT_FOR_CHECKSUM_2;
            checksum_rx = (uint16_t) data;
            break;
        }

        case WAIT_FOR_CHECKSUM_2: {
            state = WAIT_FOR_LENGTH;
            checksum_rx |= (uint16_t) data << 8;
            if (checksum_calc == checksum_rx) {
                if (cmd == SERVO_CMD_CODE) {
                    for (int i = 0; i < MAX_CHANNELS; i++) {
                        uint16_t msb = (uint16_t) ibus_buf[2*i + 1] << 8;
                        uint16_t lsb = (uint16_t) ibus_buf[2*i];
                        rx.channel_data[i] = msb | lsb;
                    }
                    rx.channel_data_valid = 1;
                }
            }
            break;
        }
    }
}