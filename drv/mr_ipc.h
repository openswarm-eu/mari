#ifndef __MR_IPC_H
#define __MR_IPC_H

/**
 * @brief       Read the RNG peripheral
 *
 * @{
 * @file
 * @author Alexandre Abadie <alexandre.abadie@inria.fr>
 * @author Geovane Fedrecheski <geovane.fedrecheski@inria.fr>
 * @copyright Inria, 2025-now
 * @}
 */

#include <stdint.h>
#include <stdbool.h>

//=========================== defines ==========================================

typedef enum {
    MR_IPC_REQ_NONE,  ///< Sorry, but nothing
} ipc_req_t;

typedef enum {
    MR_IPC_CHAN_REQ        = 0,  ///< Channel used for request events
    MR_IPC_CHAN_MIRA_EVENT = 1,  ///< Channel used for mira events
} ipc_channels_t;

typedef struct __attribute__((packed)) {
    bool      net_ready;  ///< Network core is ready
    bool      net_ack;    ///< Network core acked the latest request
    ipc_req_t req;        ///< IPC network request
} ipc_shared_data_t;

//=========================== prototypes =======================================

#endif  // __MR_IPC_H
