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
#include "maclow.h"
#include "timer_hf.h"

//=========================== callbacks ========================================

static void rx_cb(uint8_t *packet, uint8_t length)
{
    printf("Gateway application received packet of length %d: ", length);
    for (int i = 0; i < length; i++) {
        printf("%02X ", packet[i]);
    }
    printf("\n");
}

//=========================== variables ========================================

uint8_t packet[4] = { 0xFF, 0xFE, 0xFD, 0xFC };

//=========================== main =============================================

int main(void)
{
    printf("Hello Blink Gateway\n");
    bl_timer_hf_init(BLINK_TIMER_DEV);

    bl_init(NODE_TYPE_GATEWAY, &rx_cb, NULL);

    size_t i = 0;
    while (1) {
        printf("Sending packet %d\n", i++);
        bl_tx(packet, 4);
        bl_timer_hf_delay_ms(BLINK_TIMER_DEV, 1000);
    }

    while (1) {
        __SEV();
        __WFE();
        __WFE();
    }
}
