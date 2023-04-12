/******************************************************************************
 * @file robus
 * @brief User functionalities of the robus communication protocol
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <robus.h>

#include <string.h>
#include <stdbool.h>
#include "transmission.h"
#include "reception.h"
#include "port_manager.h"
#include "context.h"
#include "robus_hal.h"
#include "luos_hal.h"
#include "luos_utils.h"

#include "msg_alloc.h"
#include "luos_engine.h"
#include "filter.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
// Creation of the robus context. This variable is used in all files of this lib.
volatile context_t ctx;
luos_phy_t *phy_robus;

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Initialisation of the Robus communication protocole
 * @param None
 * @return None
 ******************************************************************************/
void Robus_Init(void)
{
    // Init hal
    RobusHAL_Init();

    // Init port structure
    PortMng_Init();

    // Init transmission
    Transmit_Init();

    // Instantiate the phy struct
    phy_robus = Phy_Create();
    LUOS_ASSERT(phy_robus);

    // Init reception
    Recep_Init(phy_robus);
}

/******************************************************************************
 * @brief Loop of the Robus communication protocole
 * @param None
 * @return None
 ******************************************************************************/
void Robus_Loop(void)
{
    RobusHAL_Loop();
}

/******************************************************************************
 * @brief Formalize message Set tx task and send
 * @param service to send
 * @param msg to send
 * @return error_return_t
 ******************************************************************************/
error_return_t Robus_SetTxTask(service_t *service, msg_t *msg)
{
    error_return_t error = SUCCEED;
    uint8_t ack          = 0;
    uint16_t data_size   = 0;
    uint16_t crc_val     = 0xFFFF;
    // ***************************************************
    // don't send luos messages if network is down
    // ***************************************************
    if ((msg->header.cmd >= LUOS_LAST_RESERVED_CMD) && (Node_GetState() != DETECTION_OK))
    {
        return PROHIBITED;
    }

    // Compute the full message size based on the header size info.
    if (msg->header.size > MAX_DATA_MSG_SIZE)
    {
        data_size = MAX_DATA_MSG_SIZE;
    }
    else
    {
        data_size = msg->header.size;
    }

    // Check the localhost situation
    luos_localhost_t localhost = Filter_GetLocalhost(&msg->header);
    // Add the CRC to the total size of the message
    uint16_t full_size = sizeof(header_t) + data_size + CRC_SIZE;

    uint16_t crc_max_index = full_size;

    if (Luos_IsMsgTimstamped(msg) == true)
    {
        full_size += sizeof(time_luos_t);
    }
    // Compute the CRC
    crc_val = ll_crc_compute(&msg->stream[0], crc_max_index - CRC_SIZE, 0xFFFF);

    // Check if ACK needed
    if (((msg->header.target_mode == SERVICEIDACK) || (msg->header.target_mode == NODEIDACK)) && (localhost != EXTERNALHOST))
    {
        // This is a localhost message and we need to transmit a ack. Add it at the end of the data to transmit
        ack = ctx.rx.status.unmap;
        full_size++;
    }

    // ********** Allocate the message ********************
    if (MsgAlloc_SetTxTask(service, (uint8_t *)msg->stream, crc_val, full_size, localhost, ack) == FAILED)
    {
        error = FAILED;
    }
// **********Try to send the message********************
#ifndef VERBOSE_LOCALHOST
    if (localhost != LOCALHOST)
    {
#endif
        Transmit_Process();
#ifndef VERBOSE_LOCALHOST
    }
#endif
    return error;
}

/******************************************************************************
 * @brief Save a node id in the port table
 * @param nodeid to save
 * @return None.
 ******************************************************************************/
void Robus_SaveNodeID(uint16_t nodeid)
{
    PortMng_SaveNodeID(nodeid);
}

/******************************************************************************
 * @brief Reset the node id of the port table
 * @param None
 * @return None.
 ******************************************************************************/
void Robus_ResetNodeID(void)
{
    PortMng_Init();
}

/******************************************************************************
 * @brief Is Robus busy
 * @param None
 * @return nodeid.
 ******************************************************************************/
bool Robus_Busy(void)
{
    return PortMng_Busy();
}

/******************************************************************************
 * @brief Find the next neighbour on this phy
 * @param None
 * @return error_return_t
 ******************************************************************************/
error_return_t Robus_FindNeighbour(void)
{
    return PortMng_PokeNextPort();
}

/******************************************************************************
 * @brief add the phy pointer to the callback
 * @param pointer to the received data
 * @return None
 ******************************************************************************/
_CRITICAL void Recep_data(volatile uint8_t *data)
{
    ctx.rx.callback(phy_robus, data);
}

/******************************************************************************
 * @brief Get the phy pointer
 * @return luos_phy_t*
 ******************************************************************************/
luos_phy_t *Robus_GetPhy(void)
{
    return phy_robus;
}
