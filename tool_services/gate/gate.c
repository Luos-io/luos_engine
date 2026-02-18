/******************************************************************************
 * @file gate
 * @brief Service gate
 * @author Luos
 ******************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include "gate_config.h"
#include "gate.h"
#include "data_manager.h"
#include "convert.h"
#include "pipe_link.h"
#include "routing_table.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
static void Gate_msgHandler(service_t *service, const msg_t *msg);

/*******************************************************************************
 * Variables
 ******************************************************************************/
service_t *gate;
volatile gate_state_t gate_running = NOT_RUNNING;
volatile bool first_conversion     = false;
uint32_t LastReceptionDate         = 0;

time_luos_t update_time = {GATE_REFRESH_TIME_S};
/*******************************************************************************
 * Function
 ******************************************************************************/
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Gate_Init(void)
{
    revision_t revision = {.major = 2, .minor = 0, .build = 0};
    gate                = Luos_CreateService(Gate_msgHandler, GATE_TYPE, "gate", revision);
#ifndef NODETECTION
    uint32_t init_timer = Luos_GetSystick();
    while (Luos_GetSystick() - init_timer < INIT_TIME)
        ;
    Luos_Detect(gate);
#endif
}

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Gate_Loop(void)
{
    // We don't receive anything.
    // After 1s void reception send void data allowing client to send commands (because client could be synchronized to reception).
    if (LastReceptionDate == 0)
    {
        LastReceptionDate = Luos_GetSystick();
    }
}

void Gate_msgHandler(service_t *service, const msg_t *msg)
{
    // Check the detection status.
    if (Luos_IsDetected() == false)
    {
        update_time       = TimeOD_TimeFrom_s(GATE_REFRESH_TIME_S);
        LastReceptionDate = 0;
    }
    else
    {
        // Network have been detected, We are good to go
        LastReceptionDate = Luos_GetSystick();
        if (gate_running == RUNNING)
        {
            // Manage input and output data
            DataManager_Run(service, msg);
            if (first_conversion == true)
            {
                // This is the first time we perform a convertion
                // Update refresh rate for all services of the network
                DataManager_collect(service);
                first_conversion = false;
            }
        }
        else
        {
            DataManager_RunPipeOnly(service, msg);
        }
    }
}
