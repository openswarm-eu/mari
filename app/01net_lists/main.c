/**
 * @file
 * @ingroup     net_lists
 *
 * @brief       Example on how to use lists modules
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024
 */
#include <nrf.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "dotlink.h"
#include "scan.h"
#include "device.h"

int main(void) {

    dl_scan_add(1, 1, 1, 1);
    dl_scan_add(1, 2, 1, 2); // add new rssi info from gateway_id = 1

    dl_scan_add(2, 1, 1, 3);
    dl_scan_add(3, 1, 1, 4);
    dl_scan_add(4, 1, 1, 5);
    dl_scan_add(5, 1, 1, 6);
    dl_scan_add(6, 1, 1, 7);
    dl_scan_add(7, 1, 1, 8);
    dl_scan_add(8, 1, 1, 9);
    dl_scan_add(9, 1, 1, 10);
    dl_scan_add(10, 1, 1, 11);
    dl_scan_add(11, 1, 1, 12); // scan list is full, override oldest scan

    while (1) {
        __WFE();
    }
}
