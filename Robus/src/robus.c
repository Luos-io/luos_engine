/******************************************************************************
 * @file robus
 * @brief User functionalities of the robus communication protocol
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <robus.h>

#include <string.h>
#include "sys_msg.h"
#include "reception.h"
#include "detection.h"
#include "context.h"
#include "luosHAL.h"
#include "msgAlloc.h"
#include <stdbool.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
// Creation of the robus context. This variable is used in all files of this lib.
volatile context_t ctx;

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Initialisation of the Robus communication protocole
 * @param Luos function pointer into the rx callback interrupt.
 * @return None
 ******************************************************************************/
void Robus_Init(void)
{
    // Init the number of created  virtual module.
    ctx.vm_number = 0;
    // Initialize the reception state machine
    ctx.data_cb = Recep_GetHeader;
    // Set default module id. This id is a void id used if no module is created.
    ctx.id = DEFAULTID;
    // VOID Module type
    ctx.type = 0;
    // no transmission lock
    ctx.tx_lock = FALSE;
    // Save luos baudrate
    ctx.baudrate = DEFAULTBAUDRATE;

    // init detection structure
    Detec_InitDetection();

    // Clear message allocation buffer table
    MsgAlloc_Init();

    // Initialize the robus module status
    ctx.status.unmap = 0;
    ctx.status.identifier = 0xF;
    // Init hal
    LuosHAL_Init();
}
/******************************************************************************
 * @brief crete a module in route table
 * @param type of module create
 * @return None
 ******************************************************************************/
vm_t *Robus_ModuleCreate(unsigned char type)
{
    // Set the module type
    ctx.vm_table[ctx.vm_number].type = type;
    // Initialise the module id, TODO the ID could be stored in EEprom, the default ID could be set in factory...
    ctx.vm_table[ctx.vm_number].id = DEFAULTID;
    // Initialize dead module detection
    ctx.vm_table[ctx.vm_number].dead_module_spotted = 0;
    // Return the freshly initialized vm pointer.
    return (vm_t *)&ctx.vm_table[ctx.vm_number++];
}
/******************************************************************************
 * @brief clear module list in route table
 * @param None
 * @return None
 ******************************************************************************/
void Robus_ModulesClear(void)
{
    // Clear vm table
    memset((void *)ctx.vm_table, 0, sizeof(vm_t) * MAX_VM_NUMBER);
    // Reset the number of created modules
    ctx.vm_number = 0;
}
/******************************************************************************
 * @brief Send Msg to a module
 * @param module to send
 * @param msg to send
 * @return Error
 ******************************************************************************/
uint8_t Robus_SendMsg(vm_t *vm, msg_t *msg)
{
    // Compute the full message size based on the header size info.
    unsigned short data_size = 0;
    unsigned char fail = 0;
    if (msg->header.size > MAX_DATA_MSG_SIZE)
        data_size = MAX_DATA_MSG_SIZE;
    else
        data_size = msg->header.size;
    unsigned short full_size = sizeof(header_t) + data_size;
    unsigned char nbr_nak_retry = 0;
    // Set protocol revision and source ID on the message
    msg->header.protocol = PROTOCOL_REVISION;
    msg->header.source = vm->id;
    // compute the CRC
    LuosHAL_ComputeCRC(msg->stream, full_size, (unsigned char *)&msg->data[data_size]);
    // Add the CRC to the total size of the message
    full_size += 2;
    ctx.vm_last_send = vm;
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
        ctx.data_cb = Recep_GetHeader;
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
    if (msg->header.target_mode == IDACK)
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
            ctx.data_cb = Recep_CatchAck;
            LuosHAL_SetIrqState(true);
            volatile int time_out = 0;
            while (!ctx.ack & (time_out < (120 * (1000000 / ctx.baudrate))))
            {
                time_out++;
            }
            status_t status;
            status.unmap = ctx.ack;
            if ((status.rx_error) | (status.identifier != 0xF))
            {
                if (ctx.ack && status.identifier != 0xF)
                {
                    // This is probably a part of another message
                    // Send it to header
                    LuosHAL_SetIrqState(false);
                    ctx.data_cb = Recep_GetHeader;
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
                    // Set the dead module ID into the VM
                    vm->dead_module_spotted = msg->header.target;
                    fail = 1;
                }
            }
            ctx.ack = 0;
        }
    }
    // localhost management
    if (Recep_NodeConcerned(&msg->header))
    {
        // Reset potential residue of collision detection
        MsgAlloc_InvalidMsg();
        LuosHAL_SetIrqState(false);

        for (int i = 0; i < (sizeof(header_t)); i++)
        {
            MsgAlloc_SetData((uint8_t)msg->stream[i]);
        }
        // Generate alloc list by computing concerned modules
        Recep_ModuleConcerned(&msg->header);
        // Secure the message memory by copying it into msg buffer
        for (int i = sizeof(header_t); i < (sizeof(header_t) + msg->header.size + 2); i++)
        {
            MsgAlloc_SetData((uint8_t)msg->stream[i]);
        }
        MsgAlloc_EndMsg();
        Recep_Reset();
        LuosHAL_SetIrqState(true);
    }
    return fail;
}
/******************************************************************************
 * @brief get branch where node is connected
 * @param branch
 * @return None
 ******************************************************************************/
uint16_t *Robus_GetNodeBranches(uint8_t *size)
{
    *size = NO_BRANCH;
    return (uint16_t *)ctx.detection.branches;
}