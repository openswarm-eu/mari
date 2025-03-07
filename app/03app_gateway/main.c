/**
 * @file
 * @ingroup     app
 *
 * @brief       Blink Gateway application example
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024
 */
#include <nrf.h>
#include <stdio.h>

#include "timer_hf.h"
#include "blink.h"
#include "mac.h"
#include "protocol.h"

//=========================== defines ==========================================

#define BLINK_APP_TIMER_DEV 1

#define DATA_LEN 4

typedef struct {
    uint8_t connected_nodes;
} gateway_vars_t;

//=========================== variables ========================================

gateway_vars_t gateway_vars = { 0 };

uint8_t packet[BLINK_PACKET_MAX_SIZE] = { 0 };
uint8_t payload[] = { 0xFA, 0xFA, 0xFA, 0xFA, 0xFA };
uint8_t payload_len = 5;

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
        case BLINK_NODE_JOINED:
            printf("New node joined: %016llX\n", event_data.data.node_info.node_id);
            gateway_vars.connected_nodes++; // FIXME have this be managed within the blink library
            uint64_t joined_nodes[BLINK_MAX_NODES] = { 0 };
            uint8_t joined_nodes_len = 0;
            bl_get_joined_nodes(joined_nodes, &joined_nodes_len);
            // TODO: send to Edge Gateway via UART
            break;
        case BLINK_NODE_LEFT:
            printf("Node left: %016llX\n", event_data.data.node_info.node_id);
            if (gateway_vars.connected_nodes > 0) {
                gateway_vars.connected_nodes--; // FIXME have this be managed within the blink library
            }
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
    printf("Hello Blink Gateway\n");
    bl_timer_hf_init(BLINK_APP_TIMER_DEV);

    bl_init(BLINK_GATEWAY, &schedule_minuscule, &blink_event_callback);

    while (1) {
        __SEV();
        __WFE();
        __WFE();

        uint8_t packet_len = bl_build_packet_data(packet, BLINK_BROADCAST_ADDRESS, payload, payload_len);

        bl_tx(packet, packet_len);

        // sleep for 500 ms
        bl_timer_hf_delay_ms(BLINK_APP_TIMER_DEV, 500);
    }
}
