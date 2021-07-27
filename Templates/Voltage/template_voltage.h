/******************************************************************************
 * @file Template voltage
 * @brief voltage service template
 * WARING : This h file should be only included by user code or profile_*.h codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_TEMPLATE_VOLTAGE_H_
#define TEMPLATE_TEMPLATE_VOLTAGE_H_

#include <stdbool.h>
#include "luos.h"
#include "struct_voltage.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * voltage object
 */
typedef struct
{
    SERVICE_CB self;
    profile_voltage_t profile;
} template_voltage_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

service_t *TemplateVoltage_CreateService(SERVICE_CB service_cb, template_voltage_t *var, const char *alias, revision_t revision);

#endif /* TEMPLATE_TEMPLATE_VOLTAGE_H_ */
