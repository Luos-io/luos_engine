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

/*******************************************************************************
 * Variables
 ******************************************************************************/
service_t *gate;
volatile gate_state_t gate_running = NOT_RUNNING;
#ifndef GATE_POLLING
volatile bool first_conversion = false;
#endif

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
    revision_t revision = {.major = 1, .minor = 0, .build = 1};
    gate                = Luos_CreateService(0, GATE_TYPE, "gate", revision);
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
    static uint32_t last_time = 0;

    // Check the detection status.
    if (Luos_IsDetected() == false)
    {
#ifndef GATE_POLLING
        update_time = TimeOD_TimeFrom_s(GATE_REFRESH_TIME_S);
#endif
    }
    else
    {
        // Network have been detected, We are good to go
        if (gate_running == RUNNING)
        {
            // Manage input and output data
            if ((Luos_GetSystick() - last_time >= TimeOD_TimeTo_ms(update_time)) && (Luos_GetSystick() > last_time))
            {
                last_time = Luos_GetSystick();
                DataManager_Run(gate);
#ifndef GATE_POLLING
                if (first_conversion == true)
                {
                    // This is the first time we perform a convertion
    #ifdef GATE_REFRESH_AUTOSCALE
                    // Evaluate the time needed to convert all the data of this configuration and update refresh rate
                    search_result_t result;
                    RTFilter_Reset(&result);
                    // find the biggest id
                    if (result.result_table[result.result_nbr - 1]->id)
                    {
                        // update time is related to the biggest id
                        update_time = TimeOD_TimeFrom_s((float)result.result_table[result.result_nbr - 1]->id * 0.001);
                    }
                    else
                    {
                        update_time = TimeOD_TimeFrom_s(GATE_REFRESH_TIME_S);
                    }
    #endif
                    // Update refresh rate for all services of the network
                    DataManager_collect(gate);
                    first_conversion = false;
                }
#endif
            }
        }
        else
        {
            DataManager_RunPipeOnly(gate);
        }
    }
}
