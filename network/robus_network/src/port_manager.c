/******************************************************************************
 * @file detection
 * @brief detection state machine.
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

/**** Description of the PTP process **************
 *              +------------+ no      +----+
 *              | Node Start +-------->+Wait|
 *              |  detection |         |Poke|
 *              +------------+         +--+-+
 *                    |yes                |
 *                    v                   |
 *              +-----+-----+             |
 *+------+      |           +<------------+
 *|UnPoke|    no|    PTP    |
 *|  PTP +<-----+ Remaining +<-----+
 *+------+      |           |      |
 *              |           +<--+  |
 *              +-----------+   |  |
 *                    |yes      |  |
 *                    v         |  |
 *                +---+---+     |  |
 *                | Poke  | no  |  |
 *                |   PTP +-----+  |
 *                +-------+        |
 *                    |yes         |
 *                    v            |
 *                 +--+---+        |
 *                 | Wait |        |
 *                 |UnPoke+--------+
 *                 +------+
 ***************************************************/

#include <stdbool.h>
#include "port_manager.h"
#include "transmission.h"
#include "context.h"
#include "robus_hal.h"
#include "luos_hal.h"

#ifdef LUOS_DEBUG_PRINT
    #include <stdio.h>
    #define ROBUS_DBG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define ROBUS_DBG(fmt, ...) ((void)0)
#endif

#ifndef PTP_PUSH_DELAY_MS
    #define PTP_PUSH_DELAY_MS 2
#endif
#ifndef PTP_READ_DELAY_MS
    #define PTP_READ_DELAY_MS 3
#endif
#ifdef NORT
    #ifndef PTP_RELEASE_TIMEOUT_MS
        #define PTP_RELEASE_TIMEOUT_MS 500
    #endif
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum
{
    POKE,
    RELEASE,
    PUSHING
} PortState_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile PortState_t Port_ExpectedState = POKE;
uint32_t port_detected         = 0;
#ifdef NORT
static uint32_t ptp_release_enter_tick = 0;
#endif
/*******************************************************************************
 * Function
 ******************************************************************************/
_CRITICAL static void PortMng_Reset(void);
uint8_t PortMng_PokePort(uint8_t PortNbr);
/******************************************************************************
 * @brief init the portManager state machine
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void PortMng_Init(void)
{
    PortMng_Reset();
}
/******************************************************************************
 * @brief PTP interrupt handler
 * @param port id
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void PortMng_PtpHandler(uint8_t PortNbr)
{
    ROBUS_DBG("[PTP] Handler port=%d\n", PortNbr);
    if (Port_ExpectedState == RELEASE)
    {
        Port_ExpectedState = POKE;
        ctx.port.keepLine  = false;
        // Check if every line have been poked and poke it if not
        if (port_detected == (1 << NBR_PORT) - 1)
        {
            // All ports have been detected
            // We can consider our phy as done. Notify it to the phy manager
            Phy_TopologyDone(Robus_GetPhy());
            // Reset the port state
            PortMng_Reset();
        }
        // Ask Luos_phy to find another node
        Phy_TopologyNext();
    }
    else if (Port_ExpectedState == POKE)
    {
        // We just received a poke
        // Pull the line to notify your presence
        RobusHAL_PushPTP(PortNbr);
        // Save this port as detected
        port_detected |= 1 << PortNbr;
        ctx.port.activ = PortNbr;
        // This port become the topology source of this node
        // Notify luos_phy about it
        Phy_TopologySource(Robus_GetPhy(), PortNbr);
    }
}
/******************************************************************************
 * @brief Poke a PTP to detect next node on the line
 * @param port id
 * @return true if node reply else false
 ******************************************************************************/
uint8_t PortMng_PokePort(uint8_t PortNbr)
{
    Port_ExpectedState = PUSHING;
    // We can consider this port as detected
    port_detected |= 1 << PortNbr;
    // Push the ptp line
    RobusHAL_PushPTP(PortNbr);
    // Wait a little just to be sure everyone can read it
    uint32_t start_tick = LuosHAL_GetSystick();
    while (LuosHAL_GetSystick() - start_tick < PTP_PUSH_DELAY_MS)
        ;
    // Release the ptp line
    RobusHAL_SetPTPDefaultState(PortNbr);
    while (LuosHAL_GetSystick() - start_tick < PTP_READ_DELAY_MS)
        ;
    // Read the line state
    if (RobusHAL_GetPTPState(PortNbr))
    {
        // Someone reply, reverse the detection to wake up on release condition
        RobusHAL_SetPTPReverseState(PortNbr);
        Port_ExpectedState     = RELEASE;
#ifdef NORT
        ptp_release_enter_tick = LuosHAL_GetSystick();
#endif
        // Port poked by node
        ctx.port.activ    = PortNbr;
        ctx.port.keepLine = true;
        return 1;
    }
    Port_ExpectedState = POKE;
    // Nobodies reply to our poke
    return 0;
}

/******************************************************************************
 * @brief being poked so poke next node to
 * @param portId pointer to the port id
 * @return true if a port have been poke else false
 ******************************************************************************/
error_return_t PortMng_PokeNextPort(uint8_t *portId)
{
    for (uint8_t port = 0; port < NBR_PORT; port++)
    {
        if (!(port_detected & (1 << port)))
        {
            // This port have not been poked
            ROBUS_DBG("[PTP] Poking port %d\n", port);
            if (PortMng_PokePort(port))
            {
                // Poke succeed, we have a node here, return the port id
                *portId = port;
                return SUCCEED;
            }
            // Poke failed, consider this port as detected and continue
            port_detected |= 1 << port;
        }
    }
    // We check all port and no one reply, we can consider our phy detection as done.
    // Notify it to the phy manager
    Phy_TopologyDone(Robus_GetPhy());
    PortMng_Reset();
    return FAILED;
}
#ifdef NORT
/******************************************************************************
 * @brief Check for stuck PTP detection and recover
 * @param None
 * @return None
 ******************************************************************************/
void PortMng_WatchdogCheck(void)
{
    if (Port_ExpectedState == RELEASE
        && (LuosHAL_GetSystick() - ptp_release_enter_tick > PTP_RELEASE_TIMEOUT_MS))
    {
        Port_ExpectedState = POKE;
        ctx.port.keepLine  = false;
        Phy_TopologyDone(Robus_GetPhy());
        PortMng_Reset();
        Phy_TopologyNext();
    }
}
#endif
/******************************************************************************
 * @brief reinit the detection state machine
 * @param None
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void PortMng_Reset(void)
{
    ctx.port.keepLine  = false;
    ctx.port.activ     = NBR_PORT;
    Port_ExpectedState = POKE;
    // if it is finished reset all lines
    for (uint8_t port = 0; port < NBR_PORT; port++)
    {
        RobusHAL_SetPTPDefaultState(port);
    }
    port_detected = 0;
}

bool PortMng_Busy(void)
{
    return ctx.port.keepLine;
}
