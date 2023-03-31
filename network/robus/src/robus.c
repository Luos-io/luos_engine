/******************************************************************************
 * @file robus
 * @brief User functionalities of the robus communication protocol
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <robus.h>

#include <string.h>
#include <stdbool.h>
#include "transmission.h"
#include "reception.h"
#include "port_manager.h"
#include "context.h"
#include "robus_hal.h"
#include "luos_hal.h"
#include "msg_alloc.h"
#include "luos_utils.h"
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

static error_return_t Robus_MsgHandler(msg_t *input);
static error_return_t Robus_DetectNextNodes(ll_service_t *ll_service);
static error_return_t Robus_ResetNetworkDetection(ll_service_t *ll_service);
/*******************************************************************************
 * Variables
 ******************************************************************************/
// Creation of the robus context. This variable is used in all files of this lib.
volatile context_t ctx;
uint32_t baudrate; /*!< System current baudrate. */
volatile uint16_t last_node = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Initialisation of the Robus communication protocole
 * @param None
 * @return None
 ******************************************************************************/
void Robus_Init(memory_stats_t *memory_stats)
{
    // Init the number of created  virtual service.
    ctx.ll_service_number = 0;
    Node_Init();
    // no transmission lock
    ctx.tx.lock = false;
    // Init collision state
    ctx.tx.collision = false;
    // Init Tx status
    ctx.tx.status = TX_DISABLE;
    // Save luos baudrate
    baudrate = DEFAULTBAUDRATE;
    // mask
    Robus_MaskInit();

    // multicast mask init
    for (uint16_t i = 0; i < TOPIC_MASK_SIZE; i++)
    {
        ctx.TopicMask[i] = 0;
    }

    // Init reception
    Recep_Init();

    // Clear message allocation buffer table
    MsgAlloc_Init(memory_stats);

    // Init hal
    RobusHAL_Init();

    // init detection structure
    PortMng_Init();

    // Initialize the robus service status
    ctx.rx.status.unmap      = 0;
    ctx.rx.status.identifier = 0xF;

    Node_SetState(NO_DETECTION);
}
/******************************************************************************
 * @brief Reset Masks
 * @param None
 * @return None
 ******************************************************************************/
void Robus_MaskInit(void)
{
    ctx.IDShiftMask = 0;
    for (uint16_t i = 0; i < ID_MASK_SIZE; i++)
    {
        ctx.IDMask[i] = 0;
    }
}
/******************************************************************************
 * @brief Loop of the Robus communication protocole
 * @param None
 * @return None
 ******************************************************************************/
void Robus_Loop(void)
{
    // Network timeout management
    Node_Loop();
    // Execute message allocation tasks
    MsgAlloc_loop();
    // Interpreat received messages and create luos task for it.
    msg_t *msg = NULL;
    while (MsgAlloc_PullMsgToInterpret(&msg) == SUCCEED)
    {
        // Check if this message is a protocol one
        if (Robus_MsgHandler(msg) == FAILED)
        {
            // If not create luos tasks.
            Recep_InterpretMsgProtocol(msg);
        }
    }
    RobusHAL_Loop();
}
/******************************************************************************
 * @brief create a service add in local route table
 * @param type of service create
 * @return None
 ******************************************************************************/
ll_service_t *Robus_ServiceCreate(uint16_t type)
{
    // Set the service type
    ctx.ll_service_table[ctx.ll_service_number].type = type;
    // Initialise the service id, TODO the ID could be stored in EEprom, the default ID could be set in factory...
    ctx.ll_service_table[ctx.ll_service_number].id = DEFAULTID;
    // Initialize dead service detection
    ctx.ll_service_table[ctx.ll_service_number].dead_service_spotted = 0;
    // Clear stats
    ctx.ll_service_table[ctx.ll_service_number].ll_stat.max_retry = 0;
    // Clear topic number
    ctx.ll_service_table[ctx.ll_service_number].last_topic_position = 0;
    for (uint16_t i = 0; i < LAST_TOPIC; i++)
    {
        ctx.ll_service_table[ctx.ll_service_number].topic_list[i] = 0;
    }
    // Return the freshly initialized ll_service pointer.
    return (ll_service_t *)&ctx.ll_service_table[ctx.ll_service_number++];
}
/******************************************************************************
 * @brief clear service list in route table
 * @param None
 * @return None
 ******************************************************************************/
void Robus_ServicesClear(void)
{
    // Clear ll_service table
    memset((void *)ctx.ll_service_table, 0, sizeof(ll_service_t) * MAX_SERVICE_NUMBER);
    // Reset the number of created services
    ctx.ll_service_number = 0;
}
/******************************************************************************
 * @brief Formalize message Set tx task and send
 * @param service to send
 * @param msg to send
 * @return error_return_t
 ******************************************************************************/
error_return_t Robus_SetTxTask(ll_service_t *ll_service, msg_t *msg)
{
    error_return_t error = SUCCEED;
    uint8_t ack          = 0;
    uint16_t data_size   = 0;
    uint16_t crc_val     = 0xFFFF;
    // ***************************************************
    // don't send luos messages if network is down
    // ***************************************************
    if ((msg->header.cmd >= LUOS_LAST_RESERVED_CMD) && (Node_GetState() != DETECTION_OK))
    {
        return PROHIBITED;
    }

    // Compute the full message size based on the header size info.
    if (msg->header.size > MAX_DATA_MSG_SIZE)
    {
        data_size = MAX_DATA_MSG_SIZE;
    }
    else
    {
        data_size = msg->header.size;
    }
    // Add the CRC to the total size of the message
    uint16_t full_size = sizeof(header_t) + data_size + CRC_SIZE;

    uint16_t crc_max_index = full_size;

    if (Luos_IsMsgTimstamped(msg) == true)
    {
        full_size += sizeof(time_luos_t);
    }
    // Compute the CRC
    crc_val = ll_crc_compute(&msg->stream[0], crc_max_index - CRC_SIZE, 0xFFFF);

    // Check the localhost situation
    luos_localhost_t localhost = Recep_NodeConcerned(&msg->header);
    // Check if ACK needed
    if (((msg->header.target_mode == SERVICEIDACK) || (msg->header.target_mode == NODEIDACK)) && ((localhost && (msg->header.target != DEFAULTID))))
    {
        // This is a localhost message and we need to transmit a ack. Add it at the end of the data to transmit
        ack = ctx.rx.status.unmap;
        full_size++;
    }

    // ********** Allocate the message ********************
    if (MsgAlloc_SetTxTask(ll_service, (uint8_t *)msg->stream, crc_val, full_size, localhost, ack) == FAILED)
    {
        error = FAILED;
    }
// **********Try to send the message********************
#ifndef VERBOSE_LOCALHOST
    if (localhost != LOCALHOST)
    {
#endif
        Transmit_Process();
#ifndef VERBOSE_LOCALHOST
    }
#endif
    return error;
}
/******************************************************************************
 * @brief Send Msg to a service
 * @param service to send
 * @param msg to send
 * @return none
 ******************************************************************************/
error_return_t Robus_SendMsg(ll_service_t *ll_service, msg_t *msg)
{
    // ********** Prepare the message ********************
    if (ll_service->id != 0)
    {
        msg->header.source = ll_service->id;
    }
    else
    {
        msg->header.source = Node_Get()->node_id;
    }
    if (Robus_SetTxTask(ll_service, msg) == FAILED)
    {
        return FAILED;
    }
    return SUCCEED;
}
/******************************************************************************
 * @brief Start a topology detection procedure
 * @param ll_service pointer to the detecting ll_service
 * @return The number of detected node.
 ******************************************************************************/
uint16_t Robus_TopologyDetection(ll_service_t *ll_service)
{
    uint8_t redetect_nb = 0;
    bool detect_enabled = true;

    // if a detection is in progress,
    // Don't do an another detection and return 0
    if (Node_GetState() >= LOCAL_DETECTION)
    {
        return 0;
    }

    while (detect_enabled)
    {
        detect_enabled = false;

        // Reset all detection state of services on the network
        Robus_ResetNetworkDetection(ll_service);
        // Make sure that the detection is not interrupted
        if (Node_GetState() == EXTERNAL_DETECTION)
        {
            return 0;
        }
        // setup local node
        Node_Get()->node_id = 1;
        last_node           = 1;
        // setup sending ll_service
        ll_service->id = 1;

        if (Robus_DetectNextNodes(ll_service) == FAILED)
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
 * @brief reset all service port states
 * @param ll_service pointer to the detecting ll_service
 * @return The number of detected node.
 ******************************************************************************/
static error_return_t Robus_ResetNetworkDetection(ll_service_t *ll_service)
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
        // if a detection is in progress,
        // Don't do an another detection and return 0
        if (Node_GetState() >= LOCAL_DETECTION)
        {
            return 0;
        }
        // msg send not blocking
        Robus_SendMsg(ll_service, &msg);
        // need to wait until tx msg before clear msg alloc
        while (MsgAlloc_TxAllComplete() != SUCCEED)
            ;

        MsgAlloc_Init(NULL);

        // wait for some 2ms to be sure all previous messages are received and treated
        uint32_t start_tick = LuosHAL_GetSystick();
        while (LuosHAL_GetSystick() - start_tick < 2)
            ;
        try_nbr++;
    } while ((MsgAlloc_IsEmpty() != SUCCEED) || (try_nbr > 5));

    Node_Get()->node_id = 0;
    PortMng_Init();
    if (try_nbr < 5)
    {
        Node_SetState(LOCAL_DETECTION);
        return SUCCEED;
    }

    return FAILED;
}
/******************************************************************************
 * @brief run the procedure allowing to detect the next nodes on the next port
 * @param ll_service pointer to the detecting ll_service
 * @return None.
 ******************************************************************************/
static error_return_t Robus_DetectNextNodes(ll_service_t *ll_service)
{
    // Lets try to poke other nodes
    while (PortMng_PokeNextPort() == SUCCEED)
    {
        // There is someone here
        // Clear spotted dead service detection
        ll_service->dead_service_spotted = 0;
        // Ask an ID  to the detector service.
        msg_t msg;
        msg.header.config      = BASE_PROTOCOL;
        msg.header.target_mode = NODEIDACK;
        msg.header.target      = 1;
        msg.header.cmd         = WRITE_NODE_ID;
        msg.header.size        = 0;
        Robus_SendMsg(ll_service, &msg);
        // Wait the end of transmission
        while (MsgAlloc_TxAllComplete() == FAILED)
            ;
        // Check if there is a failure on transmission
        if (ll_service->dead_service_spotted != 0)
        {
            // Message transmission failure
            // Consider this port unconnected
            Node_Get()->port_table[ctx.port.activ] = 0xFFFF;
            ctx.port.activ                         = NBR_PORT;
            ctx.port.keepLine                      = false;
            continue;
        }

        // when Robus loop will receive the reply it will store and manage the new node_id and send it to the next node.
        // We just have to wait the end of the treatment of the entire branch
        uint32_t start_tick = LuosHAL_GetSystick();
        while (ctx.port.keepLine)
        {
            Robus_Loop();
            if (LuosHAL_GetSystick() - start_tick > DETECTION_TIMEOUT_MS)
            {
                // topology detection is too long, we should abort it and restart
                return FAILED;
            }
        }
    }
    return SUCCEED;
}
/******************************************************************************
 * @brief check if received messages are protocols one and manage it if it is.
 * @param msg pointer to the reeived message
 * @return error_return_t SUCCEED if the message have been consumed.
 ******************************************************************************/
static error_return_t Robus_MsgHandler(msg_t *input)
{
    msg_t output_msg;
    node_bootstrap_t node_bootstrap;
    ll_service_t *ll_service = Recep_GetConcernedLLService(&input->header);
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
                    Robus_SendMsg(ll_service, &output_msg);
                    break;
                case 2:
                    // This is a node id for the next node.
                    // This is a reply to our request to generate the next node id.
                    // This node_id is the one after the currently poked branch.
                    // We need to save this ID as a connection on a port
                    memcpy((void *)&Node_Get()->port_table[ctx.port.activ], (void *)&input->data[0], sizeof(uint16_t));
                    // Now we can send it to the next node
                    memcpy((void *)&node_bootstrap.nodeid, (void *)&input->data[0], sizeof(uint16_t));
                    node_bootstrap.prev_nodeid    = Node_Get()->node_id;
                    output_msg.header.config      = BASE_PROTOCOL;
                    output_msg.header.cmd         = WRITE_NODE_ID;
                    output_msg.header.size        = sizeof(node_bootstrap_t);
                    output_msg.header.target      = 0;
                    output_msg.header.target_mode = NODEIDACK;
                    memcpy((void *)&output_msg.data[0], (void *)&node_bootstrap.unmap[0], sizeof(node_bootstrap_t));
                    Robus_SendMsg(ll_service, &output_msg);
                    break;
                case sizeof(node_bootstrap_t):
                    if (Node_Get()->node_id != 0)
                    {
                        Node_Get()->node_id = 0;
                        MsgAlloc_Init(NULL);
                    }
                    // This is a node bootstrap information.
                    memcpy((void *)&node_bootstrap.unmap[0], (void *)&input->data[0], sizeof(node_bootstrap_t));
                    Node_Get()->node_id                    = node_bootstrap.nodeid;
                    Node_Get()->port_table[ctx.port.activ] = node_bootstrap.prev_nodeid;
                    // Continue the topology detection on our other ports.
                    Robus_DetectNextNodes(ll_service);
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
 * @brief ID Mask calculation
 * @param ID and Number of service
 * @return None
 ******************************************************************************/
void Robus_IDMaskCalculation(uint16_t service_id, uint16_t service_number)
{
    // 4096 bit address 512 byte possible
    // Create a mask of only possibility in the node
    //--------------------------->|__________|
    //	Shift byte		            byte Mask of bit address

    LUOS_ASSERT(service_id > 0);
    LUOS_ASSERT(service_id <= 4096 - MAX_SERVICE_NUMBER);
    uint16_t tempo  = 0;
    ctx.IDShiftMask = (service_id - 1) / 8; // aligned to byte

    // create a mask of bit corresponding to ID number in the node
    for (uint16_t i = 0; i < service_number; i++)
    {
        tempo = (((service_id - 1) + i) - (8 * ctx.IDShiftMask));
        ctx.IDMask[tempo / 8] |= 1 << ((tempo) % 8);
    }
}
