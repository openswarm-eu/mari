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

//=========================== prototypes ======================================
void test_scan(void);

int main(void) {

    test_scan();

    while (1) {
        __WFE();
    }
}

// NOTE: this test depends on BLINK_MAX_SCAN_LIST_SIZE being 10
void test_scan(void) {
    dl_scan_add(1, 1, 1, 1);
    dl_scan_add(1, 2, 1, 2); // update new rssi info wrt gateway_id = 1

    dl_scan_add(2, 2, 1, 3);
    dl_scan_add(3, 1, 1, 4);
    dl_scan_add(4, 1, 1, 5);
    dl_scan_add(5, 1, 1, 6);
    printf("Selected gateway should be 1: %llu\n", dl_scan_select(7));

    dl_scan_add(6, 1, 1, 7);
    dl_scan_add(7, 1, 1, 8);
    dl_scan_add(8, 1, 1, 9);
    dl_scan_add(9, 1, 1, 10);
    dl_scan_add(10, 1, 1, 11);
    dl_scan_add(11, 1, 1, 12); // scan list is full, override oldest scan (gateway_id = 1)
    printf("Selected gateway should be 2: %llu\n", dl_scan_select(13)); // and not 1, because 11 overrides 1

    dl_scan_add(8, 3, 2, 13);
    printf("Selected gateway should be 8: %llu\n", dl_scan_select(BLINK_SCAN_OLD_US+5));
}
