/*
 * message_mngr.c
 *
 *  Created on: 17 sept. 2019
 *      Author: Nicolas Rabault
 */

#include "message_mngr.h"
#include "hal.h"

// no real time callback management
volatile int module_msg_available = 0;
volatile module_t* module_msg_mngr[MSG_BUFFER_SIZE];

//**************** mngr things*********************

char mngr_message_available(void) {
    return module_msg_available;
}

void mngr_set(module_t* module, msg_t* msg) {
    if ((module_msg_available+1 < MSG_BUFFER_SIZE) || (module->message_available+1 < MSG_BUFFER_SIZE)) {
        module_msg_mngr[module_msg_available++] = module;
        module->msg_stack[module->message_available++] = msg;
    } else {
        // out of buffer. remove the oldest message and add this new one.
        mngr_t trash;
        mngr_get_msg(0, 0, &trash);
        module_msg_mngr[module_msg_available++] = module;
        module->msg_stack[module->message_available++] = msg;
    }
}

void mngr_get_msg(int module_index,int msg_index, mngr_t* chunk) {
    if ((module_index < 0) | (msg_index < 0)){
        return;
    }
    // get module
    chunk->module = module_msg_mngr[module_index];
    hal_disable_irq();
    for (int i = module_index; i < module_msg_available; i++) {
        module_msg_mngr[i] = module_msg_mngr[i+1];
    }
    module_msg_available--;
    hal_enable_irq();

    // get msg
    chunk->msg = chunk->module->msg_stack[msg_index];
    hal_disable_irq();
    for (int i = msg_index; i < chunk->module->message_available; i++) {
        chunk->module->msg_stack[i] = chunk->module->msg_stack[i+1];
    }
    chunk->module->message_available--;
    hal_enable_irq();
}

void mngr_get(int module_index, mngr_t* chunk) {
    mngr_get_msg(module_index, 0, chunk);
}

// find the next message for a module with a callback
int get_next_cb_id() {
    for (int i = 0; i < module_msg_available; i++) {
        if (module_msg_mngr[i]->mod_cb){
            //there is a callback in this module return this id
            return i;
        }
    }
    return -1;
}

// find the next message for a specific module
int get_next_module_id(module_t* module) {
    for (int i = 0; i < module_msg_available; i++) {
        if (module_msg_mngr[i] == module){
            //there is the module we are looking for
            return i;
        }
    }
    return -1;
}

// find the next message from a specific id for a specific module
int get_next_msg_id(int mngr_id, short msg_from) {
    // find the next message from the specified id
    for (int i=0; i<module_msg_mngr[mngr_id]->message_available; i++) {
        if (module_msg_mngr[mngr_id]->msg_stack[i]->header.source == msg_from) {
            return i;
        }
    }
    return -1;
}

