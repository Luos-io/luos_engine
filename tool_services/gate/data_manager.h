/******************************************************************************
 * @file data_manager
 * @brief Manage data conversion strategy.
 * @author Luos
 ******************************************************************************/
#ifndef DATA_MNGR_H
#define DATA_MNGR_H

#include "luos_engine.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

// Define the different states of the Gate
typedef enum
{
    NOT_RUNNING, // No user ask the gate to run.
    PREPARING,   // A user ask the gate to run.
    RUNNING      // user ask the gate to run and everything is ready to do.
} gate_state_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern time_luos_t update_time;
extern volatile gate_state_t gate_running;
#ifndef GATE_POLLING
extern volatile bool first_conversion;
#endif

/*******************************************************************************
 * Function
 ******************************************************************************/

// This function will manage msg collection from sensors
void DataManager_collect(service_t *service);

// This function manage entirely data conversion
void DataManager_Run(service_t *service);

// This function manage only commande incoming from pipe
void DataManager_RunPipeOnly(service_t *service);

#endif /* DATA_MNGR_H */
