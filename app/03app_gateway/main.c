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

#include "bl_radio.h"
#include "bl_timer_hf.h"
#include "blink.h"
#include "packet.h"

//=========================== defines ==========================================

#define BLINK_APP_TIMER_DEV 1

#define DATA_LEN 4

typedef struct {
    bool dummy;
} gateway_vars_t;

//=========================== variables ========================================

gateway_vars_t gateway_vars = { 0 };

uint8_t packet[BLINK_PACKET_MAX_SIZE] = { 0 };
uint8_t payload[] = { 0xFA, 0xFA, 0xFA, 0xFA, 0xFA };
uint8_t payload_len = 5;

extern schedule_t schedule_minuscule, schedule_tiny, schedule_small, schedule_huge, schedule_only_beacons, schedule_only_beacons_optimized_scan;

//=========================== prototypes =======================================

void blink_event_callback(bl_event_t event, bl_event_data_t event_data);
void _debug_print_schedule(void);

//=========================== main =============================================

int main(void)
{
    printf("Hello Blink Gateway\n");
    bl_timer_hf_init(BLINK_APP_TIMER_DEV);

    blink_init(BLINK_GATEWAY, &schedule_minuscule, &blink_event_callback);

    while (1) {
        __SEV();
        __WFE();
        __WFE();

        // for debug only, print the schedule, which includes the list of joined nodes
        _debug_print_schedule();

        // test: send a broadcast packet
        uint8_t packet_len = bl_build_packet_data(packet, BLINK_BROADCAST_ADDRESS, payload, payload_len);
        blink_tx(packet, packet_len);

        // sleep for 500 ms
        bl_timer_hf_delay_ms(BLINK_APP_TIMER_DEV, 500);

        // test: enqueue packets to all connected nodes
        uint64_t nodes[BLINK_MAX_NODES] = { 0 };
        uint8_t nodes_len = blink_gateway_get_nodes(nodes);
        for (int i = 0; i < nodes_len; i++) {
            //printf("Enqueing TX to node %d: %016llX\n", i, nodes[i]);
            payload[0] = i;
            uint8_t packet_len = bl_build_packet_data(packet, nodes[i], payload, payload_len);
            blink_tx(packet, packet_len);
        }

        // sleep for 500 ms
        bl_timer_hf_delay_ms(BLINK_APP_TIMER_DEV, 500);
    }
}

//=========================== callbacks ========================================

void blink_event_callback(bl_event_t event, bl_event_data_t event_data) {
    (void)event_data;
    switch (event) {
        case BLINK_NEW_PACKET: {
            //blink_packet_t packet = event_data.data.new_packet;
            //printf("RX %u B: src=%016llX dst=%016llX (rssi %d) payload=", packet.len, packet.header->src, packet.header->dst, bl_radio_rssi());
            //for (int i = 0; i < packet.payload_len; i++) {
            //    printf("%02X ", packet.payload[i]);
            //}
            //printf("\n");
            break;
        }
        case BLINK_NODE_JOINED:
            //printf("New node joined: %016llX\n", event_data.data.node_info.node_id);
            //uint64_t joined_nodes[BLINK_MAX_NODES] = { 0 };
            //uint8_t joined_nodes_len = blink_gateway_get_nodes(joined_nodes);
            //printf("Number of connected nodes: %d\n", joined_nodes_len);
            // TODO: send list of joined_nodes to Edge Gateway via UART
            break;
        case BLINK_NODE_LEFT:
            //printf("Node left: %016llX, reason: %u\n", event_data.data.node_info.node_id, event_data.tag);
            //printf("Number of connected nodes: %d\n", blink_gateway_count_nodes());
            break;
        case BLINK_ERROR:
            printf("Error\n");
            break;
        default:
            break;
    }
}

//=========================== private ========================================

void _debug_print_schedule(void) {
    //return; // skip printing, just enable when really debugging

    uint8_t schedule_len = schedule_minuscule.n_cells;
    printf("Schedule cells: ");
    for (int i = 0; i < schedule_len; i++) {
        cell_t cell = schedule_minuscule.cells[i];
        if (cell.type == SLOT_TYPE_UPLINK) {
            printf("%d-U-%016llX ", i, cell.assigned_node_id);
        } else if (cell.type == SLOT_TYPE_DOWNLINK) {
            printf("%d-D ", i);
        } else if (cell.type == SLOT_TYPE_BEACON) {
            printf("%d-B ", i);
        } else if (cell.type == SLOT_TYPE_SHARED_UPLINK) {
            printf("%d-S ", i);
        }
    }
    printf("\n");
}