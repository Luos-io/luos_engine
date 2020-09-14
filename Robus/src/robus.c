/*
 * robus.c
 *
 * Created: 14/02/2017 11:53:28
 *  Author: nico
 *  Abstract: User functionalities of the robus communication protocol
 */
#include <robus.h>
#include <string.h>
#include "sys_msg.h"
#include "reception.h"
#include "detection.h"
#include "context.h"
#include "hal.h"
#include "cmd.h"

// Creation of the robus context. This variable is used in all files of this lib.
volatile context_t ctx;

// Startup and network configuration
void robus_init(RX_CB callback)
{
    // Init the number of created  virtual module.
    ctx.vm_number = 0;
    // Initialize the reception state machine
    ctx.data_cb = get_header;
    // Set default module id. This id is a void id used if no module is created.
    ctx.id = DEFAULTID;
    // VOID Module type
    ctx.type = 0;
    // no transmission lock
    ctx.tx_lock = FALSE;
    // Save luos callback
    ctx.luos_cb = callback;
    // Save luos baudrate
    ctx.baudrate = DEFAULTBAUDRATE;

    // init detection structure
    reset_detection();
    for (unsigned char branch = 0; branch < NO_BRANCH; branch++)
    {
        ctx.detection.branches[branch] = 0;
    }

    // Clear message allocation buffer table
    for (int i = 0; i < MSG_BUFFER_SIZE; i++)
    {
        ctx.alloc_msg[i] = 0;
    }
    // Initialize the start case of the message buffer
    ctx.current_buffer = 0;
    // Initialize the robus module status
    ctx.status.unmap = 0;
    ctx.status.identifier = 0xF;
    // Init hal
    hal_init();
}

void robus_modules_clear(void)
{
    // Clear vm table
    memset((void *)ctx.vm_table, 0, sizeof(vm_t) * MAX_VM_NUMBER);
    // Reset the number of created modules
    ctx.vm_number = 0;
}

vm_t *robus_module_create(unsigned char type)
{
    unsigned char i = 0;

    // Set the module type
    ctx.vm_table[ctx.vm_number].type = type;
    // Initialise the module id, TODO the ID could be stored in EEprom, the default ID could be set in factory...
    ctx.vm_table[ctx.vm_number].id = DEFAULTID;
    // Clear the msg allocation table.
    for (i = 0; i < MSG_BUFFER_SIZE; i++)
    {
        ctx.alloc_msg[i] = 0;
    }
    // Initialize dead module detection
    ctx.vm_table[ctx.vm_number].dead_module_spotted = 0;
    // Return the freshly initialized vm pointer.
    return (vm_t *)&ctx.vm_table[ctx.vm_number++];
}

unsigned char robus_send(vm_t *vm, msg_t *msg)
{
    msg->header.cmd += PROTOCOL_CMD_NB;
    unsigned char ret = robus_send_sys(vm, msg);
    msg->header.cmd -= PROTOCOL_CMD_NB;
    return ret;
}

unsigned char robus_set_baudrate(vm_t *vm, unsigned int baudrate)
{
    msg_t msg;
    memcpy(msg.data, &baudrate, sizeof(unsigned int));
    msg.header.target_mode = BROADCAST;
    msg.header.target = BROADCAST_VAL;
    msg.header.cmd = SET_BAUDRATE;
    msg.header.size = sizeof(unsigned int);
    if (robus_send_sys(vm, &msg))
        return 1;
    return 0;
}

unsigned short *robus_get_node_branches(unsigned char *size)
{
    *size = NO_BRANCH;
    return (unsigned short *)ctx.detection.branches;
}

unsigned char robus_topology_detection(vm_t *vm)
{
    unsigned short newid = 1;
    // Reset all detection state of modules on the network
    reset_network_detection(vm);
    ctx.detection_mode = MASTER_DETECT;
    // wait for some us
    for (volatile unsigned int i = 0; i < (2 * TIMERVAL); i++)
        ;

    // setup sending vm
    vm->id = newid++;

    // Parse internal vm other than the sending one
    for (unsigned char i = 0; i < ctx.vm_number; i++)
    {
        if (&ctx.vm_table[i] != vm)
        {
            ctx.vm_table[i].id = newid++;
        }
    }

    ctx.detection.detected_vm = ctx.vm_number;
    ctx.detection.detection_end = TRUE;

    for (unsigned char branch = 0; branch < NO_BRANCH; branch++)
    {
        ctx.detection_mode = MASTER_DETECT;
        if (poke(branch))
        {
            // Someone reply to our poke!
            // loop while the line is released
            int module_number = 0;
            while ((ctx.detection.keepline != NO_BRANCH) & (module_number < 1024))
            {
                if (set_extern_id(vm, IDACK, DEFAULTID, newid++))
                {
                    // set extern id fail
                    // remove this id and stop topology detection
                    newid--;
                    break;
                }
                module_number++;
                // wait for some us
                for (volatile unsigned int i = 0; i < (2 * TIMERVAL); i++)
                    ;
            }
        }
    }
    ctx.detection_mode = NO_DETECT;

    return newid - 1;
}
