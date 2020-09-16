/******************************************************************************
 * @file sys_msg
 * @brief protocol system message management.
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <sys_msg.h>

#include "luosHAL.h"
#include <string.h>
#include "context.h"
#include "reception.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void wait_tx_unlock(void);
static uint8_t get_lock(void);
unsigned char transmit(unsigned char *data, unsigned short size);

/* Specific system mesages :
 * These messages don't follow generic rules of this protocol, there are
 * protocols level messages.
 * Please use it with caution
 */

void send_ack(void)
{
	LuosHAL_SetTxState(true);
	LuosHAL_SetRxState(false);
	LuosHAL_ComTransmit((unsigned char *)&ctx.status.unmap, 1);
	LuosHAL_ComTxTimeout();
	LuosHAL_SetRxState(true);
	LuosHAL_SetTxState(false);
    ctx.status.unmap = 0x0F;
}

unsigned char robus_send_sys(vm_t *vm, msg_t *msg)
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
    while (transmit((unsigned char *)msg->stream, full_size))
    {
        // There is a collision
    	LuosHAL_SetIrqState(false);
        // switch reception in header mode
        ctx.data_cb = get_header;
        LuosHAL_SetIrqState(true);
        // wait timeout of collided packet
        wait_tx_unlock();
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
        if (module_concerned(&msg->header) && (msg->header.target != DEFAULTID))
        {
            send_ack();
            ctx.ack = 0;
        }
        else
        {
            // ACK needed, change the state of state machine for wait a ACK
        	LuosHAL_SetIrqState(false);
            ctx.data_cb = catch_ack;
            LuosHAL_SetIrqState(true);
            volatile int time_out = 0;
            while (!ctx.ack & (time_out < (120 * (1000000 / ctx.baudrate))))
            {
                time_out++;
            }
            status_t status;
            status.unmap = vm->msg_pt->ack;
            if ((!ctx.ack) | (status.rx_error) | (status.identifier != 0xF))
            {
                if (ctx.ack && status.identifier != 0xF)
                {
                    // This is probably a part of another message
                    // Send it to header
                	LuosHAL_SetIrqState(false);
                    ctx.data_cb = get_header;
                    LuosHAL_SetIrqState(true);
                    get_header(&vm->msg_pt->ack);
                }
                if (nbr_nak_retry < NBR_NAK_RETRY)
                {
                    timeout();
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
    if (module_concerned(&msg->header))
    {
    	LuosHAL_SetIrqState(false);
        // Secure the message memory by copying it into msg buffer
        memcpy((void *)&ctx.msg[ctx.current_buffer], msg, sizeof(header_t) + msg->header.size + 2);
        // Manage this message
        msg_complete((msg_t *)&ctx.msg[ctx.current_buffer]);
        // Select next message buffer slot.
        ctx.current_buffer++;
        if (ctx.current_buffer == MSG_BUFFER_SIZE)
        {
            ctx.current_buffer = 0;
        }
        flush();
        LuosHAL_SetIrqState(true);
    }
    return fail;
}

//*********************** local functions ***************************

unsigned char transmit(unsigned char *data, unsigned short size)
{
    const int col_check_data_num = 5;
    // wait tx unlock
    wait_tx_unlock();
    ctx.collision = FALSE;
    // Enable TX
    LuosHAL_SetTxState(true);
    LuosHAL_SetIrqState(false);
    // switch reception in collision detection mode
    ctx.data_cb = get_collision;
    ctx.tx_data = data;
    LuosHAL_SetIrqState(true);
    // re-lock the transmission
    if (ctx.collision | get_lock())
    {
        // We receive something during our configuration, stop this transmission
    	LuosHAL_SetTxState(false);
        ctx.collision = FALSE;
        return 1;
    }
    ctx.tx_lock = true;
    // Try to detect a collision during the "col_check_data_num" first bytes
    if (LuosHAL_ComTransmit(data, col_check_data_num))
    {
    	LuosHAL_SetTxState(false);
        ctx.collision = FALSE;
        return 1;
    }
    // No collision occure, stop collision detection mode and continue to transmit
    LuosHAL_SetIrqState(false);
    ctx.data_cb = get_header;
    LuosHAL_SetIrqState(true);
    LuosHAL_SetRxState(false);
    LuosHAL_ComTransmit(data + col_check_data_num, size - col_check_data_num);
    LuosHAL_ComTxTimeout();
    // get ready to receive a ack just in case
    // disable TX and Enable RX
    LuosHAL_SetRxState(true);
    LuosHAL_SetTxState(false);
    // Force Usart Timeout
    timeout();
    return 0;
}

void wait_tx_unlock(void) // TODO : This function could be in HAL and replace HAL_is_tx_lock. By the way timeout management here is shity
{
    volatile int timeout = 0;
    while (get_lock() && (timeout < 64000))
    {
        timeout++;
    }
}

unsigned char set_extern_id(vm_t *vm, target_mode_t target_mode, unsigned short target, unsigned short newid)
{
    msg_t msg;
    msg.header.target = target;
    msg.header.target_mode = target_mode;
    msg.header.cmd = WRITE_ID;
    msg.header.size = 2;
    msg.data[1] = newid;
    msg.data[0] = (newid << 8);
    if (robus_send_sys(vm, &msg))
    {
        return 1;
    }
    return 0;
}

static uint8_t get_lock(void)
{
	ctx.tx_lock |= LuosHAL_GetTxLockState();
	return ctx.tx_lock;
}
