/**
 * @file
 * @brief       Board-specific implementations.
 * @details     
 *              
 */
#include "board.h"
#include "board_config.h"
#include "mr_gpio.h"
#include <nrf.h>

// --- MINIMOTE IMPLEMENTATION ---

#ifdef MINIMOTE

static mr_gpio_t _r_led_pin = {.port = 0,.pin = 28 };
static mr_gpio_t _g_led_pin = {.port = 0,.pin = 2 };
static mr_gpio_t _b_led_pin = {.port = 0,.pin = 3 };

void board_init(void) {
#ifdef BOARD_NRF52833DK
    // Make sure the mini-mote is running at 3.0v
    if (NRF_UICR->REGOUT0!= UICR_REGOUT0_VOUT_3V0) {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NRF_UICR->REGOUT0 = UICR_REGOUT0_VOUT_3V0;
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
    }
#endif
    mr_gpio_init(&_r_led_pin, MR_GPIO_OUT);
    mr_gpio_init(&_g_led_pin, MR_GPIO_OUT);
    mr_gpio_init(&_b_led_pin, MR_GPIO_OUT);
    board_set_rgb(BLUE);

    mr_gpio_t _reg_pin = {.port = 0,.pin = 30 };
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
        case BLUE:
            mr_gpio_set(&_r_led_pin);
            mr_gpio_set(&_g_led_pin);
            mr_gpio_clear(&_b_led_pin);
            break;
        case OTHER:
            mr_gpio_clear(&_r_led_pin);
            mr_gpio_clear(&_g_led_pin);
            mr_gpio_set(&_b_led_pin);
            break;
        case OFF:
        default:
            mr_gpio_set(&_r_led_pin);
            mr_gpio_set(&_g_led_pin);
            mr_gpio_set(&_b_led_pin);
            break;
    }
}

#endif 

// --- NRF52840 DONGLE(PCA10059) IMPLEMENTATION ---

#ifdef NRF52840_DONGLE

static mr_gpio_t _r_led_pin = {.port = 0,.pin = 8 };  // LED 2 Red
static mr_gpio_t _g_led_pin = {.port = 1,.pin = 9 };  // LED 2 Green
static mr_gpio_t _b_led_pin = {.port = 0,.pin = 12 };  // LED 2 Blue

void board_init(void) {
    mr_gpio_init(&_r_led_pin, MR_GPIO_OUT);
    mr_gpio_init(&_g_led_pin, MR_GPIO_OUT);
    mr_gpio_init(&_b_led_pin, MR_GPIO_OUT);
    
    board_set_rgb(BLUE);
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
        case BLUE:
            mr_gpio_set(&_r_led_pin);
            mr_gpio_set(&_g_led_pin);
            mr_gpio_clear(&_b_led_pin);
            break;
        case OTHER: 
            mr_gpio_clear(&_r_led_pin);
            mr_gpio_clear(&_g_led_pin);
            mr_gpio_clear(&_b_led_pin);
            break;
        case OFF:
        default:
            mr_gpio_set(&_r_led_pin);
            mr_gpio_set(&_g_led_pin);
            mr_gpio_set(&_b_led_pin);
            break;
    }
}

#endif 


