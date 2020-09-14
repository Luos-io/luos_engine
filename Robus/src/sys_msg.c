/*
 * sys_msg.c
 *
 * Created: 14/02/2017 11:53:28
 *  Author: Nicolas Rabault
 *  Abstract: protocol system message management.
 */
#include <robus.h>
#include "sys_msg.h"
#include "hal.h"
#include <string.h>
#include "context.h"
#include "reception.h"

void wait_tx_unlock(void);
unsigned char transmit(unsigned char *data, unsigned short size);

/* Specific system mesages :
 * These messages don't follow generic rules of this protocol, there are
 * protocols level messages.
 * Please use it with caution
 */

void send_ack(void)
{
    hal_enable_tx();
    hal_disable_rx();
    hal_transmit((unsigned char *)&ctx.status.unmap, 1);
    hal_wait_transmit_end();
    hal_enable_rx();
    hal_disable_tx();
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
    crc(msg->stream, full_size, (unsigned char *)&msg->data[data_size]);
    // Add the CRC to the total size of the message
    full_size += 2;
    ctx.vm_last_send = vm;
ack_restart:
    nbr_nak_retry++;
    hal_disable_irq();
    ctx.ack = FALSE;
    hal_enable_irq();
    // Send message
    while (transmit((unsigned char *)msg->stream, full_size))
    {
        // There is a collision
        hal_disable_irq();
        // switch reception in header mode
        ctx.data_cb = get_header;
        hal_enable_irq();
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
            hal_disable_irq();
            ctx.data_cb = catch_ack;
            hal_enable_irq();
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
                    hal_disable_irq();
                    ctx.data_cb = get_header;
                    hal_enable_irq();
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
        hal_disable_irq();
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
        hal_enable_irq();
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
    hal_enable_tx();
    hal_disable_irq();
    // switch reception in collision detection mode
    ctx.data_cb = get_collision;
    ctx.tx_data = data;
    hal_enable_irq();
    // re-lock the transmission
    if (ctx.collision | HAL_is_tx_lock())
    {
        // We receive something during our configuration, stop this transmission
        hal_disable_tx();
        ctx.collision = FALSE;
        return 1;
    }
    ctx.tx_lock = TRUE;
    // Try to detect a collision during the "col_check_data_num" first bytes
    if (hal_transmit(data, col_check_data_num))
    {
        hal_disable_tx();
        ctx.collision = FALSE;
        return 1;
    }
    // No collision occure, stop collision detection mode and continue to transmit
    hal_disable_irq();
    ctx.data_cb = get_header;
    hal_enable_irq();
    hal_disable_rx();
    hal_transmit(data + col_check_data_num, size - col_check_data_num);
    hal_wait_transmit_end();
    // get ready to receive a ack just in case
    // disable TX and Enable RX
    hal_enable_rx();
    hal_disable_tx();
    // Force Usart Timeout
    timeout();
    return 0;
}

void wait_tx_unlock(void) // TODO : This function could be in HAL and replace HAL_is_tx_lock. By the way timeout management here is shity
{
    volatile int timeout = 0;
    while (HAL_is_tx_lock() && (timeout < 64000))
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
