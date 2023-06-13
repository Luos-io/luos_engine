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

/******************************************************************************
 * @struct phy_target_t
 * @brief Transmit message direction
 ******************************************************************************/
typedef uint8_t phy_target_t;

/******************************************************************************
 * @struct luos_phy_t
 * @brief the data structure allowing the phy to interract with LuosIO
 ******************************************************************************/
typedef struct luos_phy_t
{
    // *************** RX information ***************
    volatile int64_t rx_timestamp; // Timestamp of the last received message. We will use it to compute the date based on received latency.
    uint8_t *rx_buffer_base;       // Pointer to the base of the buffer where we will store the begining of received data.
    union
    {
        volatile uint8_t *rx_data; // Pointer to write received bytes.
        volatile msg_t *rx_msg;    // Pointer to the message where we will write received bytes.
    };
    volatile uint16_t received_data; // Number of bytes already received.
    volatile uint16_t rx_size;       // Size of data to receive.
    struct
    {
        volatile uint8_t rx_alloc_job : 1; // If true, Luosio_Loop funciton will need to filter already received data, if filter is ok, set rx_keep, alloc needed space, move *rx_data to point at the good place on allocator, put the number of bytes remaining on rx_size, and copy already received data in the allocated space.
        volatile uint8_t rx_keep : 1;      // True if we want to keep the received data.
        volatile uint8_t rx_ack : 1;       // True if we need to generate an acknoledgement for this message.
    };

    // RX Private data
    phy_target_t rx_phy_filter; // The phy concerned by this message.

    // *************** TX informations ***************
    // Jobs are used to send messages. during the message send phy may save a job pointer so we cann't move any job in the job list.
    // When a job is created we have to guarantee that the job will stay. So we have to manage it as a circular buffer.
    phy_job_t job[MAX_MSG_NB];    // List of phy jobs to send.
    uint8_t job_nb;               // Number of jobs to send.
    uint16_t oldest_job_index;    // Index of the oldest job.
    uint16_t available_job_index; // Index of the next available job.

    void (*phy_cb)(struct luos_phy_t *phy_ptr, phy_job_t *job); // Callback
} luos_phy_t;

typedef void (*PHY_CB)(luos_phy_t *phy_ptr, phy_job_t *job);

// Phy creation
luos_phy_t *Phy_Create(PHY_CB phy_cb); // Use it to reference your phy to Luos.

// Rx management
void Phy_ComputeHeader(luos_phy_t *phy_ptr); // After receiving the first 7 bytes (the header) call this function to compute how you should manage the incoming message.
void Phy_ValidMsg(luos_phy_t *phy_ptr);      // After receiving as much valid bytes as phy_ptr.rx_size, call this function to validate the message.

// Tx management
time_luos_t Phy_ComputeTimestamp(phy_job_t *job);
uint16_t Phy_GetNodeId(void);

// Job management
void Phy_DeadTargetSpotted(luos_phy_t *phy_ptr, phy_job_t *job); // If some messages failed to be sent, call this function to consider the target as dead
phy_job_t *Phy_GetJob(luos_phy_t *phy_ptr);                      // Use it to get the first job to send.
phy_job_t *Phy_GetNextJob(luos_phy_t *phy_ptr, phy_job_t *job);  // Use it to get the next job to send.
void Phy_RmJob(luos_phy_t *phy_ptr, phy_job_t *job);             // Use it to remove a job from your phy job list when it's done.
uint16_t Phy_GetJobNbr(luos_phy_t *phy_ptr);                     // Use it to get the number of job you currently have to send on your phy_ptr.job list.

#endif /* _LUOS_PHY_H_ */
