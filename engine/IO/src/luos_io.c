/******************************************************************************
 * @file luos_io.c
 * @brief Interface file between Luos and a physical layer
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include <luos_io.h>
#include "transmission.h"
#include "reception.h"
#include "msg_alloc.h"
#include "robus.h"
#include "service.h"
#include "filter.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/

/******************************************************************************
 * @brief Init the interface file.
 * @param None
 * @return None
 ******************************************************************************/
void LuosIO_Init(memory_stats_t *memory_stats)
{
    // Init filter
    Filter_IdInit();
    Filter_TopicInit();

    // Init transmission
    Transmit_Init();

    // Init reception
    Recep_Init();

    // Clear message allocation buffer table
    MsgAlloc_Init(memory_stats);

    // Init Robus
    Robus_Init();
}

/******************************************************************************
 * @brief Loop of the IO level
 * @param None
 * @return None
 ******************************************************************************/
void LuosIO_Loop(void)
{
    // Execute message allocation tasks
    MsgAlloc_loop();
    // Interpreat received messages and create luos task for it.
    msg_t *msg;
    while (MsgAlloc_PullMsgToInterpret(&msg) == SUCCEED)
    {
        // Check if this message is a protocol one
        if (Robus_MsgHandler(msg) == FAILED)
        {
            // If not create luos tasks for all services.
            Service_AllocMsg(msg);
        }
    }
    Robus_Loop();
}
