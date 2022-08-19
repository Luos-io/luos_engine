/******************************************************************************
 * @file robus_utils
 * @brief Some tools used to debug
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos_utils.h"
#include "luos_engine.h"
#include "port_manager.h"
#include "string.h"
#include "luos_hal.h"
#include "msg_alloc.h"
#include "stdbool.h"

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief This function can be redefined by users to manage specific procedure on assert
 * @param file : File name as a string
 * @param line : line number
 * @return None
 ******************************************************************************/
__attribute__((weak)) void node_assert(char *file, uint32_t line)
{
    return;
}

/******************************************************************************
 * @brief Luos assertion management
 * @param file : File name as a string
 * @param line : Line number
 * @return None
 *  warning : this function can be redefined only for mock testing purpose
 ******************************************************************************/
_CRITICAL __attribute__((weak)) void Luos_assert(char *file, uint32_t line)
{
    // prepare a message as a node.
    // To do that we have to reset the service ID and clear PTP states to unlock others.
    PortMng_Init();
    // completely reinit the allocator
    MsgAlloc_Init(NULL);
    msg_t msg;
    msg.header.target_mode = BROADCAST;
    msg.header.target      = BROADCAST_VAL;
    msg.header.cmd         = ASSERT;
    msg.header.size        = sizeof(line) + strlen(file);
    memcpy(msg.data, &line, sizeof(line));
    memcpy(&msg.data[sizeof(line)], file, strlen(file));
    while (Luos_SendMsg(0, &msg) != SUCCEED)
        ;
    node_assert(file, line);
    // wait for the transmission to finish before killing IRQ
    while (MsgAlloc_TxAllComplete() == FAILED)
        ;
    LuosHAL_SetIrqState(false);
    while (1)
    {
    }
}
