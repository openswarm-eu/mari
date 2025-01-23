#ifndef __SCAN_H
#define __SCAN_H

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

void dl_scan_add(uint64_t gateway_id, int8_t rssi, uint8_t channel, uint32_t ts);

uint64_t dl_scan_select(uint32_t ts_now);

#endif // __SCAN_H
