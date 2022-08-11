/******************************************************************************
 * @file Mock_RoutingTB_WaitRoutingTable.c
 * @brief Mock function RoutingTB_WaitRoutingTable()
 * @author Luos
 * @version 1.0.0
 ******************************************************************************/
#ifdef UNIT_TEST
#include "unit_test.h"
#else
#include "luos_hal.h"
#endif
#include "service_structs.h"

/*******************************************************************************
 * Function
 ******************************************************************************/
/******************************************************************************
 * @brief Mock : Redefine function RoutingTB_WaitRoutingTable
 * @param service receive
 * @param intro msg in route table
 * @return None
 ******************************************************************************/
bool RoutingTB_WaitRoutingTable(service_t *service, msg_t *intro_msg)
{
    const uint8_t timeout = 150; // timeout in ms
    Luos_SendMsg(service, intro_msg);
    uint32_t timestamp = LuosHAL_GetSystick();
    Luos_Loop();
    while ((LuosHAL_GetSystick() - timestamp) < timeout)
        ;
    Luos_Loop();
    return true;
}