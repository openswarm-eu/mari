/**
 * @file
 * @ingroup     blink_assoc
 *
 * @brief       Association procedure for the blink protocol
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

#include "radio.h"
#include "association.h"
#include "scan.h"
#include "queue.h"

//=========================== debug ============================================

#ifndef DEBUG // FIXME: remove before merge. Just to make VS Code enable code behind `#ifdef DEBUG`
#define DEBUG
#endif

#ifdef DEBUG
#include "gpio.h" // for debugging
// the 4 LEDs of the DK are on port 0, pins 13, 14, 15, 16
gpio_t led0 = { .port = 0, .pin = 13 };
gpio_t led1 = { .port = 0, .pin = 14 };
gpio_t led2 = { .port = 0, .pin = 15 };
gpio_t led3 = { .port = 0, .pin = 16 };
#define DEBUG_GPIO_TOGGLE(pin) db_gpio_toggle(pin)
#define DEBUG_GPIO_SET(pin) db_gpio_set(pin)
#define DEBUG_GPIO_CLEAR(pin) db_gpio_clear(pin)
#else
// No-op when DEBUG is not defined
#define DEBUG_GPIO_TOGGLE(pin) ((void)0))
#define DEBUG_GPIO_SET(pin) ((void)0))
#define DEBUG_GPIO_CLEAR(pin) ((void)0))
#endif // DEBUG

//=========================== defines =========================================

typedef struct {
    bl_assoc_state_t state;
    bl_event_cb_t blink_event_callback;
} assoc_vars_t;

//=========================== variables =======================================

static assoc_vars_t assoc_vars = { 0 };

//=========================== prototypes ======================================


//=========================== public ==========================================

void bl_assoc_init(bl_event_cb_t event_callback) {
#ifdef DEBUG
    db_gpio_init(&led0, DB_GPIO_OUT);
    db_gpio_init(&led1, DB_GPIO_OUT);
    db_gpio_init(&led2, DB_GPIO_OUT);
    db_gpio_init(&led3, DB_GPIO_OUT);
    // remember: the LEDs are active low
#endif

    assoc_vars.blink_event_callback = event_callback;

    bl_assoc_set_state(JOIN_STATE_IDLE);
}

bool bl_assoc_node_ready_to_join(void) {
    // TODO: also call bl_backoff_is_ready() here
    return assoc_vars.state == JOIN_STATE_SYNCED;
}

bool bl_assoc_gateway_pending_join_response(void) {
    return assoc_vars.state == JOIN_STATE_JOINING;
}

bool bl_assoc_node_is_joined(void) {
    return assoc_vars.state == JOIN_STATE_JOINED;
}

inline void bl_assoc_set_state(bl_assoc_state_t state) {
    assoc_vars.state = state;

#ifdef DEBUG
    DEBUG_GPIO_SET(&led0); DEBUG_GPIO_SET(&led1); DEBUG_GPIO_SET(&led2); DEBUG_GPIO_SET(&led3);
    switch (state) {
        case JOIN_STATE_IDLE:
            // DEBUG_GPIO_CLEAR(&pin1);
            break;
        case JOIN_STATE_SCANNING:
            // DEBUG_GPIO_SET(&pin1);
            DEBUG_GPIO_CLEAR(&led0);
            break;
        case JOIN_STATE_SYNCED:
            // DEBUG_GPIO_CLEAR(&pin1);
            DEBUG_GPIO_CLEAR(&led1);
            break;
        case JOIN_STATE_JOINING:
            DEBUG_GPIO_CLEAR(&led2);
            break;
        case JOIN_STATE_JOINED:
            DEBUG_GPIO_CLEAR(&led3);
            break;
        default:
            break;
    }
#endif
}

void bl_assoc_handle_beacon(uint8_t *packet, uint8_t length, uint8_t channel, uint32_t ts) {
    (void)length;

    if (packet[1] != BLINK_PACKET_BEACON) {
        return;
    }

    // now that we know it's a beacon packet, parse and process it
    bl_beacon_packet_header_t *beacon = (bl_beacon_packet_header_t *)packet;

    if (beacon->version != BLINK_PROTOCOL_VERSION) {
        // ignore packet with different protocol version
        return;
    }

    if (beacon->remaining_capacity == 0) {
        // this gateway is full, ignore it
        return;
    }

    // save this scan info
    bl_scan_add(*beacon, bl_radio_rssi(), channel, ts, 0); // asn not used anymore during scan

    return;
}

void bl_assoc_handle_packet(uint8_t *packet, uint8_t length) {
    (void)length;
    bl_packet_header_t *header = (bl_packet_header_t *)packet;

    if (bl_get_node_type() == BLINK_GATEWAY) {
        if (header->type == BLINK_PACKET_JOIN_REQUEST) {
            // try to assign a cell to the node
            int16_t cell_id = bl_scheduler_assign_next_available_uplink_cell(header->src);
            if (cell_id >= 0) {
                bl_queue_set_join_response(header->src, (uint8_t)cell_id);
            } else {
                assoc_vars.blink_event_callback(BLINK_ERROR, (bl_event_data_t){ 0 });
            }
        }
    } else if (bl_get_node_type() == BLINK_NODE) {
        if (header->type == BLINK_PACKET_JOIN_RESPONSE) {
            // cell_id is just after the header
            uint8_t cell_id = packet[sizeof(bl_packet_header_t)];
            if (bl_scheduler_assign_myself_to_cell(cell_id)) {
                bl_assoc_set_state(JOIN_STATE_JOINED);
                bl_event_data_t event_data = { .data.gateway_info.gateway_id = header->src };
                assoc_vars.blink_event_callback(BLINK_CONNECTED, event_data);
            } else {
                assoc_vars.blink_event_callback(BLINK_ERROR, (bl_event_data_t){ 0 });
            }
        }
    }

}

//=========================== callbacks =======================================

//=========================== private =========================================
