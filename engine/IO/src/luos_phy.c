/******************************************************************************
 * @file phy.c
 * @brief This is the interface with all th phy layers This file manage all the jobs of all the phys
 * @author Nicolas Rabault
 * @version 0.0.0
 ******************************************************************************/

#include <string.h>
#include "luos_phy.h"
#include "filter.h"
#include "msg_alloc.h"
#include "engine_config.h"
#include "luos_hal.h"
#include "node.h"
#include "_timestamp.h"
#include "robus.h"
#include "luos_io.h"
#include "_luos_io.h"
#include "service.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    uint64_t timestamp;
    msg_t *alloc_msg;
    luos_localhost_t phy_filter;
} luos_phy_job_t;

typedef struct
{
    luos_phy_t phy;
    luos_phy_job_t phy_job[MAX_MSG_NB];
    volatile uint16_t phy_job_nb;
} luos_phy_ctx_t;

static void Phy_alloc(luos_phy_t *phy);
static void Phy_Dispatch(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
luos_phy_ctx_t phy_ctx;

/*******************************************************************************
 * Functions
 ******************************************************************************/

/******************************************************************************
 * @brief POhy initialization
 * @param None
 * @return None
 ******************************************************************************/
void Phy_Init(void)
{
    // Put everything to 0
    memset((void *)phy_ctx.phy_job, 0, sizeof(phy_ctx.phy_job));
    phy_ctx.phy_job_nb = 0;
}

/******************************************************************************
 * @brief Phy loop
 * @param None
 * @return None
 ******************************************************************************/
void Phy_Loop(void)
{
    // Manage received data allocation
    if (phy_ctx.phy.rx_alloc_job)
    {
        Phy_alloc(&phy_ctx.phy);
    }
    // Manage complete message received dispatching
    Phy_Dispatch();
    Robus_Loop();
}

/******************************************************************************
 * @brief Instanciate a physical layer
 * @param None
 * @return None
 ******************************************************************************/
luos_phy_t *Phy_Create(void)
{
    return &phy_ctx.phy;
}

/******************************************************************************
 * @brief Send data on the physical layer
 * @param data Data to send
 * @param size Size of the data to send
 * @return None
 ******************************************************************************/
void Phy_Send(luos_phy_t *phy_ptr, uint8_t *data, uint16_t size)
{
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
    // Now we can create a phy_job to dispatch the tx_job later
    LUOS_ASSERT(phy_ctx.phy_job_nb < MAX_MSG_NB);
    LuosHAL_SetIrqState(false);
    phy_ctx.phy_job[phy_ctx.phy_job_nb].timestamp  = phy_ptr->rx_timestamp;
    phy_ctx.phy_job[phy_ctx.phy_job_nb].alloc_msg  = (msg_t *)phy_ptr->rx_data;
    phy_ctx.phy_job[phy_ctx.phy_job_nb].phy_filter = phy_ptr->rx_phy_filter;
    phy_ctx.phy_job_nb++;
    // Then reset the phy to receive the next message
    phy_ptr->rx_data       = phy_ptr->rx_buffer_base;
    phy_ptr->received_data = 0;
    LuosHAL_SetIrqState(true);
}

/******************************************************************************
 * @brief Compute the size of the message
 * @param phy_ptr Pointer to the phy concerned by this message
 * @return None
 ******************************************************************************/
inline void Phy_ComputeMsgSize(luos_phy_t *phy_ptr)
{

    // Compute the size of the data to allocate
    if (((header_t *)phy_ptr->rx_buffer_base)->size > MAX_DATA_MSG_SIZE)
    {
        // Cap the size to the maximum size of a message
        phy_ptr->rx_size = MAX_DATA_MSG_SIZE + sizeof(header_t);
    }
    else
    {
        phy_ptr->rx_size = ((header_t *)phy_ptr->rx_buffer_base)->size + sizeof(header_t);
        // We need to check if we have a timestamped message and increase the data size if yes
        if (Luos_IsMsgTimstamped((msg_t *)(phy_ptr->rx_data)) == true) // TODO move it to TX
        {
            phy_ptr->rx_size += sizeof(time_luos_t);
        }
    }
}

/******************************************************************************
 * @brief Allocate the received data if needed
 * @param phy_ptr Pointer to the phy concerned by this message
 * @return None
 ******************************************************************************/
_CRITICAL static void Phy_alloc(luos_phy_t *phy_ptr)
{
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
    LUOS_ASSERT((phy_ptr->received_data >= sizeof(header_t)) && (phy_ptr->received_data <= MAX_DATA_MSG_SIZE + sizeof(header_t)));
    // Check if their is a mistake on the buffer allocation. In this case, the phy.rx_data was not properly set to rx_buffer_base before the data reception.
    LUOS_ASSERT(phy_ptr->rx_data == phy_ptr->rx_buffer_base);
    LuosHAL_SetIrqState(true);

    // We need to allocate this phy received data
    Phy_ComputeMsgSize(phy_ptr);

    // Compute the phy concerned by this message
    luos_localhost_t phy_filter = Filter_GetLocalhost((header_t *)phy_ptr->rx_buffer_base);

    // Now we can check if we need to store the received data
    if (phy_filter != EXTERNALHOST)
    {
        // We need to store the received data.
        // Update the informations allowing reception to continue and directly copy the data into the allocated buffer
        LuosHAL_SetIrqState(false);
        if (phy_ptr->rx_alloc_job)
        {
            uint16_t phy_stored_data_size = phy_ptr->received_data;
            phy_ptr->rx_alloc_job         = false;
            // Now allocate it
            rx_data = MsgAlloc_Alloc(phy_ptr->rx_size);
            // Assert if the allocation failed. We don't allow to loose a RX message.
            LUOS_ASSERT(rx_data != NULL);
            phy_ptr->rx_data = rx_data;
            // Job is done
            LuosHAL_SetIrqState(true);
            phy_ptr->rx_keep       = true;
            phy_ptr->rx_phy_filter = phy_filter;
            copy_from              = (void *)phy_ptr->rx_buffer_base;

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
            // We don't need to store the received data.
            phy_ptr->rx_keep = false;
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
    // Interpreat received messages and create luos task for it.
    while (phy_ctx.phy_job_nb)
    {
        // Get the oldest job
        LuosHAL_SetIrqState(false);
        luos_phy_job_t job = phy_ctx.phy_job[--phy_ctx.phy_job_nb];
        LuosHAL_SetIrqState(true);
        // If message is timestamped, convert the latency to date
        if (Luos_IsMsgTimstamped(job.alloc_msg))
        {
            // This conversion also remove the timestamp from the message size.
            Timestamp_ConvertToDate(job.alloc_msg, job.timestamp);
        }
        // Check if this message is a protocol one
        if (LuosIO_MsgHandler(job.alloc_msg) == FAILED)
        {
            // If not create luos tasks for all services.
            Service_AllocMsg(job.alloc_msg);
            // Later this service level dispatch will need to be done in core.
            // LuosIO shoudn't care about service level, and should just list the message as a task to do on the Luos side.
        }
    }
    // Localhost message are still stored on the MsgAlloc msg_task buffer, we have to deal with it too.
    msg_t *msg;
    while (MsgAlloc_PullMsgToInterpret(&msg) == SUCCEED)
    {
        // Check if this message is a protocol one
        if (LuosIO_MsgHandler(msg) == FAILED)
        {
            // If not create luos tasks for all services.
            Service_AllocMsg(msg);
        }
    }
}
