/******************************************************************************
 * @file robusHAL
 * @brief Robus Hardware Abstration Layer. Describe Low layer fonction
 * @Family x86/Linux/Mac
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "ws_hal.h"
#include "_ws_network.h"
#include "luos_utils.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static const char *s_url = WS_NETWORK_BROKER_ADDR;

/*******************************************************************************
 * Function
 ******************************************************************************/

/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void WsHAL_Init(void)
{

    // Create a client WebSocket instance to s_url
}

/******************************************************************************
 * @brief Luos HAL general loop
 * @param None
 * @return None
 ******************************************************************************/
void WsHAL_Loop(void)
{
    // Get the messages and put them on the reception function
    // Ws_Reception(uint8_t *msg, (uint32_t)messageSize);
    // If you have a callback you can create a callback instead of this function and call it here
}

/******************************************************************************
 * @brief Transmit data
 * @param data to send
 * @param size of data to send
 * @return None
 ******************************************************************************/
void WsHAL_Send(const uint8_t *data, uint16_t size)
{
    // Send data to the websocket in binary mode
}
