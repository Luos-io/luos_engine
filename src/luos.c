#include "luos.h"
#include "l0.h"

int luos_msg_handler(vm_t* vm, msg_t* input, msg_t* output) {
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
    if (input->header.cmd == REVISION & input->header.size == 0) {
        output->header.cmd = REVISION;
        output->header.target_mode = ID;
#ifndef FIRM_REV
#define FIRM_REV "unknown"
#endif
        memcpy(output->data, FIRM_REV, sizeof("unknown"));
        output->header.size = strlen(output->data);
        output->header.target = input->header.source;
        return REVISION;
    }
    if (input->header.cmd == UUID & input->header.size == 0) {
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
