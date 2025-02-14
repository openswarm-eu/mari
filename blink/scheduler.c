/**
 * @file
 * @ingroup     net_scheduler
 *
 * @brief       The blink scheduler
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

#include "scheduler.h"
#include "mac.h"
#include "device.h"
#if defined(NRF5340_XXAA) && defined(NRF_NETWORK)
#include "ipc.h"
#endif

#include "all_schedules.c"

//=========================== defines ==========================================


//=========================== variables ========================================

typedef struct {
    bl_node_type_t node_type; // whether the node is a gateway or a dotbot

    // counters and indexes
    uint64_t asn; // absolute slot number
    schedule_t *active_schedule_ptr; // pointer to the currently active schedule
    uint32_t slotframe_counter; // used to cycle beacon channels through slotframes (when listening for beacons at uplink slot_durations)

    // static data
    schedule_t available_schedules[BLINK_N_SCHEDULES];
    size_t available_schedules_len;
} schedule_vars_t;

static schedule_vars_t _schedule_vars = { 0 };

//========================== prototypes ========================================

// Compute the radio action when the node is a gateway
void _compute_gateway_action(cell_t cell, bl_radio_event_t *radio_event);

// Compute the radio action when the node is a dotbot
void _compute_dotbot_action(cell_t cell, bl_radio_event_t *radio_event);

//=========================== public ===========================================

void bl_scheduler_init(bl_node_type_t node_type, schedule_t *application_schedule) {
    _schedule_vars.node_type = node_type;

    if (_schedule_vars.available_schedules_len == BLINK_N_SCHEDULES) return; // FIXME: this is just to simplify debugging (allows calling init multiple times)

    _schedule_vars.available_schedules[_schedule_vars.available_schedules_len++] = schedule_only_beacons;
    _schedule_vars.available_schedules[_schedule_vars.available_schedules_len++] = schedule_only_beacons_optimized_scan;

    _schedule_vars.available_schedules[_schedule_vars.available_schedules_len++] = schedule_minuscule;
    _schedule_vars.available_schedules[_schedule_vars.available_schedules_len++] = schedule_tiny;

    if (application_schedule != NULL) {
        _schedule_vars.available_schedules[_schedule_vars.available_schedules_len++] = *application_schedule;
        _schedule_vars.active_schedule_ptr = application_schedule;
    }
}

bool bl_scheduler_set_schedule(uint8_t schedule_id) {
    for (size_t i = 0; i < BLINK_N_SCHEDULES; i++) {
        if (_schedule_vars.available_schedules[i].id == schedule_id) {
            _schedule_vars.active_schedule_ptr = &_schedule_vars.available_schedules[i];
            return true;
        }
    }
    return false;
}

// to be called at the GATEWAY when processing a JOIN_REQUEST
bool bl_scheduler_assign_next_available_uplink_cell(uint64_t node_id) {
    for (size_t i = 0; i < _schedule_vars.active_schedule_ptr->n_cells; i++) {
        cell_t *cell = &_schedule_vars.active_schedule_ptr->cells[i];
        if (cell->type == SLOT_TYPE_UPLINK && cell->assigned_node_id == NULL) {
            cell->assigned_node_id = node_id;
            return true;
        }
    }
    return false;
}

// to be called at the NODE when processing a JOIN_RESPONSE
bool bl_scheduler_assign_myself_to_cell(uint8_t cell_index) {
    for (size_t i = 0; i < _schedule_vars.active_schedule_ptr->n_cells; i++) {
        cell_t *cell = &_schedule_vars.active_schedule_ptr->cells[i];
        if (cell->type == SLOT_TYPE_UPLINK && i == cell_index) {
            cell->assigned_node_id = db_device_id();
            return true;
        }
    }
    return false;
}

bool bl_scheduler_deassign_uplink_cell(uint64_t node_id) {
    for (size_t i = 0; i < _schedule_vars.active_schedule_ptr->n_cells; i++) {
        cell_t *cell = &_schedule_vars.active_schedule_ptr->cells[i];
        if (cell->type == SLOT_TYPE_UPLINK && cell->assigned_node_id == node_id) {
            cell->assigned_node_id = NULL;
            return true;
        }
    }
    return false;
}

bl_radio_event_t bl_scheduler_tick(uint64_t asn) {
    // get the current cell
    size_t cell_index = asn % (_schedule_vars.active_schedule_ptr)->n_cells;
    cell_t cell = (_schedule_vars.active_schedule_ptr)->cells[cell_index];

    bl_radio_event_t radio_event = {
        .radio_action = BLINK_RADIO_ACTION_SLEEP,
        .channel = bl_scheduler_get_channel(cell.type, asn, cell.channel_offset),
        .slot_type = cell.type, // FIXME: only for debugging, remove before merge
    };
    if (_schedule_vars.node_type == BLINK_GATEWAY) {
        _compute_gateway_action(cell, &radio_event);
    } else {
        _compute_dotbot_action(cell, &radio_event);
    }

    // if the slotframe wrapped, keep track of how many slotframes have passed (used to cycle beacon channels)
    if (asn != 0 && cell_index == 0) {
        _schedule_vars.slotframe_counter++;
    }

    return radio_event;
}

uint8_t bl_scheduler_get_channel(slot_type_t slot_type, uint64_t asn, uint8_t channel_offset) {
#if(BLINK_FIXED_CHANNEL != 0)
    (void)slot_type;
    (void)asn;
    (void)channel_offset;
    return BLINK_FIXED_CHANNEL;
#endif
    if (slot_type == SLOT_TYPE_BEACON) {
        // special handling in case the cell is a beacon
        return BLINK_N_BLE_REGULAR_CHANNELS + (asn % BLINK_N_BLE_ADVERTISING_CHANNELS);
    } else {
        // As per RFC 7554:
        //   frequency = F {(ASN + channelOffset) mod nFreq}
        return (asn + channel_offset) % BLINK_N_BLE_REGULAR_CHANNELS;
    }
}

uint8_t bl_scheduler_get_active_schedule_id(void) {
    return _schedule_vars.active_schedule_ptr->id;
}

//=========================== private ==========================================

void _compute_gateway_action(cell_t cell, bl_radio_event_t *radio_event) {
    switch (cell.type) {
        case SLOT_TYPE_BEACON:
        case SLOT_TYPE_DOWNLINK:
            radio_event->radio_action = BLINK_RADIO_ACTION_TX;
            break;
        case SLOT_TYPE_SHARED_UPLINK:
        case SLOT_TYPE_UPLINK:
            radio_event->radio_action = BLINK_RADIO_ACTION_RX;
            break;
    }
}

void _compute_dotbot_action(cell_t cell, bl_radio_event_t *radio_event) {
    switch (cell.type) {
        case SLOT_TYPE_BEACON:
        case SLOT_TYPE_DOWNLINK:
            radio_event->radio_action = BLINK_RADIO_ACTION_RX;
            break;
        case SLOT_TYPE_SHARED_UPLINK:
            // TODO: implement backoff algorithm
            radio_event->radio_action = BLINK_RADIO_ACTION_TX;
            break;
        case SLOT_TYPE_UPLINK:
            if (cell.assigned_node_id == db_device_id()) {
                radio_event->radio_action = BLINK_RADIO_ACTION_TX;
            } else {
#ifdef BLINK_LISTEN_DURING_UNSCHEDULED_UPLINK
                // OPTIMIZATION: listen for beacons during unassigned uplink slot
                // listen to the same beacon channel for a whole slotframe
                radio_event->radio_action = BLINK_RADIO_ACTION_RX;
#if(BLINK_FIXED_CHANNEL != 0)
                radio_event->channel = BLINK_FIXED_CHANNEL;
#else
                radio_event->channel = BLINK_N_BLE_REGULAR_CHANNELS + (_schedule_vars.slotframe_counter % BLINK_N_BLE_ADVERTISING_CHANNELS);
#endif // BLINK_FIXED_CHANNEL
#endif // BLINK_LISTEN_DURING_UNSCHEDULED_UPLINK
            }
            break;
        default:
            break;
    }
}
