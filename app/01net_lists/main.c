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

#include "dotlink.h"
#include "scan.h"
#include "device.h"

int main(void) {

    dl_scan_add(1, 1, 1, 1);
    dl_scan_add(2, 1, 1, 2);
    dl_scan_add(3, 1, 1, 3);
    dl_scan_add(4, 1, 1, 4);
    dl_scan_add(5, 1, 1, 5);

    while (1) {
        __WFE();
    }
}
