/**
 * @file
 * @ingroup     app
 *
 * @brief       Blink Gateway application example
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024
 */
#include <nrf.h>
#include <stdio.h>

#include "blink.h"

int main(void)
{
    printf("Hello Blink Gateway\n");

    bl_init(NODE_TYPE_GATEWAY, NULL, NULL);

    while (1) {
        __WFE();
    }
}
