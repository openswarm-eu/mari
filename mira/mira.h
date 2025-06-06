#ifndef __MIRA_H
#define __MIRA_H

/**
 * @defgroup    mira      Mira
 * @brief       Implementation of the mira protocol
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

#define MIRA_MAX_NODES         101  // FIXME: find a way to sync with the pre-stored schedules
#define MIRA_BROADCAST_ADDRESS 0xFFFFFFFFFFFFFFFF

//=========================== prototypes ==========================================

void           mira_init(mr_node_type_t node_type, uint16_t net_id, schedule_t *app_schedule, mr_event_cb_t app_event_callback);
void           mira_event_loop(void);
void           mira_tx(uint8_t *packet, uint8_t length);
mr_node_type_t mira_get_node_type(void);
void           mira_set_node_type(mr_node_type_t node_type);

size_t mira_gateway_get_nodes(uint64_t *nodes);
size_t mira_gateway_count_nodes(void);

void     mira_node_tx_payload(uint8_t *payload, uint8_t payload_len);
bool     mira_node_is_connected(void);
uint64_t mira_node_gateway_id(void);

// -------- internal api --------
void mr_handle_packet(uint8_t *packet, uint8_t length);

#endif  // __MIRA_H
