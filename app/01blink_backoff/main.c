#include <nrf.h>
#include <stdio.h>

#include "bl_rng.h"
#include "bl_timer_hf.h"
#include "association.h"

//=========================== defines ==========================================

#define BLINK_APP_TIMER_DEV 1

//=========================== prototypes =======================================

//============================ main ============================================

int main(void) {
    printf("Test Blink Backoff\n");
    bl_timer_hf_init(BLINK_APP_TIMER_DEV);

    bl_assoc_init(NULL);

    // warm up rng
    for (size_t i = 0; i < 5; i++) {
        uint32_t start_ts = bl_timer_hf_now(BLINK_APP_TIMER_DEV);

         //bl_assoc_node_register_collision_backoff();
        uint8_t val;
        bl_rng_read_range(&val, 0, 255);

        uint32_t end_ts = bl_timer_hf_now(BLINK_APP_TIMER_DEV);
        uint32_t elapsed = end_ts - start_ts;
        printf("Warm up collision backoff %d: %d\n", i, elapsed);
    }

    size_t n_runs = 10;
    uint32_t max_elapsed = 0;
    uint32_t sum_elapsed = 0;
    for (size_t i = 0; i < n_runs; i++) {
        uint32_t start_ts = bl_timer_hf_now(BLINK_APP_TIMER_DEV);
        bl_assoc_node_register_collision_backoff();
        uint32_t end_ts = bl_timer_hf_now(BLINK_APP_TIMER_DEV);
        uint32_t elapsed = end_ts - start_ts;
        printf("Collision backoff %d: %d\n", i, elapsed);
        sum_elapsed += elapsed;
        if (elapsed > max_elapsed) {
            max_elapsed = elapsed;
        }
    }

    uint32_t avg_elapsed = sum_elapsed / n_runs;
    printf("Average elapsed us: %d\n", avg_elapsed);
    printf("Max elapsed us: %d\n", max_elapsed);

    // main loop
    while(1) {
        __SEV();
        __WFE();
        __WFE();
    }
}
