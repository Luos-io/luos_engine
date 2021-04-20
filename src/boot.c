/******************************************************************************
 * @file Boot
 * @brief fonctionnalities for luos bootloader
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "main.h"
#include "boot.h"
#include "string.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define REV {1,0,0}

/*******************************************************************************
 * Variables
 ******************************************************************************/
typedef enum
{
    BOOT_APP = LUOS_LAST_TYPE
} App_type_t;

uint16_t boot_blinktime = 100;
unsigned long boot_my_time; //Used to keep track of time
uint8_t boot_led_last_state = 0; //Is the LED on or off?
uint8_t boot_count = 0;
uint8_t nb_blink = 3;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Boot_MsgHandler(container_t *container, msg_t *msg);

/******************************************************************************
 * @brief change blinktime
 * @param None
 * @return None
 ******************************************************************************/
void change_blink(uint8_t new_nb_blink)
{
  nb_blink = new_nb_blink;
}

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Boot_Init(void)
{
	revision_t revision = {.unmap = REV};
  boot_container = Luos_CreateContainer(Boot_MsgHandler, BOOT_APP, "boot_app", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Boot_Loop(void)
{
  //Check to see if we have overshot our counter
	if (boot_my_time < HAL_GetTick())
	{
    //Reset the counter
    boot_my_time = HAL_GetTick() + boot_blinktime;

    if (boot_count < nb_blink)
    {
      boot_count++;
      boot_blinktime = 100;
    }
    else
    {
      boot_count = 0;
      boot_blinktime = 1000;
    }

    //Invert the LED state
    boot_led_last_state = (boot_led_last_state == 1 ? 0: 1);

    if (boot_led_last_state)
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    else
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	}
}
/******************************************************************************
 * @brief Msg manager callback when a msg receive for this container
 * @param Container destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Boot_MsgHandler(container_t *container, msg_t *msg)
{
}