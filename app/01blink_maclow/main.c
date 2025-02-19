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

#include "mac.h"
#include "scheduler.h"
#include "device.h"

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

static void radio_callback(uint8_t *packet, uint8_t length);
static void print_slot_timing(void);

int main(void) {
    // initialize schedule
    schedule_t schedule = schedule_only_beacons;

    //bl_node_type_t node_type = BLINK_GATEWAY;
    bl_node_type_t node_type = BLINK_NODE;

    print_slot_timing();

    bl_scheduler_init(node_type, &schedule);
    printf("\n==== Device of type %c and id %llx is using schedule 0x%0X ====\n\n", node_type, db_device_id(), schedule.id);

    printf("BLINK_FIXED_CHANNEL = %d\n", BLINK_FIXED_CHANNEL);

    // initialize the TSCH driver
    //bl_default_slot_timing.end_guard = 1000 * 1000; // add an extra second of delay.
    bl_mac_init(node_type, radio_callback);
    //printf("Slot duration: %d us\n", bl_default_slot_timing.whole_slot);

    while (1) {
        __WFE();
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

static void radio_callback(uint8_t *packet, uint8_t length) {
    (void) packet;
    (void) length;
    //printf("Received packet of length %d\n", length);
    //for (uint8_t i = 0; i < length; i++) {
    //    printf("%02x ", packet[i]);
    //}
    //puts("");
}
