/******************************************************************************
 * @file robus
 * @brief User functionalities of the robus communication protocol
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <string.h>
#include <stdbool.h>
#include "robus.h"
#include "transmission.h"
#include "reception.h"
#include "port_manager.h"
#include "context.h"
#include "robus_hal.h"
#include "robus_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
// Phy callback definitions
static void Robus_JobHandler(luos_phy_t *phy_ptr, phy_job_t *job);
static error_return_t Robus_RunTopology(luos_phy_t *phy_ptr, uint8_t *portId);
static void Robus_Reset(luos_phy_t *phy_ptr);

/*******************************************************************************
 * Variables
 ******************************************************************************/
// Creation of the robus context. This variable is used in all files of this lib.
volatile context_t ctx;
luos_phy_t *phy_robus;
robus_encaps_t encaps[MAX_MSG_NB]; // Store all the CRC for each msg to transmit.

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
    phy_robus = Phy_Create(Robus_JobHandler, Robus_RunTopology, Robus_Reset);
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
 * @brief Robus message handler
 * @param phy_ptr
 * @param job
 * @return None
 ******************************************************************************/
void Robus_JobHandler(luos_phy_t *phy_ptr, phy_job_t *job)
{
    static uint8_t encaps_index = 0;
    // Luos ask Robus to send a message

    // Compute the CRC and create the encapsulation context
    encaps[encaps_index].crc  = ll_crc_compute(job->data_pt, job->size, 0xFFFF);
    encaps[encaps_index].size = CRC_SIZE;

    // Save the precomputed encapsulation in the job
    LuosHAL_SetIrqState(false);
    job->phy_data = (void *)&encaps[encaps_index];
    LuosHAL_SetIrqState(true);

    // Get the next encapsulation index for the next job
    encaps_index++;
    if (encaps_index >= MAX_MSG_NB)
    {
        encaps_index = 0;
    }

    // Then, try to directly transmit... Who knows perhaps the line is free
    Transmit_Process();
}

/******************************************************************************
 * @brief Reset the node id of the port table
 * @param phy_ptr not used
 * @return None.
 ******************************************************************************/
void Robus_Reset(luos_phy_t *phy_ptr)
{
    PortMng_Init();
    Recep_Reset();
    Transmit_Init();
}

/******************************************************************************
 * @brief Find the next neighbour on this phy
 * @param None
 * @return error_return_t
 ******************************************************************************/
error_return_t Robus_RunTopology(luos_phy_t *phy_ptr, uint8_t *portId)
{
    return PortMng_PokeNextPort(portId);
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
