/**
 * @file
 * @ingroup     app
 *
 * @brief       Mari Node application example
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2025
 */
#include <nrf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "mr_gpio.h"
#include "mr_device.h"
#include "mr_radio.h"
#include "mr_timer_hf.h"
#include "mari.h"
#include "packet.h"

#include "board.h"

//=========================== defines ==========================================

#define MARI_APP_NET_ID MARI_NET_ID_DEFAULT

#define MARI_APP_TIMER_DEV 1

// -2 is for the type and needs_ack fields
#define DEFAULT_PAYLOAD_SIZE MARI_PACKET_MAX_SIZE - sizeof(mr_packet_header_t) - 2

typedef enum {
    PAYLOAD_TYPE_APPLICATION      = 1,
    PAYLOAD_TYPE_METRICS_REQUEST  = 128,
    PAYLOAD_TYPE_METRICS_RESPONSE = 129,
    PAYLOAD_TYPE_METRICS_LOAD     = 130,
} default_payload_type_t;

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t value[DEFAULT_PAYLOAD_SIZE];
} default_payload_t;

typedef struct {
    mr_event_t      event;
    mr_event_data_t event_data;
    bool            event_ready;
    bool            led_blink_state;  // for blinking when not connected
} node_vars_t;

typedef struct __attribute__((packed)) {
    uint64_t marilib_timestamp;
    uint32_t rx_counter;
    uint32_t tx_counter;
} node_stats_t;

//=========================== variables ========================================

node_vars_t  node_vars  = { 0 };
node_stats_t node_stats = { 0 };

extern schedule_t schedule_minuscule, schedule_tiny, schedule_huge;

schedule_t *schedule_app = &schedule_huge;

//=========================== private ==========================================

static void _led_blink_callback(void) {
    if (!mari_node_is_connected()) {
        // not connected: blink blue (alternate between OFF and BLUE every 10ms)
        board_set_led_mari(node_vars.led_blink_state ? OFF : BLUE);
        node_vars.led_blink_state = !node_vars.led_blink_state;
    }
}

static void mari_event_callback(mr_event_t event, mr_event_data_t event_data) {
    memcpy(&node_vars.event, &event, sizeof(mr_event_t));
    memcpy(&node_vars.event_data, &event_data, sizeof(mr_event_data_t));
    node_vars.event_ready = true;
}

static void handle_metrics_payload(default_payload_t *payload, uint8_t len) {
    (void)len;
    if (payload->type == PAYLOAD_TYPE_METRICS_REQUEST) {
        node_stats.rx_counter++;
        // save received timestamp to node stats
        memcpy(&node_stats.marilib_timestamp, &payload->value, 8);
        // create response payload
        default_payload_t payload_response = {
            .type = PAYLOAD_TYPE_METRICS_RESPONSE,
        };
        memcpy(&payload_response.value, (uint8_t *)&node_stats, sizeof(node_stats_t));
        // send response payload
        mari_node_tx_payload((uint8_t *)&payload_response, sizeof(uint8_t) + sizeof(node_stats_t));
        node_stats.tx_counter++;
    } else if (payload->type == PAYLOAD_TYPE_METRICS_LOAD) {
        // just do nothing here!
    }
}

//=========================== main =============================================

int main(void) {
    printf("Hello Mari Node %016llX\n", mr_device_id());
    mr_timer_hf_init(MARI_APP_TIMER_DEV);

    board_init();
    board_set_led_mari(RED);

    mari_init(MARI_NODE, MARI_APP_NET_ID, schedule_app, &mari_event_callback);

    // blink blue every 100ms
    mr_timer_hf_set_periodic_us(MARI_APP_TIMER_DEV, 0, 100 * 1000, &_led_blink_callback);

    board_set_led_mari(OFF);

    while (1) {
        __SEV();
        __WFE();
        __WFE();

        if (node_vars.event_ready) {
            node_vars.event_ready = false;

            mr_event_t      event      = node_vars.event;
            mr_event_data_t event_data = node_vars.event_data;

            switch (event) {
                case MARI_NEW_PACKET:
                {
                    mari_packet_t     packet  = event_data.data.new_packet;
                    default_payload_t payload = { 0 };
                    memcpy(&payload, packet.payload, packet.payload_len);

                    if (payload.type == PAYLOAD_TYPE_APPLICATION) {
                        // TBD custom application logic
                    } else {
                        handle_metrics_payload(&payload, packet.payload_len);
                    }

                    break;
                }
                case MARI_CONNECTED:
                {
                    uint64_t gateway_id = event_data.data.gateway_info.gateway_id;
                    printf("Connected to gateway %016llX\n", gateway_id);
                    board_set_led_mari_gateway(gateway_id);
                    break;
                }
                case MARI_DISCONNECTED:
                {
                    uint64_t gateway_id = event_data.data.gateway_info.gateway_id;
                    printf("Disconnected from gateway %016llX, reason: %u\n", gateway_id, event_data.tag);
                    board_set_led_mari(OFF);
                    break;
                }
                default:
                    break;
            }
        }

        mari_event_loop();
    }
}
