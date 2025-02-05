#include <nrf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "protocol.h"
#include "maclow.h"
#include "radio.h"

//=========================== defines ==========================================

#define BLINK_DEFAULT_FREQUENCY (8)

typedef struct {
    bl_node_type_t node_type;
    bool is_connected;
    bl_rx_cb_t app_rx_callback;
    bl_event_cb_t app_event_callback;
} blink_vars_t;

//=========================== variables ========================================

static blink_vars_t _blink_vars = { 0 };

//=========================== prototypes =======================================

static void _bl_callback(uint8_t *packet, uint8_t length);

//=========================== public ===========================================

void bl_init(bl_node_type_t node_type, bl_rx_cb_t rx_callback, bl_event_cb_t event_callback) {
    _blink_vars.node_type = node_type;
    _blink_vars.is_connected = true; // FIXME true just for tests
    _blink_vars.app_rx_callback = rx_callback;
    _blink_vars.app_event_callback = event_callback;

    bl_radio_init(&_bl_callback, DB_RADIO_BLE_2MBit);
    bl_radio_set_frequency(BLINK_DEFAULT_FREQUENCY); // temporary value
    bl_radio_rx();

    // TODO:
    // - init maclow, e.g., bl_maclow_init(node_type, rx_callback);
    // - init scheduler, e.g., bl_scheduler_init(node_type, &schedule_test);
}

void bl_tx(uint8_t *packet, uint8_t length) {
    bl_radio_disable();
    bl_radio_tx(packet, length); // TODO: use a packet queue
}

// TODO: implement
void bl_get_joined_nodes(uint64_t *nodes, uint8_t *num_nodes) {
    (void)nodes;
    (void)num_nodes;
}

//=========================== callbacks ===========================================

static void _bl_callback(uint8_t *packet, uint8_t length) {
    printf("BLINK: Received packet of length %d\n", length);

    // Check if the packet is big enough to have a header
    size_t header_len = sizeof(bl_packet_header_t);
    if (length < header_len) {
        return;
    }

    bl_packet_header_t *header = (bl_packet_header_t *)packet;

    switch (header->type) {
        case BLINK_PACKET_JOIN_RESPONSE: // NOTE: handle this in the maclow instead?
            if (_blink_vars.node_type == NODE_TYPE_NODE) {
                _blink_vars.is_connected = true;
                _blink_vars.app_event_callback(BLINK_CONNECTED);
            }
            break;
        case BLINK_PACKET_DATA:
            if (_blink_vars.app_rx_callback && (_blink_vars.node_type == NODE_TYPE_GATEWAY || _blink_vars.is_connected)) {
                _blink_vars.app_rx_callback(packet + header_len, length - header_len);
            }
            break;
        default:
            printf("BLINK: Received packet of type %d\n", header->type);
            break;
    }
}
