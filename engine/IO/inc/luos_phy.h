/******************************************************************************
 * @file phy.h
 * @brief This file have to be imported by phy layers to interract with Luos
 * @author Nicolas Rabault
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOS_PHY_H_
#define _LUOS_PHY_H_

#include <stdint.h>
#include <stdbool.h>
#include "struct_phy.h"
#include "time_luos.h"
#include "luos_utils.h"
#include "luos_hal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef void (*JOB_CB)(luos_phy_t *phy_ptr, phy_job_t *job);
typedef error_return_t (*RUN_TOPO)(luos_phy_t *phy_ptr, uint8_t *portId);
typedef void (*RESET_PHY)(luos_phy_t *phy_ptr);
typedef void (*IRQ_STATE)(bool state);

// Irq management
void Phy_SetIrqStateFunciton(IRQ_STATE irq_state); // Use it to reference your phy specific Irq state management to Luos.
void Phy_SetIrqState(bool state);                  // Use it to globally enable/disable all the phy irq.

// Phy creation
luos_phy_t *Phy_Create(JOB_CB job_cb, RUN_TOPO run_topo, RESET_PHY reset_phy); // Use it to reference your phy to Luos.

// Topology management
void Phy_TopologyNext(void);                                   // Use it to find the next node that need to be detected accross phys.
void Phy_TopologySource(luos_phy_t *phy_ptr, uint8_t port_id); // The phy will call this function when a new node is detected on a specific port.
void Phy_TopologyDone(luos_phy_t *phy_ptr);                    // The phy will call this function when all its port detection are done.

// Rx management
void Phy_ComputeHeader(luos_phy_t *phy_ptr); // After receiving the first 7 bytes (the header) call this function to compute how you should manage the incoming message.
void Phy_ValidMsg(luos_phy_t *phy_ptr);      // After receiving as much valid bytes as phy_ptr.rx_size, call this function to validate the message.
void Phy_ResetMsg(luos_phy_t *phy_ptr);      // Call this function to reset the rx process.

// Tx management
time_luos_t Phy_ComputeMsgTimestamp(phy_job_t *job); // Use it to compute the timestamp of the message to send.
time_luos_t Phy_GetTimestamp(void);                  // Use it to get the current timestamp.
uint16_t Phy_GetNodeId(void);                        // Use it to get your current node id. (This can be used to compute priority or controled latency avoiding infinite collision condition)

// Job management
void Phy_FailedJob(luos_phy_t *phy_ptr, phy_job_t *job); // If some messages failed to be sent, call this function to consider the target as dead
phy_job_t *Phy_GetJob(luos_phy_t *phy_ptr);              // Use it to get the first job to send.
void Phy_RmJob(luos_phy_t *phy_ptr, phy_job_t *job);     // Use it to remove a job from your phy job list when it's done.
uint16_t Phy_GetJobNumber(luos_phy_t *phy_ptr);          // Use it to get the number of job to send.

#endif /* _LUOS_PHY_H_ */
