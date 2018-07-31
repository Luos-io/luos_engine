#ifndef LUOS_H
#define LUOS_H

#include "module_list.h"
#include "module_structs.h"
#include "robus.h"

int luos_msg_handler(vm_t* vm, msg_t* input, msg_t* output);

#endif /* LUOS_H */
