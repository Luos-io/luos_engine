/******************************************************************************
 * @file template
 * @brief App x8
 * @author Luos
 * @version 1.0.0
 ******************************************************************************/
#include <stdio.h>
#include "context.h"
#include "unit_test.h"
#include "scenario_template.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
enum detection_machine_state
{
    NOT_DETECTED = 0,
    DETECT_FIRST_APP,
    DETECT_SECOND_APP,
    DETECTION_OK
};

typedef enum
{
    detection_TYPE = LUOS_LAST_STD_CMD,
    DUMMY_TYPE_1,
    DUMMY_TYPE_2,
    SCENARIO_template_LAST_TYPE
} scenario_template_type_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t detection_state = NO_DETECTION;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void App_Detection_Init(void);
static void dummy_App_1_Init(void);
static void dummy_App_2_Init(void);

static void launch_detection(void);

static void App_Detection_MsgHandler(service_t *service, msg_t *msg);
static void dummy_App_1_MsgHandler(service_t *service, msg_t *msg);
static void dummy_App_2_MsgHandler(service_t *service, msg_t *msg);

/******************************************************************************
 * @brief Launch a detection to detect local dummy services
 * @param None
 * @return None
 ******************************************************************************/
void launch_detection(void)
{
    uint16_t services_nb;
    uint32_t last_detection_date;

    services_nb = RoutingTB_GetServiceNB();
    printf("*** [INFO] %d services are active before detection\n", services_nb);

    last_detection_date = 0;
    printf("*** [INFO] Launch Detection\n");

    Luos_Detect(detect_app);
    Luos_Loop();

    // Verify all dummy services are created
    services_nb = RoutingTB_GetServiceNB();
    printf("*** [INFO] %d services are active\n", services_nb);
    TEST_ASSERT_EQUAL(DUMMY_SERVICE_NUMBER, services_nb);
}

void Scenario_template_Context_Init(int detection)
{
    uint32_t last_detection_date;
    uint16_t services_nb;

    // Init Luos Context
    Luos_Init();
    RoutingTB_Erase();

    // Init dummy Apps
    App_Detection_Init();
    dummy_App_1_Init();
    dummy_App_2_Init();

    if (detection == DETECTION)
    {
        launch_detection();
    }
}

/******************************************************************************
 * @brief Init Service App_Detection
 * @param None
 * @return None
 ******************************************************************************/
void App_Detection_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    detect_app          = Luos_CreateService(App_Detection_MsgHandler, detection_TYPE, "App_Detect", revision);
}

/******************************************************************************
 * @brief Init Service dummy_App_1
 * @param None
 * @return None
 ******************************************************************************/
void dummy_App_1_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    dummy_App_1         = Luos_CreateService(dummy_App_1_MsgHandler, DUMMY_TYPE_1, "dummy_App_1", revision);
}

/******************************************************************************
* @brief Init Service dummy_App_2
 * @param None
 * @return None
 ******************************************************************************/
void dummy_App_2_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    dummy_App_2         = Luos_CreateService(dummy_App_2_MsgHandler, DUMMY_TYPE_2, "dummy_App_2", revision);
}

/******************************************************************************
 * @brief Loop Service Detection
 * @param stop is set to 1 when detection scenario is finished
 * @return None
 ******************************************************************************/
void Scenario_template_Loop(int *stop)
{
    static uint32_t last_detection_time = 0;
    static uint8_t first_detection      = 0;
    uint16_t id;

    switch (detection_state)
    {
        case DETECTION_OK:
            *stop = 1;
            break;

        case NOT_DETECTED:
            if (first_detection == 0)
            {
                if (Luos_GetSystick() - last_detection_time > DETECTION_LATENCY)
                {
                    first_detection = 1;
                    printf("*** Launch Detection\n");
                    Luos_Detect(detect_app);
                }
            }
            break;

        case DETECT_FIRST_APP:
            id = RoutingTB_IDFromType(DUMMY_TYPE_1);
            if (id > 0)
            {
                msg_t msg;
                msg.header.source      = RoutingTB_IDFromType(detection_TYPE);
                msg.header.target      = id;
                msg.header.target_mode = IDACK;
                msg.header.cmd         = SET_CMD;
                msg.header.size        = sizeof(uint8_t);
                msg.data[0]            = 1;
                while (Luos_SendMsg(detect_app, &msg) != SUCCEED)
                {
                    Luos_Loop();
                }
            }
            break;

        case DETECT_SECOND_APP:
            id = RoutingTB_IDFromType(DUMMY_TYPE_2);

            if (id > 0)
            {
                msg_t msg;
                msg.header.source      = RoutingTB_IDFromType(detection_TYPE);
                msg.header.target      = id;
                msg.header.target_mode = IDACK;
                msg.header.cmd         = SET_CMD;
                msg.header.size        = sizeof(uint8_t);
                msg.data[0]            = 2;
                while (Luos_SendMsg(detect_app, &msg) != SUCCEED)
                {
                    Luos_Loop();
                }
            }
            break;

        default:
            break;
    }
}

/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void App_Detection_MsgHandler(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == END_DETECTION)
    {
        printf("[ Rx ] - [APP Detection]  - End detection\n");
        detection_state = DETECT_FIRST_APP;
    }
    //printf("*** MsgHandler - APP detect: %d\n", msg->header.cmd);
}

/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void dummy_App_1_MsgHandler(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == SET_CMD)
    {
        printf("[ Rx ] - [Dummy App 1]  - Receive data: %d\n", msg->data[0]);
        detection_state = DETECT_SECOND_APP;
    }
    //printf("*** MsgHandler - APP 1: %d\n", msg->header.cmd);
}

/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void dummy_App_2_MsgHandler(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == SET_CMD)
    {
        printf("[ Rx ] - [Dummy App 2]  - Receive data: %d\n", msg->data[0]);
        detection_state = DETECTION_OK;
    }
    //printf("*** MsgHandler - APP 2: %d\n", msg->header.cmd);
}
