#ifndef __BLINK_H
#define __BLINK_H

#include <stdint.h>

//=========================== defines ==========================================

#define BLINK_MAX_NODES 10 // TODO: find a way to sync with the pre-stored schedules

//=========================== variables ========================================

typedef enum {
    CONNECTED,
    DISCONNECTED,
} bl_event_t;

typedef enum {
    NODE_TYPE_GATEWAY = 'G',
    NODE_TYPE_NODE = 'D',
} bl_node_type_t;

typedef void (*bl_rx_cb_t)(uint8_t *packet, uint8_t length);  ///< Function pointer to the callback function called on packet receive
typedef void (*bl_events_cb_t)(bl_event_t event);             ///< Function pointer to the callback function called for network events

//=========================== prototypes ==========================================

void bl_init(bl_node_type_t node_type, bl_rx_cb_t rx_app_callback, bl_events_cb_t events_callback);

void bl_tx(uint8_t *packet, uint8_t length);

void bl_get_joined_nodes(uint64_t *nodes, uint8_t *num_nodes);

#endif // __BLINK_H
