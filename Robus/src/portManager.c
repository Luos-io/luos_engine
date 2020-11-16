/******************************************************************************
 * @file detection
 * @brief detection state machine.
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdbool.h>
#include "portManager.h"
#include "transmission.h"
#include "context.h"
#include "LuosHAL.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum
{
    POKE,
    RELEASE
} PortState_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/
PortState_t Port_ExpectedState = POKE;
/*******************************************************************************
 * Function
 ******************************************************************************/

static void PortMng_Reset(void);
/******************************************************************************
 * @brief init the portManager state machine
 * @param None
 * @return None
 ******************************************************************************/
void PortMng_Init(void)
{
    PortMng_Reset();
    // Reinit ll_container id
    for (uint8_t i = 0; i < ctx.ll_container_number; i++)
    {
        ctx.ll_container_table[i].id = DEFAULTID;
    }
    // Reinit port table
    for (uint8_t port = 0; port < NBR_PORT; port++)
    {
        ctx.node.port_table[port] = 0;
    }
}
/******************************************************************************
 * @brief PTP interrupt handler
 * @param port id
 * @return None
 ******************************************************************************/
void PortMng_PtpHandler(uint8_t PortNbr)
{
    if (Port_ExpectedState == RELEASE)
    {
        Port_ExpectedState = POKE;
        ctx.port.keepLine = false;
        // Check if every line have been poked and poke it if not
        for (uint8_t port = 0; port < NBR_PORT; port++)
        {
            if (ctx.node.port_table[port] == 0)
            {
                return;
            }
        }
        PortMng_Reset();
    }
    else if (Port_ExpectedState == POKE)
    {
        // we receive a poke, pull the line to notify your presence
        LuosHAL_PushPTP(PortNbr);
        ctx.port.activ = PortNbr;
    }
}
/******************************************************************************
 * @brief Poke
 * @param port id
 * @return None
 ******************************************************************************/
uint8_t PortMng_PokePort(uint8_t PortNbr)
{
    // push the ptp line
    LuosHAL_PushPTP(PortNbr);
    // wait a little just to be sure everyone can read it
    for (volatile uint16_t i = 0; i < TIMERVAL; i++)
        ;
    // release the ptp line
    LuosHAL_SetPTPDefaultState(PortNbr);
    for (volatile uint16_t i = 0; i < TIMERVAL; i++)
        ;
    // Save port as empty by default
    ctx.node.port_table[PortNbr] = 0xFFFF;
    // read the line state
    if (LuosHAL_GetPTPState(PortNbr))
    {
        // Someone reply, reverse the detection to wake up on line release
        LuosHAL_SetPTPReverseState(PortNbr);
        Port_ExpectedState = RELEASE;
        // Port poked by node
        ctx.port.activ = PortNbr;
        ctx.port.keepLine = true;
        return 1;
    }
    // Nobodies reply to our poke
    return 0;
}
/******************************************************************************
 * @brief detect the next module by poke ptp line
 * @param None
 * @return true if a port have been poke else false
 ******************************************************************************/
error_return_t PortMng_PokeNextPort(void)
{
    for (uint8_t port = 0; port < NBR_PORT; port++)
    {
        if (ctx.node.port_table[port] == 0)
        {
            // this port have not been poked
            if (PortMng_PokePort(port))
            {
                return SUCESS;
            }
            else
            {
                // nobody is here
                ctx.node.port_table[port] = 0xFFFF;
            }
        }
    }
    PortMng_Reset();
    return FAIL;
}
/******************************************************************************
 * @brief reinit the detection state machine
 * @param None
 * @return None
 ******************************************************************************/
void PortMng_Reset(void)
{
    ctx.port.keepLine = false;
    ctx.port.activ = NBR_PORT;
    Port_ExpectedState = POKE;
    // if it is finished reset all lines
    for (uint8_t port = 0; port < NBR_PORT; port++)
    {
        LuosHAL_SetPTPDefaultState(port);
    }
}
