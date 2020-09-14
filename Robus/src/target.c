/*
 * target.c
 *
 * Created: 14/02/2017 11:53:28
 *  Author: Nicolas Rabault
 *  Abstract: manage multicast address bank
 */
#include "target.h"

unsigned char multicast_target_bank(vm_t *vm, unsigned short val)
{
    unsigned char i;
    for (i = 0; i < vm->max_multicast_target; i++)
    {
        if (vm->multicast_target_bank[i] == val)
            return TRUE;
    }
    return FALSE;
}

void add_multicast_target(vm_t *vm, unsigned short target)
{
    vm->multicast_target_bank[vm->max_multicast_target++] = target;
}
