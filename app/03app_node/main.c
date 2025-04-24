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

#include "bl_gpio.h"
#include "bl_radio.h"
#include "bl_timer_hf.h"
#include "blink.h"
#include "packet.h"

//=========================== defines ==========================================

#define BLINK_APP_TIMER_DEV 1

typedef struct {
    bool dummy;
} node_vars_t;

typedef enum {
    OFF,
    RED,
    GREEN,
    BLUE,
} led_color_t;

//=========================== variables ========================================

bl_gpio_t _r_led_pin = { .port = 0, .pin = 28 };
bl_gpio_t _g_led_pin = { .port = 0, .pin = 2 };
bl_gpio_t _b_led_pin = { .port = 0, .pin = 3 };

node_vars_t node_vars = { 0 };

uint8_t payload[] = { 0xF0, 0xF0, 0xF0, 0xF0, 0xF0 };
uint8_t payload_len = 5;

extern schedule_t schedule_minuscule, schedule_tiny, schedule_small, schedule_huge, schedule_only_beacons, schedule_only_beacons_optimized_scan;

//=========================== prototypes =======================================

static void debug_init_minimote(void);
static void debug_set_rgb(led_color_t color);
static void blink_event_callback(bl_event_t event, bl_event_data_t event_data);

//=========================== main =============================================

int main(void)
{
    printf("Hello Blink Node\n");
    bl_timer_hf_init(BLINK_APP_TIMER_DEV);
    debug_init_minimote();
    debug_set_rgb(RED);

    blink_init(BLINK_NODE, &schedule_minuscule, &blink_event_callback);

    while (1) {
        __SEV();
        __WFE();
        __WFE();

        if (blink_node_is_connected()) {
            blink_node_tx_payload(payload, payload_len);

            // sleep for 500 ms
            bl_timer_hf_delay_ms(BLINK_APP_TIMER_DEV, 500);
        }
    }
}

//=========================== callbacks ========================================

static void blink_event_callback(bl_event_t event, bl_event_data_t event_data) {
    switch (event) {
        case BLINK_NEW_PACKET: {
            blink_packet_t packet = event_data.data.new_packet;
            printf("RX %u B: src=%016llX dst=%016llX (rssi %d) payload=", packet.len, packet.header->src, packet.header->dst, bl_radio_rssi());
            for (int i = 0; i < packet.payload_len; i++) {
                printf("%02X ", packet.payload[i]);
            }
            printf("\n");
            debug_set_rgb(BLUE);
            bl_timer_hf_delay_ms(BLINK_APP_TIMER_DEV, 50);
            debug_set_rgb(GREEN);
            break;
        }
        case BLINK_CONNECTED: {
            uint64_t gateway_id = event_data.data.gateway_info.gateway_id;
            printf("Connected to gateway %016llX\n", gateway_id);
            debug_set_rgb(GREEN);
            break;
        }
        case BLINK_DISCONNECTED: {
            uint64_t gateway_id = event_data.data.gateway_info.gateway_id;
            printf("Disconnected from gateway %016llX, reason: %u\n", gateway_id, event_data.tag);
            debug_set_rgb(RED);
            break;
        }
        case BLINK_ERROR:
            printf("Error\n");
            break;
        default:
            break;
    }
}

//=========================== debug ========================================

static void debug_init_minimote(void) {
#ifndef BOARD_NRF52833DK
    return;
#endif
    // Make sure the mini-mote is running at 3.0v
    // Might need a re-start to take effect
    if (NRF_UICR->REGOUT0 != UICR_REGOUT0_VOUT_3V0) {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
        }
        NRF_UICR->REGOUT0 = UICR_REGOUT0_VOUT_3V0;

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
        }
    }
    bl_gpio_init(&_r_led_pin, BL_GPIO_OUT);
    bl_gpio_init(&_g_led_pin, BL_GPIO_OUT);
    bl_gpio_init(&_b_led_pin, BL_GPIO_OUT);

    bl_gpio_t _reg_pin = { .port = 0, .pin = 30 };
    // Turn ON the DotBot board regulator if provided
    bl_gpio_init(&_reg_pin, BL_GPIO_OUT);
    bl_gpio_set(&_reg_pin);
}


static void debug_set_rgb(led_color_t color) {
#ifndef BOARD_NRF52833DK
    return;
#endif
    switch (color) {
        case RED:
            bl_gpio_clear(&_r_led_pin);
            bl_gpio_set(&_g_led_pin);
            bl_gpio_set(&_b_led_pin);
            break;

        case GREEN:
            bl_gpio_set(&_r_led_pin);
            bl_gpio_clear(&_g_led_pin);
            bl_gpio_set(&_b_led_pin);
            break;

        case BLUE:
            bl_gpio_set(&_r_led_pin);
            bl_gpio_set(&_g_led_pin);
            bl_gpio_clear(&_b_led_pin);
            break;

        case OFF:
            bl_gpio_set(&_r_led_pin);
            bl_gpio_set(&_g_led_pin);
            bl_gpio_set(&_b_led_pin);
            break;

        default:
            bl_gpio_set(&_r_led_pin);
            bl_gpio_set(&_g_led_pin);
            bl_gpio_set(&_b_led_pin);
            break;
    }
}
