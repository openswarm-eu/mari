#include <nrf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "protocol.h"
#include "mac.h"
#include "scheduler.h"
#include "radio.h"

//=========================== defines ==========================================

#define BLINK_PACKET_QUEUE_SIZE (8) // must be a power of 2

typedef struct {
    uint8_t length;
    uint8_t buffer[BLINK_PACKET_MAX_SIZE];
} blink_packet_t;

typedef struct {
    uint8_t         current;                            ///< Current position in the queue
    uint8_t         last;                               ///< Position of the last item added in the queue
    blink_packet_t  packets[BLINK_PACKET_QUEUE_SIZE];
} blink_packet_queue_t;

typedef struct {
    bl_node_type_t          node_type;
    blink_packet_queue_t    packet_queue;
    bl_rx_cb_t              app_rx_callback;
    bl_event_cb_t           app_event_callback;

    // node data
    bool                    is_connected;

    // gateway data
    uint64_t                joined_nodes[BLINK_MAX_NODES];
    uint8_t                 joined_nodes_len;
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

    bl_scheduler_init(node_type, NULL);

    // TODO: bl_mac_init(node_type, rx_callback);

    // TMP: using the radio directly
    bl_radio_init(&_bl_callback, DB_RADIO_BLE_2MBit);
    bl_radio_set_channel(BLINK_FIXED_CHANNEL); // temporary value
    bl_radio_rx();
}

void bl_tx(uint8_t *packet, uint8_t length) {
    // enqueue for transmission
    bl_queue_add(packet, length);

    // TMP: send immediately, for now just ignoring the queue
    bl_radio_disable();
    bl_radio_tx(packet, length); // TODO: use a packet queue
}

void bl_get_joined_nodes(uint64_t *nodes, uint8_t *num_nodes) {
    *num_nodes = _blink_vars.joined_nodes_len;
    memcpy(nodes, _blink_vars.joined_nodes, _blink_vars.joined_nodes_len * sizeof(uint64_t));
}

//--------------------------- packet queue -------------------------------------

void bl_queue_add(uint8_t *packet, uint8_t length) {
    // enqueue for transmission
    memcpy(_blink_vars.packet_queue.packets[_blink_vars.packet_queue.last].buffer, packet, length);
    _blink_vars.packet_queue.packets[_blink_vars.packet_queue.last].length = length;
    // increment the `last` index
    _blink_vars.packet_queue.last = (_blink_vars.packet_queue.last + 1) % BLINK_PACKET_QUEUE_SIZE;
}

bool bl_queue_peek(uint8_t *packet, uint8_t *length) {
    if (_blink_vars.packet_queue.current == _blink_vars.packet_queue.last) {
        return false;
    }

    memcpy(packet, _blink_vars.packet_queue.packets[_blink_vars.packet_queue.current].buffer, _blink_vars.packet_queue.packets[_blink_vars.packet_queue.current].length);
    *length = _blink_vars.packet_queue.packets[_blink_vars.packet_queue.current].length;

    // do not increment the `current` index here, as this is just a peek

    return true;
}

bool bl_queue_pop(void) {
    if (_blink_vars.packet_queue.current == _blink_vars.packet_queue.last) {
        return false;
    } else {
        // increment the `current` index
        _blink_vars.packet_queue.current = (_blink_vars.packet_queue.current + 1) % BLINK_PACKET_QUEUE_SIZE;
        return true;
    }
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
        case BLINK_PACKET_JOIN_REQUEST: // NOTE: handle this in the maclow instead?
            if (_blink_vars.node_type == NODE_TYPE_GATEWAY && _blink_vars.joined_nodes_len < BLINK_MAX_NODES) {
                // TODO:
                // - check if node can join
                // - send/enqueue join response
                _blink_vars.joined_nodes[_blink_vars.joined_nodes_len++] = header->src;
                _blink_vars.app_event_callback(BLINK_NODE_JOINED);
            }
            break;
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
