/******************************************************************************
 * @file Profile voltage
 * @brief voltage object
 * WARING : This h file should be only included by profile_*.c codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_CLASS_VOLTAGE_H_
#define TEMPLATE_CLASS_VOLTAGE_H_

#include "struct_voltage.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

void ProfileVoltage_Handler(service_t *service, msg_t *msg, profile_voltage_t *voltage_profile);

#endif /* TEMPLATE_CLASS_VOLTAGE_H_ */
