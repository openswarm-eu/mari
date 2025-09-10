/**
 * @file
 * @ingroup     app
 *
 * @brief       Mari Gateway application (uart side)
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 * @author Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @copyright Inria, 2025-now
 */
#include <nrf.h>
#include <stdio.h>

#include "ipc.h"

#include "mr_clock.h"
#include "mr_device.h"
#include "hdlc.h"
#include "uart.h"

#include "mr_gpio.h"
mr_gpio_t pin_hdlc_error        = { .port = 1, .pin = 5 };
mr_gpio_t pin_hdlc_ready_decode = { .port = 1, .pin = 10 };
mr_gpio_t pin_dgb_ipc           = { .port = 1, .pin = 7 };
mr_gpio_t pin_dbg_uart          = { .port = 1, .pin = 8 };
mr_gpio_t pin_dgb_uart_write    = { .port = 1, .pin = 9 };

//=========================== defines ==========================================

#define MR_UART_INDEX (1)  ///< Index of UART peripheral to use
//  #define MR_UART_BAUDRATE (1000000UL)  ///< UART baudrate used by the gateway
#define MR_UART_BAUDRATE (921600L)  ///< UART baudrate used by the gateway

typedef struct {
    bool    mari_frame_received;
    bool    uart_byte_received;
    uint8_t uart_byte;
    uint8_t hdlc_encode_buffer[1024];  // Should be large enough
    bool    tx_pending;                // Flag for deferred TX
    size_t  tx_frame_len;              // Length of frame to transmit
} gateway_app_vars_t;

// UART RX and TX pins
static const mr_gpio_t _mr_uart_tx_pin = { .port = 1, .pin = 1 };
static const mr_gpio_t _mr_uart_rx_pin = { .port = 1, .pin = 0 };

static gateway_app_vars_t                                           _app_vars = { 0 };
volatile __attribute__((section(".shared_data"))) ipc_shared_data_t ipc_shared_data;

static void _setup_debug_pins(void) {
    // Assign P0.28 to P0.31 to the network core (for debugging association.c via LEDs)
    NRF_P0_S->PIN_CNF[28] = GPIO_PIN_CNF_MCUSEL_NetworkMCU << GPIO_PIN_CNF_MCUSEL_Pos;
    NRF_P0_S->PIN_CNF[29] = GPIO_PIN_CNF_MCUSEL_NetworkMCU << GPIO_PIN_CNF_MCUSEL_Pos;
    NRF_P0_S->PIN_CNF[30] = GPIO_PIN_CNF_MCUSEL_NetworkMCU << GPIO_PIN_CNF_MCUSEL_Pos;
    NRF_P0_S->PIN_CNF[31] = GPIO_PIN_CNF_MCUSEL_NetworkMCU << GPIO_PIN_CNF_MCUSEL_Pos;

    // Assign P1.02 to P1.05 to the network core (for debugging mac.c via logic analyzer)
    NRF_P1_S->PIN_CNF[2] = GPIO_PIN_CNF_MCUSEL_NetworkMCU << GPIO_PIN_CNF_MCUSEL_Pos;
    NRF_P1_S->PIN_CNF[3] = GPIO_PIN_CNF_MCUSEL_NetworkMCU << GPIO_PIN_CNF_MCUSEL_Pos;
    NRF_P1_S->PIN_CNF[4] = GPIO_PIN_CNF_MCUSEL_NetworkMCU << GPIO_PIN_CNF_MCUSEL_Pos;
    NRF_P1_S->PIN_CNF[5] = GPIO_PIN_CNF_MCUSEL_NetworkMCU << GPIO_PIN_CNF_MCUSEL_Pos;

    // Configure all GPIOs as non secure
    NRF_SPU_S->GPIOPORT[0].PERM = 0;
    NRF_SPU_S->GPIOPORT[1].PERM = 0;
}

static void _configure_ram_non_secure(uint8_t start_region, size_t length) {
    for (uint8_t region = start_region; region < start_region + length; region++) {
        NRF_SPU_S->RAMREGION[region].PERM = (SPU_RAMREGION_PERM_READ_Enable << SPU_RAMREGION_PERM_READ_Pos |
                                             SPU_RAMREGION_PERM_WRITE_Enable << SPU_RAMREGION_PERM_WRITE_Pos |
                                             SPU_RAMREGION_PERM_EXECUTE_Enable << SPU_RAMREGION_PERM_EXECUTE_Pos |
                                             SPU_RAMREGION_PERM_SECATTR_Non_Secure << SPU_RAMREGION_PERM_SECATTR_Pos);
    }
}

static void _init_ipc(void) {
    NRF_IPC_S->INTENSET                            = (1 << IPC_CHAN_RADIO_TO_UART);
    NRF_IPC_S->SEND_CNF[IPC_CHAN_UART_TO_RADIO]    = (1 << IPC_CHAN_UART_TO_RADIO);
    NRF_IPC_S->RECEIVE_CNF[IPC_CHAN_RADIO_TO_UART] = (1 << IPC_CHAN_RADIO_TO_UART);

    NVIC_EnableIRQ(IPC_IRQn);
    NVIC_ClearPendingIRQ(IPC_IRQn);
    NVIC_SetPriority(IPC_IRQn, IPC_IRQ_PRIORITY);
}

static void _release_network_core(void) {
    // Do nothing if network core is already started and ready
    if (!NRF_RESET_S->NETWORK.FORCEOFF && ipc_shared_data.net_ready) {
        return;
    } else if (!NRF_RESET_S->NETWORK.FORCEOFF) {
        ipc_shared_data.net_ready = false;
    }

    NRF_RESET_S->NETWORK.FORCEOFF = (RESET_NETWORK_FORCEOFF_FORCEOFF_Release << RESET_NETWORK_FORCEOFF_FORCEOFF_Pos);

    // add an extra delay to ensure the network core is released
    // NOTE: this is very hacky, but since this only happens once, we don't want to consume another timer
    for (uint32_t i = 0; i < 500000; i++) {
        __NOP();
    }

    while (!ipc_shared_data.net_ready) {}
}

static void _uart_callback(uint8_t byte) {
    _app_vars.uart_byte          = byte;
    _app_vars.uart_byte_received = true;
}

int main(void) {
    printf("Hello Mari Gateway App Core (UART) %016llX\n", mr_device_id());

    _setup_debug_pins();

    mr_gpio_init(&pin_hdlc_error, MR_GPIO_OUT);
    mr_gpio_init(&pin_hdlc_ready_decode, MR_GPIO_OUT);
    mr_gpio_init(&pin_dgb_ipc, MR_GPIO_OUT);
    mr_gpio_init(&pin_dbg_uart, MR_GPIO_OUT);
    mr_gpio_init(&pin_dgb_uart_write, MR_GPIO_OUT);

    // Enable HFCLK with external 32MHz oscillator
    mr_hfclk_init();

    _configure_ram_non_secure(2, 1);
    _init_ipc();
    mr_uart_init(MR_UART_INDEX, &_mr_uart_rx_pin, &_mr_uart_tx_pin, MR_UART_BAUDRATE, &_uart_callback);

    _release_network_core();
    // this is a bit hacky -- sometimes it does not work without this
    NRF_RESET_S->NETWORK.FORCEOFF = 0;

    while (1) {
        __WFE();

        if (_app_vars.uart_byte_received) {
            _app_vars.uart_byte_received = false;

            // Disable IPC interrupts during HDLC processing to prevent TX interference
            mr_hdlc_state_t prev_state = mr_hdlc_peek_state();
            mr_hdlc_state_t hdlc_state = mr_hdlc_rx_byte(_app_vars.uart_byte);

            // Manage IPC interrupt based on HDLC state transitions
            if (prev_state != MR_HDLC_STATE_RECEIVING && hdlc_state == MR_HDLC_STATE_RECEIVING) {
                // Started receiving - disable IPC interrupts
                NVIC_DisableIRQ(IPC_IRQn);
            } else if (prev_state == MR_HDLC_STATE_RECEIVING && hdlc_state != MR_HDLC_STATE_RECEIVING) {
                // Finished receiving - re-enable IPC interrupts
                NVIC_EnableIRQ(IPC_IRQn);
            }

            switch ((uint8_t)hdlc_state) {
                case MR_HDLC_STATE_IDLE:
                case MR_HDLC_STATE_RECEIVING:
                case MR_HDLC_STATE_ERROR:
                    break;
                case MR_HDLC_STATE_READY:
                {
                    mr_gpio_set(&pin_hdlc_ready_decode);
                    size_t msg_len                    = mr_hdlc_decode((uint8_t *)(void *)ipc_shared_data.uart_to_radio);
                    ipc_shared_data.uart_to_radio_len = msg_len;
                    if (msg_len) {
                        NRF_IPC_S->TASKS_SEND[IPC_CHAN_UART_TO_RADIO] = 1;
                    }
                    mr_gpio_clear(&pin_hdlc_ready_decode);
                } break;
                default:
                    break;
            }
            if (hdlc_state == MR_HDLC_STATE_ERROR) {
                mr_gpio_set(&pin_hdlc_error);
                mr_gpio_clear(&pin_hdlc_error);
            }
        }

        if (_app_vars.mari_frame_received) {
            _app_vars.mari_frame_received = false;
            _app_vars.tx_frame_len        = mr_hdlc_encode((uint8_t *)ipc_shared_data.radio_to_uart, ipc_shared_data.radio_to_uart_len, _app_vars.hdlc_encode_buffer);

            // Try to send immediately, or defer if UART is busy
            if (!mr_uart_tx_busy(MR_UART_INDEX) && mr_hdlc_peek_state() != MR_HDLC_STATE_RECEIVING) {
                mr_gpio_set(&pin_dgb_uart_write);
                mr_uart_write(MR_UART_INDEX, _app_vars.hdlc_encode_buffer, _app_vars.tx_frame_len);
                mr_gpio_clear(&pin_dgb_uart_write);
            } else {
                _app_vars.tx_pending = true;
            }
        }

        // Handle deferred TX when UART becomes available
        if (_app_vars.tx_pending && !mr_uart_tx_busy(MR_UART_INDEX) && mr_hdlc_peek_state() != MR_HDLC_STATE_RECEIVING) {
            _app_vars.tx_pending = false;
            mr_gpio_set(&pin_dgb_uart_write);
            mr_uart_write(MR_UART_INDEX, _app_vars.hdlc_encode_buffer, _app_vars.tx_frame_len);
            mr_gpio_clear(&pin_dgb_uart_write);
        }
    }
}

void IPC_IRQHandler(void) {
    mr_gpio_set(&pin_dgb_ipc);
    if (NRF_IPC_S->EVENTS_RECEIVE[IPC_CHAN_RADIO_TO_UART]) {
        NRF_IPC_S->EVENTS_RECEIVE[IPC_CHAN_RADIO_TO_UART] = 0;
        _app_vars.mari_frame_received                     = true;
    }
    mr_gpio_clear(&pin_dgb_ipc);
}
