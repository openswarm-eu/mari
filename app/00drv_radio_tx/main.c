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

#include "radio.h"
#include "timer_hf.h"
#include "protocol.h"
#include "scheduler.h"

//=========================== debug ============================================

#include "gpio.h"
gpio_t pin0 = { .port = 1, .pin = 2 }; // variable names reflect the logic analyzer channels
gpio_t pin1 = { .port = 1, .pin = 3 };
gpio_t pin2 = { .port = 1, .pin = 4 };
gpio_t pin3 = { .port = 1, .pin = 5 };
#define DEBUG_GPIO_TOGGLE(pin) db_gpio_toggle(pin)
#define DEBUG_GPIO_SET(pin) db_gpio_set(pin)
#define DEBUG_GPIO_CLEAR(pin) db_gpio_clear(pin)

//=========================== defines =========================================

typedef struct {
    uint64_t asn;
} tx_vars_t;

//=========================== variables =======================================

tx_vars_t tx_vars = { 0 };

extern schedule_t schedule_only_beacons;

//=========================== prototypes ======================================

static void send_beacon(void);

static void isr_radio_rx(uint8_t *packet, uint8_t length);
//static void isr_radio_start_frame(uint32_t ts);
//static void isr_radio_end_frame(uint32_t ts);

//=========================== main ============================================

int main(void) {
    bl_timer_hf_init(BLINK_TIMER_DEV);
    db_gpio_init(&pin0, DB_GPIO_OUT);

    bl_radio_init(&isr_radio_rx, NULL, NULL, DB_RADIO_BLE_2MBit);
    bl_radio_set_channel(BLINK_FIXED_CHANNEL);

    printf("BLINK_FIXED_CHANNEL = %d\n", BLINK_FIXED_CHANNEL);

    tx_vars.asn = 32; // start at arbitrary value

    while (1) {
        send_beacon();
        //printf("Sent beacon, asn = %lld\n", tx_vars.asn);
        bl_timer_hf_delay_us(BLINK_TIMER_DEV, 1000); // 1 s
    }
}

//=========================== private =========================================

static void send_beacon(void) {
    uint8_t packet[BLINK_PACKET_MAX_SIZE] = { 0 };
    size_t len = bl_build_packet_beacon(packet, tx_vars.asn++, 10, schedule_only_beacons.id);
    bl_radio_tx_prepare(packet, len);
    bl_radio_disable();
    DEBUG_GPIO_SET(&pin0);
    bl_radio_tx_dispatch();
    DEBUG_GPIO_CLEAR(&pin0);
}

static void isr_radio_rx(uint8_t *packet, uint8_t length) {
    (void) packet;
    (void) length;
    //printf("Received packet of length %d\n", length);
    //for (uint8_t i = 0; i < length; i++) {
    //    printf("%02x ", packet[i]);
    //}
    //puts("");
}
