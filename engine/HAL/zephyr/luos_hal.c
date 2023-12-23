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
#include <zephyr/kernel.h>

/*******************************************************************************
 * Variables
 ******************************************************************************/
// timestamp variable
static ll_timestamp_t ll_timestamp;
static uint32_t irq_nest = 0;
static uint32_t irq_key;

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
    LuosHAL_SystickInit();
    LuosHAL_StartTimestamp();
}

/******************************************************************************
 * @brief Luos HAL general disable IRQ
 * @param Enable : Set to "True" to enable IRQ, "False" otherwise
 * @return None
 ******************************************************************************/
void LuosHAL_SetIrqState(uint8_t Enable)
{
    if (Enable == true)
    {
        if(irq_nest)
        {
            irq_nest--;
            if(!irq_nest) 
            {
                irq_unlock(irq_key);
            }
        }

    }
    else
    {
        if(!irq_nest)
        {
            irq_key = irq_lock();
        }
        irq_nest++;
    }
}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms initialize
 * @param None
 * @return Tick Counter
 ******************************************************************************/
static void LuosHAL_SystickInit(void)
{

}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms
 * @param None
 * @return Tick Counter
 ******************************************************************************/
uint32_t LuosHAL_GetSystick(void)
{
    return k_uptime_get_32();
}
/******************************************************************************
 * @brief Luos GetTimestamp
 * @param None
 * @return uint64_t
 ******************************************************************************/
uint64_t LuosHAL_GetTimestamp(void)
{
    uint64_t timestamp = 1000000  * (k_cycle_get_32() / sys_clock_hw_cycles_per_sec());
    return timestamp;
}

/******************************************************************************
 * @brief Luos start Timestamp
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_StartTimestamp(void)
{
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

void LuosHAL_Reboot(void)
{

}

void LuosHAL_SetMode(uint8_t mode)
{
}

void LuosHAL_SaveNodeID(uint16_t id)
{
}



