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
#define BLINK_MAX_SCAN_LIST_SIZE 3

//=========================== variables =======================================
typedef struct {
    uint64_t gateway_id;
    uint8_t rssi;
    uint32_t timestamp;
} dl_scan_t;

typedef struct {
    size_t length;
    size_t current;
    dl_scan_t scans[BLINK_MAX_SCAN_LIST_SIZE];
} dl_scan_rb_t;

//=========================== prototypes ======================================

void dl_scan_add(uint64_t gateway_id, uint8_t rssi);
void dl_scan_clear_old(void);
uint64_t dl_scan_select(void);

#endif // __SCAN_H
