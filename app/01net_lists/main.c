#include <nrf.h>
#include <stdio.h>

#include "scan.h"

//=========================== prototypes ======================================

void test_scan(void);

//============================ main ============================================

int main(void) {
    test_scan();

    // main loop
    while(1) {
        // make sure the event register is cleared
        __SEV();
        __WFE();
        // wait for events, effectively entering System ON sleep mode
        __WFE();
    }
}

// NOTE: this test depends on BLINK_MAX_SCAN_LIST_SIZE being 10
void test_scan(void) {
    bl_scan_add(1, 1, 37, 1);
    bl_scan_add(1, 2, 37, 2); // update new rssi info wrt gateway_id = 1

    bl_scan_add(2, 2, 37, 3);
    bl_scan_add(3, 1, 37, 4);
    bl_scan_add(4, 1, 37, 5);
    bl_scan_add(5, 1, 37, 6);
    printf("Selected gateway should be 1: %llu\n", bl_scan_select(7));

    bl_scan_add(6, 1, 37, 7);
    bl_scan_add(7, 1, 37, 8);
    bl_scan_add(8, 1, 37, 9);
    bl_scan_add(9, 1, 37, 10);
    bl_scan_add(10, 1, 37, 11);
    bl_scan_add(11, 1, 37, 12); // scan list is full, override oldest scan (gateway_id = 1)
    printf("Selected gateway should be 2: %llu\n", bl_scan_select(13)); // and not 1, because 11 overrides 1

    bl_scan_add(8, 3, 38, 13);
    printf("Selected gateway should be 8: %llu\n", bl_scan_select(BLINK_SCAN_OLD_US+5));
}
