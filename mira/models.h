#ifndef __MODELS_H
#define __MODELS_H

/**
 * @defgroup    models      common models
 * @ingroup     mira
 * @brief       Common models used in the mira protocol
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

#define MIRA_N_BLE_REGULAR_CHANNELS     37
#define MIRA_N_BLE_ADVERTISING_CHANNELS 3

// #ifndef MIRA_FIXED_CHANNEL
#define MIRA_FIXED_CHANNEL      0   // to hardcode the channel, use a valid value other than 0
#define MIRA_FIXED_SCAN_CHANNEL 37  // to hardcode the channel, use a valid value other than 0
// #endif

#define MIRA_N_CELLS_MAX 137

#define MIRA_ENABLE_BACKGROUND_SCAN 0

#define MIRA_PACKET_MAX_SIZE 255

//=========================== types ===========================================

typedef enum {
    MIRA_GATEWAY = 'G',
    MIRA_NODE    = 'D',
} mr_node_type_t;

typedef enum {
    MIRA_NEW_PACKET,
    MIRA_CONNECTED,
    MIRA_DISCONNECTED,
    MIRA_NODE_JOINED,
    MIRA_NODE_LEFT,
    MIRA_ERROR,
} mr_event_t;

typedef enum {
    MIRA_NONE              = 0,
    MIRA_HANDOVER          = 1,
    MIRA_OUT_OF_SYNC       = 2,
    MIRA_PEER_LOST         = 3,
    MIRA_GATEWAY_FULL      = 4,
    MIRA_PEER_LOST_TIMEOUT = 5,
    MIRA_PEER_LOST_BLOOM   = 6,
} mr_event_tag_t;

typedef struct {
    uint8_t             len;
    mr_packet_header_t *header;
    uint8_t            *payload;
    uint8_t             payload_len;
} mira_packet_t;

typedef struct {
    union {
        mira_packet_t new_packet;
        struct {
            uint64_t node_id;
        } node_info;
        struct {
            uint64_t gateway_id;
        } gateway_info;
    } data;
    mr_event_tag_t tag;
} mr_event_data_t;

typedef enum {
    MIRA_RADIO_ACTION_SLEEP = 'S',
    MIRA_RADIO_ACTION_RX    = 'R',
    MIRA_RADIO_ACTION_TX    = 'T',
} mr_radio_action_t;

typedef enum {
    SLOT_TYPE_BEACON        = 'B',
    SLOT_TYPE_SHARED_UPLINK = 'S',
    SLOT_TYPE_DOWNLINK      = 'D',
    SLOT_TYPE_UPLINK        = 'U',
} slot_type_t;

typedef struct {
    mr_radio_action_t radio_action;
    uint8_t           channel;
    slot_type_t       type;
} mr_slot_info_t;

typedef struct {
    slot_type_t type;
    uint8_t     channel_offset;
    uint64_t    assigned_node_id;
    uint64_t    last_received_asn;  ///< ASN marking the last time the node was heard from
} cell_t;

typedef struct {
    uint8_t id;                       // unique identifier for the schedule
    uint8_t max_nodes;                // maximum number of nodes that can be scheduled, equivalent to the number of uplink slot_durations
    uint8_t backoff_n_min;            // minimum exponent for the backoff algorithm
    uint8_t backoff_n_max;            // maximum exponent for the backoff algorithm
    size_t  n_cells;                  // number of cells in this schedule
    cell_t  cells[MIRA_N_CELLS_MAX];  // cells in this schedule. NOTE(FIXME?): the first 3 cells must be beacons
} schedule_t;

typedef struct {
    uint8_t  channel;
    int8_t   rssi;
    uint32_t start_ts;
    uint32_t end_ts;
    uint64_t asn;
    bool     to_me;
    uint8_t  packet[MIRA_PACKET_MAX_SIZE];
    uint8_t  packet_len;
} mr_received_packet_t;

//=========================== callbacks =======================================

typedef void (*mr_event_cb_t)(mr_event_t event, mr_event_data_t event_data);

#endif  // __MODELS_H
