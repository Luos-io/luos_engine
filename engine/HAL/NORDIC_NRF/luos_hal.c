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
#include "nrf_drv_timer.h"
#include "nrf.h"
/*************************************************************************
 * This file is a template and documentation for the Luos engine HAL layer.
 * Feel free to duplicate it and customize it to your needs.
 *************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
// timestamp variable
static ll_timestamp_t ll_timestamp;
uint32_t ms_tick = 0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void LuosHAL_SystickInit(void);
static void LuosHAL_FlashInit(void);
static void  LuosHAL_SystickHandler(nrf_timer_event_t event_type, void* p_context);

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
_CRITICAL void LuosHAL_SetIrqState(uint8_t Enable)
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
         __enable_irq();
    }
    else
    {
        /*************************************************************************
         * This function turn off all the IRQ in the MCU enabling atomic
         * For exemple with CMSIS __disable_irq();
         ************************************************************************/
         __disable_irq();
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
     uint32_t time_ticks;
     const nrf_drv_timer_t timer_instance = NRF_DRV_TIMER_INSTANCE(0);

     nrf_drv_timer_config_t timer_config = NRF_DRV_TIMER_DEFAULT_CONFIG;
     if (nrf_drv_timer_init(&timer_instance, &timer_config, LuosHAL_SystickHandler) != NRF_SUCCESS)
     {
        return;
     }

     time_ticks = nrf_drv_timer_ms_to_ticks(&timer_instance, 1);

     nrf_drv_timer_extended_compare(&timer_instance, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
     nrf_drv_timer_enable(&timer_instance);
}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms
 * @param None
 * @return Tick Counter
 ******************************************************************************/
_CRITICAL uint32_t LuosHAL_GetSystick(void)
{
    /*************************************************************************
     * This function return the value of Luos systick since MCU has been started
     ************************************************************************/
    return ms_tick; // return  tick
}

static void LuosHAL_SystickHandler(nrf_timer_event_t event_type, void* p_context)
{
    ms_tick++;
}
/******************************************************************************
 * @brief Luos GetTimestamp
 * @param None
 * @return uint64_t
 ******************************************************************************/
_CRITICAL uint64_t LuosHAL_GetTimestamp(void)
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
    //ll_timestamp.lower_timestamp  = (nrf_systick_load_get() - nrf_systick_val_get()) * (1000000000 / MCUFREQ);
    ll_timestamp.higher_timestamp = (uint64_t)(LuosHAL_GetSystick() - ll_timestamp.start_offset);

    return ll_timestamp.higher_timestamp * 1000000 + (uint64_t)ll_timestamp.lower_timestamp;
}

/******************************************************************************
 * @brief Luos start Timestamp
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void LuosHAL_StartTimestamp(void)
{
    /*************************************************************************
     * This function set the ll_timestamp.start_offset witch is the initial value of the timestamp.
     * the timestamp value is an int64
     ************************************************************************/
    // set ll_timestamp.lower_timestamp
    // set ll_timestamp.higher_timestamp
    ll_timestamp.start_offset = LuosHAL_GetSystick();
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

/******************************************************************************
 * @brief Flash Initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_FlashInit(void)
{
}

_CRITICAL void LuosHAL_SetMode(uint8_t mode)
{
}

_CRITICAL void LuosHAL_SaveNodeID(uint16_t node_id)
{
}

void LuosHAL_Reboot(void)
{
}

void LuosHAL_JumpToAddress(uint32_t addr)
{
}

uint8_t LuosHAL_GetMode(void)
{
}

#ifdef BOOTLOADER
/******************************************************************************
 * @brief DeInit Bootloader peripherals
 * @param None
 * @return
 ******************************************************************************/
void LuosHAL_DeInit(void)
{
    HAL_RCC_DeInit();
    HAL_DeInit();
}

/******************************************************************************
 * @brief Get node id saved in flash memory
 * @param Address
 * @return node_id
 ******************************************************************************/
uint16_t LuosHAL_GetNodeID(void)
{
    uint32_t *p_start = (uint32_t *)SHARED_MEMORY_ADDRESS;
    uint32_t data     = *p_start & NODE_ID_MASK;
    uint16_t node_id  = (uint16_t)(data >> NODE_ID_OFFSET);

    return node_id;
}

/******************************************************************************
 * @brief Erase sectors in flash memory
 * @param Address, size
 * @return
 ******************************************************************************/
void LuosHAL_EraseMemory(uint32_t address, uint16_t size)
{
    uint32_t nb_sectors_to_erase = APP_END_SECTOR - APP_START_SECTOR + 1;
    uint32_t sector_to_erase     = APP_START_SECTOR;

    uint32_t sector_error = 0;
    FLASH_EraseInitTypeDef s_eraseinit;
    s_eraseinit.TypeErase    = FLASH_TYPEERASE_SECTORS;
    s_eraseinit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    s_eraseinit.NbSectors    = 1;

    int i = 0;
    for (i = 0; i < nb_sectors_to_erase; i++)
    {
        s_eraseinit.Sector = sector_to_erase;

        // Unlock flash
        HAL_FLASH_Unlock();
        // Erase Page
        HAL_FLASHEx_Erase(&s_eraseinit, &sector_error);
        // re-lock FLASH
        HAL_FLASH_Lock();

        // update page to erase
        sector_to_erase += 1;
    }
}

/******************************************************************************
 * @brief Programm flash memory
 * @param address : Start address 
 * @param size :: Data size
 * @param data : Pointer to data
 * @return
 ******************************************************************************/
void LuosHAL_ProgramFlash(uint32_t address, uint16_t size, uint8_t *data)
{
    // Unlock flash
    HAL_FLASH_Unlock();
    // ST hal flash program function write data by uint64_t raw data
    for (uint32_t i = 0; i < size; i += sizeof(uint32_t))
    {
        while (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, i + address, *(uint32_t *)(&data[i])) != HAL_OK)
            ;
    }
    // re-lock FLASH
    HAL_FLASH_Lock();
}
#endif


