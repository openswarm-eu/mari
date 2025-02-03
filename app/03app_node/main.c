/**
 * @file
 * @ingroup     app
 *
 * @brief       Blink Node application example
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
    printf("Hello Blink Node\n");

    bl_init(NODE_TYPE_NODE, NULL, NULL);

    while (1) {
        __WFE();
    }
}
