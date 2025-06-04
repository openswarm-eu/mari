/**
 * @file
 * @ingroup     app
 *
 * @brief       Mira Node application example
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2025
 */

#include <nrf.h>
#include "mr_gpio.h"

#include "minimote.h"

mr_gpio_t _r_led_pin = { .port = 0, .pin = 28 };
mr_gpio_t _g_led_pin = { .port = 0, .pin = 2 };
mr_gpio_t _b_led_pin = { .port = 0, .pin = 3 };

void board_init(void) {
#ifdef BOARD_NRF52833DK
    // Make sure the mini-mote is running at 3.0v
    // Might need a re-start to take effect
    if (NRF_UICR->REGOUT0 != UICR_REGOUT0_VOUT_3V0) {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
        }
        NRF_UICR->REGOUT0 = UICR_REGOUT0_VOUT_3V0;

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
        }
    }
#endif
    mr_gpio_init(&_r_led_pin, MR_GPIO_OUT);
    mr_gpio_init(&_g_led_pin, MR_GPIO_OUT);
    mr_gpio_init(&_b_led_pin, MR_GPIO_OUT);
    board_set_rgb(BLUE);

    mr_gpio_t _reg_pin = { .port = 0, .pin = 30 };
    // Turn ON the DotBot board regulator if provided
    mr_gpio_init(&_reg_pin, MR_GPIO_OUT);
    mr_gpio_set(&_reg_pin);
}

void board_set_rgb(led_color_t color) {
    switch (color) {
        case RED:
            mr_gpio_clear(&_r_led_pin);
            mr_gpio_set(&_g_led_pin);
            mr_gpio_set(&_b_led_pin);
            break;

        case GREEN:
            mr_gpio_set(&_r_led_pin);
            mr_gpio_clear(&_g_led_pin);
            mr_gpio_set(&_b_led_pin);
            break;

        case OTHER:
            mr_gpio_set(&_r_led_pin);
            mr_gpio_clear(&_g_led_pin);
            mr_gpio_clear(&_b_led_pin);
            break;

        case BLUE:
            mr_gpio_set(&_r_led_pin);
            mr_gpio_set(&_g_led_pin);
            mr_gpio_clear(&_b_led_pin);
            break;

        case OFF:
            mr_gpio_set(&_r_led_pin);
            mr_gpio_set(&_g_led_pin);
            mr_gpio_set(&_b_led_pin);
            break;

        default:
            mr_gpio_set(&_r_led_pin);
            mr_gpio_set(&_g_led_pin);
            mr_gpio_set(&_b_led_pin);
            break;
    }
}
