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

//=========================== defines ==========================================

typedef struct {
    bool is_connected;
} node_vars_t;

//=========================== variables ========================================

node_vars_t node_vars = { 0 };

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

    while (1) {
        __SEV();
        __WFE();
        __WFE();

        printf("Node is %s\n", node_vars.is_connected ? "connected" : "disconnected");
    }
}
