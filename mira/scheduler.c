/**
 * @file
 * @ingroup     net_scheduler
 *
 * @brief       The mira scheduler
 *
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 *
 * @copyright Inria, 2024
 */
#include <nrf.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "mr_device.h"
#if defined(NRF5340_XXAA) && defined(NRF_NETWORK)
#include "ipc.h"
#endif

#include "scheduler.h"
#include "all_schedules.c"
#include "association.c"

//=========================== defines ==========================================

//=========================== variables ========================================

typedef struct {
    mr_node_type_t node_type;  // whether the node is a gateway or a dotbot

    // counters and indexes
    schedule_t *active_schedule_ptr;  // pointer to the currently active schedule
    uint32_t    slotframe_counter;    // used to cycle beacon channels through slotframes (when listening for beacons at uplink slot_durations)

    uint8_t num_assigned_uplink_nodes;  // number of nodes with assigned uplink slots

    // static data
    schedule_t *available_schedules[MIRA_N_SCHEDULES];
    size_t      available_schedules_len;
} schedule_vars_t;

static schedule_vars_t _schedule_vars = { 0 };

//========================== prototypes ========================================

// Compute the radio action when the node is a gateway
void _compute_gateway_action(cell_t cell, mr_slot_info_t *slot_info);

// Compute the radio action when the node is a dotbot
void _compute_dotbot_action(cell_t cell, mr_slot_info_t *slot_info);

//=========================== public ===========================================

void mr_scheduler_init(mr_node_type_t node_type, schedule_t *application_schedule) {
    _schedule_vars.node_type = node_type;

    if (_schedule_vars.available_schedules_len == MIRA_N_SCHEDULES)
        return;  // FIXME: this is just to simplify debugging (allows calling init multiple times)

    // FIXME: schedules only used for debugging
    //_schedule_vars.available_schedules[_schedule_vars.available_schedules_len++] = schedule_test;

    _schedule_vars.available_schedules[_schedule_vars.available_schedules_len++] = &schedule_minuscule;
    _schedule_vars.available_schedules[_schedule_vars.available_schedules_len++] = &schedule_tiny;
    _schedule_vars.available_schedules[_schedule_vars.available_schedules_len++] = &schedule_huge;

    if (application_schedule != NULL) {
        _schedule_vars.available_schedules[_schedule_vars.available_schedules_len++] = application_schedule;
        _schedule_vars.active_schedule_ptr                                           = application_schedule;
    }
}

bool mr_scheduler_set_schedule(uint8_t schedule_id) {
    for (size_t i = 0; i < MIRA_N_SCHEDULES; i++) {
        if (_schedule_vars.available_schedules[i]->id == schedule_id) {
            _schedule_vars.active_schedule_ptr = _schedule_vars.available_schedules[i];
            return true;
        }
    }
    return false;
}

// ------------ node functions ------------

// to be called at the NODE when processing a JOIN_RESPONSE
bool mr_scheduler_node_assign_myself_to_cell(uint16_t cell_index) {
    for (size_t i = 0; i < _schedule_vars.active_schedule_ptr->n_cells; i++) {
        cell_t *cell = &_schedule_vars.active_schedule_ptr->cells[i];
        if (cell->type == SLOT_TYPE_UPLINK && i == cell_index) {
            cell->assigned_node_id = mr_device_id();
            return true;
        }
    }
    return false;
}

void mr_scheduler_node_deassign_myself_from_schedule(void) {
    for (size_t i = 0; i < _schedule_vars.active_schedule_ptr->n_cells; i++) {
        cell_t *cell = &_schedule_vars.active_schedule_ptr->cells[i];
        if (cell->type == SLOT_TYPE_UPLINK && cell->assigned_node_id == mr_device_id()) {
            cell->assigned_node_id  = NULL;
            cell->last_received_asn = 0;
        }
    }
}

// ------------ gateway functions ---------

// to be called at the GATEWAY when processing a JOIN_REQUEST
int16_t mr_scheduler_gateway_assign_next_available_uplink_cell(uint64_t node_id, uint64_t asn) {
    for (size_t i = 0; i < _schedule_vars.active_schedule_ptr->n_cells; i++) {
        cell_t *cell = &_schedule_vars.active_schedule_ptr->cells[i];
        // normally the cell is available if empty, but it may also be that case that
        // the node just temporarily lost connection, so we can just re-assign the same cell_id
        if (cell->type == SLOT_TYPE_UPLINK && (cell->assigned_node_id == NULL || cell->assigned_node_id == node_id)) {
            cell->assigned_node_id  = node_id;
            cell->last_received_asn = asn;
            _schedule_vars.num_assigned_uplink_nodes++;
            return i;
        }
    }
    return -1;
}

// to be called at the GATEWAY when a node leaves
inline void mr_scheduler_gateway_decrease_nodes_counter(void) {
    _schedule_vars.num_assigned_uplink_nodes--;
}

// to be called at the GATEWAY to build a beacon
uint8_t mr_scheduler_gateway_remaining_capacity(void) {
    // TODO: can be optimized, if we pre-compute the number of uplink slots in a schedule
    uint8_t remaining_capacity = 0;
    for (size_t i = 0; i < _schedule_vars.active_schedule_ptr->n_cells; i++) {
        cell_t *cell = &_schedule_vars.active_schedule_ptr->cells[i];
        if (cell->type == SLOT_TYPE_UPLINK && cell->assigned_node_id == NULL) {
            remaining_capacity++;
        }
    }
    return remaining_capacity;
}

// to be called at the GATEWAY to build a beacon
uint8_t mr_scheduler_gateway_get_nodes_count(void) {
    return _schedule_vars.num_assigned_uplink_nodes;
}

uint8_t mr_scheduler_gateway_get_nodes(uint64_t *nodes) {
    uint8_t count = 0;
    for (size_t i = 0; i < _schedule_vars.active_schedule_ptr->n_cells; i++) {
        cell_t *cell = &_schedule_vars.active_schedule_ptr->cells[i];
        if (cell->type == SLOT_TYPE_UPLINK && cell->assigned_node_id != NULL) {
            nodes[count++] = cell->assigned_node_id;
        }
    }
    return count;
}

// ------------ general functions ---------

mr_slot_info_t mr_scheduler_tick(uint64_t asn) {
    // get the current cell
    size_t cell_index = asn % (_schedule_vars.active_schedule_ptr)->n_cells;
    cell_t cell       = (_schedule_vars.active_schedule_ptr)->cells[cell_index];

    mr_slot_info_t slot_info = {
        .radio_action = MIRA_RADIO_ACTION_SLEEP,
        .channel      = mr_scheduler_get_channel(cell.type, asn, cell.channel_offset),
        .type         = cell.type,  // FIXME: only for debugging, remove before merge
    };
    if (_schedule_vars.node_type == MIRA_GATEWAY) {
        _compute_gateway_action(cell, &slot_info);
    } else {
        _compute_dotbot_action(cell, &slot_info);
        mr_assoc_node_tick_backoff();
    }

    // if the slotframe wrapped, keep track of how many slotframes have passed (used to cycle beacon channels)
    if (asn != 0 && cell_index == 0) {
        _schedule_vars.slotframe_counter++;
    }

    return slot_info;
}

uint8_t mr_scheduler_get_channel(slot_type_t slot_type, uint64_t asn, uint8_t channel_offset) {
#if (MIRA_FIXED_CHANNEL != 0)
    (void)slot_type;
    (void)asn;
    (void)channel_offset;
    return MIRA_FIXED_CHANNEL;
#endif
    if (slot_type == SLOT_TYPE_BEACON) {
#ifdef MIRA_FIXED_SCAN_CHANNEL
        return MIRA_FIXED_SCAN_CHANNEL;
#else
        // special handling in case the cell is a beacon
        return MIRA_N_BLE_REGULAR_CHANNELS + (asn % MIRA_N_BLE_ADVERTISING_CHANNELS);
#endif
    } else {
        // As per RFC 7554:
        //   frequency = F {(ASN + channelOffset) mod nFreq}
        return (asn + channel_offset) % MIRA_N_BLE_REGULAR_CHANNELS;
    }
}

schedule_t *mr_scheduler_get_active_schedule_ptr(void) {
    return _schedule_vars.active_schedule_ptr;
}

uint8_t mr_scheduler_get_active_schedule_id(void) {
    return _schedule_vars.active_schedule_ptr->id;
}

uint8_t mr_scheduler_get_active_schedule_slot_count(void) {
    return _schedule_vars.active_schedule_ptr->n_cells;
}

cell_t mr_scheduler_node_peek_slot(uint64_t asn) {
    size_t cell_index = (asn) % (_schedule_vars.active_schedule_ptr)->n_cells;
    cell_t cell       = (_schedule_vars.active_schedule_ptr)->cells[cell_index];

    return cell;
}

//=========================== private ==========================================

void _compute_gateway_action(cell_t cell, mr_slot_info_t *slot_info) {
    switch (cell.type) {
        case SLOT_TYPE_BEACON:
        case SLOT_TYPE_DOWNLINK:
            slot_info->radio_action = MIRA_RADIO_ACTION_TX;
            break;
        case SLOT_TYPE_SHARED_UPLINK:
        case SLOT_TYPE_UPLINK:
            slot_info->radio_action = MIRA_RADIO_ACTION_RX;
            break;
    }
}

void _compute_dotbot_action(cell_t cell, mr_slot_info_t *slot_info) {
    switch (cell.type) {
        case SLOT_TYPE_BEACON:
        case SLOT_TYPE_DOWNLINK:
            slot_info->radio_action = MIRA_RADIO_ACTION_RX;
            break;
        case SLOT_TYPE_SHARED_UPLINK:
            slot_info->radio_action = MIRA_RADIO_ACTION_TX;
            break;
        case SLOT_TYPE_UPLINK:
            if (cell.assigned_node_id == mr_device_id()) {
                slot_info->radio_action = MIRA_RADIO_ACTION_TX;
            } else {
                slot_info->radio_action = MIRA_RADIO_ACTION_SLEEP;
            }
            break;
        default:
            break;
    }
}
