#ifndef DEFAULT_SCENARIO_H
#define DEFAULT_SCENARIO_H

#include "unit_test.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DUMMY_SERVICE_NUMBER           3
#define DUMMY_STREAMING_CHANNEL_NUMBER 3
#define STREAM_BUFFER_SIZE             1024

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
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void Reset_Context(void);
void Init_Context(void);
void App_1_Loop(void);
void App_2_Loop(void);
void App_3_Loop(void);

#endif // DEFAULT_SCENARIO_H
