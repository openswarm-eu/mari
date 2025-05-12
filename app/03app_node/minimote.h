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

typedef enum {
    OFF,
    RED,
    GREEN,
    BLUE,
    OTHER,
} led_color_t;

void board_init(void);
void board_set_rgb(led_color_t color);
