/******************************************************************************
 * @file phy.c
 * @brief This is the interface with all th phy layers This file manage all the jobs of all the phys
 * @author Nicolas Rabault
 * @version 0.0.0
 ******************************************************************************/

/******************************************************************************
 *
 * Phy receive messages from Luos and any other phy and dispatch all received messages to all phys
 *
 ************************************************* RX timeline representation *************************************************
 * ------------------+------------------------------------------------------------------------------------------------------------------+
 *                   | +---+-----------------+-----------------------+------------+---+                                                 |
 *    Phy RX bus     | | ? |      Header     |      data             |  Timestamp | ? |                                                 |
 *                   | +---+-----------------+-----------------------+------------+---+                                                 |
 * ------------------+-----------------------|----------------------------------------|-------------------------------------------------+
 *                   |                       V                                        V                                                 |
 *                   |                       +-------------------+                    +--------------+                                  |
 *    IRQ            |                       | Phy_ComputeHeader |                    | Phy_ValidMsg |                                  |
 *                   |                       +-------------------+                    +--------------+                                  |
 *                   |                        [Phy status updated]--+                             |                                     |
 * ------------------+----------------------------------------------|-----------------------------|-------------------------------------+
 *                   |                                              V                             |                                     |
 *                   |                                              +-----------+                 |        +--------------+             |
 *    Loop           |.......................|                   |..| Phy_alloc |.....|           |  |.....| Phy_Dispatch |.............|
 *                   |                                              +-----------+                 |        +--------------+             |
 *                   |                                                                            |        ^         |                  |
 * ------------------+----------------------------------------------------------------------------|--------|---------|------------------+
 *                   |                                                                            V        |         |                  |
 *                   |                                                                            +--------+         |  +------------+  |
 *    Generated      |                                                                            | io_job |         +->| Phy[0].Job |  |
 *    tasks          |                                                                            +--------+         |  +------------+  |
 *                   |                                                                                               |  +------------+  |
 * (Each generated   |                                                                                               +->| Phy[1].Job |  |
 *  job is inserted  |                                                                                               |  +------------+  |
 *  in a job table)  |                                                                                               |  +------------+  |
 *                   |                                                                                               +->| Phy[3].Job |  |
 *                   |                                                                                               |  +------------+  |
 *                   |                                                                                               |  +------------+  |
 *                   |                                                                                               +->| Phy[x].Job |  |
 *                   |                                                                                                  +------------+  |
 * ------------------+------------------------------------------------------------------------------------------------------------------+
 *
 ******************************************************************************/

#include <string.h>
#include "luos_phy.h"
#include "_luos_phy.h"
#include "filter.h"
#include "msg_alloc.h"
#include "luos_hal.h"
#include "_timestamp.h"
#include "luos_io.h"
// #include "service.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PHY_NB 2

typedef struct __attribute__((__packed__))
{
    uint64_t timestamp;
    msg_t *alloc_msg;
    phy_target_t phy_filter;
    uint16_t size;
} IO_job_t;

typedef struct
{
    // ******************** Phy management ********************
    luos_phy_t phy[PHY_NB]; // phy[0] is the local phy, phy[1] is the remote phy.
    uint8_t phy_nb;         // Number of phy instantiated in the phy_ctx.phy array.

    // ******************** job management ********************
    // io_jobs are stores from the newest to the oldest.
    // This will add time in IRQ when we will pull a job but allow to keep pointers constant.
    IO_job_t io_job[MAX_MSG_NB]; // Table of all the io_jobs to dispatch.
    volatile uint16_t io_job_nb; // Number of io_jobs in the io_job table.
    phy_job_t failed_job[4];     // Table of all the failed jobs we have to deal with.
    uint8_t failed_job_nb;       // Number of failed jobs in the failed_job table.
} luos_phy_ctx_t;

static void Phy_alloc(luos_phy_t *phy);
static void Phy_Dispatch(void);
static void Phy_ManageFailedJob(void);
static phy_job_t *Phy_AddJob(luos_phy_t *phy_ptr, phy_job_t *phy_job);
static int Phy_GetJobId(luos_phy_t *phy_ptr, phy_job_t *job);
static int Phy_GetPhyId(luos_phy_t *phy_ptr);

/*******************************************************************************
 * Variables
 ******************************************************************************/
luos_phy_ctx_t phy_ctx;

/*******************************************************************************
 * Functions
 ******************************************************************************/

/******************************************************************************
 * @brief Phy initialization
 * @param None
 * @return None
 ******************************************************************************/
void Phy_Init(void)
{
    Phy_Reset();
    phy_ctx.phy_nb = 1;
}

/******************************************************************************
 * @brief Reset the phy
 * @param None
 * @return None
 ******************************************************************************/
void Phy_Reset(void)
{
    // Put everything to 0
    memset((void *)phy_ctx.io_job, 0, sizeof(phy_ctx.io_job));
    phy_ctx.io_job_nb = 0;
    memset((void *)phy_ctx.failed_job, 0, sizeof(phy_ctx.failed_job));
    phy_ctx.failed_job_nb = 0;
    for (uint8_t i = 0; i < phy_ctx.phy_nb; i++)
    {
        memset((void *)&phy_ctx.phy[i].job, 0, sizeof(phy_ctx.phy[0].job));
        phy_ctx.phy[i].job_nb = 0;
    }
}

/******************************************************************************
 * @brief Phy loop
 * @param None
 * @return None
 ******************************************************************************/
void Phy_Loop(void)
{
    // Manage received data allocation
    // This is only needed for Robus for now.
    if (phy_ctx.phy[1].rx_alloc_job)
    {
        Phy_alloc(&phy_ctx.phy[1]);
    }
    Phy_ManageFailedJob();
    // Manage complete message received dispatching
    Phy_Dispatch();
    // Compute phy job statistics
    /*
    uint8_t stat = (uint8_t)((job nbr * 100) / (MAX_MSG_NB));
    if (stat > mem_stat->rx_msg_stack_ratio)
    {
        memory_stats_t mem_stat->rx_msg_stack_ratio = stat;
    }*/
}

/******************************************************************************
 * @brief Instanciate a physical layer
 * @param phy_cb callback to call when we want to transmit a message
 * @return None
 ******************************************************************************/
luos_phy_t *Phy_Create(PHY_CB phy_cb)
{
    return Phy_Get(phy_ctx.phy_nb++, phy_cb);
}

/******************************************************************************
 * @brief return the local physical layer
 * @param id of the phy we want
 * @param phy_cb callback to call when we want to transmit a message
 * @return None
 ******************************************************************************/
luos_phy_t *Phy_Get(uint8_t id, PHY_CB phy_cb)
{
    LUOS_ASSERT((id <= PHY_NB)
                && (phy_cb != NULL));
    // Set the callback
    phy_ctx.phy[id].phy_cb = phy_cb;
    // Return the phy pointer
    return &phy_ctx.phy[id];
}

/******************************************************************************
 * @brief Compute the size of the message
 * @param phy_ptr Pointer to the phy concerned by this message
 * @return None
 ******************************************************************************/
_CRITICAL void Phy_ComputeHeader(luos_phy_t *phy_ptr)
{
    LUOS_ASSERT(phy_ptr != NULL);
    // Compute the size of the data to allocate
    if (((header_t *)phy_ptr->rx_buffer_base)->size > MAX_DATA_MSG_SIZE)
    {
        // Cap the size to the maximum size of a message
        phy_ptr->rx_size = MAX_DATA_MSG_SIZE + sizeof(header_t);
    }
    else
    {
        phy_ptr->rx_size = ((header_t *)phy_ptr->rx_buffer_base)->size + sizeof(header_t);
    }
    // We need to check if we have a timestamped message and increase the data size if yes
    if (Luos_IsMsgTimstamped((msg_t *)(phy_ptr->rx_data)) == true)
    {
        phy_ptr->rx_size += sizeof(time_luos_t);
    }

    // Compute the phy concerned by this message
    phy_ptr->rx_phy_filter = Filter_GetPhyTarget((header_t *)phy_ptr->rx_buffer_base);
    // Remove the phy asking to compute the header to avoid to retransmit it, except for Luos because Luos can do localhost.
    uint32_t index = Phy_GetPhyId(phy_ptr);
    if (index != 0)
    {
        // remove the phy asking to compute the header, except if it is Luos because Luos can do localhost.
        phy_ptr->rx_phy_filter &= ~(0x01 << index);
    }
    if (phy_ptr->rx_phy_filter != 0)
    {
        // Someone need to receive this message
        phy_ptr->rx_keep      = true;
        phy_ptr->rx_alloc_job = true;
        phy_ptr->rx_ack       = ((((header_t *)phy_ptr->rx_buffer_base)->target_mode == SERVICEIDACK) || (((header_t *)phy_ptr->rx_buffer_base)->target_mode == NODEIDACK));
    }
    else
    {
        // No one need to receive this message
        phy_ptr->rx_keep      = false;
        phy_ptr->rx_alloc_job = false;
        phy_ptr->rx_ack       = false;
        return;
    }
}

/******************************************************************************
 * @brief Consider the message as valid
 * @param phy_ptr Pointer to the phy concerned by the allocation
 * @return None
 ******************************************************************************/
_CRITICAL void Phy_ValidMsg(luos_phy_t *phy_ptr)
{
    LUOS_ASSERT(phy_ptr != NULL);
    // Check if the allocation of the received message have been done
    if (phy_ptr->rx_alloc_job)
    {
        // We did not allocate the received data yet
        // This can happen if we did not had the time to execute Phy_Loop function before the end of the message reception or if this phy get the complete mgs in one time.
        Phy_alloc(phy_ptr);
    }
    if (phy_ptr->rx_keep == true)
    {
        // Now we can create a phy_job to dispatch the tx_job later
        LUOS_ASSERT(phy_ctx.io_job_nb < MAX_MSG_NB);
        LuosHAL_SetIrqState(false);
        phy_ctx.io_job[phy_ctx.io_job_nb].timestamp  = phy_ptr->rx_timestamp;
        phy_ctx.io_job[phy_ctx.io_job_nb].alloc_msg  = (msg_t *)phy_ptr->rx_data;
        phy_ctx.io_job[phy_ctx.io_job_nb].phy_filter = phy_ptr->rx_phy_filter;
        phy_ctx.io_job[phy_ctx.io_job_nb].size       = phy_ptr->rx_size;
        phy_ctx.io_job_nb++;
        // Then reset the phy to receive the next message
        phy_ptr->rx_data       = phy_ptr->rx_buffer_base;
        phy_ptr->received_data = 0;
        LuosHAL_SetIrqState(true);
    }
}

/******************************************************************************
 * @brief Compute the timestamp to send with the message
 * @param job Pointer to the job concerned by this message
 * @return None
 ******************************************************************************/
time_luos_t Phy_ComputeTimestamp(phy_job_t *job)
{
    LUOS_ASSERT((job != NULL) && (job->msg_pt != NULL) && (job->timestamp == true));
    return Timestamp_ConvertToLatency(job->msg_pt);
}

/******************************************************************************
 * @brief return the node id
 * @return Node ID value
 ******************************************************************************/
uint16_t Phy_GetNodeId(void)
{
    return Node_Get()->node_id;
}

/******************************************************************************
 * @brief Allocate the received data if needed
 * @param phy_ptr Pointer to the phy concerned by this message
 * @return None
 ******************************************************************************/
_CRITICAL static void Phy_alloc(luos_phy_t *phy_ptr)
{
    LUOS_ASSERT(phy_ptr != NULL);
    void *rx_data;
    void *copy_from;

    LuosHAL_SetIrqState(false);
    // Check if this phy really need to alloc
    if (phy_ptr->rx_alloc_job == false)
    {
        LuosHAL_SetIrqState(true);
        return;
    }
    // Check if we receive enougth data to be able to allocate the complete message
    LUOS_ASSERT((phy_ptr->received_data >= sizeof(header_t))
                && (phy_ptr->received_data <= MAX_DATA_MSG_SIZE + sizeof(header_t))
                // Check if their is a mistake on the buffer allocation. In this case, the phy.rx_data was not properly set to rx_buffer_base before the data reception.
                && (phy_ptr->rx_data == phy_ptr->rx_buffer_base));
    LuosHAL_SetIrqState(true);

    // Now we can check if we need to store the received data
    if (phy_ptr->rx_keep)
    {
        // We need to store the received data.
        // Update the informations allowing reception to continue and directly copy the data into the allocated buffer
        LuosHAL_SetIrqState(false);
        if (phy_ptr->rx_alloc_job)
        {
            uint16_t phy_stored_data_size = phy_ptr->received_data;
            phy_ptr->rx_alloc_job         = false;
            // Now allocate it
            rx_data          = MsgAlloc_Alloc(phy_ptr->rx_size, (uint8_t)phy_ptr->rx_phy_filter);
            phy_ptr->rx_data = rx_data;
            if ((phy_ptr == &phy_ctx.phy[0]) && (rx_data == NULL))
            {
                // We don't successfully allocated the message we are trying to send.
                // return and the transmitter will be able to wait to get more space...
                LuosHAL_SetIrqState(true);
                phy_ptr->rx_keep = false;
                return;
            }
            LUOS_ASSERT(rx_data != NULL); // Assert if the allocation failed. We don't allow to loose a message comming from outside.
            // Job is done
            LuosHAL_SetIrqState(true);
            copy_from = (void *)phy_ptr->rx_buffer_base;

            // Now we can copy the data already received
            memcpy(rx_data, copy_from, phy_stored_data_size);
            return;
        }
        LuosHAL_SetIrqState(true);
    }
    else
    {
        LuosHAL_SetIrqState(false);
        if (phy_ptr->rx_alloc_job)
        {
            // Job is done
            phy_ptr->rx_alloc_job = false;
            LuosHAL_SetIrqState(true);
            return;
        }
        LuosHAL_SetIrqState(true);
    }
}

/******************************************************************************
 * @brief Dispatch the received message
 * @param None
 * @return None
 ******************************************************************************/
static void Phy_Dispatch(void)
{
    static bool running = false;
    int i               = 0;
    if (running)
    {
        return;
    }
    // Interpreat received messages and create tasks for it.
    LuosHAL_SetIrqState(false);
    while (i < phy_ctx.io_job_nb)
    {
        LuosHAL_SetIrqState(true);
        // Get the oldest job
        IO_job_t *job = &phy_ctx.io_job[i];
        i++;
        running = false;
        LUOS_ASSERT((job->alloc_msg != NULL)
                    && (job->size >= sizeof(header_t)));
        running = true;
        // If message is timestamped, convert the latency to date
        if (Luos_IsMsgTimstamped(job->alloc_msg))
        {
            Timestamp_ConvertToDate(job->alloc_msg, job->timestamp);
        }
        // Network phy first then Luos in the end
        for (int y = phy_ctx.phy_nb - 1; y >= 0; y--)
        {
            // Loop in all phys
            if ((job->phy_filter >> y) & 0x01)
            {
                // Phy[y] is concerned by this message.
                // Generate the job and put it in the phy queue
                phy_job_t phy_job;
                phy_job.msg_pt    = job->alloc_msg;
                phy_job.size      = job->size;
                phy_job.ack       = ((job->alloc_msg->header.target_mode == NODEIDACK) || (job->alloc_msg->header.target_mode == SERVICEIDACK));
                phy_job.timestamp = Luos_IsMsgTimstamped(job->alloc_msg);

                // Write the job in the phy queue and get back the pointer to it
                phy_job_t *job_ptr = Phy_AddJob(&phy_ctx.phy[y], &phy_job);
                // Notify this phy that a job is available and give it the concerned job on his queue
                phy_ctx.phy[y].phy_cb(&phy_ctx.phy[y], job_ptr);
            }
        }
        LuosHAL_SetIrqState(false);
    }
    phy_ctx.io_job_nb = 0;
    LuosHAL_SetIrqState(true);
    running = false;
}

/******************************************************************************
 * @brief Manage a failed trasmit job.
 * @param phy_ptr pointer to the phy that failed to send a message.
 * @param job pointer to the job that failed.
 * @return None
 ******************************************************************************/
_CRITICAL void Phy_DeadTargetSpotted(luos_phy_t *phy_ptr, phy_job_t *job)
{
    // A phy failed to send a message, we need to be sure that our node don't try to contact this target again.
    LUOS_ASSERT((job != NULL)
                && (phy_ptr != NULL));
    LUOS_ASSERT((job->msg_pt != NULL)
                && (job->msg_pt->header.target_mode == NODEIDACK
                    || job->msg_pt->header.target_mode == SERVICEIDACK
                    || job->msg_pt->header.target_mode == NODEID
                    || job->msg_pt->header.target_mode == SERVICEID));

    // Store the job in the dead service spotted list
    phy_ctx.failed_job[phy_ctx.failed_job_nb++] = *job;
    uint16_t target                             = job->msg_pt->header.target;
    uint16_t target_mode                        = job->msg_pt->header.target_mode;

    // Remove all job targeting this target on this phy job queue;
    int i = 0;
    while (i < phy_ptr->job_nb)
    {
        if ((phy_ptr->job[i].msg_pt->header.target == target) && (phy_ptr->job[i].msg_pt->header.target_mode == target_mode))
        {
            // This job is targeting the dead target, remove it from the queue
            phy_ptr->job_nb--;
            for (int j = i; j < phy_ptr->job_nb; j++)
            {
                phy_ptr->job[j] = phy_ptr->job[j + 1];
            }
        }
        else
        {
            // Treat the next job
            i++;
        }
    }
}

/******************************************************************************
 * @brief Manage a failed trasmit job.
 * @param None
 * @return None
 ******************************************************************************/
static void Phy_ManageFailedJob(void)
{
    // This is allowing to avoid recursive call to this function
    static bool managing_failed_job = false;
    // Check if we have failed job to manage
    if ((phy_ctx.failed_job_nb != 0) && (managing_failed_job == false))
    {
        managing_failed_job = true;
        // We have failed jobs to manage
        for (int i = 0; i < phy_ctx.failed_job_nb; i++)
        {
            // Create a broadcast message to inform everyone that this target is dead
            msg_t dead_msg;
            dead_msg.header.cmd         = DEADTARGET;
            dead_msg.header.target_mode = BROADCAST;
            dead_msg.header.target      = BROADCAST_VAL;
            dead_msg.header.size        = sizeof(dead_target_t);

            dead_target_t *dead_target = (dead_target_t *)dead_msg.data;
            if (phy_ctx.failed_job[i].msg_pt->header.target_mode == NODEIDACK || phy_ctx.failed_job[i].msg_pt->header.target_mode == NODEID)
            {
                dead_target->node_id    = phy_ctx.failed_job[i].msg_pt->header.target;
                dead_target->service_id = 0;
            }
            else
            {
                dead_target->node_id    = 0;
                dead_target->service_id = phy_ctx.failed_job[i].msg_pt->header.target;
            }
            Luos_SendMsg(0, &dead_msg);
        }
        phy_ctx.failed_job_nb = 0;
        managing_failed_job   = false;
    }
}

/******************************************************************************
 * @brief Add a job to the phy queue
 * @param phy_ptr Phy to add the job to
 * @param phy_job Job to add
 * @return None
 ******************************************************************************/
static phy_job_t *Phy_AddJob(luos_phy_t *phy_ptr, phy_job_t *phy_job)
{
    // Check if the phy queue is full
    LUOS_ASSERT((phy_job != NULL) && (phy_ptr != NULL));
    LUOS_ASSERT(phy_ptr->job_nb < MAX_MSG_NB);
    // Add the job to the queue
    LuosHAL_SetIrqState(false);
    phy_ptr->job[phy_ptr->job_nb++] = *phy_job;
    LuosHAL_SetIrqState(true);
    return &phy_ptr->job[phy_ptr->job_nb - 1];
}

/******************************************************************************
 * @brief Get the oldest job from the phy queue
 * @param phy_ptr Phy to get the job from
 * @return Job pointer
 ******************************************************************************/
_CRITICAL inline phy_job_t *Phy_GetJob(luos_phy_t *phy_ptr)
{
    LUOS_ASSERT(phy_ptr != NULL);
    if (phy_ptr->job_nb == 0)
    {
        return NULL;
    }
    return &phy_ptr->job[0];
}

/******************************************************************************
 * @brief Get the job id from the job pointer
 * @param phy_ptr Phy to get the job id from
 * @param job Job pointer
 * @return Job id
 ******************************************************************************/
inline int Phy_GetJobId(luos_phy_t *phy_ptr, phy_job_t *job)
{
    LUOS_ASSERT((phy_ptr != NULL)
                && (job >= phy_ptr->job)
                && (job < &phy_ptr->job[MAX_MSG_NB]));
    return (((uintptr_t)job - (uintptr_t)phy_ptr->job) / sizeof(phy_job_t));
}

inline int Phy_GetPhyId(luos_phy_t *phy_ptr)
{
    LUOS_ASSERT((phy_ptr >= phy_ctx.phy)
                && (phy_ptr < &phy_ctx.phy[PHY_NB]));
    return ((uintptr_t)phy_ptr - (uintptr_t)phy_ctx.phy) / sizeof(luos_phy_t);
}

/******************************************************************************
 * @brief Remove the oldest job from the phy queue
 * @param phy_ptr Phy to remove the job from
 * @return None
 ******************************************************************************/
_CRITICAL void Phy_RmJob(luos_phy_t *phy_ptr, phy_job_t *job)
{
    LUOS_ASSERT((phy_ptr != NULL)
                && (job >= phy_ptr->job)
                && (job < &phy_ptr->job[MAX_MSG_NB])
                && (phy_ptr->job_nb != 0));
    // We potentially need to free this message from msg allocator
    // Get the Phy index
    int phy_index = Phy_GetPhyId(phy_ptr);
    MsgAlloc_Free(phy_index, job->data_pt);

    // Remove the job from the queue
    uint8_t id = Phy_GetJobId(phy_ptr, job);
    for (int i = id; i < phy_ptr->job_nb; i++)
    {
        phy_ptr->job[i] = phy_ptr->job[i + 1];
    }
    phy_ptr->job_nb--;
}

/******************************************************************************
 * @brief Get the number of job in the phy queue
 * @param phy_ptr Phy to get the number of job from
 * @return Number of job
 ******************************************************************************/
_CRITICAL inline uint16_t Phy_GetJobNbr(luos_phy_t *phy_ptr)
{
    LUOS_ASSERT(phy_ptr != NULL);
    return phy_ptr->job_nb;
}

/******************************************************************************
 * @brief define is there is something waiting to be sent or not
 * @return Succeed if nothing is waiting to be sent
 ******************************************************************************/
error_return_t Phy_TxAllComplete(void)
{
    // We don't check the first phy because it is Luos
    for (int i = 1; i < phy_ctx.phy_nb; i++)
    {
        if (Phy_GetJobNbr(&phy_ctx.phy[i]) != 0)
        {
            return FAILED;
        }
    }
    return SUCCEED;
}
