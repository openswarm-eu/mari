#ifndef __MACLOW_H
#define __MACLOW_H

/**
 * @defgroup    net_maclow      MAC-low radio driver
 * @ingroup     drv
 * @brief       Driver for Time-Slotted Channel Hopping (TSCH)
 *
 * @{
 * @file
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 * @copyright Inria, 2024-now
 * @}
 */

#include <stdint.h>
#include <stdlib.h>
#include <nrf.h>

#include "blink.h"

//=========================== defines ==========================================

#define BLINK_TIMER_DEV 2 ///< HF timer device used for the TSCH scheduler
#define BLINK_TIMER_INTER_SLOT_CHANNEL 0 ///< Channel for ticking the whole slot
#define BLINK_TIMER_INTRA_SLOT_CHANNEL 1 ///< Channel for ticking intra-slot sections, such as tx offset and tx max
#define BLINK_TIMER_DESYNC_WINDOW_CHANNEL 2 ///< Channel for ticking the desynchronization window

// Bytes per millisecond in BLE 2M mode
#define BLE_2M (1000 * 1000 * 2) // 2 Mbps
#define BLE_2M_B_MS (BLE_2M / 8 / 1000) // 250 bytes/ms
#define BLE_2M_US_PER_BYTE (1000 / BLE_2M_B_MS) // 4 us

#define _BLINK_START_GUARD_TIME (200)
#define _BLINK_END_GUARD_TIME (100)
#define _BLINK_PACKET_TOA (BLE_2M_US_PER_BYTE * DB_BLE_PAYLOAD_MAX_LENGTH) // Time on air for the maximum payload.
#define _BLINK_PACKET_TOA_WITH_PADDING (_BLINK_PACKET_TOA + (BLE_2M_US_PER_BYTE * 32)) // Add some padding just in case.

//=========================== variables ========================================

typedef enum {
    BLINK_RADIO_ACTION_SLEEP = 'S',
    BLINK_RADIO_ACTION_RX = 'R',
    BLINK_RADIO_ACTION_TX = 'T',
} bl_radio_action_t;

typedef enum {
    SLOT_TYPE_BEACON = 'B',
    SLOT_TYPE_SHARED_UPLINK = 'S',
    SLOT_TYPE_DOWNLINK = 'D',
    SLOT_TYPE_UPLINK = 'U',
} slot_type_t; // FIXME: slot_type or cell_type?

/* Timing of intra-slot sections */
typedef struct {
    uint32_t rx_offset; ///< Offset for the receiver to start receiving.
    uint32_t rx_max; ///< Maximum time the receiver can be active.
    uint32_t tx_offset; ///< Offset for the transmitter to start transmitting. Has to be bigger than rx_offset.
    uint32_t tx_max; ///< Maximum time the transmitter can be active. Has to be smaller than rx_max.
    uint32_t end_guard; ///< Time to wait after the end of the slot, so that the radio can fully turn off. Can be overriden with a large value to facilitate debugging.
    uint32_t total_duration; ///< Total duration of the slot
} bl_slot_timing_t;

extern bl_slot_timing_t bl_default_slot_timing;

typedef struct {
    bl_radio_action_t radio_action;
    uint8_t channel;
    slot_type_t slot_type;
} bl_radio_event_t;

//=========================== prototypes ==========================================

/**
 * @brief Initializes the TSCH scheme
 *
 * @param[in] callback             pointer to a function that will be called each time a packet is received.
 *
 */
void bl_maclow_init(bl_node_type_t node_type, bl_rx_cb_t rx_app_callback);

#endif // __MACLOW_H
