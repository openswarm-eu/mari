/**
 * @file
 * @ingroup     net_scan_list
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

#include "scan.h"
#include "timer_hf.h"

//=========================== variables =======================================
typedef struct {
    dl_scan_rb_t scan_rb;
} scan_vars_t;

scan_vars_t scan_vars = { 0 };

//=========================== public ===========================================

void dl_scan_add(uint64_t gateway_id, uint8_t rssi) {
    scan_vars.scan_rb.current = (scan_vars.scan_rb.current + 1) % BLINK_MAX_SCAN_LIST_SIZE;

    scan_vars.scan_rb.scans[scan_vars.scan_rb.current].gateway_id = gateway_id;
    scan_vars.scan_rb.scans[scan_vars.scan_rb.current].rssi = rssi;
    scan_vars.scan_rb.scans[scan_vars.scan_rb.current].timestamp = dl_timer_hf_now(DOTLINK_TIMER_DEV);
}
