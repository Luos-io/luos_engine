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
    volatile uint8_t job_nb;      // Number of jobs to send.
    uint16_t oldest_job_index;    // Index of the oldest job.
    uint16_t available_job_index; // Index of the next available job.

    void (*job_cb)(struct luos_phy_t *phy_ptr, phy_job_t *job);              // Callback
    error_return_t (*run_topo)(struct luos_phy_t *phy_ptr, uint8_t *portId); // try to find another node on branches, return success if a node is found and the id of the port were we detect this node.
    void (*reset_phy)(struct luos_phy_t *phy_ptr);                           // Reset the phy transmission reception and topology.

} luos_phy_t;

typedef struct __attribute__((__packed__))
{
    uint16_t node_id;
    uint8_t phy_id;
    uint8_t port_id;
} port_t;

typedef struct __attribute__((__packed__))
{
    port_t parent;
    port_t child;
} connection_t;

#endif /* _STRUCT_PHY_H_ */
