#ifndef __BLINK_H
#define __BLINK_H

/**
 * @defgroup    blink      Blink
 * @brief       Implementation of the blink protocol
 *
 * @{
 * @file
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 * @copyright Inria, 2024-now
 * @}
 */

#include <stdint.h>
#include <stdbool.h>
#include "models.h"

//=========================== defines ==========================================

#define BLINK_MAX_NODES 10 // TODO: find a way to sync with the pre-stored schedules
#define BLINK_BROADCAST_ADDRESS 0xFFFFFFFFFFFFFFFF

//=========================== prototypes ==========================================

void blink_init(bl_node_type_t node_type, schedule_t *app_schedule, bl_event_cb_t app_event_callback);
void blink_tx(uint8_t *packet, uint8_t length);
bl_node_type_t blink_get_node_type(void);
void blink_set_node_type(bl_node_type_t node_type);

size_t blink_gateway_get_nodes(uint64_t *nodes);
size_t blink_gateway_count_nodes(void);

void blink_node_tx(uint8_t *payload, uint8_t payload_len);
bool blink_node_is_connected(void);
uint64_t blink_node_gateway_id(void);

// -------- internal api --------
void bl_handle_packet(uint8_t *packet, uint8_t length);

#endif // __BLINK_H
