#ifndef __MODELS_H
#define __MODELS_H

#include <stdint.h>
#include <stdlib.h>
#include <nrf.h>


typedef enum {
    BLINK_GATEWAY = 'G',
    BLINK_NODE = 'D',
} bl_node_type_t;

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

#endif // __MODELS_H
