/******************************************************************************
 * @file robus_utils
 * @brief some tools used to debug
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos_utils.h"
#include "luos.h"
#include "port_manager.h"
#include "string.h"
#include "luos_hal.h"

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief This function can be redefine by users to manage specifi procedure on assert
 * @param file name as a string
 * @param line number
 * @return None
 ******************************************************************************/
__attribute__((weak)) void node_assert(char *file, uint32_t line)
{
    return;
}

/******************************************************************************
 * @brief Luos assertion management
 * @param file name as a string
 * @param line number
 * @return None
 ******************************************************************************/
void Luos_assert(char *file, uint32_t line)
{
    // prepare a message as a node.
    // To do that we have to reset the container ID and clear PTP states to unlock others.
    PortMng_Init();
    msg_t msg;
    msg.header.target_mode = BROADCAST;
    msg.header.target = BROADCAST_VAL;
    msg.header.cmd = ASSERT;
    msg.header.size = sizeof(line) + strlen(file);
    memcpy(msg.data, &line, sizeof(line));
    memcpy(&msg.data[sizeof(line)], file, strlen(file));
    Luos_SendMsg(0, &msg);
    node_assert(file, line);
    LuosHAL_SetIrqState(FALSE);
    while (1)
    {
    }
}