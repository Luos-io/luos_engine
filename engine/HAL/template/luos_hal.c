/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family XXX
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos_hal.h"
#include <stdbool.h>
#include <string.h>

/*************************************************************************
 * This file is a template and documentation for the Luos engine HAL layer.
 * Feel free to duplicate it and customize it to your needs.
 *************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
// timestamp variable
static ll_timestamp_t ll_timestamp;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void LuosHAL_SystickInit(void);
static void LuosHAL_FlashInit(void);

/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_Init(void)
{
    /*************************************************************************
     * This function is called once at the beginning of the program.
     * It is used to initialize the hardware and the variables.
     * You can add your own initialisation code here.
     *************************************************************************/

    // Systick Initialization
    LuosHAL_SystickInit();

    // Flash Initialization
    LuosHAL_FlashInit();

    // start timestamp
    LuosHAL_StartTimestamp();
}
/******************************************************************************
 * @brief Luos HAL general disable IRQ
 * @param Enable : Set to "True" to enable IRQ, "False" otherwise
 * @return None
 ******************************************************************************/
void LuosHAL_SetIrqState(bool Enable)
{
    /*************************************************************************
     * This function turn on and off all the IRQ in the MCU allowing atomic
     * execution of some critical part of the code avoidign data race.
     ************************************************************************/
    if (Enable == true)
    {
        /*************************************************************************
         * This function turn on all the IRQ in the MCU disabling atomic
         * For exemple with CMSIS __enable_irq();
         ************************************************************************/
    }
    else
    {
        /*************************************************************************
         * This function turn off all the IRQ in the MCU enabling atomic
         * For exemple with CMSIS __disable_irq();
         ************************************************************************/
    }
}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms initialize
 * @param None
 * @return Tick Counter
 ******************************************************************************/
static void LuosHAL_SystickInit(void)
{
    /*************************************************************************
     * This function is used to initialize the systick timer.
     * The Luos systick than most system systick.
     * It allow user to count sent ms since the systick has been started
     ************************************************************************/
}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms
 * @param None
 * @return Tick Counter
 ******************************************************************************/
uint32_t LuosHAL_GetSystick(void)
{
    /*************************************************************************
     * This function return the value of Luos systick since MCU has been started
     ************************************************************************/
    return; // return  tick
}
/******************************************************************************
 * @brief Luos GetTimestamp
 * @param None
 * @return uint64_t
 ******************************************************************************/
uint64_t LuosHAL_GetTimestamp(void)
{
    /*************************************************************************
     * Timestamp is a kind of Systick with a ns resolution.
     * For more informations about timestamp see the Luos documentation => https://www.luos.io/docs/luos-technology/services/timestamp
     * Alternatively there is a more deep dive in it on this blogpost => https://www.luos.io/blog/distributed-latency-based-time-synchronization
     * You can create the timestamp value with your own timer or by
     * reading the timer counter value of the systick timer register of your MCU.
     *
     * This timestamps value will be as precise as the MCU frequency is.
     * The timestamp value is an int64_t value.
     * Make sure to return the value at a ns scale even if your MCU is slower.
     ************************************************************************/

    return // Your timestamp value;
}

/******************************************************************************
 * @brief Luos start Timestamp
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_StartTimestamp(void)
{
    /*************************************************************************
     * This function set the ll_timestamp.start_offset witch is the initial value of the timestamp.
     * the timestamp value is an int64
     ************************************************************************/
    // set ll_timestamp.lower_timestamp
    // set ll_timestamp.higher_timestamp
    return ll_timestamp.higher_timestamp * 1000000 + (uint64_t)ll_timestamp.lower_timestamp;
}

/******************************************************************************
 * @brief Luos stop Timestamp
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_StopTimestamp(void)
{
    /*************************************************************************
     * This function Reset your timestamp counter to 0.
     ************************************************************************/
    // Reset your timestamp struct
    ll_timestamp.lower_timestamp  = 0;
    ll_timestamp.higher_timestamp = 0;
    ll_timestamp.start_offset     = 0;
}
