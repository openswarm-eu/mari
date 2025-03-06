/**
 * @file
 * @ingroup     blink_assoc
 *
 * @brief       Association procedure for the blink protocol
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024
 */

#include <nrf.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "association.h"
#include "scan.h"
#include "radio.h"

//=========================== defines =========================================

typedef struct {
    // bl_node_type_t          node_type; // get from blink: bl_get_node_type()
    bl_assoc_state_t state;

    // // SCANNING state
    // uint8_t scan_max_slots; ///< Maximum number of slots to scan
    // uint32_t scan_started_ts; ///< Timestamp of the start of the scan
    // uint64_t scan_started_asn; ///< ASN when the scan started
    // uint32_t current_scan_item_ts; ///< Timestamp of the current scan item
    // bl_channel_info_t selected_channel_info;

    // // SYNC/JOINING state
    // bool waiting_join_response;

    // // JOINED state
    // bool is_background_scanning; ///< Whether the node is scanning in the background
} assoc_vars_t;

//=========================== variables =======================================

static assoc_vars_t assoc_vars = { 0 };

//=========================== prototypes ======================================


//=========================== public ==========================================

void bl_assoc_init(void) {
    assoc_vars.state = JOIN_STATE_IDLE;
}

bool bl_assoc_pending_join_packet(void) {
    // return assoc_vars.state == JOIN_STATE_JOINING;
    return false; // FIXME
}

inline void bl_assoc_set_state(bl_assoc_state_t state) {
    assoc_vars.state = state;

#ifdef DEBUG
    // remember: the LEDs are active low
    // DEBUG_GPIO_SET(&led0); DEBUG_GPIO_SET(&led1); DEBUG_GPIO_SET(&led2); DEBUG_GPIO_SET(&led3);
    switch (state) {
        case JOIN_STATE_IDLE:
            // DEBUG_GPIO_CLEAR(&pin1);
            break;
        case JOIN_STATE_SCANNING:
            // DEBUG_GPIO_SET(&pin1);
            // DEBUG_GPIO_CLEAR(&led0);
            break;
        case JOIN_STATE_SYNCED:
            // DEBUG_GPIO_CLEAR(&pin1);
            // DEBUG_GPIO_CLEAR(&led1);
            break;
        case JOIN_STATE_JOINING:
            // DEBUG_GPIO_CLEAR(&led2);
            break;
        case JOIN_STATE_JOINED:
            // DEBUG_GPIO_CLEAR(&led3);
            break;
        default:
            break;
    }
#endif
}

void bl_assoc_handle_beacon(uint8_t *packet, uint8_t length, uint8_t channel, uint32_t ts) {
    (void)length;

    if (packet[1] != BLINK_PACKET_BEACON) {
        return;
    }

    // now that we know it's a beacon packet, parse and process it
    bl_beacon_packet_header_t *beacon = (bl_beacon_packet_header_t *)packet;

    if (beacon->version != BLINK_PROTOCOL_VERSION) {
        // ignore packet with different protocol version
        return;
    }

    if (beacon->remaining_capacity == 0) {
        // this gateway is full, ignore it
        return;
    }

    // save this scan info
    bl_scan_add(*beacon, bl_radio_rssi(), channel, ts, 0); // asn not used anymore during scan

    return;
}

void bl_assoc_handle_packet(uint8_t *packet, uint8_t length) {
    (void)packet;
    (void)length;
    // bl_packet_header_t *header = (bl_packet_header_t *)packet;


    // if (beacon->remaining_capacity == 0) {
    //     // this gateway is full, ignore it
    //     break;
    //

    // // now that we know it's a beacon packet, parse and process it
    // bl_beacon_packet_header_t *beacon = (bl_beacon_packet_header_t *)packet;

    // if (beacon->version != BLINK_PROTOCOL_VERSION) {
    //     // ignore packet with different protocol version
    //     break;
    // }

    // // save this scan info
    // bl_scan_add(*beacon, bl_radio_rssi(), BLINK_FIXED_CHANNEL, assoc_vars.current_scan_item_ts, assoc_vars.asn);


}

bl_channel_info_t bl_assoc_select_gateway(uint32_t start_ts, uint32_t end_ts) {
    return bl_scan_select(start_ts, end_ts);
}

//=========================== callbacks =======================================

//=========================== private =========================================
