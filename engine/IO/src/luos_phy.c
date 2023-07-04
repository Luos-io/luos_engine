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
    luos_phy_t phy[PHY_NB];           // phy[0] is the local phy, phy[1] is the remote phy.
    uint8_t phy_nb;                   // Number of phy instantiated in the phy_ctx.phy array.
    IRQ_STATE phy_irq_states[PHY_NB]; // Store the irq state functions of phys aving one.

    // ******************** Topology management ********************
    port_t topology_source; // The source port. Where we receive the topological detection signal from.
    uint32_t topology_done; // We put bits to 1 when a phy ended the topology detection.
    bool topology_running;  // We put bits to 1 when a phy is running the topology detection.
    bool find_next_node_job;

    // ******************** Job management ********************
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
    memset(phy_ctx.phy_irq_states, 0, sizeof(phy_ctx.phy_irq_states));
}

/******************************************************************************
 * @brief Reset the phy context
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
        phy_ctx.phy[i].job_nb              = 0;
        phy_ctx.phy[i].oldest_job_index    = 0;
        phy_ctx.phy[i].available_job_index = 0;
    }
    memset((void *)&phy_ctx.topology_source, 0xFFFF, sizeof(phy_ctx.topology_source));
    phy_ctx.topology_done      = 0;
    phy_ctx.topology_running   = false;
    phy_ctx.find_next_node_job = false;
}

/******************************************************************************
 * @brief Reset the phy context and all the phy instances
 * @param None
 * @return None
 ******************************************************************************/
void Phy_ResetAll(void)
{
    Phy_Reset();
    // Now call the reset fuction of each phy
    for (uint8_t i = 0; i < phy_ctx.phy_nb; i++)
    {
        phy_ctx.phy[i].reset_phy(&phy_ctx.phy[i]);
    }
}

/******************************************************************************
 * @brief Check if a phy is actually detecting nodes on one of its port
 * @param None
 * @return None
 ******************************************************************************/
bool Phy_Busy(void)
{
    return phy_ctx.topology_running;
}

/******************************************************************************
 * @brief Phy loop
 * @param None
 * @return None
 ******************************************************************************/
void Phy_Loop(void)
{
    // Manage received data allocation
    // This is only needed for external phy for now.
    if (phy_ctx.phy[1].rx_alloc_job)
    {
        Phy_alloc(&phy_ctx.phy[1]);
    }
    Phy_ManageFailedJob();
    // Manage complete message received dispatching
    Phy_Dispatch();
    // Check if we need to find the next node
    if (phy_ctx.find_next_node_job == true)
    {
        phy_ctx.find_next_node_job = false;
        // Wait for the node to send all its messages.
        while (Phy_TxAllComplete() == FAILED)
            ;
        Phy_FindNextNode();
    }
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
luos_phy_t *Phy_Create(JOB_CB job_cb, RUN_TOPO run_topo, RESET_PHY reset_phy)
{
    return Phy_Get(phy_ctx.phy_nb++, job_cb, run_topo, reset_phy);
}

/******************************************************************************
 * @brief Reference a specific irq state function to a physical layer
 * @param irq_state function pointer to the IRQ state function
 * @return None
 ******************************************************************************/
void Phy_SetIrqStateFunciton(IRQ_STATE irq_state)
{
    int i = 0;
    while (phy_ctx.phy_irq_states[i] != NULL)
    {
        i++;
        if (i >= PHY_NB)
        {
            // We exceed the number of phy
            LUOS_ASSERT(0);
            return;
        }
    }
    // Save the function pointer
    phy_ctx.phy_irq_states[i] = irq_state;
}

/******************************************************************************
 * @brief Run all the phy irq state function referenced
 * @param state state of the IRQ
 * @return None
 ******************************************************************************/
void Phy_SetIrqState(bool state)
{
    // Loop int the phy_irq_states and call each function referenced
    int i = 0;
    while (phy_ctx.phy_irq_states[i] != NULL)
    {
        phy_ctx.phy_irq_states[i](state);
        i++;
        if (i >= PHY_NB)
        {
            // We exceed the number of phy
            LUOS_ASSERT(0);
            return;
        }
    }
}

/******************************************************************************
 * @brief save a flag allowing to run a new discovering outside of IRQ (because this function is very long and can't be run in IRQ)
 * @return None
 ******************************************************************************/
_CRITICAL void Phy_FindNextNodeJob(void)
{
    phy_ctx.find_next_node_job = true;
}
/******************************************************************************
 * @brief Try to find the next node connected a phy port
 * @return SUCCESS if a node is found, FAILED if not
 ******************************************************************************/
error_return_t Phy_FindNextNode(void)
{
    // Loop through all the phys except Luos and check if they need to be detected
    // We have to make the source phy the last one to check because it is the one that will send back the token when all its ports will be detected.
    // So we need to finish detections of all other phys before checking the source phy.
    for (uint8_t i = 1; i < phy_ctx.phy_nb; i++)
    {
        if ((!(phy_ctx.topology_done & (1 << i))) && (phy_ctx.topology_source.phy_id != i))
        {
            // This phy still have port to detect
            uint8_t port_id;
            // Check if a node is connected
            phy_ctx.topology_running = true;
            if (phy_ctx.phy[i].run_topo(&phy_ctx.phy[i], &port_id) == SUCCEED)
            {
                port_t output_port;
                output_port.node_id = Node_Get()->node_id;
                output_port.port_id = port_id;
                output_port.phy_id  = i;

                // We find a new node on this specific output_port
                // Send the output_port information to master as a partial CONNECTION_DATA and ask it to generate and send a new node_id.
                msg_t msg;
                msg.header.target_mode = NODEIDACK;
                msg.header.target      = 1;
                msg.header.cmd         = CONNECTION_DATA;
                msg.header.size        = sizeof(port_t);
                memcpy(msg.data, &output_port, sizeof(port_t));
                Luos_SendMsg(0, &msg);
                return SUCCEED;
            }
            phy_ctx.topology_running = false;
        }
    }
    // We checked all the phys except the source one.
    // Check if the source phy still have port to detect
    if (phy_ctx.topology_done != (1 << phy_ctx.phy_nb) - 2)
    {
        // We still have a phy to detect. This must be the source phy.
        // Check that only the source phy and the luos phy are not detected.
        LUOS_ASSERT((phy_ctx.topology_done | (1 << phy_ctx.topology_source.phy_id)) == ((1 << phy_ctx.phy_nb) - 2));
        uint8_t port_id;
        // Check if a node is connected
        phy_ctx.topology_running = true;
        if (phy_ctx.phy[phy_ctx.topology_source.phy_id].run_topo(&phy_ctx.phy[phy_ctx.topology_source.phy_id], &port_id) == SUCCEED)
        {
            port_t output_port;
            output_port.node_id = Node_Get()->node_id;
            output_port.port_id = port_id;
            output_port.phy_id  = phy_ctx.topology_source.phy_id;

            // We find a new node on this specific output_port
            // Send the output_port information to master as a partial CONNECTION_DATA and ask it to generate and send a new node_id.
            msg_t msg;
            msg.header.target_mode = NODEIDACK;
            msg.header.target      = 1;
            msg.header.cmd         = CONNECTION_DATA;
            msg.header.size        = sizeof(port_t);
            memcpy(msg.data, &output_port, sizeof(port_t));
            Luos_SendMsg(0, &msg);
            return SUCCEED;
        }
        phy_ctx.topology_running = false;
    }
    // This is the end of detection for our node.
    return FAILED;
}

/******************************************************************************
 * @brief A phy port have been detected
 * @param phy_ptr pointer on the phy which have been detected
 * @param port_id id of the port detected in the phy
 * @return None
 ******************************************************************************/
void Phy_Topologysource(luos_phy_t *phy_ptr, uint8_t port_id)
{
    LUOS_ASSERT((phy_ptr != NULL)
                && (port_id < 0xFF));
    // This port is the source of a topology request. it become the input port.
    // We have to save it in the node context.
    phy_ctx.topology_source.phy_id  = Phy_GetPhyId(phy_ptr);
    phy_ctx.topology_source.port_id = port_id;
    phy_ctx.topology_source.node_id = 0xFFFF;
    // We don't have the node id yet, we will fill it out when we will receive it from the master.
    // Put a flag to indicate that we are waiting for a node id.
    Node_WillGetId();
}

void Phy_TopologyDone(luos_phy_t *phy_ptr)
{
    LUOS_ASSERT(phy_ptr != NULL);
    phy_ctx.topology_done    = (1 << Phy_GetPhyId(phy_ptr));
    phy_ctx.topology_running = false;
}

/******************************************************************************
 * @brief A phy port have been detected
 * @return pointer on the input port
 ******************************************************************************/
port_t *Phy_GetTopologysource(void)
{
    return &phy_ctx.topology_source;
}

/******************************************************************************
 * @brief return the local physical layer (only used by LuosIO, this function is private)
 * @param id of the phy we want
 * @param phy_cb callback to call when we want to transmit a message
 * @return None
 ******************************************************************************/
luos_phy_t *Phy_Get(uint8_t id, JOB_CB job_cb, RUN_TOPO run_topo, RESET_PHY reset_phy)
{
    LUOS_ASSERT((id <= PHY_NB)
                && (job_cb != NULL)
                && (run_topo != NULL));
    // Set the callbacks
    phy_ctx.phy[id].job_cb    = job_cb;
    phy_ctx.phy[id].run_topo  = run_topo;
    phy_ctx.phy[id].reset_phy = reset_phy;
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
        // This message is ok we can reference it in the allocator
        MsgAlloc_Reference((uint8_t *)phy_ptr->rx_data, (uint8_t)phy_ptr->rx_phy_filter);

        // Now we can create a phy_job to dispatch the tx_job later
        LUOS_ASSERT(phy_ctx.io_job_nb < MAX_MSG_NB);
        Phy_SetIrqState(false);
        uint16_t my_job = phy_ctx.io_job_nb++;
        Phy_SetIrqState(true);
        // Now copy the data in the job
        phy_ctx.io_job[my_job].timestamp  = phy_ptr->rx_timestamp;
        phy_ctx.io_job[my_job].alloc_msg  = (msg_t *)phy_ptr->rx_data;
        phy_ctx.io_job[my_job].phy_filter = phy_ptr->rx_phy_filter;
        phy_ctx.io_job[my_job].size       = phy_ptr->rx_size;

        // Then reset the phy to receive the next message
        phy_ptr->rx_data       = phy_ptr->rx_buffer_base;
        phy_ptr->received_data = 0;
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

    Phy_SetIrqState(false);
    // Check if this phy really need to alloc
    if (phy_ptr->rx_alloc_job == false)
    {
        Phy_SetIrqState(true);
        return;
    }
    // Check if we receive enougth data to be able to allocate the complete message
    LUOS_ASSERT((phy_ptr->received_data >= sizeof(header_t))
                && (phy_ptr->received_data <= MAX_DATA_MSG_SIZE + sizeof(header_t))
                // Check if their is a mistake on the buffer allocation. In this case, the phy.rx_data was not properly set to rx_buffer_base before the data reception.
                && (phy_ptr->rx_data == phy_ptr->rx_buffer_base));
    Phy_SetIrqState(true);

    // Now we can check if we need to store the received data
    if (phy_ptr->rx_keep)
    {
        // We need to store the received data.
        // Update the informations allowing reception to continue and directly copy the data into the allocated buffer
        Phy_SetIrqState(false);
        if (phy_ptr->rx_alloc_job)
        {
            uint16_t phy_stored_data_size = phy_ptr->received_data;
            phy_ptr->rx_alloc_job         = false;
            // Now allocate it
            rx_data          = MsgAlloc_Alloc(phy_ptr->rx_size, (uint8_t)phy_ptr->rx_phy_filter);
            phy_ptr->rx_data = rx_data;
            Phy_SetIrqState(true);
            // Check if this message is a luos transmission and if allocation succeed
            if ((phy_ptr == &phy_ctx.phy[0]) && (rx_data == NULL))
            {
                // We don't successfully allocated the message we are trying to send.
                // return and the transmitter will be able to wait to get more space...
                phy_ptr->rx_keep = false;
                return;
            }
            LUOS_ASSERT(rx_data != NULL); // Assert if the allocation failed. We don't allow to loose a message comming from outside.
            // Job is done
            copy_from = (void *)phy_ptr->rx_buffer_base;

            // Now we can copy the data already received
            memcpy(rx_data, copy_from, phy_stored_data_size);
            return;
        }
        Phy_SetIrqState(true);
    }
    else
    {
        // We don't want to keep it so we don't allocate it.
        phy_ptr->rx_alloc_job = false;
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
    if ((running) || (phy_ctx.io_job_nb == 0))
    {
        return;
    }
    // Interpreat received messages and create tasks for it.
    Phy_SetIrqState(false);
    while (i < phy_ctx.io_job_nb)
    {
        Phy_SetIrqState(true);
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
                phy_job.phy_data  = NULL;

                // Write the job in the phy queue and get back the pointer to it
                phy_job_t *job_ptr = Phy_AddJob(&phy_ctx.phy[y], &phy_job);
                // Notify this phy that a job is available and give it the concerned job on his queue
                phy_ctx.phy[y].job_cb(&phy_ctx.phy[y], job_ptr);
            }
        }
        Phy_SetIrqState(false);
    }
    phy_ctx.io_job_nb = 0;
    Phy_SetIrqState(true);
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

    int i = phy_ptr->oldest_job_index;
    while (i != phy_ptr->available_job_index)
    {
        if (phy_ptr->job[i].msg_pt != NULL)
        {
            if ((phy_ptr->job[i].msg_pt->header.target == target) && (phy_ptr->job[i].msg_pt->header.target_mode == target_mode))
            {
                // This job is targeting the dead target, remove it from the queue
                Phy_RmJob(phy_ptr, &phy_ptr->job[i]);
            }
        }
        i++;
        if (i >= MAX_MSG_NB)
        {
            i = 0;
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
    Phy_SetIrqState(false);
    phy_job_t *returned_job = &phy_ptr->job[phy_ptr->available_job_index++];
    if (phy_ptr->available_job_index >= MAX_MSG_NB)
    {
        phy_ptr->available_job_index = 0;
    }
    LUOS_ASSERT(phy_ptr->available_job_index != phy_ptr->oldest_job_index);
    Phy_SetIrqState(true);
    phy_ptr->job_nb++;
    // Copy the actual job data to the allocated job
    *returned_job = *phy_job;
    return returned_job;
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
    return &phy_ptr->job[phy_ptr->oldest_job_index];
}

/******************************************************************************
 * @brief Get the next job from the phy queue
 * @param phy_ptr Phy to get the job from
 * @return Job pointer
 ******************************************************************************/
_CRITICAL phy_job_t *Phy_GetNextJob(luos_phy_t *phy_ptr, phy_job_t *job)
{
    LUOS_ASSERT(phy_ptr != NULL);
    if (job == NULL)
    {
        return Phy_GetJob(phy_ptr);
    }
    if (phy_ptr->job_nb == 0)
    {
        return NULL;
    }
    int job_id = Phy_GetJobId(phy_ptr, job);
    do
    {
        job_id++;
        if (job_id >= MAX_MSG_NB)
        {
            job_id = 0;
        }
        if (job_id == phy_ptr->available_job_index)
        {
            return NULL;
        }
    } while (phy_ptr->job[job_id].msg_pt == NULL);
    return &phy_ptr->job[job_id];
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

/******************************************************************************
 * @brief Get the phy id from the phy pointer
 * @param phy_ptr Phy pointer
 * @return Phy id
 ******************************************************************************/
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

    // Clear this job values
    memset(job, 0, sizeof(phy_job_t));
    // Remove the job from the list
    phy_ptr->job_nb--;
    uint8_t id = Phy_GetJobId(phy_ptr, job);
    if (id == phy_ptr->oldest_job_index)
    {
        // We are removing the oldest job
        while (phy_ptr->oldest_job_index != phy_ptr->available_job_index)
        {
            phy_ptr->oldest_job_index++;
            if (phy_ptr->oldest_job_index >= MAX_MSG_NB)
            {
                phy_ptr->oldest_job_index = 0;
            }
            if (phy_ptr->job[phy_ptr->oldest_job_index].data_pt != NULL)
            {
                break;
            }
        }
    }
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
        LUOS_ASSERT(&phy_ctx.phy[i] != NULL);
        if (phy_ctx.phy[i].job_nb != 0)
        {
            return FAILED;
        }
    }
    return SUCCEED;
}
