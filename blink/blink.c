#include <nrf.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "models.h"
#include "mac.h"
#include "scheduler.h"
#include "association.h"
#include "queue.h"
#include "blink.h"

//=========================== defines ==========================================

typedef struct {
    bl_node_type_t          node_type;
    bl_event_cb_t           app_event_callback;

    // gateway only
    uint64_t                joined_nodes[BLINK_MAX_NODES];
    uint8_t                 joined_nodes_len;
} blink_vars_t;

//=========================== variables ========================================

static blink_vars_t _blink_vars = { 0 };

//=========================== prototypes =======================================

static void event_callback(bl_event_t event, bl_event_data_t event_data);

//=========================== public ===========================================

void bl_init(bl_node_type_t node_type, schedule_t *app_schedule, bl_event_cb_t app_event_callback) {
    _blink_vars.node_type = node_type;
    _blink_vars.app_event_callback = app_event_callback;

    bl_assoc_init(event_callback);
    bl_scheduler_init(node_type, app_schedule);
    bl_mac_init(node_type, event_callback);
}

void bl_tx(uint8_t *packet, uint8_t length) {
    // enqueue for transmission
    bl_queue_add(packet, length);
}

void bl_get_joined_nodes(uint64_t *nodes, uint8_t *num_nodes) {
    *num_nodes = _blink_vars.joined_nodes_len;
    memcpy(nodes, _blink_vars.joined_nodes, _blink_vars.joined_nodes_len * sizeof(uint64_t));
}

bl_node_type_t bl_get_node_type(void) {
    return _blink_vars.node_type;
}

void bl_set_node_type(bl_node_type_t node_type) {
    _blink_vars.node_type = node_type;
}

//=========================== callbacks ===========================================

static void event_callback(bl_event_t event, bl_event_data_t event_data) {
    // NOTE: this intermediate callback is rather useless now, but it could be used to store debug info, i.e, number of packets received, etc.
    if (_blink_vars.app_event_callback) {
        _blink_vars.app_event_callback(event, event_data);
    }
}
