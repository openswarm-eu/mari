/**
 * @file
 * @ingroup     app
 *
 * @brief       Blink Node application example
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2025
 */

typedef enum {
    OFF,
    RED,
    GREEN,
    BLUE,
} led_color_t;

void board_init(void);
void board_set_rgb(led_color_t color);
