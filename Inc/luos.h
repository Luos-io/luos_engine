#ifndef LUOS_H
#define LUOS_H

#include "module_list.h"
#include "module_structs.h"
#include "robus.h"

// Luos unic ID => ARM serial number
typedef struct __attribute__((__packed__)){
    union {
        uint32_t uuid[3];
        uint8_t unmap[3 * sizeof(uint32_t)];                /*!< Uncmaped form. */
    };
}luos_uuid_t;

int luos_msg_handler(vm_t* vm, msg_t* input, msg_t* output);

#endif /* LUOS_H */
