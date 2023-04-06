/******************************************************************************
 * @file luos_io.c
 * @brief Interface file between Luos and a physical layer
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include <string.h>
#include "luos_io.h"
#include "transmission.h"
#include "reception.h"
#include "msg_alloc.h"
#include "robus.h"
#include "service.h"
#include "filter.h"
#include "node.h"
#include "luos_utils.h"
#include "luos_hal.h"
#include "luos_engine.h"

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
static error_return_t LuosIO_MsgHandler(msg_t *input);
static error_return_t LuosIO_DetectNextNodes(service_t *service);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint16_t last_node = 0;

/*******************************************************************************
 * Functions
 ******************************************************************************/

/******************************************************************************
 * @brief Init the interface file.
 * @param None
 * @return None
 ******************************************************************************/
void LuosIO_Init(memory_stats_t *memory_stats)
{
    // Init filter
    Filter_IdInit();
    Filter_TopicInit();

    // Clear message allocation buffer table
    MsgAlloc_Init(memory_stats);

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
    MsgAlloc_loop();
    // Interpreat received messages and create luos task for it.
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
    Robus_Loop();
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

        if (LuosIO_DetectNextNodes(service) == FAILED)
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

    msg.header.config      = BASE_PROTOCOL;
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
        while (MsgAlloc_TxAllComplete() != SUCCEED)
            ;
        // Reinit services id
        Service_ClearId();
        // Reinit msg alloc
        MsgAlloc_Init(NULL);
        // Wait 2ms to be sure all previous messages are received and treated by other nodes
        uint32_t start_tick = LuosHAL_GetSystick();
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
error_return_t LuosIO_MsgHandler(msg_t *input)
{
    msg_t output_msg;
    node_bootstrap_t node_bootstrap;
    service_t *service = Service_GetConcerned(&input->header);
    switch (input->header.cmd)
    {
        case WRITE_NODE_ID:
            // Depending on the size of the received data we have to do different things
            switch (input->header.size)
            {
                case 0:
                    // Someone asking us a new node id (we are the detecting service)
                    // Increase the number of node_nb and send it back
                    last_node++;
                    output_msg.header.config      = BASE_PROTOCOL;
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
                    output_msg.header.config      = BASE_PROTOCOL;
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
                        MsgAlloc_Init(NULL);
                    }
                    // This is a node bootstrap information.
                    memcpy((void *)&node_bootstrap.unmap[0], (void *)&input->data[0], sizeof(node_bootstrap_t));
                    Node_Get()->node_id = node_bootstrap.nodeid;
                    Robus_SaveNodeID(node_bootstrap.prev_nodeid);
                    // Continue the topology detection on our other ports.
                    LuosIO_DetectNextNodes(service);
                default:
                    break;
            }
            return SUCCEED;
            break;
        case START_DETECTION:
            return SUCCEED;
            break;
        case END_DETECTION:
            // Detect end of detection
            Node_SetState(DETECTION_OK);
            return FAILED;
            break;
        default:
            return FAILED;
            break;
    }
    return FAILED;
}

/******************************************************************************
 * @brief run the procedure allowing to detect the next nodes on the next physical layer port.
 * @param service pointer to the detecting service
 * @return None.
 ******************************************************************************/
static error_return_t LuosIO_DetectNextNodes(service_t *service)
{
    // Lets try to poke other nodes
    while (Robus_FindNeighbour() == SUCCEED)
    {
        // There is someone here
        // Clear spotted dead service detection
        service->dead_service_spotted = 0;
        // Ask an ID  to the detector service.
        msg_t msg;
        msg.header.config      = BASE_PROTOCOL;
        msg.header.target_mode = NODEIDACK;
        msg.header.target      = 1;
        msg.header.cmd         = WRITE_NODE_ID;
        msg.header.size        = 0;
        Luos_SendMsg(service, &msg);
        // Wait the end of transmission
        while (MsgAlloc_TxAllComplete() == FAILED)
            ;
        // Check if there is a failure on transmission
        if (service->dead_service_spotted != 0)
        {
            // Message transmission failure
            // Consider this port unconnected by sending a 0xFFFF node id meaning that this port is not connected
            Robus_SaveNodeID(0xFFFF);
            continue;
        }

        // when Robus loop will receive the reply it will store and manage the new node_id and send it to the next node.
        // We just have to wait the end of the treatment of the entire branch
        uint32_t start_tick = LuosHAL_GetSystick();
        while (Robus_Busy())
        {
            LuosIO_Loop();
            if (LuosHAL_GetSystick() - start_tick > DETECTION_TIMEOUT_MS)
            {
                // topology detection is too long, we should abort it and restart
                return FAILED;
            }
        }
    }
    return SUCCEED;
}
