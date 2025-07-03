/**
 * @file
 * @ingroup     app
 *
 * @brief       Mira Gateway application (uart side)
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 * @author Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @copyright Inria, 2025
 */
#include <nrf.h>
#include <stdio.h>

#include "mr_device.h"
#include "mira.h"

//=========================== defines ==========================================

typedef struct {
    bool dummy;
} gateway_app_vars_t;

//=========================== variables ========================================

gateway_app_vars_t gateway_app_vars = { 0 };

//=========================== prototypes =======================================

//=========================== main =============================================

int main(void) {
    printf("Hello Mira Gateway UART %016llX\n", mr_device_id());

    while (1) {
        __SEV();
        __WFE();
        __WFE();
    }
}

//=========================== callbacks ========================================

//=========================== private ========================================
