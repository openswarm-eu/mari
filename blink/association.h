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

//=========================== defines ==========================================

// default scan duration in us
#define BLINK_SCAN_MAX_SLOTS (7) // how many slots to scan for. should probably be the size of the largest schedule

//=========================== variables ========================================

//=========================== prototypes =======================================

void bl_assoc_init(void);
bool bl_assoc_pending_join_packet(void);

#endif // __ASSOCIATION_H
