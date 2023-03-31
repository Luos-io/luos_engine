#ifndef DEFAULT_SCENARIO_H
#define DEFAULT_SCENARIO_H

#include "unit_test.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    service_t *app;
    msg_t last_rx_msg;
} dummy_app_t;

typedef struct
{
    dummy_app_t App_1;
    dummy_app_t App_2;
    dummy_app_t App_3;
} default_scenario_t;

typedef enum
{
    DEFAULT_CMD = LUOS_LAST_STD_CMD // a dummy command for apps
} default_scenario_cmd_t;

/*******************************************************************************
 * Function
 ******************************************************************************/
void Init_Context(void);

#endif // DEFAULT_SCENARIO_H
