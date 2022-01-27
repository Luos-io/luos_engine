#ifndef SCENARIO_TEMPLATE_H
#define SCENARIO_TEMPLATE_H

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DETECTION_LATENCY    500 //ms
#define DUMMY_SERVICE_NUMBER 3

enum detection_activation
{
    NO_DETECTION = 0,
    DETECTION
};

/*******************************************************************************
 * Variables
 ******************************************************************************/
service_t *detect_app;
service_t *dummy_App_1;
service_t *dummy_App_2;

/*******************************************************************************
 * Function
 ******************************************************************************/
void Scenario_template_Context_Init(int detection);
void Scenario_template_Loop(int *stop);

#endif //SCENARIO_TEMPLATE_H