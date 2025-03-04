#ifndef __BLINK_H
#define __BLINK_H

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"

//=========================== defines ==========================================

#define BLINK_MAX_NODES 10 // TODO: find a way to sync with the pre-stored schedules

#define BLINK_BROADCAST_ADDRESS 0xFFFFFFFFFFFFFFFF

//=========================== variables ========================================

typedef enum {
    BLINK_CONNECTED,
    BLINK_DISCONNECTED,
    BLINK_NODE_JOINED,
    BLINK_NODE_LEFT,
} bl_event_t;

typedef enum {
    BLINK_GATEWAY = 'G',
    BLINK_NODE = 'D',
} bl_node_type_t;

typedef void (*bl_rx_cb_t)(uint8_t *packet, uint8_t length);  ///< Function pointer to the callback function called on packet receive
typedef void (*bl_event_cb_t)(bl_event_t event);             ///< Function pointer to the callback function called for network events

//=========================== prototypes ==========================================

void bl_init(bl_node_type_t node_type, bl_rx_cb_t rx_app_callback, bl_event_cb_t events_callback);

void bl_tx(uint8_t *packet, uint8_t length);

void bl_queue_add(uint8_t *packet, uint8_t length);
uint8_t bl_queue_next_packet(slot_type_t slot_type, uint8_t *packet);
bool bl_queue_peek(uint8_t *packet, uint8_t *length);
bool bl_queue_pop(void);

void bl_queue_set_join_packet(uint64_t node_id, bl_packet_type_t packet_type);
bool bl_queue_has_join_packet(void);
void bl_queue_get_join_packet(uint8_t *packet, uint8_t *length);

void bl_get_joined_nodes(uint64_t *nodes, uint8_t *num_nodes);

#endif // __BLINK_H
