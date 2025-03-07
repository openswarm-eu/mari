/**
 * @file
 * @ingroup     app
 *
 * @brief       Blink Node application example
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024
 */
#include <nrf.h>
#include <stdio.h>
#include <stdbool.h>

#include "timer_hf.h"
#include "blink.h"
#include "protocol.h"

//=========================== defines ==========================================

#define BLINK_APP_TIMER_DEV 1

typedef struct {
    bool is_connected;
    uint64_t connected_gateway;
} node_vars_t;

//=========================== variables ========================================

node_vars_t node_vars = { 0 };

uint8_t packet[BLINK_PACKET_MAX_SIZE] = { 0 };
uint8_t hello[] = { 0x48, 0x65, 0x6C, 0x6C, 0x6F }; // "Hello"

extern schedule_t schedule_minuscule, schedule_small, schedule_huge, schedule_only_beacons, schedule_only_beacons_optimized_scan;

//=========================== callbacks ========================================

void blink_event_callback(bl_event_t event, bl_event_data_t event_data) {
    switch (event) {
        case BLINK_NEW_PACKET:
            printf("Blink received data packet of length %d: ", event_data.data.new_packet.length);
            for (int i = 0; i < event_data.data.new_packet.length; i++) {
                printf("%02X ", event_data.data.new_packet.packet[i]);
            }
            printf("\n");
            break;
        case BLINK_CONNECTED:
            printf("Connected\n");
            node_vars.is_connected = true;
            node_vars.connected_gateway = event_data.data.gateway_info.gateway_id;
            break;
        case BLINK_DISCONNECTED:
            printf("Disconnected\n");
            break;
        case BLINK_ERROR:
            printf("Error\n");
            break;
        default:
            break;
    }
}

//=========================== main =============================================

int main(void)
{
    printf("Hello Blink Node\n");
    bl_timer_hf_init(BLINK_APP_TIMER_DEV);

    bl_init(BLINK_NODE, &schedule_minuscule, &blink_event_callback);

    while (1) {
        __SEV();
        __WFE();
        __WFE();

        if (node_vars.is_connected) {
            // enqueue a 'hello' packet
            uint8_t packet_len = bl_build_packet_data(packet, node_vars.connected_gateway, hello, 5);

            bl_tx(packet, packet_len);

            // sleep for 500 ms
            bl_timer_hf_delay_ms(BLINK_APP_TIMER_DEV, 500);
        }
    }
}
