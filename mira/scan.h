#ifndef __SCAN_H
#define __SCAN_H

/**
 * @ingroup     mira
 * @brief       Scan management
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

#include "packet.h"
#include "models.h"

//=========================== defines =========================================

#define MIRA_MAX_SCAN_LIST_SIZE       (5)
#define MIRA_SCAN_OLD_US              (1000 * 500)       // rssi reading considered old after 500 ms
#define MIRA_HANDOVER_RSSI_HYSTERESIS (9)                // hysteresis (in dBm) for handover
#define MIRA_HANDOVER_MIN_INTERVAL    (1000 * 1000 * 3)  // minimum interval between handovers (in us)

//=========================== variables =======================================

typedef struct {
    int8_t                    rssi;
    uint32_t                  timestamp;
    uint64_t                  captured_asn;
    mr_beacon_packet_header_t beacon;
} mr_channel_info_t;

typedef struct {
    uint64_t          gateway_id;
    mr_channel_info_t channel_info[MIRA_N_BLE_ADVERTISING_CHANNELS];  // channels 37, 38, 39
} mr_gateway_scan_t;

//=========================== prototypes ======================================

void mr_scan_add(mr_beacon_packet_header_t beacon, int8_t rssi, uint8_t channel, uint32_t ts_scan, uint64_t asn_scan);

bool mr_scan_select(mr_channel_info_t *best_channel_info, uint32_t ts_scan_started, uint32_t ts_scan_ended);

#endif  // __SCAN_H
