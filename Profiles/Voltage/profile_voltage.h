/******************************************************************************
 * @file Profile voltage
 * @brief voltage object
 * WARING : This h file should be only included by profile_*.c codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef PROFILE_VOLTAGE_H_
#define PROFILE_VOLTAGE_H_

#include "profile_core.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * Voltage data
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
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void ProfileVoltage_link(uint8_t, profile_voltage_t *);
service_t *ProfileVoltage_CreateService(profile_voltage_t *, SERVICE_CB, const char *, revision_t);

#endif /* PROFILE_VOLTAGE_H_ */
