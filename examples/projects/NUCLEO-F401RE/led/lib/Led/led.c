/******************************************************************************
 * @file Led
 * @brief driver example a simple Led
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "gpio.h"
#include "led.h"
#include "alias_save.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t Led_last_state = 0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void Led_MsgHandler(service_t *service, const msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Led_Init(void)
{
    revision_t revision        = {.unmap = {1, 0, 0}};
    service_t *myService       = Luos_CreateService(Led_MsgHandler, STATE_TYPE, "led", revision);
    char alias[MAX_ALIAS_SIZE] = {0};
    int size                   = Alias_read(0, alias);
    Luos_UpdateAlias(myService, alias, size);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Led_Loop(void)
{
}
/******************************************************************************
 * @brief Msg manager callback when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Led_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == IO_STATE)
    {
        if (msg->data[0] != Led_last_state)
        {
            Led_last_state = msg->data[0];
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, Led_last_state);
        }
    }
    if (msg->header.cmd == WRITE_ALIAS)
    {
        Alias_write(0, msg->data, msg->header.size);
    }
}
