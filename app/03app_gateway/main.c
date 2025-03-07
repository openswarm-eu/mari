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

#include "blink.h"
#include "mac.h"
#include "protocol.h"
#include "timer_hf.h"

//=========================== defines ==========================================

#define DATA_LEN 4

typedef struct {
    uint8_t connected_nodes;
} gateway_vars_t;

//=========================== variables ========================================

gateway_vars_t gateway_vars = { 0 };

uint8_t packet[BLINK_PACKET_MAX_SIZE] = { 0 };
uint8_t data[DATA_LEN] = { 0xFF, 0xFE, 0xFD, 0xFC };
uint64_t dst = 0x1;

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
    }
}

//=========================== main =============================================

int main(void)
{
    printf("Hello Blink Gateway\n");
    bl_timer_hf_init(BLINK_TIMER_DEV);

    bl_init(BLINK_GATEWAY, &blink_event_callback);

    size_t i = 0;
    size_t packet_len = 0;
    while (1) {
        if (i++ % 10 == 0) {
            // for now, the join is very artificial, just to test the callbacks
            printf("Sending JOIN_RESPONSE packet\n");
            packet_len = bl_build_packet_join_response(packet, dst);
        } else {
            printf("Sending DATA packet %d\n", i);
            packet_len = bl_build_packet_data(packet, dst, data, DATA_LEN);
        }
        bl_tx(packet, packet_len);

        bl_timer_hf_delay_ms(BLINK_TIMER_DEV, 1000);
    }
}
