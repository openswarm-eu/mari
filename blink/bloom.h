#ifndef __BLOOM_H
#define __BLOOM_H

/**
 * @ingroup     blink
 * @brief       Bloom filter header file
 *
 * @{
 * @file
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 * @copyright Inria, 2024-now
 * @}
 */

#include <nrf.h>
#include <stdint.h>
#include <stdbool.h>

//=========================== defines =========================================

#define BLINK_BLOOM_M_BITS 1024
#define BLINK_BLOOM_M_BYTES (BLINK_BLOOM_M_BITS / 8)
#define BLINK_BLOOM_K_HASHES 2

#define BLINK_BLOOM_FNV1A_H2_SALT 0x5bd1e995

//=========================== variables =======================================

//=========================== prototypes ======================================

void bl_bloom_gateway_init(void);
void bl_bloom_gateway_set_dirty(void);
void bl_bloom_gateway_set_clean(void);
bool bl_bloom_gateway_is_dirty(void);
bool bl_bloom_gateway_is_available(void);
uint8_t bl_bloom_gateway_copy(uint8_t *output);
void bl_bloom_gateway_compute(void);
void bl_bloom_gateway_event_loop(void);

bool bl_bloom_node_contains(uint64_t node_id, const uint8_t *bloom);

#endif // __BLOOM_H
