#include <nrf.h>
#include <stdint.h>

#include "maclow.h"

//=========================== variables ========================================

//=========================== prototypes =======================================

//=========================== public ===========================================

void bl_init(bl_node_type_t node_type, bl_rx_cb_t rx_app_callback, bl_events_cb_t events_callback) {
    (void)node_type;
    (void)rx_app_callback;
    (void)events_callback;
    // init maclow, e.g., bl_maclow_init(node_type, rx_app_callback);
    // init shceudler, e.g., bl_scheduler_init(node_type, &schedule_test);
}

void bl_tx(uint8_t *packet, uint8_t length) {
    (void)packet;
    (void)length;
    // equeue packet
}
