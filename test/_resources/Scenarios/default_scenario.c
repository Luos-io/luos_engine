/******************************************************************************
 * @file template
 * @brief App x8
 * @author Luos
 * @version 1.0.0
 ******************************************************************************/
#include <stdio.h>
#include "default_scenario.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
default_scenario_t default_sc;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Detection(service_t *service);
static void App_1_MsgHandler(service_t *service, msg_t *msg);
static void App_2_MsgHandler(service_t *service, msg_t *msg);
static void App_3_MsgHandler(service_t *service, msg_t *msg);

/******************************************************************************
 * @brief Init scenario
 * @param None
 * @return None
 ******************************************************************************/
void Init_Context(void)
{
    // Reset context
    RESET_ASSERT();
    Luos_ServicesClear();
    RoutingTB_Erase(); // Delete RTB
    Luos_Init();
    if (IS_ASSERT())
    {
        printf("[FATAL] Can't reset scenario context\n");
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
    Luos_Loop();

    RESET_ASSERT();
    Luos_Init();

    // Create services
    revision_t revision  = {.major = 1, .minor = 0, .build = 0};
    default_sc.App_1.app = Luos_CreateService(App_1_MsgHandler, VOID_TYPE, "Dummy_App_1", revision);
    default_sc.App_2.app = Luos_CreateService(App_2_MsgHandler, VOID_TYPE, "Dummy_App_2", revision);
    default_sc.App_3.app = Luos_CreateService(App_3_MsgHandler, VOID_TYPE, "Dummy_App_3", revision);

    // Detection
    Detection(default_sc.App_1.app);
    Luos_Loop();

    if (IS_ASSERT())
    {
        printf("[FATAL] Can't initialize scenario context\n");
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

/******************************************************************************
 * @brief Launch a detection
 * @param Service who launches the detection
 * @return None
 ******************************************************************************/
static void Detection(service_t *service)
{
    search_result_t result;

    Luos_Detect(service);
    do
    {
        Luos_Loop();
    } while (!Luos_IsDetected());

    RTFilter_Reset(&result);
    printf("[INFO] %d services are active\n", result.result_nbr);
    TEST_ASSERT_EQUAL(3, result.result_nbr);
}

/******************************************************************************
 * @brief Msg Handler callback : save last message
 * @param service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void App_1_MsgHandler(service_t *service, msg_t *msg)
{
    memcpy(&default_sc.App_1.last_rx_msg, msg, sizeof(msg_t));
}

/******************************************************************************
 * @brief Msg Handler callback : save last message
 * @param service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void App_2_MsgHandler(service_t *service, msg_t *msg)
{
    memcpy(&default_sc.App_2.last_rx_msg, msg, sizeof(msg_t));
}

/******************************************************************************
 * @brief Msg Handler callback : save last message
 * @param service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void App_3_MsgHandler(service_t *service, msg_t *msg)
{
    memcpy(&default_sc.App_3.last_rx_msg, msg, sizeof(msg_t));
}
