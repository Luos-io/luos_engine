/******************************************************************************
 * @file target
 * @brief multicast protocole description
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "target.h"
#include "stdbool.h"
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
uint8_t Trgt_MulticastTargetBank(ll_container_t *ll_container, uint16_t val)
{
    unsigned char i;
    for (i = 0; i < ll_container->max_multicast_target; i++)
    {
        if (ll_container->multicast_target_bank[i] == val)
            return true;
    }
    return false;
}
/******************************************************************************
 * @brief add a target to the bank
 * @param container in multicast
 * @param target to add
 * @return Error
 ******************************************************************************/
void Trgt_AddMulticastTarget(ll_container_t *ll_container, uint16_t target)
{
    ll_container->multicast_target_bank[ll_container->max_multicast_target++] = target;
}
