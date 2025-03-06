#ifndef __MODELS_H
#define __MODELS_H

#include <stdint.h>
#include <stdlib.h>
#include <nrf.h>


#define BLINK_N_BLE_REGULAR_CHANNELS 37
#define BLINK_N_BLE_ADVERTISING_CHANNELS 3

typedef enum {
    BLINK_GATEWAY = 'G',
    BLINK_NODE = 'D',
} bl_node_type_t;

typedef enum {
    BLINK_NEW_PACKET,
    BLINK_CONNECTED,
    BLINK_DISCONNECTED,
    BLINK_NODE_JOINED,
    BLINK_NODE_LEFT,
} bl_event_t;

typedef struct {
    union {
        struct {
            uint8_t *packet;
            uint8_t length;
        } new_packet;
        struct {
            uint64_t node_id;
        } node_info;
    } data;
} bl_event_data_t;

typedef enum {
    BLINK_RADIO_ACTION_SLEEP = 'S',
    BLINK_RADIO_ACTION_RX = 'R',
    BLINK_RADIO_ACTION_TX = 'T',
} bl_radio_action_t;

typedef enum {
    SLOT_TYPE_BEACON = 'B',
    SLOT_TYPE_SHARED_UPLINK = 'S',
    SLOT_TYPE_DOWNLINK = 'D',
    SLOT_TYPE_UPLINK = 'U',
} slot_type_t;

typedef struct {
    bl_radio_action_t radio_action;
    uint8_t channel;
    slot_type_t type;
    bool available_for_scan;
    bool slot_can_join;
} bl_slot_info_t;

// // NOTE: could have these be a single callback (*bl_event_cb_t)(bl_event_t event, uint8_t *packet, uint8_t length), and then one of the events can be BLINK_RX
// typedef void (*bl_rx_cb_t)(uint8_t *packet, uint8_t length);  ///< Function pointer to the callback function called on packet receive
// typedef void (*bl_event_cb_t)(bl_event_t event);             ///< Function pointer to the callback function called for network events

typedef void (*bl_event_cb_t)(bl_event_t event, bl_event_data_t event_data);

#endif // __MODELS_H
