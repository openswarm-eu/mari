#ifndef __MODELS_H
#define __MODELS_H

/**
 * @defgroup    models      common models
 * @ingroup     blink
 * @brief       Common models used in the blink protocol
 *
 * @{
 * @file
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 * @copyright Inria, 2024-now
 * @}
 */

#include <stdint.h>
#include <stdlib.h>
#include <nrf.h>
#include <stdbool.h>

#include "packet.h"

//=========================== defines =========================================

#define BLINK_N_BLE_REGULAR_CHANNELS 37
#define BLINK_N_BLE_ADVERTISING_CHANNELS 3

// #ifndef BLINK_FIXED_CHANNEL
#define BLINK_FIXED_CHANNEL 0 // to hardcode the channel, use a valid value other than 0
#define BLINK_FIXED_SCAN_CHANNEL 37 // to hardcode the channel, use a valid value other than 0
// #endif

#define BLINK_N_CELLS_MAX 137

#define BLINK_ENABLE_BACKGROUND_SCAN 0

#define BLINK_PACKET_MAX_SIZE 255

//=========================== types ===========================================

typedef enum {
    BLINK_GATEWAY = 'G',
    BLINK_NODE = 'D',
} bl_node_type_t;

typedef enum {
    BLINK_NEW_PACKET,
    BLINK_CONNECTED,
    BLINK_DISCONNECTED,
    BLINK_NODE_JOINED,
    BLINK_NODE_LEFT,
    BLINK_ERROR,
} bl_event_t;

typedef enum {
    BLINK_HANDOVER = 1,
    BLINK_OUT_OF_SYNC = 2,
    BLINK_PEER_LOST = 3,
} bl_event_tag_t;

typedef struct {
    uint8_t len;
    bl_packet_header_t *header;
    uint8_t *payload;
    uint8_t payload_len;
} blink_packet_t;

typedef struct {
    union {
        blink_packet_t new_packet;
        struct {
            uint64_t node_id;
        } node_info;
        struct {
            uint64_t gateway_id;
        } gateway_info;
    } data;
    bl_event_tag_t tag;
} bl_event_data_t;

typedef enum {
    BLINK_RADIO_ACTION_SLEEP = 'S',
    BLINK_RADIO_ACTION_RX = 'R',
    BLINK_RADIO_ACTION_TX = 'T',
} bl_radio_action_t;

typedef enum {
    SLOT_TYPE_BEACON = 'B',
    SLOT_TYPE_SHARED_UPLINK = 'S',
    SLOT_TYPE_DOWNLINK = 'D',
    SLOT_TYPE_UPLINK = 'U',
} slot_type_t;

typedef struct {
    bl_radio_action_t radio_action;
    uint8_t channel;
    slot_type_t type;
} bl_slot_info_t;

typedef struct {
    slot_type_t type;
    uint8_t channel_offset;
    uint64_t assigned_node_id;
} cell_t;

typedef struct {
    uint8_t id; // unique identifier for the schedule
    uint8_t max_nodes; // maximum number of nodes that can be scheduled, equivalent to the number of uplink slot_durations
    uint8_t backoff_n_min; // minimum exponent for the backoff algorithm
    uint8_t backoff_n_max; // maximum exponent for the backoff algorithm
    size_t n_cells; // number of cells in this schedule
    cell_t cells[BLINK_N_CELLS_MAX]; // cells in this schedule. NOTE(FIXME?): the first 3 cells must be beacons
} schedule_t;

typedef struct {
    uint8_t channel;
    int8_t rssi;
    uint32_t start_ts;
    uint32_t end_ts;
    uint64_t asn;
    bool to_me;
    uint8_t packet[BLINK_PACKET_MAX_SIZE];
    uint8_t packet_len;
} bl_received_packet_t;

//=========================== callbacks =======================================

typedef void (*bl_event_cb_t)(bl_event_t event, bl_event_data_t event_data);

#endif // __MODELS_H
