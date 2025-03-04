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
#include "blink.h"
#include "mac.h"
#include "scan.h"
#include "scheduler.h"

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

inline void bl_assoc_set_state(bl_assoc_state_t join_state) {
    mac_vars.join_state = join_state;

#ifdef DEBUG
    // remember: the LEDs are active low
    DEBUG_GPIO_SET(&led0); DEBUG_GPIO_SET(&led1); DEBUG_GPIO_SET(&led2); DEBUG_GPIO_SET(&led3);
    switch (join_state) {
        case JOIN_STATE_IDLE:
            DEBUG_GPIO_CLEAR(&pin1);
            break;
        case JOIN_STATE_SCANNING:
            DEBUG_GPIO_SET(&pin1);
            DEBUG_GPIO_CLEAR(&led0);
            break;
        case JOIN_STATE_SYNCED:
            DEBUG_GPIO_CLEAR(&pin1);
            DEBUG_GPIO_CLEAR(&led1);
            break;
        case JOIN_STATE_JOINING:
            DEBUG_GPIO_CLEAR(&led2);
            break;
        case JOIN_STATE_JOINED:
            DEBUG_GPIO_CLEAR(&led3);
            if (mac_vars.is_background_scanning) {
                DEBUG_GPIO_CLEAR(&led0);
            }
            break;
        default:
            break;
    }
#endif
}

void bl_assoc_handle_packet(uint8_t *packet, uint8_t length) {
    (void)packet;
    (void)length;
    // bl_packet_header_t *header = (bl_packet_header_t *)packet;
}

//=========================== callbacks =======================================

//=========================== private =========================================
