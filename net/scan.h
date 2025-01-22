#ifndef __SCAN_H
#define __SCAN_H

/**
 * @file
 * @ingroup     net_scan
 *
 * @brief       Scan list implementation
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024
 */

#include <nrf.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "device.h"
#include "protocol.h"
#include "scheduler.h"

//=========================== defines =========================================
#define BLINK_MAX_SCAN_LIST_SIZE (10)
#define BLINK_SCAN_OLD_US (1000*1000*3) // rssi reading considered old after 3 seconds
#define BLINK_SCAN_HANDOVER_HYSTERESIS (6) // hysteresis (in dBm) for handover

//=========================== variables =======================================
typedef struct {
    int8_t rssi;
    uint32_t timestamp;
} dl_rssi_t;

typedef struct {
    uint64_t gateway_id;
    dl_rssi_t rssi[DOTLINK_N_BLE_ADVERTISING_FREQUENCIES]; // channels 37, 38, 39
} dl_scan_t;

//=========================== prototypes ======================================

/**
 * Adds a new rssi reading
 *
 * @param gateway_id the gateway id
 * @param rssi the rssi reading in dBm
 * @param channel the advertising channel (37, 38, or 39 for BLE)
 * @param ts the timestamp of the rssi reading
 * */
void dl_scan_add(uint64_t gateway_id, int8_t rssi, uint8_t channel, uint32_t ts);

/**
 * Selects the gateway with the highest rssi
 *
 * @param ts_now current timestamp
 *
 * @return the gateway_id with the highest rssi. If no gateway is found, returns 0.
 * */
uint64_t dl_scan_select(uint32_t ts_now);

#endif // __SCAN_H
