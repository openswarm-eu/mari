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

#include "blink.h"
#include "protocol.h"

//=========================== defines ==========================================

typedef struct {
    bool is_connected;
} node_vars_t;

//=========================== variables ========================================

node_vars_t node_vars = { 0 };

uint8_t packet[BLINK_PACKET_MAX_SIZE] = { 0 };
uint64_t dst = 0xb67d4fc5f7679e6b; // hardcoded gateway

//=========================== callbacks ========================================

// NOTE: to test this callback right now, manually set is_connected to true in blink.c
void rx_cb(uint8_t *packet, uint8_t length)
{
    printf("Node application received packet of length %d: ", length);
    for (int i = 0; i < length; i++) {
        printf("%02X ", packet[i]);
    }
    printf("\n");
}

void event_cb(bl_event_t event)
{
    switch (event) {
    case BLINK_CONNECTED:
        node_vars.is_connected = true;
        break;
    case BLINK_DISCONNECTED:
        node_vars.is_connected = false;
        break;
    default:
        break;
    }
}

//=========================== main =============================================

int main(void)
{
    printf("Hello Blink Node\n");

    bl_init(NODE_TYPE_NODE, &rx_cb, &event_cb);

    size_t i = 0;
    while (1) {
        __SEV();
        __WFE();
        __WFE();

        printf("Node is %s\n", node_vars.is_connected ? "connected" : "disconnected");

        if (i++ % 10 == 0) {
            // for now, the join is very artificial, just to test the callbacks
            printf("Sending JOIN_REQUEST packet\n");
            size_t packet_len = bl_build_packet_join_request(packet, dst);
            bl_tx(packet, packet_len);
        }
    }
}
