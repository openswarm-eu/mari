/**
 * @brief       Build a blink packets
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024
 */
#include <stdint.h>
#include <string.h>
#include "bl_device.h"
#include "packet.h"

//=========================== prototypes =======================================

static size_t _set_header(uint8_t *buffer, uint64_t dst, bl_packet_type_t packet_type);

//=========================== public ===========================================

size_t bl_build_packet_data(uint8_t *buffer, uint64_t dst, uint8_t *data, size_t data_len) {
    size_t header_len = _set_header(buffer, dst, BLINK_PACKET_DATA);
    memcpy(buffer + header_len, data, data_len);
    return header_len + data_len;
}

size_t bl_build_packet_keepalive(uint8_t *buffer, uint64_t dst) {
    return _set_header(buffer, dst, BLINK_PACKET_KEEPALIVE);
}

size_t bl_build_packet_join_request(uint8_t *buffer, uint64_t dst) {
    return _set_header(buffer, dst, BLINK_PACKET_JOIN_REQUEST);
}

size_t bl_build_packet_join_response(uint8_t *buffer, uint64_t dst) {
    return _set_header(buffer, dst, BLINK_PACKET_JOIN_RESPONSE);
}

size_t bl_build_packet_beacon(uint8_t *buffer, uint64_t asn, uint8_t remaining_capacity, uint8_t active_schedule_id) {
    bl_beacon_packet_header_t beacon = {
        .version = BLINK_PROTOCOL_VERSION,
        .type = BLINK_PACKET_BEACON,
        .asn = asn,
        .src = bl_device_id(),
        .remaining_capacity = remaining_capacity,
        .active_schedule_id = active_schedule_id,
    };
    memcpy(buffer, &beacon, sizeof(bl_beacon_packet_header_t));
    return sizeof(bl_beacon_packet_header_t);
}

//=========================== private ==========================================

static size_t _set_header(uint8_t *buffer, uint64_t dst, bl_packet_type_t packet_type) {
    uint64_t src = bl_device_id();

    bl_packet_header_t header = {
        .version = BLINK_PROTOCOL_VERSION,
        .type    = packet_type,
        .dst     = dst,
        .src     = src,
    };
    memcpy(buffer, &header, sizeof(bl_packet_header_t));
    return sizeof(bl_packet_header_t);
}
