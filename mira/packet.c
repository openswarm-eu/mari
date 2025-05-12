/**
 * @brief       Build a mira packets
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024
 */
#include <stdint.h>
#include <string.h>
#include "mr_device.h"
#include "packet.h"

//=========================== prototypes =======================================

static size_t _set_header(uint8_t *buffer, uint64_t dst, mr_packet_type_t packet_type);

//=========================== public ===========================================

size_t mr_build_packet_data(uint8_t *buffer, uint64_t dst, uint8_t *data, size_t data_len) {
    size_t header_len = _set_header(buffer, dst, MIRA_PACKET_DATA);
    memcpy(buffer + header_len, data, data_len);
    return header_len + data_len;
}

size_t mr_build_packet_keepalive(uint8_t *buffer, uint64_t dst) {
    return _set_header(buffer, dst, MIRA_PACKET_KEEPALIVE);
}

size_t mr_build_packet_join_request(uint8_t *buffer, uint64_t dst) {
    return _set_header(buffer, dst, MIRA_PACKET_JOIN_REQUEST);
}

size_t mr_build_packet_join_response(uint8_t *buffer, uint64_t dst) {
    return _set_header(buffer, dst, MIRA_PACKET_JOIN_RESPONSE);
}

size_t mr_build_packet_beacon(uint8_t *buffer, uint64_t asn, uint8_t remaining_capacity, uint8_t active_schedule_id) {
    mr_beacon_packet_header_t beacon = {
        .version = MIRA_PROTOCOL_VERSION,
        .type = MIRA_PACKET_BEACON,
        .asn = asn,
        .src = mr_device_id(),
        .remaining_capacity = remaining_capacity,
        .active_schedule_id = active_schedule_id,
    };
    memcpy(buffer, &beacon, sizeof(mr_beacon_packet_header_t));
    return sizeof(mr_beacon_packet_header_t);
}

//=========================== private ==========================================

static size_t _set_header(uint8_t *buffer, uint64_t dst, mr_packet_type_t packet_type) {
    uint64_t src = mr_device_id();

    mr_packet_header_t header = {
        .version = MIRA_PROTOCOL_VERSION,
        .type    = packet_type,
        .dst     = dst,
        .src     = src,
    };
    memcpy(buffer, &header, sizeof(mr_packet_header_t));
    return sizeof(mr_packet_header_t);
}
