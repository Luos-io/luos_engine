#include "luos.h"
#include "l0.h"
#include "robus.h"
#include "sys_msg.h"
#include <string.h>

static vm_t *luos_vm_pointer;
static volatile msg_t luos_pub_msg;
static volatile int luos_pub = LUOS_PROTOCOL_NB;

static int luos_msg_handler(vm_t* vm, msg_t* input, msg_t* output) {
    if (input->header.cmd == IDENTIFY_CMD) {
        output->header.cmd = INTRODUCTION_CMD;
        output->header.target_mode = ID;
        output->header.size = MAX_ALIAS_SIZE+1;
        output->header.target = input->header.source;
        for (int i=0; i<MAX_ALIAS_SIZE; i++) {
            output->data[i] = vm->alias[i];
        }
        output->data[MAX_ALIAS_SIZE] = vm->type;
        return IDENTIFY_CMD;
    }
    if ((input->header.cmd == REVISION) & (input->header.size == 0)) {
        output->header.cmd = REVISION;
        output->header.target_mode = ID;
#ifndef FIRM_REV
#define FIRM_REV "unknown"
#endif
        memcpy(output->data, FIRM_REV, sizeof("unknown"));
        output->header.size = strlen((char*)output->data);
        output->header.target = input->header.source;
        return REVISION;
    }
    if ((input->header.cmd == UUID) & (input->header.size == 0)) {
        output->header.cmd = UUID;
        output->header.target_mode = ID;
        output->header.size = sizeof(luos_uuid_t);
        output->header.target = input->header.source;
        luos_uuid_t uuid;
        uuid.uuid[0] = LUOS_UUID[0];
        uuid.uuid[1] = LUOS_UUID[1];
        uuid.uuid[2] = LUOS_UUID[2];
        memcpy(output->data, &uuid.unmap, sizeof(luos_uuid_t));
        return UUID;
    }
    return LUOS_PROTOCOL_NB;
}


void luos_cb(vm_t *vm, msg_t *msg) {
    // Luos message management
    int pub_type = luos_msg_handler(vm, msg, &luos_pub_msg);
    if (pub_type != LUOS_PROTOCOL_NB) {
        luos_vm_pointer = vm;
        luos_pub = pub_type;
        return;
    }
    // L0 message management
    pub_type = l0_msg_handler(vm, msg, &luos_pub_msg);
    if (pub_type == L0_LED) {
        return;
    }
    if (pub_type != LUOS_PROTOCOL_NB) {
        luos_vm_pointer = vm;
        luos_pub = pub_type;
        return;
    }
    if (vm->rx_cb != 0) {
        vm->rx_cb(vm, msg);
    }
}


void luos_init(void){
    robus_init(luos_cb);
}


void luos_loop(void) {
    if (luos_pub != LUOS_PROTOCOL_NB) {
      robus_send(luos_vm_pointer, &luos_pub_msg);
      luos_pub = LUOS_PROTOCOL_NB;
    }
}

void luos_modules_clear(void) {
    robus_modules_clear();
}

vm_t* luos_module_create(RX_CB mod_cb, unsigned char type, const char *alias) {
    return robus_module_create(mod_cb, type, alias);
}

unsigned char luos_send(vm_t* vm, msg_t *msg) {
    return robus_send(vm, msg);
}

unsigned char luos_send_alias(vm_t* vm, msg_t *msg) {
    msg->header.cmd = WRITE_ALIAS;
    return robus_send_sys(vm, msg);
}
