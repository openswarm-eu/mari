/**
 * @file
 * @ingroup     bloom
 *
 * @brief       Bloom filter
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2025
 */

#include <nrf.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "bloom.h"
#include "scheduler.h"

//=========================== defines ==========================================

typedef struct {
    // used by the gateway
    bool is_dirty; // true if the bloom filter needs to be re-computed
    bool is_available; // true if the bloom filter is being computed
    uint8_t bloom[BLINK_BLOOM_M_BYTES]; // bloom filter output
} bloom_vars_t;

//=========================== variables ========================================

static bloom_vars_t bloom_vars = { 0 };

//=========================== prototypes =======================================

static uint64_t bl_fnv1a64(uint64_t input);

//=========================== public ===========================================

// -------- gateway ---------

void bl_bloom_gateway_init(void) {
    bloom_vars.is_dirty = false;
    bloom_vars.is_available = false;
}

void bl_bloom_gateway_set_dirty(void) {
    bloom_vars.is_dirty = true;
}

void bl_bloom_gateway_set_clean(void) {
    bloom_vars.is_dirty = false;
}

bool bl_bloom_gateway_is_dirty(void) {
    return bloom_vars.is_dirty;
}

bool bl_bloom_gateway_get(uint8_t *bloom) {
    if (bloom_vars.is_available) {
        return false; // bloom filter is not available
    }
    memcpy(bloom, bloom_vars.bloom, BLINK_BLOOM_M_BYTES);
    return true;
}

void bl_bloom_gateway_compute(void) {
    bloom_vars.is_available = false;
    memset(bloom_vars.bloom, 0, BLINK_BLOOM_M_BYTES);

    schedule_t *schedule_ptr = bl_scheduler_get_active_schedule_ptr();

    for (size_t i = 0; i < schedule_ptr->n_cells; i++) {
        cell_t *cell = &schedule_ptr->cells[i];
        if (cell->type != SLOT_TYPE_UPLINK) {
            continue; // skip non-uplink cells
        }
        if (cell->assigned_node_id == NULL) {
            continue; // skip empty cells
        }
        uint64_t id = cell->assigned_node_id;

        uint64_t h1 = bl_fnv1a64(id);
        uint64_t h2 = bl_fnv1a64(id ^ BLINK_BLOOM_FNV1A_H2_SALT);

        for (int k = 0; k < BLINK_BLOOM_K_HASHES; k++) {
            uint64_t idx = (h1 + k * h2) & (BLINK_BLOOM_M_BITS - 1); // Fast bitmask instead of division
            bloom_vars.bloom[idx / 8] |= (1 << (idx % 8));
        }
    }
    bloom_vars.is_available = true;
}

void bl_bloom_gateway_event_loop(void) {
    // check if the bloom filter needs to be re-computed
    if (bl_bloom_gateway_is_dirty()) {
        bl_bloom_gateway_compute();
        bl_bloom_gateway_set_clean();
    }
}

// -------- node ---------

bool bl_bloom_node_contains(uint64_t node_id, const uint8_t *bloom) {
    uint64_t h1 = bl_fnv1a64(node_id);
    uint64_t h2 = bl_fnv1a64(node_id ^ BLINK_BLOOM_FNV1A_H2_SALT);

    for (int k = 0; k < BLINK_BLOOM_K_HASHES; k++) {
        uint64_t idx = (h1 + k * h2) & (BLINK_BLOOM_M_BITS - 1); // Fast bitmask instead of division
        if ((bloom[idx / 8] & (1 << (idx % 8))) == 0) {
            return false;
        }
    }
    return true;
}

//=========================== private ==========================================

// FNV-1a 64-bit hash
static uint64_t bl_fnv1a64(uint64_t input) {
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (int b = 0; b < 8; b++) {
        uint8_t byte = (input >> (56 - b * 8)) & 0xFF;
        hash ^= byte;
        hash *= 0x100000001b3ULL;
    }
    return hash;
}
