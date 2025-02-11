/**
 * @file
 * @ingroup     net_mac
 *
 * @brief       Lower MAC driver for Blink
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024
 */
#include <nrf.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "blink.h"
#include "mac.h"
#include "scheduler.h"
#include "radio.h"
#include "timer_hf.h"
#include "protocol.h"
#include "device.h"
#if defined(NRF5340_XXAA) && defined(NRF_NETWORK)
#include "ipc.h"
#endif

//=========================== debug ============================================

#ifndef DEBUG // FIXME: remove before merge. Just to make VS Code enable code behind `#ifdef DEBUG`
#define DEBUG
#endif

#ifdef DEBUG // comes from segger config
#endif

#ifdef DEBUG
#include "gpio.h" // for debugging
gpio_t pin0 = { .port = 1, .pin = 2 }; // variable names reflect the logic analyzer channels
gpio_t pin1 = { .port = 1, .pin = 3 };
gpio_t pin2 = { .port = 1, .pin = 4 };
gpio_t pin3 = { .port = 1, .pin = 5 };
#define DEBUG_GPIO_TOGGLE(pin) db_gpio_toggle(pin)
#define DEBUG_GPIO_SET(pin) db_gpio_set(pin)
#define DEBUG_GPIO_CLEAR(pin) db_gpio_clear(pin)
#else
// No-op when DEBUG is not defined
#define DEBUG_GPIO_TOGGLE(pin) ((void)0))
#define DEBUG_GPIO_SET(pin) ((void)0))
#endif // DEBUG

//=========================== defines ==========================================

typedef enum {
    // common,
    STATE_SLEEP,

    // sync,
    STATE_SYNC_LISTEN = 11,
    STATE_SYNC_RX = 12,
    STATE_SYNC_PROCESS = 13,

    // transmitter,
    STATE_TX_OFFSET = 21,
    STATE_TX_DATA = 22,

    // receiver,
    STATE_RX_OFFSET = 31,
    STATE_RX_DATA_LISTEN = 32,
    STATE_RX_DATA = 33,

} bl_mac_state_t; // TODO: actually use this state in the handlers below

typedef struct {
    bl_node_type_t node_type; //< whether the node is a gateway or a dotbot
    uint64_t device_id; ///< Device ID

    uint64_t asn; ///< Absolute slot number

    bl_mac_state_t state; ///< State within the slot

    uint32_t start_slot_ts; ///< Timestamp of the start of the slot

    bl_rx_cb_t app_rx_callback; ///< Function pointer, stores the application callback
} mac_vars_t;

//=========================== variables ========================================

mac_vars_t mac_vars = { 0 };

bl_slot_timing_t slot_timing = {
    .total_duration = 1000,
};

//=========================== prototypes =======================================

static inline void _set_state(bl_mac_state_t state);

//static void _bl_state_machine_handler(void);

static void _bl_new_slot(void);

// --------------------- timers ---------------------
static inline void _set_timer_and_compensate(uint8_t channel, uint32_t duration, uint32_t start_ts, timer_hf_cb_t callback);

//=========================== public ===========================================

void bl_mac_init(bl_node_type_t node_type, bl_rx_cb_t rx_callback) {
    (void)node_type;
    (void)rx_callback;
#ifdef DEBUG
    db_gpio_init(&pin0, DB_GPIO_OUT);
    db_gpio_init(&pin1, DB_GPIO_OUT);
    db_gpio_init(&pin2, DB_GPIO_OUT);
    db_gpio_init(&pin3, DB_GPIO_OUT);
#endif

    // initialize the high frequency timer
    bl_timer_hf_init(BLINK_TIMER_DEV);

    mac_vars.node_type = node_type;
    mac_vars.device_id = db_device_id();
    mac_vars.app_rx_callback = rx_callback;

    _set_state(STATE_SLEEP);
    _bl_new_slot();
}

//=========================== private ==========================================

// static void _bl_state_machine_handler(void) {
// }

static inline void _set_state(bl_mac_state_t state) {
    mac_vars.state = state;
}

static void _bl_new_slot(void) {
    mac_vars.start_slot_ts = bl_timer_hf_now(BLINK_TIMER_DEV);

    DEBUG_GPIO_SET(&pin0); DEBUG_GPIO_CLEAR(&pin0);

    // set the timer for the next slot for TOTAL DURATION
    _set_timer_and_compensate(
        BLINK_TIMER_INTER_SLOT_CHANNEL,
        slot_timing.total_duration,
        mac_vars.start_slot_ts,
        &_bl_new_slot
    );
}

// --------------------- timers ---------------------

static inline void _set_timer_and_compensate(uint8_t channel, uint32_t duration, uint32_t start_ts, timer_hf_cb_t callback) {
    uint32_t elapsed_ts = bl_timer_hf_now(BLINK_TIMER_DEV) - start_ts;
    // printf("Setting timer for duration %d, compensating for elapsed %d gives: %d\n", duration, elapsed_ts, duration - elapsed_ts);
    bl_timer_hf_set_oneshot_us(
        BLINK_TIMER_DEV,
        channel,
        duration - elapsed_ts,
        callback
    );
}
