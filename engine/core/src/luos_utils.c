/******************************************************************************
 * @file luos_utils
 * @brief Some tools used to debug
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdbool.h>
#include "luos_utils.h"
#include "luos_engine.h"
#include "port_manager.h"
#include "string.h"
#include "luos_hal.h"
#include "msg_alloc.h"
#include "filter.h"
#include "luos_phy.h"

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
 * @brief Jump to bootloader by restarting the MCU
 * @param None
 * @return None
 ******************************************************************************/
void Luos_JumpToBootloader(void)
{
    // Set bootlaoder mode
    LuosHAL_SetMode((uint8_t)APP_RELOAD_MODE);

    // Save the current node id in flash to be ready to be reloaded
    LuosHAL_SaveNodeID(Node_Get()->node_id);

    // Reset the MCU
    LuosHAL_Reboot();
}

#ifndef UNIT_TEST
/******************************************************************************
 * @brief Luos assertion management
 * @param file : File name as a string
 * @param line : Line number
 * @return None
 *  warning : this function can be redefined only for mock testing purpose
 ******************************************************************************/
_CRITICAL void Luos_assert(char *file, uint32_t line)
{
    // prepare a message as a node.
    // To do that we have to reset the service ID and clear PTP states to unlock others.
    Luos_Init();
    // completely reinit the allocator
    MsgAlloc_Init(NULL);
    Filter_IdInit(); // Mask filter for service ID
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
    while (Luos_TxComplete() == FAILED)
        ;
    #ifdef WITH_BOOTLOADER
    // We're in a failed app,
    // Restart this node in bootloader mode instead of don't do anything
    // We will come back on this app after a reboot.
    // Set bootloader mode, save node ID and reboot
    Luos_JumpToBootloader();
    #endif
    Phy_SetIrqState(false);
    while (1)
    {
    }
}
#endif
