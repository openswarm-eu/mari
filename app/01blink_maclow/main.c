/**
 * @file
 * @ingroup     net_maclow
 *
 * @brief       Example on how to use the maclow driver
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024
 */
#include <nrf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "device.h"
#include "mac.h"
#include "scheduler.h"
#include "queue.h"

/* Very simple test schedule */
schedule_t schedule_test = {
    .id = 32, // make sure it doesn't collide
    .max_nodes = 0,
    .backoff_n_min = 5,
    .backoff_n_max = 9,
    .n_cells = 5,
    .cells = {
        //{'B', 0, NULL},
        //{'S', 1, NULL},
        //{'D', 2, NULL},
        //{'U', 3, NULL},
        //{'U', 4, NULL},

        {'S', 0, NULL},
        {'B', 1, NULL},
        {'B', 2, NULL},
        {'B', 3, NULL},
        {'B', 4, NULL},

        //{'U', 0, NULL},
        //{'U', 1, NULL},
        //{'U', 2, NULL},
        //{'U', 3, NULL},
        //{'U', 4, NULL},
    }
};

extern schedule_t schedule_minuscule, schedule_small, schedule_huge, schedule_only_beacons, schedule_only_beacons_optimized_scan;
extern bl_slot_durations_t slot_durations;

// static void radio_callback(uint8_t *packet, uint8_t length);
void blink_event_callback(bl_event_t event, bl_event_data_t event_data);
static void print_slot_timing(void);

int main(void) {
    // initialize schedule
    schedule_t schedule = schedule_huge;

    //bl_node_type_t node_type = BLINK_GATEWAY;
    bl_node_type_t node_type = BLINK_NODE;

    print_slot_timing();

    bl_set_node_type(node_type);

    bl_assoc_init(blink_event_callback);

    bl_scheduler_init(node_type, &schedule);
    printf("\n==== Device of type %c and id %llx is using schedule 0x%0X ====\n\n", node_type, db_device_id(), schedule.id);

    //printf("BLINK_FIXED_CHANNEL = %d\n", BLINK_FIXED_CHANNEL);

    // initialize the mac
    bl_mac_init(node_type, blink_event_callback);

    while (1) {
        __WFE();
    }
}

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
            break;
        case BLINK_NODE_LEFT:
            printf("Node left: %016llX\n", event_data.data.node_info.node_id);
            break;
        case BLINK_CONNECTED:
            printf("Connected\n");
            // enqueue a 'hello' packet
            uint8_t packet[BLINK_PACKET_MAX_SIZE] = { 0 };
            uint8_t data[] = { 0x48, 0x65, 0x6C, 0x6C, 0x6F }; // "Hello"

            uint8_t packet_len = bl_build_packet_data(packet, event_data.data.gateway_info.gateway_id, data, 5);

            bl_queue_add(packet, packet_len);
            bl_queue_add(packet, packet_len);
            bl_queue_add(packet, packet_len);
            break;
        case BLINK_DISCONNECTED:
            printf("Disconnected\n");
            break;
        case BLINK_ERROR:
            printf("Error\n");
            break;
    }
}

static void print_slot_timing(void) {
    printf("Slot timing:\n");
    printf("  tx_offset: %d\n", slot_durations.tx_offset);
    printf("  tx_max: %d\n", slot_durations.tx_max);
    printf("  rx_guard: %d\n", slot_durations.rx_guard);
    printf("  rx_offset: %d\n", slot_durations.rx_offset);
    printf("  rx_max: %d\n", slot_durations.rx_max);
    printf("  end_guard: %d\n", slot_durations.end_guard);
    printf("  whole_slot: %d\n", slot_durations.whole_slot);
}

// static void radio_callback(uint8_t *packet, uint8_t length) {
//     (void) packet;
//     (void) length;
//     //printf("Received packet of length %d\n", length);
//     //for (uint8_t i = 0; i < length; i++) {
//     //    printf("%02x ", packet[i]);
//     //}
//     //puts("");
// }
