/******************************************************************************
 * @file phy.h
 * @brief This file have to be imported by phy layers to interract with Luos
 * @author Nicolas Rabault
 * @version 0.0.0
 ******************************************************************************/
#ifndef _STRUCT_PHY_H_
#define _STRUCT_PHY_H_

#include <stdint.h>
#include <stdbool.h>
#include "struct_luos.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * @struct phy_job_t
 * @brief Definition of a phy job (task to do by the phy)
 ******************************************************************************/
typedef struct
{
    // This is allowing to access the data as data pointer or message
    union
    {
        const uint8_t *data_pt; // Start pointer of the data on msg_buffer.
        const msg_t *msg_pt;    // Pointer to the message.
    };
    uint16_t size;  // size of the data.
    bool ack;       // True if we want to receive a message acknoledgement.
    bool timestamp; // True if this job is timestamped.
    void *phy_data; // Phy developpers can use this pointer to store any data they need.
} phy_job_t;

#endif /* _STRUCT_PHY_H_ */
