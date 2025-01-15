/**
 * @file
 * @ingroup     app
 *
 * @brief       BotLink Gateway application example
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024
 */
#include <nrf.h>
#include <stdio.h>

int main(void)
{
    printf("Hello World\n");

    while (1) {
        __WFE();
    }
}
