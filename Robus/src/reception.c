/*
 * reception.c
 *
 * Created: 14/02/2017 11:53:28
 *  Author: Nicolas Rabault
 *  Abstract: reception state machine
 */
//#define DEBUG
#include <string.h>
#include "reception.h"
#include "hal.h"
#include "cmd.h"
#include "target.h"
#include "sys_msg.h"
#include "time.h"

#ifdef DEBUG
#include <stdio.h>
#endif

#define CURRENTMSG ctx.msg[ctx.current_buffer]
#define CURRENTMODULE ctx.vm_table[ctx.alloc_msg[ctx.current_buffer]]

unsigned char keep = FALSE;
unsigned char concernedmodules[MAX_VM_NUMBER] = {FALSE};
unsigned short data_count = 0;
unsigned short data_size = 0;

unsigned char module_concerned(header_t *header)
{
    unsigned char concerned = FALSE;
    // Find if we are concerned by this message.
    switch (header->target_mode)
    {
    case IDACK:
        ctx.status.rx_error = FALSE;
    case ID:
        // Get ID even if this is default ID and we have an activ branch waiting to be linked to a module id
        if ((header->target == ctx.id) && (ctx.detection.activ_branch != NO_BRANCH))
        {
            concerned = TRUE;
            ctx.alloc_msg[ctx.current_buffer] = 0;
            ctx.data_cb = get_data;
            break;
        }
        // Check all VM id
        for (int i = 0; i < ctx.vm_number; i++)
        {
            concerned = (header->target == ctx.vm_table[i].id);
            if (concerned)
            {
                ctx.alloc_msg[ctx.current_buffer] = i;
                ctx.data_cb = get_data;
                break;
            }
        }
        break;
    case TYPE:
        //check default type
        if (header->target == ctx.type)
        {
            concerned = TRUE;
            concernedmodules[0] = TRUE;
            ctx.data_cb = get_data;
            break;
        }
        // Check all VM type
        for (int i = 0; i < ctx.vm_number; i++)
        {
            if (header->target == ctx.vm_table[i].type)
            {
                concerned = TRUE;
                concernedmodules[i] = TRUE;
                ctx.data_cb = get_data;
            }
        }
        break;
    case BROADCAST:
        concerned = (header->target == BROADCAST_VAL);
        ctx.data_cb = get_data;
        if (concerned)
        {
            for (int i = 0; i < ctx.vm_number; i++)
            {
                concernedmodules[i] = TRUE;
            }
        }
        break;
    case MULTICAST:
        for (int i = 0; i < ctx.vm_number; i++)
        {
            if (multicast_target_bank((vm_t *)&ctx.vm_table[i], header->target))
            { //TODO manage multiple slave concerned
                concerned = TRUE;
                concernedmodules[i] = TRUE;
            }
        }
        ctx.data_cb = get_data;
        break;
    default:
        return concerned;
        break;
    }
    return concerned;
}

/**
 * \fn void timeout(flush)
 * \brief manage timeout event
 *
 * \return
 */
void timeout(void)
{
    if (ctx.data_cb != get_header)
    {
        ctx.status.rx_timeout = TRUE;
    }
    ctx.tx_lock = FALSE;
    flush();
}

/**
 * \fn void flush(flush)
 * \brief reset the reception state machine
 *
 * \return
 */
void flush(void)
{
    hal_disable_irq();
    ctx.data_cb = get_header;
    keep = FALSE;
    data_count = 0;
    hal_enable_irq();
}
static unsigned short crc_val = 0;

/**
 * \fn void get_header(volatile unsigned char *data)
 * \brief catch a complete header
 *
 * \param *data byte received from serial
 */
void get_header(volatile unsigned char *data)
{
    ctx.tx_lock = TRUE;
    // Catch a byte.
    CURRENTMSG.header.unmap[data_count++] = *data;

    // Check if we have all we need.
    if (data_count == (sizeof(header_t)))
    {

#ifdef DEBUG
        printf("*******header data*******\n");
        printf("protocol : 0x%04x\n", CURRENTMSG.header.protocol);       /*!< Protocol version. */
        printf("target : 0x%04x\n", CURRENTMSG.header.target);           /*!< Target address, it can be (ID, Multicast/Broadcast, Type). */
        printf("target_mode : 0x%04x\n", CURRENTMSG.header.target_mode); /*!< Select targeting mode (ID, ID+ACK, Multicast/Broadcast, Type). */
        printf("source : 0x%04x\n", CURRENTMSG.header.source);           /*!< Source address, it can be (ID, Multicast/Broadcast, Type). */
        printf("cmd : 0x%04x\n", CURRENTMSG.header.cmd);                 /*!< msg definition. */
        printf("size : 0x%04x\n", CURRENTMSG.header.size);               /*!< Size of the data field. */
#endif
        // Reset the catcher.
        data_count = 0;
        // Cap size for big messages
        if (CURRENTMSG.header.size > MAX_DATA_MSG_SIZE)
            data_size = MAX_DATA_MSG_SIZE;
        else
            data_size = CURRENTMSG.header.size;
        // Reset the msg allocation
        ctx.alloc_msg[ctx.current_buffer] = 0;

        keep = module_concerned((header_t *)&CURRENTMSG.header);
        if (keep)
        {
            // start crc computation
            crc((unsigned char *)CURRENTMSG.stream, sizeof(header_t), (unsigned char *)&crc_val);
        }
    }
}

/**
 * \fn void get_infos(volatile unsigned char *data)
 * \brief catch data field.
 *
 * \param *data byte received from serial
 */
void get_data(volatile unsigned char *data)
{
    CURRENTMSG.data[data_count] = *data;
    if ((data_count < data_size) && keep)
    {
        // Continue CRC computation until the end of data
        crc((unsigned char *)&CURRENTMSG.data[data_count], 1, (unsigned char *)&crc_val);
    }
    if (data_count > data_size)
    {
        if (keep)
        {
            CURRENTMSG.crc = ((unsigned short)CURRENTMSG.data[data_size]) |
                             ((unsigned short)CURRENTMSG.data[data_size + 1] << 8);
            if (CURRENTMSG.crc == crc_val)
            {
                if ((CURRENTMSG.header.target_mode == IDACK) && (CURRENTMSG.header.target != DEFAULTID))
                {
                    send_ack();
                }
                ctx.data_cb = get_header;
                if (CURRENTMSG.header.target_mode == ID || CURRENTMSG.header.target_mode == IDACK)
                {
                    msg_complete((msg_t *)&CURRENTMSG);
                }
                else
                {
                    for (int i = 0; i < ctx.vm_number; i++)
                    {
                        if (concernedmodules[i])
                        {
                            ctx.alloc_msg[ctx.current_buffer] = i;
                            msg_complete((msg_t *)&CURRENTMSG);
                            concernedmodules[i] = FALSE;
                        }
                    }
                }
                ctx.current_buffer++;
                if (ctx.current_buffer == MSG_BUFFER_SIZE)
                {
                    ctx.current_buffer = 0;
                }
            }
            else
            {
                ctx.status.rx_error = TRUE;
                if ((CURRENTMSG.header.target_mode == IDACK))
                {
                    send_ack();
                }
            }
        }
        flush();
        return;
    }
    data_count++;
}

/**
 * \fn void get_collision(volatile unsigned char *data)
 * \brief catch bus collision.
 *
 * \param *data byte received from serial
 */
void get_collision(volatile unsigned char *data)
{
    if ((*ctx.tx_data != *data) || (!ctx.tx_lock))
    {
        //data dont match, or we don't start to send, there is a collision
        ctx.collision = TRUE;
        //Stop TX trying to save input datas
        hal_disable_tx();
        // send all received datas
        get_header(data);
    }
    ctx.tx_data = ctx.tx_data + 1;
}

/**
 * \fn void catch_ack(volatile unsigned char *data)
 * \brief catch ack.
 *
 * \param *data byte received from serial
 */
void catch_ack(volatile unsigned char *data)
{
    // set VM msg
    ctx.vm_last_send->msg_pt = (msg_t *)&CURRENTMSG;
    // Check ACK value.
    CURRENTMSG.ack = *data;
    // notify ACK reception
    ctx.ack = TRUE;
    ctx.data_cb = get_header;
}

/**
 * \fn void msg_complete()
 * \brief the message is now complete, manage it.
 *
 * \param *data byte received from serial
 */
void msg_complete(msg_t *msg)
{
    if (msg->header.target_mode == ID ||
        msg->header.target_mode == IDACK ||
        msg->header.target_mode == TYPE ||
        msg->header.target_mode == BROADCAST)
    {
        switch (msg->header.cmd)
        {
        case WRITE_ID:
            if (ctx.detection.activ_branch == NO_BRANCH)
            {
                // Get and save a new given ID
                if ((ctx.vm_table[ctx.detection.detected_vm].id == DEFAULTID) &
                    (ctx.detection.keepline != NO_BRANCH) &
                    (ctx.detection_mode != MASTER_DETECT) &
                    (!ctx.detection.detection_end))
                {
                    if (msg->header.target_mode == IDACK)
                    {
                        // Acknoledge ID reception
                        send_ack();
                    }
                    // We are on topology detection mode, and this is our turn
                    // Save id for the next module we have on this board
                    ctx.vm_table[ctx.detection.detected_vm++].id =
                        (((unsigned short)msg->data[1]) |
                         ((unsigned short)msg->data[0] << 8));
                    if (ctx.detection.detected_vm == 1)
                    {
                        // This is the first internal module, save the input branch with the previous ID
                        ctx.detection.branches[ctx.detection.keepline] = ctx.vm_table[0].id - 1;
                    }
                    // Check if that was the last virtual module
                    if (ctx.detection.detected_vm >= ctx.vm_number)
                    {
                        ctx.detection.detection_end = TRUE;
                        poke_next_branch();
                    }
                }
                else if (msg->header.target != DEFAULTID)
                {
                    CURRENTMODULE.id = (((unsigned short)msg->data[1]) |
                                        ((unsigned short)msg->data[0] << 8));
                }
            }
            else
            {
                unsigned short value = (((unsigned short)msg->data[1]) |
                                        ((unsigned short)msg->data[0] << 8));
                //We need to save this ID as a connection on a branch
                ctx.detection.branches[ctx.detection.activ_branch] = value;
                ctx.detection.activ_branch = NO_BRANCH;
            }
            break;
        case RESET_DETECTION:
            // Reinit branch state and link
            for (unsigned char branch = 0; branch < NO_BRANCH; branch++)
            {
                reset_PTP(branch);
                ctx.detection.branches[branch] = 0;
            }
            reset_detection();
            // Reinit VM id
            for (int i = 0; i < ctx.vm_number; i++)
            {
                ctx.vm_table[i].id = DEFAULTID;
            }
            ctx.detection.detection_end = FALSE;
            break;
        case SET_BAUDRATE:
            memcpy((void *)&ctx.baudrate, msg->data, msg->header.size);
            set_baudrate(ctx.baudrate);
            break;
        default:
            // set VM data
            CURRENTMODULE.msg_pt = msg;
            msg->header.cmd -= PROTOCOL_CMD_NB;
            ctx.luos_cb((vm_t *)&CURRENTMODULE, CURRENTMODULE.msg_pt);
            break;
        }
    }
    else
    {
        // set VM data
        CURRENTMODULE.msg_pt = msg;
        // call callback
        msg->header.cmd -= PROTOCOL_CMD_NB;
        ctx.luos_cb((vm_t *)&CURRENTMODULE, CURRENTMODULE.msg_pt);
        msg->header.cmd += PROTOCOL_CMD_NB;
    }
    ctx.data_cb = get_header;
}
