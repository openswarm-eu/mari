/**
 * @file
 * @ingroup     net_maclow
 *
 * @brief       Lower MAC driver for Time-Slotted Channel Hopping (TSCH)
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

#include "maclow.h"
#include "scheduler.h"
#include "radio.h"
#include "timer_hf.h"
//#include "protocol.h"
#include "device.h"
#if defined(NRF5340_XXAA) && defined(NRF_NETWORK)
#include "ipc.h"
#endif

#ifndef DEBUG // FIXME: remove before merge. Just to make VS Code enable code behind `#ifdef DEBUG`
#define DEBUG
#endif

#ifdef DEBUG // comes from segger config
#include "gpio.h" // for debugging
gpio_t pin0 = { .port = 1, .pin = 2 }; // variable names reflect the logic analyzer channels
gpio_t pin1 = { .port = 1, .pin = 3 };
gpio_t pin2 = { .port = 1, .pin = 4 };
gpio_t pin3 = { .port = 1, .pin = 5 };
#endif

#ifdef DEBUG
    #define DEBUG_GPIO_TOGGLE(pin) db_gpio_toggle(pin)
    #define DEBUG_GPIO_SET(pin) db_gpio_set(pin)
    #define DEBUG_GPIO_CLEAR(pin) db_gpio_clear(pin)
#else
    // No-op when DEBUG is not defined
    #define DEBUG_GPIO_TOGGLE(pin) ((void)0))
    #define DEBUG_GPIO_SET(pin) ((void)0))
#endif

#define BLINK_TX_TIME_ONE_BEACON ((sizeof(beacon) * BLE_2M_US_PER_BYTE) + 150)

//=========================== defines ==========================================

//=========================== variables ========================================

bl_slot_timing_t bl_default_slot_timing = {
    .rx_offset = 40 + 200, // Radio ramp-up time (40 us), + some margin for any processing needed
    .rx_max = _BLINK_START_GUARD_TIME + _BLINK_PACKET_TOA_WITH_PADDING, // Guard time + Enough time to receive the maximum payload.
    .tx_offset = 40 + 200 + _BLINK_START_GUARD_TIME, // Same as rx_offset, plus the guard time.
    .tx_max = _BLINK_PACKET_TOA_WITH_PADDING, // Enough to transmit the maximum payload.
    .end_guard = _BLINK_END_GUARD_TIME, // Extra time at the end of the slot

    // receive slot is: rx_offset / rx_max / end_guard
    // transmit slot is: tx_offset / tx_max / end_guard
    .total_duration = 40 + 100 + _BLINK_START_GUARD_TIME + _BLINK_PACKET_TOA_WITH_PADDING + _BLINK_END_GUARD_TIME, // Total duration of the slot
};

typedef enum {
    // common,
    BLINK_STATE_BEGIN_SLOT,

    // receiver,
    BLINK_STATE_WAIT_RX_OFFSET = 11,
    BLINK_STATE_DO_RX = 12,
    BLINK_STATE_IS_RXING = 13,

    // transmitter,
    BLINK_STATE_WAIT_TX_OFFSET = 20,
    BLINK_STATE_DO_TX = 21,
    BLINK_STATE_IS_TXING = 22,

} bl_state_t; // TODO: actually use this state in the handlers below

typedef struct {
    node_type_t node_type; // whether the node is a gateway or a dotbot
    uint64_t device_id; ///< Device ID

    uint64_t asn; ///< Absolute slot number

    bl_state_t state; ///< State of the TSCH state machine
    bl_radio_event_t event; ///< Current event to process

    uint8_t packet[DB_BLE_PAYLOAD_MAX_LENGTH]; ///< Buffer to store the packet to transmit
    size_t packet_len; ///< Length of the packet to transmit

    uint8_t max_beacons_per_slot; ///< Maximum number of beacons to send in a single slot
    uint8_t beacons_sent_this_slot; ///< Tracker to allow sending several beacons in the same slot
    uint8_t tx_time_one_beacon; ///< Tracker to allow sending several beacons in the same slot

    bl_cb_t application_callback; ///< Function pointer, stores the application callback
} bl_vars_t;

static bl_vars_t _bl_vars = { 0 };

uint8_t default_packet[] = {
    0x08, // size of the packet
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
};

bl_beacon_packet_header_t beacon = { 0 };

//========================== prototypes ========================================

//-------------------------- state machine handlers ----------------------------

static void _bl_state_machine_handler(void);
static void _handler_sm_begin_slot(void);

/*
* @brief Set TSCH state machine state, to be used after the timer expires.
*/
static inline void _set_next_state(bl_state_t state);

// ------------------------- other functions ------------------------------------

/*
* @brief Callback function passed to the radio driver.
*/
static void _bl_callback(uint8_t *packet, uint8_t length);

static inline void _set_timer_and_compensate(uint8_t channel, uint32_t duration, uint32_t start_ts, timer_hf_cb_t cb);
// static inline void _set_timer(uint8_t channel, uint32_t duration, timer_hf_cb_t timer_callback);


//=========================== public ===========================================

void bl_init(node_type_t node_type, bl_cb_t application_callback) {
#ifdef DEBUG
    db_gpio_init(&pin0, DB_GPIO_OUT);
    db_gpio_init(&pin1, DB_GPIO_OUT);
    db_gpio_init(&pin2, DB_GPIO_OUT);
    db_gpio_init(&pin3, DB_GPIO_OUT);
#endif

    // initialize the high frequency clock
    bl_timer_hf_init(BLINK_TIMER_DEV);

    // initialize the radio
    bl_radio_init(&_bl_callback, DB_RADIO_BLE_2MBit);  // set the radio callback to our blink catch function

    // Save the application callback to use in our interruption
    _bl_vars.application_callback = application_callback;

    // set node information
    _bl_vars.node_type = node_type;
    _bl_vars.device_id = db_device_id();

    _bl_vars.asn = 0;

    // beacon configurations
    beacon.version = 1;
    beacon.type = BLINK_PACKET_TYPE_BEACON;
    beacon.src = db_device_id();
    _bl_vars.max_beacons_per_slot = 1;//bl_default_slot_timing.tx_max / ((sizeof(beacon) * BLE_2M_US_PER_BYTE) + 180); // 180 us margin
    _bl_vars.beacons_sent_this_slot = 0;
    // _bl_vars.tx_time_one_beacon = (sizeof(beacon) * BLE_2M_US_PER_BYTE) + 100; // 100 us margin

    // NOTE: assume the scheduler has already been initialized by the application

    // set slot total duration
    bl_default_slot_timing.total_duration = bl_default_slot_timing.rx_offset + bl_default_slot_timing.rx_max + bl_default_slot_timing.end_guard;

    // initialize and start the state machine
    _set_next_state(BLINK_STATE_BEGIN_SLOT);
    uint32_t time_padding = 50; // account for function call and interrupt latency
    bl_timer_hf_set_oneshot_us(BLINK_TIMER_DEV, BLINK_TIMER_INTER_SLOT_CHANNEL, time_padding, _bl_state_machine_handler); // trigger the state machine
}

//=========================== private ==========================================

// state machine handler
void _bl_state_machine_handler(void) {
    uint32_t start_ts = bl_timer_hf_now(BLINK_TIMER_DEV);
    // printf("State: %d\n", _bl_vars.state);

    switch (_bl_vars.state) {
        case BLINK_STATE_BEGIN_SLOT:
            DEBUG_GPIO_CLEAR(&pin0); DEBUG_GPIO_SET(&pin0); // slot-wide debug pin
            DEBUG_GPIO_SET(&pin1); // intra-slot debug pin
            // set the timer for the next slot TOTAL DURATION
            // _set_timer(BLINK_TIMER_INTER_SLOT_CHANNEL, bl_default_slot_timing.total_duration, &_bl_state_machine_handler);
            _set_timer_and_compensate(BLINK_TIMER_INTER_SLOT_CHANNEL, bl_default_slot_timing.total_duration, start_ts, &_bl_state_machine_handler);

            // common logic, which doesn't depend on schedule
            _bl_vars.beacons_sent_this_slot = 0;

            // logic of beginning the slot
            _handler_sm_begin_slot();
            break;
        case BLINK_STATE_DO_RX:
            DEBUG_GPIO_CLEAR(&pin1);
            DEBUG_GPIO_SET(&pin1);
            // update state
            _set_next_state(BLINK_STATE_IS_RXING);
            // receive packets
            bl_radio_rx(); // remember: this always starts with before the actual transmission begins, i.e, rx_offset < tx_offset always holds
            _set_timer_and_compensate(BLINK_TIMER_INTRA_SLOT_CHANNEL, bl_default_slot_timing.rx_max, start_ts, &_bl_state_machine_handler);
            break;
        case BLINK_STATE_DO_TX:
            DEBUG_GPIO_CLEAR(&pin1);
            DEBUG_GPIO_SET(&pin1);
            // send the packet
            // printf("Sending packet of length %d\n", _bl_vars.packet_len);
            // for (uint8_t i = 0; i < _bl_vars.packet_len; i++) {
            //     printf("%02x ", _bl_vars.packet[i]);
            // }
            // puts("");

            if (_bl_vars.node_type == NODE_TYPE_GATEWAY && _bl_vars.event.slot_type == SLOT_TYPE_BEACON) {
                DEBUG_GPIO_SET(&pin2); DEBUG_GPIO_CLEAR(&pin2); // Gateway sending beacon NOW
            }

            _set_next_state(BLINK_STATE_IS_TXING);
            _set_timer_and_compensate(BLINK_TIMER_INTRA_SLOT_CHANNEL, bl_default_slot_timing.tx_max, start_ts, &_bl_state_machine_handler);
            // bl_radio_tx(_bl_vars.packet, _bl_vars.packet_len);
            bl_radio_tx_dispatch();
            break;
        // in case was receiving or sending, now just finish. timeslot will begin again because of the inter-slot timer
        case BLINK_STATE_IS_RXING:
        case BLINK_STATE_IS_TXING:
            DEBUG_GPIO_CLEAR(&pin1);
            // just disable the radio and set the next state
            bl_radio_disable();
            _set_next_state(BLINK_STATE_BEGIN_SLOT);
            break;
        default:
            break;
    }
}

void _handler_sm_begin_slot(void) {
    uint32_t start_ts = bl_timer_hf_now(BLINK_TIMER_DEV);

    uint32_t timer_duration = 0;

    bl_radio_event_t event = bl_scheduler_tick(_bl_vars.asn++);
    // printf("  Event %c:   %c, %d    Slot duration: %d\n", event.slot_type, event.radio_action, event.frequency, bl_default_slot_timing.total_duration);

    switch (event.radio_action) {
        case BLINK_RADIO_ACTION_TX:
            // set the packet that will be transmitted
            if (_bl_vars.node_type == NODE_TYPE_GATEWAY && event.slot_type == SLOT_TYPE_BEACON) {
                _bl_vars.packet_len = sizeof(beacon);
                memcpy(_bl_vars.packet, &beacon, _bl_vars.packet_len);
            } else {
                // TODO: get from a queue or something
                _bl_vars.packet_len = sizeof(default_packet);
                memcpy(_bl_vars.packet, default_packet, _bl_vars.packet_len);
            }

            // configure radio
            bl_radio_disable();
            bl_radio_set_frequency(event.frequency);
            bl_radio_tx_prepare(_bl_vars.packet, _bl_vars.packet_len);

            // update state
            _bl_vars.event = event;
            _set_next_state(BLINK_STATE_DO_TX);

            // get the packet to tx and save in _bl_vars
            // TODO: how to get a packet? decide based on _bl_vars.node_type and event.slot_type
            //       could the event come with a packet? sometimes maybe? or would it be confusing?

            // set timer duration to resume again after tx_offset
            timer_duration = bl_default_slot_timing.tx_offset;
            _set_timer_and_compensate(BLINK_TIMER_INTRA_SLOT_CHANNEL, timer_duration, start_ts, &_bl_state_machine_handler);
            break;
        case BLINK_RADIO_ACTION_RX:
            // configure radio
            bl_radio_disable();
            bl_radio_set_frequency(event.frequency);

            // update state
            _bl_vars.event = event;
            _set_next_state(BLINK_STATE_DO_RX);

            // set timer duration to resume again after rx_offset
            timer_duration = bl_default_slot_timing.rx_offset;
            _set_timer_and_compensate(BLINK_TIMER_INTRA_SLOT_CHANNEL, timer_duration, start_ts, &_bl_state_machine_handler);
            break;
        case BLINK_RADIO_ACTION_SLEEP:
            // just disable the radio and do nothing, then come back for the next slot
            bl_radio_disable();
            _set_next_state(BLINK_STATE_BEGIN_SLOT); // keep the same state
            DEBUG_GPIO_CLEAR(&pin1);
            // timer_duration = bl_default_slot_timing.total_duration;
            break;
    }

    // _set_timer_and_compensate(BLINK_TIMER_INTRA_SLOT_CHANNEL, timer_duration, start_ts, &_bl_state_machine_handler);
}

// --------------------- timers ---------------------

static inline void _set_next_state(bl_state_t state) {
    _bl_vars.state = state;
}

static inline void _set_timer_and_compensate(uint8_t channel, uint32_t duration, uint32_t start_ts, timer_hf_cb_t timer_callback) {
    uint32_t elapsed_ts = bl_timer_hf_now(BLINK_TIMER_DEV) - start_ts;
    // printf("Setting timer for duration %d, compensating for elapsed %d gives: %d\n", duration, elapsed_ts, duration - elapsed_ts);
    bl_timer_hf_set_oneshot_us(
        BLINK_TIMER_DEV,
        channel,
        duration - elapsed_ts,
        timer_callback
    );
}

//static inline void _set_timer(uint8_t channel, uint32_t duration, timer_hf_cb_t timer_callback) {
////    printf("Setting timer for duration %d\n", duration);
//   bl_timer_hf_set_oneshot_us(
//       BLINK_TIMER_DEV,
//       channel,
//       duration,
//       timer_callback
//   );
//}

// --------------------- others ---------------------

static void _bl_callback(uint8_t *packet, uint8_t length) {
    // printf("TSCH: Received packet of length %d\n", length);

    // Check if the packet is big enough to have a header
    size_t TODO_header_len = 32; // FIXME
    if (length < TODO_header_len) {
        return;
    }

    // Check if it is a beacon
    if (packet[1] == BLINK_PACKET_TYPE_BEACON && _bl_vars.node_type == NODE_TYPE_DOTBOT) {
        // _bl_handle_beacon(packet, length);
        DEBUG_GPIO_SET(&pin2); // DotBot received a beacon
        DEBUG_GPIO_CLEAR(&pin2);
    } else if (_bl_vars.application_callback) {
        _bl_vars.application_callback(packet, length);
    }
}
