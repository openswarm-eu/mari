#ifndef __ASSOCIATION_H
#define __ASSOCIATION_H

/**
 * @file
 * @ingroup     blink_assoc
 *
 * @brief       Association procedure for the blink protocol
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024-now
 */

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"
#include "scan.h"

//=========================== defines ==========================================

typedef enum {
    JOIN_STATE_IDLE = 1,
    JOIN_STATE_SCANNING = 2,
    JOIN_STATE_SYNCED = 4,
    JOIN_STATE_JOINING = 8,
    JOIN_STATE_JOINED = 16,
} bl_assoc_state_t;

//=========================== variables ========================================

//=========================== prototypes =======================================

void bl_assoc_init(bl_event_cb_t event_callback);
void bl_assoc_set_state(bl_assoc_state_t join_state);
bl_channel_info_t bl_assoc_select_gateway(uint32_t start_ts, uint32_t end_ts);
bool bl_assoc_node_ready_to_join(void);
void bl_assoc_handle_beacon(uint8_t *packet, uint8_t length, uint8_t channel, uint32_t ts);
void bl_assoc_handle_packet(uint8_t *packet, uint8_t length);

#endif // __ASSOCIATION_H
