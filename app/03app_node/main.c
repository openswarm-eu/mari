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

#include "blink.h"

//=========================== callbacks ========================================

void rx_cb(uint8_t *packet, uint8_t length)
{
    printf("Node application received packet of length %d: ", length);
    for (int i = 0; i < length; i++) {
        printf("%02X ", packet[i]);
    }
    printf("\n");
}

//=========================== variables ========================================

//=========================== main =============================================

int main(void)
{
    printf("Hello Blink Node\n");

    bl_init(NODE_TYPE_NODE, &rx_cb, NULL);

    while (1) {
        __SEV();
        __WFE();
        __WFE();
    }
}
