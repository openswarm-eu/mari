#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdint.h>
#include <stdlib.h>
#include <nrf.h>

//=========================== variables ========================================

typedef enum {
    DOTLINK_PACKET_TYPE_BEACON = 1,
    DOTLINK_PACKET_TYPE_JOIN_REQUEST = 2,
    DOTLINK_PACKET_TYPE_JOIN_RESPONSE = 3,
    DOTLINK_PACKET_TYPE_INFRASTRUCTURE_DATA = 8,
    DOTLINK_PACKET_TYPE_EXPERIMENT_DATA = 9,
} dl_packet_type_t;

// general packet header
typedef struct {
    uint8_t version;
    dl_packet_type_t type;
    uint64_t dst;
    uint64_t src;
} dl_packet_header_t;

// beacon packet
typedef struct {
    uint8_t version;
    dl_packet_type_t type;
    uint64_t asn;
    uint64_t src;
    uint8_t remaining_capacity;
    uint8_t active_schedule_id;
} dl_beacon_packet_header_t;

typedef struct {
    uint32_t start_ts;
    uint32_t end_ts;
    uint32_t frequency;
    union {
        dl_packet_header_t header;
        dl_beacon_packet_header_t header_beacon;
    };
} dl_annotated_header_t;

#endif
