/******************************************************************************
 * @file Struct state
 * @brief state data structure definition
 * WARING : This h file should be only included by profile_*.h or template_*.h codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_STRUCT_VOLTAGE_H_
#define TEMPLATE_STRUCT_VOLTAGE_H_

#include "luos.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * State data
 */
typedef struct
{
    access_t access;
    voltage_t voltage;

    //streaming
    control_t control;
    streaming_channel_t signal;
    time_luos_t sampling_period;
} profile_voltage_t;

#endif /* TEMPLATE_STRUCT_VOLTAGE_H_ */
