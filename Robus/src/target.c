/******************************************************************************
 * @file target
 * @brief multicast protocole description
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "target.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief lookink for a bank of target in multicast mode
 * @param container in multicast
 * @param multicast bank
 * @return Error
 ******************************************************************************/
uint8_t Trgt_MulticastTargetBank(vm_t *vm, unsigned short val)
{
    unsigned char i;
    for (i = 0; i < vm->max_multicast_target; i++)
    {
        if (vm->multicast_target_bank[i] == val)
            return TRUE;
    }
    return FALSE;
}
/******************************************************************************
 * @brief add a target to the bank
 * @param container in multicast
 * @param target to add
 * @return Error
 ******************************************************************************/
void Trgt_AddMulticastTarget(vm_t *vm, unsigned short target)
{
    vm->multicast_target_bank[vm->max_multicast_target++] = target;
}
