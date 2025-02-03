#include <nrf.h>
#include <stdint.h>

#include "maclow.h"
#include "radio.h"

//=========================== defines ==========================================

#define BLINK_DEFAULT_FREQUENCY (8)

//=========================== variables ========================================

//=========================== prototypes =======================================

//=========================== public ===========================================

void bl_init(bl_node_type_t node_type, bl_rx_cb_t rx_app_callback, bl_events_cb_t events_callback) {
    (void)node_type;
    (void)events_callback;

    bl_radio_init(rx_app_callback, DB_RADIO_BLE_2MBit);
    bl_radio_set_frequency(BLINK_DEFAULT_FREQUENCY); // temporary value
    bl_radio_rx();

    // TODO:
    // - init maclow, e.g., bl_maclow_init(node_type, rx_app_callback);
    // - init scheduler, e.g., bl_scheduler_init(node_type, &schedule_test);
}

void bl_tx(uint8_t *packet, uint8_t length) {
    bl_radio_disable();
    bl_radio_tx(packet, length); // TODO: use a packet queue
}
