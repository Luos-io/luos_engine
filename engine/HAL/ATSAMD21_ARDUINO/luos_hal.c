/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family ATSAMD21
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos_hal.h"

#include <stdbool.h>
#include <string.h>

// MCU dependencies this HAL is for family Atmel ATSAMD21 you can find

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEFAULT_TIMEOUT 30
#define TIMEOUT_ACK     DEFAULT_TIMEOUT / 4
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t tick;

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
 * @param Enable : Set to "True" to enable IRQ, "False" otherwise
 * @return None
 ******************************************************************************/
_CRITICAL void LuosHAL_SetIrqState(bool Enable)
{
    if (Enable == true)
    {
        __enable_irq();
    }
    else
    {
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
    SysTick->CTRL = 0;
    SysTick->VAL  = 0;
    SysTick->LOAD = 0xbb80 - 1;
    SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms
 * @param None
 * @return Tick Counter
 ******************************************************************************/
_CRITICAL uint32_t LuosHAL_GetSystick(void)
{
    return millis();
}

/******************************************************************************
 * @brief Luos GetTimestamp
 * @param None
 * @return uint64_t
 ******************************************************************************/
_CRITICAL uint64_t LuosHAL_GetTimestamp(void)
{
    ll_timestamp.lower_timestamp  = (SysTick->LOAD - SysTick->VAL) * (1000000000 / MCUFREQ);
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
    ll_timestamp.start_offset = LuosHAL_GetSystick();
}

/******************************************************************************
 * @brief Luos stop Timestamp
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void LuosHAL_StopTimestamp(void)
{
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
    NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_READMODE_NO_MISS_PENALTY | NVMCTRL_CTRLB_SLEEPPRM_WAKEONACCESS
                         | NVMCTRL_CTRLB_RWS(1) | NVMCTRL_CTRLB_MANW;
}
/******************************************************************************
 * @brief Set boot mode in shared flash memory
 * @param None
 * @return
 ******************************************************************************/
_CRITICAL void LuosHAL_SetMode(uint8_t mode)
{
}

/******************************************************************************
 * @brief Save node ID in shared flash memory
 * @param node_id
 * @return
 ******************************************************************************/
_CRITICAL void LuosHAL_SaveNodeID(uint16_t node_id)
{
}

/******************************************************************************
 * @brief Software reboot the microprocessor
 * @param None
 * @return
 ******************************************************************************/
void LuosHAL_Reboot(void)
{
}

#ifdef BOOTLOADER
/******************************************************************************
 * @brief Get node id saved in flash memory
 * @param Address
 * @return node_id
 ******************************************************************************/
uint16_t LuosHAL_GetNodeID(void)
{
}

/******************************************************************************
 * @brief Programm flash memory
 * @param address : Start address
 * @param size :: Data size
 * @param data : Pointer to data
 * @return
 ******************************************************************************/
void LuosHAL_ProgramFlash(uint32_t address, uint8_t page, uint16_t size, uint8_t *data)
{
}

/******************************************************************************
 * @brief DeInit Bootloader peripherals
 * @param None
 * @return
 ******************************************************************************/
void LuosHAL_DeInit(void)
{
}

/******************************************************************************
 * @brief DeInit Bootloader peripherals
 * @param None
 * @return
 ******************************************************************************/
typedef void (*pFunction)(void); /*!< Function pointer definition */

void LuosHAL_JumpToApp(uint32_t app_addr)
{
}

/******************************************************************************
 * @brief Return bootloader mode saved in flash
 * @param None
 * @return
 ******************************************************************************/
uint8_t LuosHAL_GetMode(void)
{
}
#endif
