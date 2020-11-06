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
#include "portManager.h"
#include "context.h"
#include "luosHAL.h"
#include "msgAlloc.h"

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
static error_return_t Robus_DetectNextNodes(vm_t *vm);
static error_return_t Robus_ResetNetworkDetection(vm_t *vm);
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
    // Init the number of created  virtual container.
    ctx.vm_number = 0;
    // Set default container id. This id is a void id used if no container is created.
    ctx.node.node_id = DEFAULTID;
    // By default node are not certified.
    ctx.node.certified = false;
    // no transmission lock
    ctx.tx.lock = FALSE;
    // Save luos baudrate
    baudrate = DEFAULTBAUDRATE;

    // Init reception
    Recep_Init();

    // init detection structure
    PortMng_Init();

    // Clear message allocation buffer table
    MsgAlloc_Init(memory_stats);

    // Initialize the robus container status
    ctx.rx.status.unmap = 0;
    ctx.rx.status.identifier = 0xF;
    // Init hal
    LuosHAL_Init();
}
/******************************************************************************
 * @brief Loop of the Robus communication protocole
 * @param None
 * @return None
 ******************************************************************************/
void Robus_Loop(void)
{
    // Execute message allocation tasks
    MsgAlloc_loop();
    // Interpreat received messages and create luos task for it.
    msg_t *msg = NULL;
    while (MsgAlloc_PullMsgToInterpret(&msg) == SUCESS)
    {
        // Check if this message is a protocole one
        if (Robus_MsgHandler(msg) == FAIL)
        {
            // If not create luos tasks.
            Recep_InterpretMsgProtocol(msg);
        }
    }
}
/******************************************************************************
 * @brief crete a container in route table
 * @param type of container create
 * @return None
 ******************************************************************************/
vm_t *Robus_ContainerCreate(uint16_t type)
{
    // Set the container type
    ctx.vm_table[ctx.vm_number].type = type;
    // Initialise the container id, TODO the ID could be stored in EEprom, the default ID could be set in factory...
    ctx.vm_table[ctx.vm_number].id = DEFAULTID;
    // Initialize dead container detection
    ctx.vm_table[ctx.vm_number].dead_container_spotted = 0;
    // Return the freshly initialized vm pointer.
    return (vm_t *)&ctx.vm_table[ctx.vm_number++];
}
/******************************************************************************
 * @brief clear container list in route table
 * @param None
 * @return None
 ******************************************************************************/
void Robus_ContainersClear(void)
{
    // Clear vm table
    memset((void *)ctx.vm_table, 0, sizeof(vm_t) * MAX_VM_NUMBER);
    // Reset the number of created containers
    ctx.vm_number = 0;
}
/******************************************************************************
 * @brief Send Msg to a container
 * @param container to send
 * @param msg to send
 * @return Error
 ******************************************************************************/
error_return_t Robus_SendMsg(vm_t *vm, msg_t *msg)
{
    // Compute the full message size based on the header size info.
    unsigned short data_size = 0;
    error_return_t fail = SUCCESS;
    if (msg->header.size > MAX_DATA_MSG_SIZE)
        data_size = MAX_DATA_MSG_SIZE;
    else
        data_size = msg->header.size;
    unsigned short full_size = sizeof(header_t) + data_size;
    unsigned char nbr_nak_retry = 0;
    // Set protocol revision and source ID on the message
    msg->header.protocol = PROTOCOL_REVISION;
    if (vm->id != 0)
    {
        msg->header.source = vm->id;
    }
    else
    {
        msg->header.source = ctx.node.node_id;
    }
    // Add the CRC to the total size of the message
    full_size += 2;
ack_restart:
    nbr_nak_retry++;
    LuosHAL_SetIrqState(false);
    ctx.ack = FALSE;
    LuosHAL_SetIrqState(true);
    // Send message
    while (Transmit_Process((unsigned char *)msg->stream, full_size))
    {
        // There is a collision
        LuosHAL_SetIrqState(false);
        // switch reception in header mode
        ctx.rx.callback = Recep_GetHeader;
        LuosHAL_SetIrqState(true);
        // wait timeout of collided packet
        Transmit_WaitUnlockTx();
        // timer proportional to ID
        if (vm->id > 1)
        {
            for (volatile unsigned int tempo = 0; tempo < (COLLISION_TIMER * (vm->id - 1)); tempo++)
                ;
        }
    }
    // Check if ACK needed
    if ((msg->header.target_mode == IDACK) || (msg->header.target_mode == NODEIDACK))
    {
        // Check if it is a localhost message
        if (Recep_NodeConcerned(&msg->header) && (msg->header.target != DEFAULTID))
        {
            Transmit_SendAck();
            ctx.ack = 0;
        }
        else
        {
            // ACK needed, change the state of state machine for wait a ACK
            LuosHAL_SetIrqState(false);
            ctx.rx.callback = Recep_CatchAck;
            LuosHAL_SetIrqState(true);
            volatile int time_out = 0;
            while (!ctx.ack & (time_out < (120 * (1000000 / baudrate))))
            {
                time_out++;
            }
            status_t status;
            status.unmap = ctx.ack;
            if ((status.rx_error) | (status.identifier != 0x0F))
            {
                if ((ctx.ack) && (status.identifier != 0x0F))
                {
                    // This is probably a part of another message
                    // Send it to header
                    LuosHAL_SetIrqState(false);
                    ctx.rx.callback = Recep_GetHeader;
                    LuosHAL_SetIrqState(true);
                    Recep_GetHeader(&ctx.ack);
                }
                if (nbr_nak_retry < NBR_NAK_RETRY)
                {
                    Recep_Timeout();
                    for (volatile unsigned int tempo = 0; tempo < (COLLISION_TIMER * (nbr_nak_retry)); tempo++)
                        ;
                    goto ack_restart;
                }
                else
                {
                    // Set the dead container ID into the VM
                    vm->dead_container_spotted = msg->header.target;
                    fail = FAIL;
                }
            }
            ctx.ack = 0;
        }
    }
    // localhost management
    if (Recep_NodeConcerned(&msg->header))
    {
        // Reset potential residue of collision detection
        Recep_Reset();
        MsgAlloc_InvalidMsg();
        // set message into the allocator
        MsgAlloc_SetMessage(msg);
    }
    return fail;
}
/******************************************************************************
 * @brief Start a topology detection procedure
 * @param vm pointer to the detecting vm
 * @return The number of detected node.
 ******************************************************************************/
uint16_t Robus_TopologyDetection(vm_t *vm)
{
    uint8_t redetect_nb = 0;
redetect:
    // Reset all detection state of containers on the network
    Robus_ResetNetworkDetection(vm);

    // setup local node
    ctx.node.node_id = 1;
    last_node = 1;

    // setup sending vm
    vm->id = 1;

    if (Robus_DetectNextNodes(vm) == FAIL)
    {
        // check the number of retry we made
        if (redetect_nb > 4)
        {
            // Too many retry just, there is no hope too succeed.
            while (1)
                ;
        }
        // Detection fail, restart it
        redetect_nb++;
        goto redetect;
    }

    return last_node;
}
/******************************************************************************
 * @brief reset all module port states
 * @param vm pointer to the detecting vm
 * @return The number of detected node.
 ******************************************************************************/
static error_return_t Robus_ResetNetworkDetection(vm_t *vm)
{
    msg_t msg;
    uint8_t try = 0;

    msg.header.target = BROADCAST_VAL;
    msg.header.target_mode = BROADCAST;
    msg.header.cmd = RESET_DETECTION;
    msg.header.size = 0;

    do
    {
        Robus_SendMsg(vm, &msg);

        MsgAlloc_Init(NULL);

        // wait for some 2ms to be sure all previous messages are received and treated
        uint32_t start_tick = LuosHAL_GetSystick();
        while (LuosHAL_GetSystick() - start_tick < 2);
        try++;
    }
    while((MsgAlloc_IsEmpty() != SUCESS)||(try > 5));

    ctx.node.node_id = 0;
    PortMng_Init();
    if(try < 5)
    {
        return SUCESS;
    }

    return FAIL;
}
/******************************************************************************
 * @brief run the procedure allowing to detect the next nodes on the next port
 * @param vm pointer to the detecting vm
 * @return None.
 ******************************************************************************/
static error_return_t Robus_DetectNextNodes(vm_t *vm)
{
    // Lets try to poke other nodes
    while (PortMng_PokeNextPort() == SUCESS)
    {
        // There is someone here
        // Ask an ID  to the detector container.
        msg_t msg;
        msg.header.target_mode = IDACK;
        msg.header.target = 1;
        msg.header.cmd = WRITE_NODE_ID;
        msg.header.size = 0;
        if (Robus_SendMsg(vm, &msg) == FAIL)
        {
            // Message transmission failure
            // Consider this port unconnected
            ctx.node.port_table[ctx.port.activ] = 0xFFFF;
            ctx.port.activ = NBR_PORT;
            ctx.port.keepLine = false;
            continue;
        }
        // when Robus loop will receive the reply it will store and manage the new node_id and send it to the next node.
        // We just have to wait the end of the treatment of the entire branch
        uint32_t start_tick = LuosHAL_GetSystick();
        while (ctx.port.keepLine)
        {
            Robus_Loop();
            if (LuosHAL_GetSystick() - start_tick > 1000)
            {
                // topology detection is too long, we should abort it and restart
                return FAIL;
            }
        }
    }
    return SUCESS;
}
/******************************************************************************
 * @brief check if received messages are protocols one and manage it if it is.
 * @param msg pointer to the reeived message
 * @return error_return_t success if the message have been consumed.
 ******************************************************************************/
static error_return_t Robus_MsgHandler(msg_t *input)
{
    uint32_t baudrate;
    msg_t output_msg;
    node_bootstrap_t node_bootstrap;
    vm_t *vm = Recep_GetConcernedVm(&input->header);
    switch (input->header.cmd)
    {
    case WRITE_NODE_ID:
        // Depending on the size of the received data we have to do different things
        switch (input->header.size)
        {
        case 0:
            // Someone asking us a new node id (we are the detecting module)
            // Increase the number of node_nb and send it back
            last_node++;
            output_msg.header.cmd = WRITE_NODE_ID;
            output_msg.header.size = sizeof(uint16_t);
            output_msg.header.target = input->header.source;
            output_msg.header.target_mode = NODEIDACK;
            memcpy(output_msg.data, (void *)&last_node, sizeof(uint16_t));
            Robus_SendMsg(vm, &output_msg);
            break;
        case 2:
            // This is a node id for the next node.
            // This is a reply to our request to generate the next node id.
            // This node_id is the one after the currently poked branch.
            // We need to save this ID as a connection on a port
            memcpy((void *)&ctx.node.port_table[ctx.port.activ], (void *)&input->data[0], sizeof(uint16_t));
            // Now we can send it to the next node
            memcpy((void *)&node_bootstrap.nodeid, (void *)&input->data[0], sizeof(uint16_t));
            node_bootstrap.prev_nodeid = ctx.node.node_id;
            output_msg.header.cmd = WRITE_NODE_ID;
            output_msg.header.size = sizeof(node_bootstrap_t);
            output_msg.header.target = 0;
            output_msg.header.target_mode = NODEIDACK;
            memcpy((void *)&output_msg.data[0], (void *)&node_bootstrap.unmap[0], sizeof(node_bootstrap_t));
            Robus_SendMsg(vm, &output_msg);
            break;
        case sizeof(node_bootstrap_t):
            if (ctx.node.node_id != 0)
            {
                ctx.node.node_id = 0;
                MsgAlloc_Init(NULL);
            }
            // This is a node bootstrap information.
            memcpy((void *)&node_bootstrap.unmap[0], (void *)&input->data[0], sizeof(node_bootstrap_t));
            ctx.node.node_id = node_bootstrap.nodeid;
            ctx.node.port_table[ctx.port.activ] = node_bootstrap.prev_nodeid;
            // Continue the topology detection on our other ports.
            Robus_DetectNextNodes(vm);
        default:
            break;
        }
        return SUCCESS;
        break;
    case RESET_DETECTION:
        return SUCCESS;
        break;
    case SET_BAUDRATE:
        memcpy(&baudrate, input->data, sizeof(uint32_t));
        LuosHAL_ComInit(baudrate);
        return SUCCESS;
        break;

    default:
        return FAIL;
        break;
    }
    return FAIL;
}
/******************************************************************************
 * @brief get node structure
 * @param None
 * @return Node pointer
 ******************************************************************************/
node_t *Robus_GetNode(void)
{
    return (node_t *)&ctx.node;
}
