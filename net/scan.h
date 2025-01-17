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
#define BLINK_MAX_SCAN_LIST_SIZE (4)
#define BLINK_SCAN_OLD_US (1000*1000*3) // rssi reading considered old after this amount of microseconds

//=========================== variables =======================================
typedef struct {
    uint8_t rssi;
    uint32_t timestamp;
} dl_rssi_t;

typedef struct {
    uint64_t gateway_id;
    dl_rssi_t rssi[DOTLINK_N_BLE_ADVERTISING_FREQUENCIES]; // channels 37, 38, 39
} dl_scan_t;

//=========================== prototypes ======================================

/**
 * Adds a new rssi reading
 * */
void dl_scan_add(uint64_t gateway_id, uint8_t rssi, uint8_t frequency, uint32_t ts);
uint64_t dl_scan_select(void);

#endif // __SCAN_H
