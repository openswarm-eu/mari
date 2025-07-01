/**
 * @file
 * @brief       Generic board support interface.
 * @details
 *
 */
#ifndef BOARD_H
#define BOARD_H

typedef enum {
    OFF,
    RED,
    GREEN,
    BLUE,
    OTHER,
} led_color_t;

void board_init(void);
void board_set_mira_status(led_color_t color);
void board_set_led_app(led_color_t color);

#endif
