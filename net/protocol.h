#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdint.h>
#include <stdlib.h>
#include <nrf.h>

//=========================== variables ========================================

typedef enum {
    BLINK_PACKET_TYPE_BEACON = 1,
    BLINK_PACKET_TYPE_JOIN_REQUEST = 2,
    BLINK_PACKET_TYPE_JOIN_RESPONSE = 3,
    BLINK_PACKET_TYPE_INFRASTRUCTURE_DATA = 8,
    BLINK_PACKET_TYPE_EXPERIMENT_DATA = 9,
} bl_packet_type_t;

// general packet header
typedef struct __attribute__((packed)) {
    uint8_t           version;
    bl_packet_type_t  type;
    uint64_t          dst;
    uint64_t          src;
} bl_packet_header_t;

// beacon packet
typedef struct __attribute__((packed)) {
    uint8_t           version;
    bl_packet_type_t  type;
    uint64_t          asn;
    uint64_t          src;
    uint8_t           remaining_capacity;
    uint8_t           active_schedule_id;
} bl_beacon_packet_header_t;

#endif
