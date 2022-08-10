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

// MCU dependencies this HAL is for family XXX you can find

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
    // Systick Initialization
    LuosHAL_SystickInit();

    // Flash Initialization
    LuosHAL_FlashInit();

    // start timestamp
    LuosHAL_StartTimestamp();
}
/******************************************************************************
 * @brief Luos HAL general disable IRQ
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_SetIrqState(uint8_t Enable)
{
    /*************************************************************************
     *
     * This function shutdonw all the IRQ in the MCU to make atomic execution
     * of the part of the code when it's call
     *
     ************************************************************************/
    if (Enable == true)
    {
        /*************************************************************************
         *
         * atomic exit exemple with CMSIS __enable_irq();
         *
         ************************************************************************/
    }
    else
    {
        /*************************************************************************
         *
         * atomic entry exemple with CMSIS __disable_irq();
         *
         ************************************************************************/
    }
}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms initialize
 * @param None
 * @return tick Counter
 ******************************************************************************/
static void LuosHAL_SystickInit(void)
{
    /*************************************************************************
     *
     * This function initialize Luos systick.
     * the Luos systick allow user to track time at 1ms since
     * the systick has been started
     *
     ************************************************************************/
}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms
 * @param None
 * @return tick Counter
 ******************************************************************************/
uint32_t LuosHAL_GetSystick(void)
{
    /*************************************************************************
     *
     * This function return the value of Luos systick since has been started
     *
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
     *
     * This function give a ns time measurement for timestamp value
     * you can create the timestamp value with your own timer or by
     * reading the value of the systick register
     *
     * this timestamps value will be as precise as the MCU frequency is high
     *
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
     *
     * This function Set the ll_timestamp.start_offset value to the actual
     * timestamp value
     *
     * the timestamp value is an int64
     *
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
     *
     * This function Reset your timestamp struct to
     *
     ************************************************************************/
    // Reset your timestamp struct
    ll_timestamp.lower_timestamp  = 0;
    ll_timestamp.higher_timestamp = 0;
    ll_timestamp.start_offset     = 0;
}
