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
#include "_routing_table.h"
#include "_luos_phy.h"
#include "stats.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
static int LuosIO_StartTopologyDetection(service_t *service);
static int LuosIO_DetectNextNodes(service_t *service);
static error_return_t LuosIO_ConsumeMsg(const msg_t *input);
static void LuosIO_TransmitLocalRoutingTable(service_t *service, msg_t *routeTB_msg);

// Phy_callbacks
static void LuosIO_MsgHandler(luos_phy_t *phy_ptr, phy_job_t *job);
error_return_t LuosIO_RunTopo(luos_phy_t *phy_ptr, uint8_t *portId);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint16_t last_node        = 0;
connection_t *connection_table_ptr = NULL;
luos_phy_t *luos_phy;
service_filter_t service_filter[MAX_MSG_NB]; // Service filter table. Each of these filter will be linked with jobs.
uint8_t service_filter_index = 0;            // Index of the next service filter to use.
service_t *detection_service = NULL;
bool Flag_DetectServices     = false;

/*******************************************************************************
 * Functions
 ******************************************************************************/

error_return_t LuosIO_RunTopo(luos_phy_t *phy_ptr, uint8_t *portId)
{
    // This function should not be called because Luos can't run a topology.
    LUOS_ASSERT(0);
    return FAILED;
}

void LuosIO_Reset(luos_phy_t *phy_ptr)
{
    MsgAlloc_Init(NULL);
    Node_Init();
    Node_SetState(EXTERNAL_DETECTION);
    Service_ClearId();
    // Reset the data reception context
    Luos_ReceiveData(NULL, NULL, NULL);
}

/******************************************************************************
 * @brief Init the interface file.
 * @param None
 * @return None
 ******************************************************************************/
void LuosIO_Init(void)
{
    // Init filter
    Phy_FiltersInit();
    Filter_TopicInit();
    Stats_Init();
    memory_stats_t *memory_stats = Stats_GetMemory();
    // Clear message allocation buffer table
    MsgAlloc_Init(memory_stats);

    // Init Phy
    Phy_Init();

    // Get the Luos phy struct, Luos always use the first phy
    luos_phy = Phy_Get(0, LuosIO_MsgHandler, LuosIO_RunTopo, LuosIO_Reset);
    // Set the irq management function
    Phy_SetIrqStateFunciton(LuosHAL_SetIrqState);

    // Detection init
    Flag_DetectServices = false;
}

/******************************************************************************
 * @brief Loop of the IO level
 * @param None
 * @return None
 ******************************************************************************/
void LuosIO_Loop(void)
{
    static bool detec_state_machine = false;
    // Execute message allocation tasks
    MsgAlloc_Loop();
    Phy_Loop();
    if (Flag_DetectServices == true)
    {
        LUOS_ASSERT(detection_service != NULL);
        Flag_DetectServices = false;
        // Set the detection launcher id to 1
        detection_service->id = 1;
        // Generate the filters
        Service_GenerateId(1);
        // Turn on the infinite loop to perform the detection
        detec_state_machine = true;
    }
    if (detec_state_machine == true)
    {
        // Check if the detection is finished
        if (RoutingTB_DetectServices(detection_service))
        {
            // Turn off the infinite loop
            detec_state_machine = false;
            detection_service   = NULL;
        }
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
int LuosIO_TopologyDetection(service_t *service, connection_t *connection_table)
{
    static uint8_t redetect_nb          = 0;
    static uint8_t detect_state_machine = 0;
    int result;

    switch (detect_state_machine)
    {
        case 0:
            connection_table_ptr = connection_table;
            // If a detection is in progress, don't do an another detection and return 0
            if (Node_GetState() >= LOCAL_DETECTION)
            {
                connection_table_ptr = NULL;
                return -1;
            }
            detect_state_machine++;
        case 1:
            // Reset all detection state of services on the network to start a new detection
            result = LuosIO_StartTopologyDetection(service);
            if (result <= 0)
            {
                return result;
            }
            detect_state_machine++;
        case 2:
            // Make sure that the detection is not interrupted
            if (Node_GetState() == EXTERNAL_DETECTION)
            {
                connection_table_ptr = NULL;
                return -1;
            }
            // Setup local node
            Node_Get()->node_id = 1;
            // Add this node id in the Luos phy filter allowing us to receive node messages
            memset(luos_phy->nodes, 0, sizeof(luos_phy->nodes));
            Phy_IndexSet(luos_phy->nodes, 1);
            last_node = 1;
            // Setup sending service id
            service->id = 1;
            // Consider this node as ready
            // Clear the nodeID waiting flag
            Node_WaitId();
            detect_state_machine++;
        case 3:
            // results:
            //  -1 : detection failed
            //  0 : detection in progress
            //  1 : detection finished
            result = LuosIO_DetectNextNodes(service);
            if (result == -1)
            {
                // Check the number of retry we made
                LUOS_ASSERT((redetect_nb <= 4));
                // Detection fail, restart it
                redetect_nb++;
                // Go back to stage 1
                detect_state_machine = 1;
                return 0;
            }
            if (result == 0)
            {
                // Detection in progress
                return 0;
            }
            // We finished the topology detection
            detect_state_machine = 0;
            redetect_nb          = 0;
            break;
        default:
            LUOS_ASSERT(0);
            break;
    }
    connection_table_ptr = NULL;
    return last_node;
}

/******************************************************************************
 * @brief Initiate a topology detection by reseting all service port states as a master node.
 * @param service pointer to the detecting service
 * @return The number of detected node.
 ******************************************************************************/
static int LuosIO_StartTopologyDetection(service_t *service)
{
    static uint8_t detect_state_machine = 0;
    static uint32_t start_tick;
    msg_t msg;

    switch (detect_state_machine)
    {
        case 0:
            // If a detection is in progress, don't do an another detection and return 0
            if (Node_GetState() >= LOCAL_DETECTION)
            {
                return -1;
            }
            // Load the message to send
            msg.header.target      = BROADCAST_VAL;
            msg.header.target_mode = BROADCAST;
            msg.header.cmd         = START_DETECTION;
            msg.header.size        = 0;
            Luos_SendMsg(service, &msg);
            detect_state_machine++;
        case 1:
            // Wait until the message is transmitted
            if (Phy_TxAllComplete() != SUCCEED)
            {
                return 0;
            }
            detect_state_machine++;
        case 2:
            // Reinit Phy (this will call LuosIO_Reset)
            Phy_ResetAll();
            // Wait 2ms to be sure all previous messages are received and treated by nodes
            start_tick = LuosHAL_GetSystick();
            detect_state_machine++;
        case 3:
            // Wait 2ms  for all nodes to manage the message
            if (LuosHAL_GetSystick() - start_tick < 2)
            {
                return 0;
            }
            detect_state_machine++;
        case 4:
            // Resend the message just to be sure that no other messages were revceived during the reset.
            msg.header.target      = BROADCAST_VAL;
            msg.header.target_mode = BROADCAST;
            msg.header.cmd         = START_DETECTION;
            msg.header.size        = 0;
            Luos_SendMsg(service, &msg);
            detect_state_machine++;
        case 5:
            // Wait until the message is transmitted
            if (Phy_TxAllComplete() != SUCCEED)
            {
                return 0;
            }
            start_tick = LuosHAL_GetSystick();
            detect_state_machine++;
        case 6:
            // Wait 10ms to be sure all previous messages are received and treated by all the nodes
            if (LuosHAL_GetSystick() - start_tick < 10)
            {
                return 0;
            }

            // Reinit our node id
            Node_Get()->node_id = DEFAULTID;
            memset(luos_phy->nodes, 0, sizeof(luos_phy->nodes));
            Node_SetState(LOCAL_DETECTION);
            detect_state_machine = 0;
            return 1;
        default:
            LUOS_ASSERT(0);
            break;
    }
    return -1;
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
    time_luos_t time;
    service_t *service               = Service_GetConcerned(&input->header);
    dead_target_t *dead_target       = (dead_target_t *)input->data;
    uint16_t base_id                 = 0;
    routing_table_t *route_tab       = &RoutingTB_Get()[RoutingTB_GetLastEntry()];
    static luos_phy_t *phy_index_ptr = NULL;

    switch (input->header.cmd)
    {
        //**************************************** detection section ****************************************
        // Only the master node should receive this message
        case CONNECTION_DATA:
            LUOS_ASSERT(connection_table_ptr != NULL);
            if (input->header.size == sizeof(port_t))
            {
                // This is a partial connection information (only the output part)
                // Save it we will receive the input part later
                memcpy(&connection_table_ptr[last_node++].parent, input->data, sizeof(port_t));
                // Now send the new generated node_id
                output_msg.header.cmd         = NODE_ID;
                output_msg.header.size        = sizeof(uint16_t);
                output_msg.header.target      = 0; // We target the node_id 0 because the node receiving this message don't have a node_id yet. This node need to be the only one to receive it.
                output_msg.header.target_mode = NODEIDACK;
                memcpy(output_msg.data, (void *)&last_node, sizeof(uint16_t));
                Luos_SendMsg(service, &output_msg);
            }
            else
            {
                // We receive this because a node port have a static mapping of it's connectivity, so we have to save it and consider it as detected nodes.
                // Check that we receive a full connection information
                LUOS_ASSERT(input->header.size % sizeof(connection_t) == 0);
                memcpy(&connection_table_ptr[last_node], input->data, input->header.size);
                last_node += input->header.size / sizeof(connection_t);
                // Check that node id are continuous
                LUOS_ASSERT(connection_table_ptr[last_node - 1].child.node_id == last_node);
            }
            // This message have been consumed
            return SUCCEED;
            break;

        // Only the master node should receive this message
        case PORT_DATA:
            LUOS_ASSERT(connection_table_ptr != NULL);
            // This is the last part (input port) of a connection_ data
            // Check that we receive a full port information
            LUOS_ASSERT(input->header.size == sizeof(port_t)
                        && (connection_table_ptr[last_node - 1].parent.node_id != 0xFFFF));
            memcpy(&connection_table_ptr[last_node - 1].child, input->data, sizeof(port_t));
            // This message have been consumed
            return SUCCEED;
            break;

        case NODE_ID:
            LUOS_ASSERT(input->header.size == sizeof(uint16_t));
            // This is our new node id.
            if (Node_Get()->node_id != 0)
            {
                // We didn't received the start detection message
                // Reinit our node id
                Node_Get()->node_id = DEFAULTID;
                memset(luos_phy->nodes, 0, sizeof(luos_phy->nodes));
                // A phy have already been detected, so we can't reset everything
                // Just reset LuosIO and Phy jobs.
                LuosIO_Reset(luos_phy);
                // Reinit Phy
                Phy_Reset();
            }
            // Save our new node id
            // We have to do it this way because Node_Get()->node_id is a bitfield and input->data is not well aligned.
            uint16_t node_id;
            memcpy(&node_id, input->data, sizeof(uint16_t));
            Node_Get()->node_id = node_id;
            // Add this node id in the Luos phy filter allowing us to receive node messages
            memset(luos_phy->nodes, 0, sizeof(luos_phy->nodes));
            Phy_IndexSet(luos_phy->nodes, node_id);
            // Also add all node before our node_id in the philter of the source phy allowing next node to reach our parents.
            port_t *source_port        = Phy_GetTopologysource();
            luos_phy_t *source_phy_ptr = Phy_GetPhyFromId(source_port->phy_id);
            for (uint16_t id = 1; id < node_id; id++)
            {
                Phy_IndexSet(source_phy_ptr->nodes, id);
            }
            // Now we need to send back the input part of the connection data.
            port_t *input_port  = Phy_GetTopologysource();
            input_port->node_id = Node_Get()->node_id;

            output_msg.header.target_mode = NODEIDACK;
            output_msg.header.target      = 1;
            output_msg.header.cmd         = PORT_DATA;
            output_msg.header.size        = sizeof(port_t);
            memcpy(output_msg.data, input_port, sizeof(port_t));
            Luos_SendMsg(service, &output_msg);
            // This message can't be send directly to avoid dispatch re-entrance issue.
            // To be able to send this message then run the detection of the other nodes we need to make it later on the LuosIO_Loop, so we put a flag for it.
            Phy_TopologyNext();
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
                    // Generate local ID
                    RoutingTB_Erase();
                    memcpy(&base_id, &input->data[0], sizeof(uint16_t));
                    Service_GenerateId(base_id);
                case 0:
                    // Send back a local routing table
                    output_msg.header.cmd         = RTB;
                    output_msg.header.target_mode = NODEIDACK;
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
            // We are receiving a routing table
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

        case PHY_ID:

            // We receive a phy id. We have to save it, because we will need it to save the indexes in the good phy.
            base_id = 0;
            memcpy(&base_id, input->data, sizeof(uint8_t));
            phy_index_ptr = Phy_GetPhyFromId((uint8_t)base_id);
            // This message have been consumed

            return SUCCEED;
            break;

        case NODE_INDEXES:
            // We are receiving a node index table
            LUOS_ASSERT(phy_index_ptr != NULL);
            base_id = Luos_ReceiveData(service, input, (void *)phy_index_ptr->nodes);
            if (base_id > 0)
            {
                LUOS_ASSERT(base_id <= sizeof(phy_index_ptr->nodes)); // The MAX_NODE_NUMBER of the detecting node is bigger than the one we have
                // We finished to receive the phy indexes
                phy_index_ptr = NULL;
            }
            // This message have been consumed

            return SUCCEED;
            break;

        case SERVICE_INDEXES:

            // We are receiving a service index table
            LUOS_ASSERT(phy_index_ptr != NULL);
            base_id = Luos_ReceiveData(service, input, (void *)phy_index_ptr->services);
            if (base_id > 0)
            {
                LUOS_ASSERT(base_id <= sizeof(phy_index_ptr->services)); // The MAX_SERVICE_NUMBER of the detecting node is bigger than the one we have
                // We finished to receive the phy indexes
                phy_index_ptr = NULL;
            }
            // This message have been consumed

            return SUCCEED;
            break;

        case START_DETECTION:
            // Reset All phy
            Phy_ResetAllNeeded();
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
                // Get all services of this node and remove them from the indexes
                search_result_t result;
                RTFilter_Node(RTFilter_Reset(&result), dead_target->node_id);
                for (size_t i = 0; i < result.result_nbr; i++)
                {
                    Phy_ServiceIndexRm(result.result_table[i]->id);
                }
                // remove the node from the indexes
                Phy_NodeIndexRm(dead_target->node_id);
                // remove the node services from the routing table
                RoutingTB_RemoveNode(dead_target->node_id);
            }
            if (dead_target->service_id != 0)
            {
                // Remove the service from the indexes
                Phy_ServiceIndexRm(dead_target->service_id);
                // Remove the service from the routing table
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
            Service_AddAutoUpdateTarget(service, input->header.source, (uint16_t)TimeOD_TimeTo_ms(time));
            return SUCCEED;
            break;
            //**************************************** bootloader section ****************************************

        case BOOTLOADER_RESET:
            LuosHAL_SetMode((uint8_t)BOOT_MODE);
            LuosHAL_Reboot();
            return SUCCEED;
            break;

#ifdef WITH_BOOTLOADER
        case BOOTLOADER_START:
            // We're in the app,
            // set bootloader mode, save node ID and reboot
            Luos_JumpToBootloader();
            return SUCCEED;
            break;
#endif
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
static int LuosIO_DetectNextNodes(service_t *service)
{
    static uint8_t detect_state_machine = 0;
    static uint32_t start_tick;

    switch (detect_state_machine)
    {
        case 0:
            // Lets try to find other nodes
            if (Phy_FindNextNode() != SUCCEED)
            {
                // No more nodes to detect
                return 1;
            }
            detect_state_machine++;
        case 1:
            // Wait the end of transmission
            if (Phy_TxAllComplete() == FAILED)
            {
                // Transmission is not finished
                return 0;
            }
            start_tick = LuosHAL_GetSystick();
            detect_state_machine++;
        case 2:
            // When phy loop will receive the reply it will store and manage the new node_id and send it to the next node.
            // We just have to wait the end of the treatment of the entire branch
            if (Phy_Busy())
            {
                if (LuosHAL_GetSystick() - start_tick > DETECTION_TIMEOUT_MS)
                {
                    // Topology detection is too long, we should abort it and restart
                    return -1;
                }
                return 0;
            }
            detect_state_machine = 0;
            return 1;
        default:
            LUOS_ASSERT(0);
            break;
    }
    return -1;
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
