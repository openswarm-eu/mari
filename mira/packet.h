#ifndef __PACKET_H
#define __PACKET_H

/**
 * @ingroup     mira
 * @brief       Packet format and building functions
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

//=========================== defines ==========================================

#define MIRA_PROTOCOL_VERSION 1

//=========================== variables ========================================

typedef enum {
    MIRA_PACKET_BEACON        = 1,
    MIRA_PACKET_JOIN_REQUEST  = 2,
    MIRA_PACKET_JOIN_RESPONSE = 4,
    MIRA_PACKET_KEEPALIVE     = 8,
    MIRA_PACKET_DATA          = 16,
} mr_packet_type_t;

// general packet header
typedef struct __attribute__((packed)) {
    uint8_t          version;
    mr_packet_type_t type;
    uint64_t         dst;
    uint64_t         src;
} mr_packet_header_t;

// beacon packet
typedef struct __attribute__((packed)) {
    uint8_t          version;
    mr_packet_type_t type;
    uint64_t         asn;
    uint64_t         src;
    uint8_t          remaining_capacity;
    uint8_t          active_schedule_id;
} mr_beacon_packet_header_t;

//=========================== prototypes =======================================

size_t mr_build_packet_data(uint8_t *buffer, uint64_t dst, uint8_t *data, size_t data_len);

size_t mr_build_packet_join_request(uint8_t *buffer, uint64_t dst);

size_t mr_build_packet_join_response(uint8_t *buffer, uint64_t dst);

size_t mr_build_packet_keepalive(uint8_t *buffer, uint64_t dst);

size_t mr_build_packet_beacon(uint8_t *buffer, uint64_t asn, uint8_t remaining_capacity, uint8_t active_schedule_id);

#endif
