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
void board_set_rgb(led_color_t color);

#endif
