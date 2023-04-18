/******************************************************************************
 * @file start controller
 * @brief application example a start controller
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "detection_button.h"
#include "product_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define UPDATE_PERIOD_MS          20
#define MIN_TIME_BETWEEN_DETEC_MS 300

/*******************************************************************************
 * Variables
 ******************************************************************************/
service_t *app;
uint32_t last_detection_date_ms = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void DetectionButton_MsgHandler(service_t *service, const msg_t *msg);
static void Setup_button(void);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void DetectionButton_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    // Create App
    app = Luos_CreateService(DetectionButton_MsgHandler, DETECTION_BUTTON_APP, "detection_button", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void DetectionButton_Loop(void)
{
    static int previous_id = -1;

    // ********** hot plug management ************
    // Check if we have done the first init or if service Id have changed
    if (Luos_IsDetected() == false)
    {
        // We don't have any ID, meaning no detection occure or detection is occuring.
        if (previous_id == -1)
        {
            // This is the really first init, we have to make it.
            // Be sure the network is powered up 1500 ms before starting a detection
            if (Luos_GetSystick() > 100)
            {
                // No detection occure, do it
                Luos_Detect(app);
                last_detection_date_ms = Luos_GetSystick();
                previous_id            = 0;
            }
        }
        return;
    }
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void DetectionButton_MsgHandler(service_t *service, const msg_t *msg)
{
    static uint8_t last_btn_state = 0;
    if (msg->header.cmd == IO_STATE)
    {
        if (((!last_btn_state) & (last_btn_state != msg->data[0])) && ((Luos_GetSystick() - last_detection_date_ms) > MIN_TIME_BETWEEN_DETEC_MS))
        {
            last_btn_state = msg->data[0];
            Luos_Detect(app);
            last_detection_date_ms = Luos_GetSystick();
        }
        last_btn_state = msg->data[0];
    }
    else if (msg->header.cmd == END_DETECTION)
    {
        Setup_button();
    }
}

void Setup_button(void)
{
    search_result_t result;
    RTFilter_Type(RTFilter_Reset(&result), STATE_TYPE);
    if (result.result_nbr > 0)
    {
        msg_t msg;
        msg.header.target      = result.result_table[0]->id;
        msg.header.target_mode = SERVICEIDACK;
        // Setup auto update each UPDATE_PERIOD_MS on button
        // This value is resetted on all service at each detection
        // It's important to setting it each time.
        time_luos_t time = TimeOD_TimeFrom_ms(UPDATE_PERIOD_MS);
        TimeOD_TimeToMsg(&time, &msg);
        msg.header.cmd = UPDATE_PUB;
        while (Luos_SendMsg(app, &msg) != SUCCEED)
        {
            Luos_Loop();
        }
    }
}
