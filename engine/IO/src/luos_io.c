/******************************************************************************
 * @file luos_io.c
 * @brief Interface file between Luos and phy. This is also managing all protocol messages.
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include <string.h>
#include "luos_io.h"
#include "msg_alloc.h"
#include "service.h"
#include "filter.h"
#include "luos_utils.h"
#include "luos_hal.h"
#include "luos_engine.h"
#include "bootloader_core.h"
#include "_luos_phy.h"
#include "stats.h"

#include "robus.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint16_t prev_nodeid;
            uint16_t nodeid;
        };
        uint8_t unmap[sizeof(uint16_t) * 2];
    };
} node_bootstrap_t;

static error_return_t LuosIO_StartTopologyDetection(service_t *service);
static error_return_t LuosIO_DetectNextNodes(service_t *service, bool wait_for_answer);
static void LuosIO_MsgHandler(luos_phy_t *phy_ptr, phy_job_t *job);
static error_return_t LuosIO_ConsumeMsg(const msg_t *input);
static void LuosIO_TransmitLocalRoutingTable(service_t *service, msg_t *routeTB_msg);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint16_t last_node = 0;
luos_phy_t *luos_phy;
service_filter_t service_filter[MAX_MSG_NB]; // Service filter table. Each of these filter will be linked with jobs.
uint8_t service_filter_index = 0;            // Index of the next service filter to use.
service_t *detection_service = NULL;
bool Flag_DetectServices     = false;

/*******************************************************************************
 * Functions
 ******************************************************************************/

/******************************************************************************
 * @brief Init the interface file.
 * @param None
 * @return None
 ******************************************************************************/
void LuosIO_Init(void)
{
    // Init filter
    Filter_IdInit();
    Filter_TopicInit();
    Stats_Init();
    memory_stats_t *memory_stats = Stats_GetMemory();
    // Clear message allocation buffer table
    MsgAlloc_Init(memory_stats);

    // Init Phy
    Phy_Init();

    // Get the Luos phy struct, Luos always use the first phy
    luos_phy = Phy_Get(0, LuosIO_MsgHandler);

    // Init Robus
    Robus_Init();
}

/******************************************************************************
 * @brief Loop of the IO level
 * @param None
 * @return None
 ******************************************************************************/
void LuosIO_Loop(void)
{
    // Execute message allocation tasks
    MsgAlloc_Loop();
    Phy_Loop();
    Robus_Loop();
    if (Flag_DetectServices == true)
    {
        LUOS_ASSERT(detection_service != NULL);
        Flag_DetectServices = false;
        // Set the detection launcher id to 1
        detection_service->id = 1;
        // Generate the filters
        Service_GenerateId(1);
        RoutingTB_DetectServices(detection_service);
        detection_service = NULL;
    }
}

/******************************************************************************
 * @brief manage a message reception
 * @param None
 * @return None
 ******************************************************************************/
static void LuosIO_MsgHandler(luos_phy_t *phy_ptr, phy_job_t *job)
{
    LUOS_ASSERT((phy_ptr == luos_phy)
                && (job >= luos_phy->job)
                && (job < &luos_phy->job[MAX_MSG_NB]));
    // Check if this message is a protocol one
    if (LuosIO_ConsumeMsg(job->msg_pt) == FAILED)
    {
        // If not create a service filter for this job.
        service_filter[service_filter_index] = Service_GetFilter(job->msg_pt);
        job->phy_data                        = &service_filter[service_filter_index];
        service_filter_index++;
        if (service_filter_index >= MAX_MSG_NB)
        {
            service_filter_index = 0;
        }
    }
    else
    {
        // This message was a protocol one, we can remove it.
        // When we receive a start detection we reset everything so we don't need to remove the message anymore.
        if (luos_phy->job_nb != 0)
        {
            Phy_RmJob(luos_phy, job);
        }
    }
}

/******************************************************************************
 * @brief Send a message to the network
 * @param service pointer to the service
 * @param msg pointer to the message
 * @return error return
 ******************************************************************************/
error_return_t LuosIO_Send(service_t *service, msg_t *msg)
{
    error_return_t error = SUCCEED;

    // ***************************************************
    // Don't send luos messages if the network is down
    // ***************************************************
    if ((msg->header.cmd >= LUOS_LAST_RESERVED_CMD) && (Node_GetState() != DETECTION_OK))
    {
        return PROHIBITED;
    }

    // Save message information in the Luosphy struct
    luos_phy->rx_buffer_base = (uint8_t *)msg;
    luos_phy->rx_data        = luos_phy->rx_buffer_base;
    // For now let just consider that we received the header allowing us to compute the message things based on it.
    luos_phy->received_data = sizeof(header_t);
    luos_phy->rx_keep       = true; // Tell phy that we want to keep this message
    Phy_ComputeHeader(luos_phy);
    LUOS_ASSERT(luos_phy->rx_keep == true);
    // The header computation give us how many byte we need to have.
    // Just update the received data to the full message size.
    luos_phy->received_data = luos_phy->rx_size;
    // Validate the message to allocate it and generate tasks.
    Phy_ValidMsg(luos_phy);
    if (luos_phy->rx_data == NULL)
    {
        // The message wasn't kept, there is no more space on the buffer, or the message is corrupted.
        // Return a failure to notify user.
        return FAILED;
    }
    // Execute phy loop to dispatch the allocated message.
    Phy_Loop();
    return error;
}

/******************************************************************************
 * @brief Run a topology detection procedure as a master node.
 * @param service pointer to the detecting service
 * @return The number of detected node.
 ******************************************************************************/
uint16_t LuosIO_TopologyDetection(service_t *service)
{
    uint8_t redetect_nb = 0;
    bool detect_enabled = true;

    // If a detection is in progress, don't do an another detection and return 0
    if (Node_GetState() >= LOCAL_DETECTION)
    {
        return 0;
    }
    while (detect_enabled)
    {
        detect_enabled = false;

        // Reset all detection state of services on the network to start a new detection
        LuosIO_StartTopologyDetection(service);
        // Make sure that the detection is not interrupted
        if (Node_GetState() == EXTERNAL_DETECTION)
        {
            return 0;
        }
        // Setup local node
        Node_Get()->node_id = 1;
        last_node           = 1;
        // Setup sending service id
        service->id = 1;

        if (LuosIO_DetectNextNodes(service, true) == FAILED)
        {
            // check the number of retry we made
            LUOS_ASSERT((redetect_nb <= 4));
            // Detection fail, restart it
            redetect_nb++;
            detect_enabled = true;
        }
    }
    return last_node;
}

/******************************************************************************
 * @brief Initiate a topology detection by reseting all service port states as a master node.
 * @param service pointer to the detecting service
 * @return The number of detected node.
 ******************************************************************************/
static error_return_t LuosIO_StartTopologyDetection(service_t *service)
{
    msg_t msg;
    uint8_t try_nbr = 0;

    msg.header.target      = BROADCAST_VAL;
    msg.header.target_mode = BROADCAST;
    msg.header.cmd         = START_DETECTION;
    msg.header.size        = 0;
    do
    {
        // If a detection is in progress, don't do an another detection and return 0
        if (Node_GetState() >= LOCAL_DETECTION)
        {
            return 0;
        }
        // Load the message to send
        Luos_SendMsg(service, &msg);
        // Wait until message is actually transmitted
        while (Phy_TxAllComplete() != SUCCEED)
            ;
        // Reinit services id
        Service_ClearId();
        // Reinit msg alloc
        MsgAlloc_Init(NULL);
        // Reinit service filter
        Filter_IdInit();
        // Reinit Phy
        Phy_Reset();
        // Wait 2ms to be sure all previous messages are received and treated by other nodes
        uint32_t start_tick = LuosHAL_GetSystick();
        while (LuosHAL_GetSystick() - start_tick < 2)
            ;
        // Resend the message just to be sure that no other messages were revceived during the reset.
        Luos_SendMsg(service, &msg);
        // Wait until message is actually transmitted
        while (Phy_TxAllComplete() != SUCCEED)
            ;
        // Wait 2ms to be sure all previous messages are received and treated by other nodes
        start_tick = LuosHAL_GetSystick();
        while (LuosHAL_GetSystick() - start_tick < 2)
            ;
        try_nbr++;

    } while ((MsgAlloc_IsEmpty() != SUCCEED) || (try_nbr > 5));
    // Reinit our node id
    Node_Get()->node_id = 0;
    // Clear any saved node id on physical layer ports
    Robus_ResetNodeID();
    if (try_nbr < 5)
    {
        Node_SetState(LOCAL_DETECTION);
        return SUCCEED;
    }
    return FAILED;
}

/******************************************************************************
 * @brief check if received messages are protocols one and manage it if it is.
 * @param msg pointer to the reeived message
 * @return error_return_t SUCCEED if the message have been consumed.
 ******************************************************************************/
error_return_t LuosIO_ConsumeMsg(const msg_t *input)
{
    LUOS_ASSERT(input != NULL);
    msg_t output_msg;
    node_bootstrap_t node_bootstrap;
    time_luos_t time;
    service_t *service         = Service_GetConcerned(&input->header);
    dead_target_t *dead_target = (dead_target_t *)input->data;
    uint16_t base_id           = 0;
    routing_table_t *route_tab = &RoutingTB_Get()[RoutingTB_GetLastEntry()];

    switch (input->header.cmd)
    {
        //**************************************** detection section ****************************************
        case WRITE_NODE_ID:
            // Depending on the size of the received data we have to do different things
            switch (input->header.size)
            {
                case 0:
                    // Someone asking us a new node id (we are the detecting service)
                    // Increase the number of node_nb and send it back
                    last_node++;
                    output_msg.header.cmd         = WRITE_NODE_ID;
                    output_msg.header.size        = sizeof(uint16_t);
                    output_msg.header.target      = input->header.source;
                    output_msg.header.target_mode = NODEIDACK;
                    memcpy(output_msg.data, (void *)&last_node, sizeof(uint16_t));
                    Luos_SendMsg(service, &output_msg);
                    break;
                case 2:
                    // This is a node id for the next node.
                    // This is a reply to our request to generate the next node id.
                    // This node_id is the one after the currently poked branch.
                    // Extract the node id from the received data
                    memcpy((void *)&node_bootstrap.nodeid, (void *)&input->data[0], sizeof(uint16_t));
                    // We need to save this node ID as a connection to a port
                    Robus_SaveNodeID(node_bootstrap.nodeid);
                    // Now we can send it to the next node
                    node_bootstrap.prev_nodeid    = Node_Get()->node_id;
                    output_msg.header.cmd         = WRITE_NODE_ID;
                    output_msg.header.size        = sizeof(node_bootstrap_t);
                    output_msg.header.target      = 0;
                    output_msg.header.target_mode = NODEIDACK;
                    memcpy((void *)&output_msg.data[0], (void *)&node_bootstrap.unmap[0], sizeof(node_bootstrap_t));
                    Luos_SendMsg(service, &output_msg);
                    break;
                case sizeof(node_bootstrap_t):
                    if (Node_Get()->node_id != 0)
                    {
                        Node_Get()->node_id = 0;
                        // Reinit service id
                        Service_ClearId();
                        // Reinit msg alloc
                        MsgAlloc_Init(NULL);
                        // Reinit service filter
                        Filter_IdInit();
                        // Reinit Phy
                        Phy_Reset();
                    }
                    // This is a node bootstrap information.
                    memcpy((void *)&node_bootstrap.unmap[0], (void *)&input->data[0], sizeof(node_bootstrap_t));
                    Node_Get()->node_id = node_bootstrap.nodeid;
                    Robus_SaveNodeID(node_bootstrap.prev_nodeid);
                    // Continue the topology detection on our other ports.
                    LuosIO_DetectNextNodes(service, false);
                    break;
                default:
                    LUOS_ASSERT(0);
                    break;
            }
            // This message have been consumed
            return SUCCEED;
            break;

        case LOCAL_RTB:
            // Depending on the size of this message we have to make different operations
            // If size is 0 someone ask to get local_route table back
            // If size is 2 someone ask us to generate a local route table based on the given service ID then send local route table back.
            switch (input->header.size)
            {
                case 2:
                    // generate local ID
                    RoutingTB_Erase();
                    memcpy(&base_id, &input->data[0], sizeof(uint16_t));
                    Service_GenerateId(base_id);
                case 0:
                    // send back a local routing table
                    output_msg.header.cmd         = RTB;
                    output_msg.header.target_mode = SERVICEIDACK;
                    output_msg.header.target      = input->header.source;
                    LuosIO_TransmitLocalRoutingTable(0, &output_msg);
                    break;
                default:
                    LUOS_ASSERT(0);
                    break;
            }
            return SUCCEED;
            break;

        case RTB:
            // We are receiving a rouiting table
            // Check routing table overflow
            LUOS_ASSERT(((uintptr_t)route_tab + input->header.size) <= ((uintptr_t)RoutingTB_Get() + (sizeof(routing_table_t) * MAX_RTB_ENTRY)));
            if (Luos_ReceiveData(service, input, (void *)route_tab) > 0)
            {
                // route table section reception complete
                RoutingTB_ComputeRoutingTableEntryNB();
                Luos_ResetStatistic();
            }
            return SUCCEED;
            break;

        case START_DETECTION:
            Phy_Reset();
            MsgAlloc_Init(NULL);
            Node_Init();
            Node_SetState(EXTERNAL_DETECTION);
            Service_ClearId();
            // Reset the data reception context
            Luos_ReceiveData(NULL, NULL, NULL);
            Robus_ResetNodeID();
            // This message have been consumed
            return SUCCEED;
            break;

        case END_DETECTION:
            // Detect end of detection
            Node_SetState(DETECTION_OK);
            return FAILED;
            break;

        case ASK_DETECTION:
            if (Node_GetState() < LOCAL_DETECTION)
            {
                detection_service   = service;
                Flag_DetectServices = true;
            }
            return SUCCEED;
            break;

        //**************************************** failure section ****************************************
        case ASSERT:
            // A service assert remove all services of the asserted node in routing table
            RoutingTB_RemoveNode(input->header.source);
            // This assert information could be usefull for services, do not remove it.
            return FAILED;
            break;

        case DEADTARGET:
            if (dead_target->node_id != 0)
            {
                RoutingTB_RemoveNode(dead_target->node_id);
            }
            if (dead_target->service_id != 0)
            {
                RoutingTB_RemoveService(dead_target->service_id);
            }
            // This assert information could be usefull for services, do not remove it.
            return FAILED;
            break;

            //**************************************** info section **********************************************

        case REVISION:
            if (input->header.size == 0)
            {
                output_msg.header.cmd         = REVISION;
                output_msg.header.target_mode = SERVICEID;
                output_msg.header.size        = sizeof(revision_t);
                output_msg.header.target      = input->header.source;
                memcpy(output_msg.data, service->revision.unmap, sizeof(revision_t));
                Luos_SendMsg(service, &output_msg);
                return SUCCEED;
            }
            break;

        case LUOS_REVISION:
            if (input->header.size == 0)
            {
                const revision_t *luos_version = Luos_GetVersion();
                output_msg.header.cmd          = LUOS_REVISION;
                output_msg.header.target_mode  = SERVICEID;
                output_msg.header.size         = sizeof(revision_t);
                output_msg.header.target       = input->header.source;
                memcpy(output_msg.data, &luos_version->unmap, sizeof(revision_t));
                Luos_SendMsg(service, &output_msg);
                return SUCCEED;
            }
            break;

        case LUOS_STATISTICS:
            if (input->header.size == 0)
            {
                output_msg.header.cmd          = LUOS_STATISTICS;
                output_msg.header.target_mode  = SERVICEID;
                output_msg.header.size         = sizeof(general_stats_t);
                output_msg.header.target       = input->header.source;
                general_stats_t *general_stats = Stats_Get();
                memcpy(&general_stats->service_stat, service->statistics.unmap, sizeof(service_stats_t));
                memcpy(output_msg.data, &general_stats->unmap, sizeof(general_stats_t));
                Luos_SendMsg(service, &output_msg);
                return SUCCEED;
            }
            break;
            //**************************************** service section ****************************************

        case WRITE_ALIAS:
            // Save this alias into the service
            Luos_UpdateAlias(service, (const char *)input->data, input->header.size);
            // Send this message to user
            return FAILED;
            break;

        case UPDATE_PUB:
            // This service need to be auto updated
            TimeOD_TimeFromMsg(&time, input);
            service->auto_refresh.target      = input->header.source;
            service->auto_refresh.time_ms     = (uint16_t)TimeOD_TimeTo_ms(time);
            service->auto_refresh.last_update = LuosHAL_GetSystick();
            return SUCCEED;
            break;
        //**************************************** bootloader section ****************************************
        case BOOTLOADER_CMD:
            // send data to the bootloader
            LuosBootloader_MsgHandler(input);
            return SUCCEED;
            break;

        default:
            return FAILED;
            break;
    }
    return FAILED;
}

/******************************************************************************
 * @brief Transmit local RTB to network
 * @param service
 * @param routeTB_msg : Loca RTB message to transmit
 * @return none
 ******************************************************************************/
static inline void LuosIO_TransmitLocalRoutingTable(service_t *service, msg_t *routeTB_msg)
{
    LUOS_ASSERT(routeTB_msg != NULL);
    uint16_t entry_nb = 0;
    routing_table_t local_routing_table[Service_GetNumber() + 1];

    // start by saving node entry
    RoutingTB_ConvertNodeToRoutingTable(&local_routing_table[entry_nb], Node_Get());
    entry_nb++;
    // save services entry
    for (uint16_t i = 0; i < Service_GetNumber(); i++)
    {
        RoutingTB_ConvertServiceToRoutingTable((routing_table_t *)&local_routing_table[entry_nb++], &Service_GetTable()[i]);
    }
    Luos_SendData(service, routeTB_msg, (void *)local_routing_table, (entry_nb * sizeof(routing_table_t)));
}

/******************************************************************************
 * @brief run the procedure allowing to detect the next nodes on the next physical layer port.
 * @param service pointer to the detecting service
 * @return None.
 ******************************************************************************/
static error_return_t LuosIO_DetectNextNodes(service_t *service, bool wait_for_answer)
{
    // Lets try to poke other nodes
    while (Robus_FindNeighbour() == SUCCEED)
    {
        // There is someone here
        // Ask an ID  to the detector service.
        msg_t msg;
        msg.header.config      = BASE_PROTOCOL;
        msg.header.target_mode = NODEIDACK;
        msg.header.target      = 1;
        msg.header.cmd         = WRITE_NODE_ID;
        msg.header.size        = 0;
        Luos_SendMsg(service, &msg);
        if (wait_for_answer == false)
        {
            return SUCCEED;
        }
        // Wait the end of transmission
        while (Phy_TxAllComplete() == FAILED)
            ;
        // When Robus loop will receive the reply it will store and manage the new node_id and send it to the next node.
        // We just have to wait the end of the treatment of the entire branch
        uint32_t start_tick = LuosHAL_GetSystick();
        while (Robus_Busy())
        {
            LuosIO_Loop();
            if (LuosHAL_GetSystick() - start_tick > DETECTION_TIMEOUT_MS)
            {
                // Topology detection is too long, we should abort it and restart
                return FAILED;
            }
        }
    }
    return SUCCEED;
}

/******************************************************************************
 * @brief try to get a job from the job list.
 * @param job_id the id of the job to get
 * @param job pointer to the job pointer
 * @return SUCEED if the job is available, FAILED if not.
 ******************************************************************************/
error_return_t LuosIO_GetNextJob(phy_job_t **job)
{
    LUOS_ASSERT(job != NULL);
    MSGALLOC_MUTEX_LOCK
    phy_job_t *the_job = Phy_GetNextJob(luos_phy, *job);
    *job               = the_job;
    if (the_job != NULL)
    {
        MSGALLOC_MUTEX_UNLOCK
        return SUCCEED;
    }
    MSGALLOC_MUTEX_UNLOCK
    return FAILED;
}

/******************************************************************************
 * @brief remove a job from the job list.
 * @param job pointer to the job to remove
 * @return None.
 ******************************************************************************/
void LuosIO_RmJob(phy_job_t *job)
{
    LUOS_ASSERT((job >= &luos_phy->job[0])
                && (job < &luos_phy->job[MAX_MSG_NB])
                && (job->phy_data != NULL));
    // Be sure every service has finished to use this job
    if (*(service_filter_t *)job->phy_data != 0)
    {
        // This job is still used by a service, we can't remove it now
        return;
    }
    Phy_RmJob(luos_phy, job);
}

/******************************************************************************
 * @brief get the number of job in the job list.
 * @param None.
 * @return the number of job in the job list.
 ******************************************************************************/
uint16_t LuosIO_GetJobNb(void)
{
    return luos_phy->job_nb;
}

/******************************************************************************
 * @brief define is theire is something waiting to be sent or not
 * @return Succeed if nothing is waiting to be sent
 ******************************************************************************/
error_return_t LuosIO_TxAllComplete(void)
{
    return Phy_TxAllComplete();
}
