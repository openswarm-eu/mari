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

#include "blink.h"
#include "mac.h"
#include "scan.h"
#include "scheduler.h"

//=========================== defines =========================================

typedef enum {
    JOIN_STATE_IDLE = 1,
    JOIN_STATE_SCANNING = 2,
    JOIN_STATE_SYNCED = 4,
    JOIN_STATE_JOINING = 8,
    JOIN_STATE_JOINED = 16,
} bl_assoc_state_t;

typedef struct {
    // bl_node_type_t          node_type; // get from blink: bl_get_node_type()
    bl_assoc_state_t state;
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

//=========================== callbacks =======================================

//=========================== private =========================================
