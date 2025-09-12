/**
 * @file
 * @ingroup bsp_uart
 *
 * @brief  nRF52833-specific definition of the "uart" bsp module.
 *
 * @author Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @copyright Inria, 2022
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <nrf.h>
#include <nrf_peripherals.h>

#include "mr_gpio.h"
#include "uart.h"

//=========================== defines ==========================================

#if defined(NRF5340_XXAA) && defined(NRF_APPLICATION)
#define NRF_POWER (NRF_POWER_S)
#elif defined(NRF5340_XXAA) && defined(NRF_NETWORK)
#define NRF_POWER (NRF_POWER_NS)
#endif
#define MR_UARTE_CHUNK_SIZE (64U)

typedef struct {
    NRF_UARTE_Type *p;
    IRQn_Type       irq;
} uart_conf_t;

typedef struct {
    uint8_t      byte;       ///< the byte where received byte on UART is stored
    uart_rx_cb_t callback;   ///< pointer to the callback function
    uint8_t     *tx_buffer;  ///< current TX buffer
    size_t       tx_length;  ///< total bytes to transmit
    size_t       tx_pos;     ///< current position in TX buffer
    bool         tx_busy;    ///< flag indicating TX is in progress
} uart_vars_t;

//=========================== variables ========================================

static const uart_conf_t _devs[UARTE_COUNT] = {
#if defined(NRF5340_XXAA) && defined(NRF_APPLICATION)
    {
#if defined(NRF_TRUSTZONE_NONSECURE)
        .p = NRF_UARTE0_NS,
#else
        .p = NRF_UARTE0_S,
#endif
        .irq = SERIAL0_IRQn,
    },
    {
#if defined(NRF_TRUSTZONE_NONSECURE)
        .p = NRF_UARTE1_NS,
#else
        .p = NRF_UARTE1_S,
#endif
        .irq = SERIAL1_IRQn,
    },
    {
#if defined(NRF_TRUSTZONE_NONSECURE)
        .p = NRF_UARTE2_NS,
#else
        .p = NRF_UARTE2_S,
#endif
        .irq = SERIAL2_IRQn,
    },
    {
#if defined(NRF_TRUSTZONE_NONSECURE)
        .p = NRF_UARTE3_NS,
#else
        .p = NRF_UARTE3_S,
#endif
        .irq = SERIAL3_IRQn,
    },
#elif defined(NRF5340_XXAA) && defined(NRF_NETWORK)
    {
        .p   = NRF_UARTE0_NS,
        .irq = SERIAL0_IRQn,
    },
#else
    {
        .p   = NRF_UARTE0,
        .irq = UARTE0_UART0_IRQn,
    },
    {
        .p   = NRF_UARTE1,
        .irq = UARTE1_IRQn,
    },
#endif
};

static uart_vars_t _uart_vars[UARTE_COUNT] = { 0 };  ///< variable handling the UART context

//=========================== public ===========================================

void mr_uart_init(uart_t uart, const mr_gpio_t *rx_pin, const mr_gpio_t *tx_pin, uint32_t baudrate, uart_rx_cb_t callback) {

#if defined(NRF5340_XXAA)
    if (baudrate > 460800) {
        // On nrf53 configure constant latency mode for better performances with high baudrates
        NRF_POWER->TASKS_CONSTLAT = 1;
    }
#endif

    // configure UART pins (RX as input, TX as output);
    mr_gpio_init(rx_pin, MR_GPIO_IN_PU);
    mr_gpio_init(tx_pin, MR_GPIO_OUT);

    // configure UART
    _devs[uart].p->CONFIG   = 0;
    _devs[uart].p->PSEL.RXD = (rx_pin->port << UARTE_PSEL_RXD_PORT_Pos) |
                              (rx_pin->pin << UARTE_PSEL_RXD_PIN_Pos) |
                              (UARTE_PSEL_RXD_CONNECT_Connected << UARTE_PSEL_RXD_CONNECT_Pos);
    _devs[uart].p->PSEL.TXD = (tx_pin->port << UARTE_PSEL_TXD_PORT_Pos) |
                              (tx_pin->pin << UARTE_PSEL_TXD_PIN_Pos) |
                              (UARTE_PSEL_TXD_CONNECT_Connected << UARTE_PSEL_TXD_CONNECT_Pos);
    _devs[uart].p->PSEL.RTS = 0xffffffff;  // pin disconnected
    _devs[uart].p->PSEL.CTS = 0xffffffff;  // pin disconnected

    // configure baudrate
    switch (baudrate) {
        case 1200:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud1200 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 9600:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud9600 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 14400:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud14400 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 19200:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud19200 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 28800:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud28800 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 31250:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud31250 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 38400:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud38400 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 56000:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud56000 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 57600:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud57600 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 76800:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud76800 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 115200:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud115200 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 230400:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud230400 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 250000:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud250000 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 460800:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud460800 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 921600:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud921600 << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        case 1000000:
            _devs[uart].p->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud1M << UARTE_BAUDRATE_BAUDRATE_Pos);
            break;
        default:
            // error, return without enabling UART
            return;
    }

    _devs[uart].p->ENABLE = (UARTE_ENABLE_ENABLE_Enabled << UARTE_ENABLE_ENABLE_Pos);

    if (callback) {
        _uart_vars[uart].callback    = callback;
        _devs[uart].p->RXD.MAXCNT    = 1;
        _devs[uart].p->RXD.PTR       = (uint32_t)&_uart_vars[uart].byte;
        _devs[uart].p->INTENSET      = (UARTE_INTENSET_ENDRX_Enabled << UARTE_INTENSET_ENDRX_Pos);
        _devs[uart].p->SHORTS        = (UARTE_SHORTS_ENDRX_STARTRX_Enabled << UARTE_SHORTS_ENDRX_STARTRX_Pos);
        _devs[uart].p->TASKS_STARTRX = 1;
        NVIC_EnableIRQ(_devs[uart].irq);
        NVIC_SetPriority(_devs[uart].irq, MR_UART_IRQ_PRIORITY);
        NVIC_ClearPendingIRQ(_devs[uart].irq);
    }
}

void mr_uart_write(uart_t uart, uint8_t *buffer, size_t length) {
    // Don't start new TX if one is already in progress
    if (_uart_vars[uart].tx_busy) {
        return;
    }

    // Store TX state
    _uart_vars[uart].tx_buffer = buffer;
    _uart_vars[uart].tx_length = length;
    _uart_vars[uart].tx_pos    = 0;
    _uart_vars[uart].tx_busy   = true;

    // Enable TX interrupt
    _devs[uart].p->INTENSET |= (UARTE_INTENSET_ENDTX_Enabled << UARTE_INTENSET_ENDTX_Pos);

    // Start first chunk
    _devs[uart].p->EVENTS_ENDTX  = 0;
    _devs[uart].p->TXD.PTR       = (uint32_t)&buffer[0];
    size_t chunk_size            = (length > MR_UARTE_CHUNK_SIZE) ? MR_UARTE_CHUNK_SIZE : length;
    _devs[uart].p->TXD.MAXCNT    = chunk_size;
    _devs[uart].p->TASKS_STARTTX = 1;
}

bool mr_uart_tx_busy(uart_t uart) {
    return _uart_vars[uart].tx_busy;
}

//=========================== interrupts =======================================

#include "mr_gpio.h"
extern mr_gpio_t pin_dbg_uart;
static void      _uart_isr(uart_t uart) {
    mr_gpio_set(&pin_dbg_uart);

    // check if the interrupt was caused by a fully received package
    if (_devs[uart].p->EVENTS_ENDRX) {
        _devs[uart].p->EVENTS_ENDRX = 0;
        // make sure we actually received new data
        if (_devs[uart].p->RXD.AMOUNT != 0) {
            // process received byte
            _uart_vars[uart].callback(_uart_vars[uart].byte);
        }
    }

    // check if the interrupt was caused by TX completion
    if (_devs[uart].p->EVENTS_ENDTX) {
        _devs[uart].p->EVENTS_ENDTX = 0;

        // Update position
        _uart_vars[uart].tx_pos += MR_UARTE_CHUNK_SIZE;

        // Check if more chunks need to be sent
        if (_uart_vars[uart].tx_pos < _uart_vars[uart].tx_length) {
            // Send next chunk
            size_t remaining  = _uart_vars[uart].tx_length - _uart_vars[uart].tx_pos;
            size_t chunk_size = (remaining > MR_UARTE_CHUNK_SIZE) ? MR_UARTE_CHUNK_SIZE : remaining;

            _devs[uart].p->TXD.PTR       = (uint32_t)&_uart_vars[uart].tx_buffer[_uart_vars[uart].tx_pos];
            _devs[uart].p->TXD.MAXCNT    = chunk_size;
            _devs[uart].p->TASKS_STARTTX = 1;
        } else {
            // TX complete
            _uart_vars[uart].tx_busy = false;
            // Disable TX interrupt
            _devs[uart].p->INTENCLR = (UARTE_INTENCLR_ENDTX_Clear << UARTE_INTENCLR_ENDTX_Pos);
        }
    }

    mr_gpio_clear(&pin_dbg_uart);
};

#if defined(NRF5340_XXAA)
void SERIAL0_IRQHandler(void) {
    _uart_isr(0);
}

#if defined(NRF5340_XXAA_APPLICATION)
void SERIAL1_IRQHandler(void) {
    _uart_isr(1);
}

void SERIAL2_IRQHandler(void) {
    _uart_isr(2);
}

void SERIAL3_IRQHandler(void) {
    _uart_isr(3);
}
#endif  // NRF5340_XXAA_APPLICATION

#else  // NRF5340_XXAA
void UARTE0_UART0_IRQHandler(void) {
    _uart_isr(0);
}

void UARTE1_IRQHandler(void) {
    _uart_isr(1);
}
#endif
