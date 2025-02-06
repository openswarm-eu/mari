/**
 * @file
 *
 * @brief       Example on how to use the maclow driver
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024
 */
#include <stdint.h>
#include <string.h>
#include "device.h"
#include "protocol.h"

//=========================== prototypes =======================================

static size_t _set_header(uint8_t *buffer, uint64_t dst, bl_packet_type_t packet_type);

//=========================== public ===========================================

size_t bl_build_packet_data(uint8_t *buffer, uint64_t dst, uint8_t *data, size_t data_len) {
    size_t header_len = _set_header(buffer, dst, BLINK_PACKET_DATA);
    memcpy(buffer + header_len, data, data_len);
    return header_len + data_len;
}

size_t bl_build_packet_join_request(uint8_t *buffer, uint64_t dst) {
    return _set_header(buffer, dst, BLINK_PACKET_JOIN_REQUEST);
}

size_t bl_build_packet_join_response(uint8_t *buffer, uint64_t dst) {
    return _set_header(buffer, dst, BLINK_PACKET_JOIN_RESPONSE);
}

//=========================== private ==========================================

static size_t _set_header(uint8_t *buffer, uint64_t dst, bl_packet_type_t packet_type) {
    uint64_t src = db_device_id();

    bl_packet_header_t header = {
        .version = BLINK_PROTOCOL_VERSION,
        .type    = packet_type,
        .dst     = dst,
        .src     = src,
    };
    memcpy(buffer, &header, sizeof(bl_packet_header_t));
    return sizeof(bl_packet_header_t);
}
