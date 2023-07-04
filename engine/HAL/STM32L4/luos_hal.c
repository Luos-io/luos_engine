/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family STM32L4
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos_hal.h"

#include <stdbool.h>
#include <string.h>

// MCU dependencies this HAL is for family STM32l4 you can find
// the HAL stm32cubel4 on ST web site
#include "stm32l4xx_ll_system.h"

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
static void LuosHAL_VectorTableRemap(void);
/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_Init(void)
{
    // Remap Vector Table
    LuosHAL_VectorTableRemap();

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
}
/******************************************************************************
 * @brief Luos HAL remap Vector table in given address in flash
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_VectorTableRemap(void)
{
    SCB->VTOR = LUOS_VECT_TAB;
}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms
 * @param None
 * @return Tick Counter
 ******************************************************************************/
_CRITICAL uint32_t LuosHAL_GetSystick(void)
{
    return HAL_GetTick();
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
}
/******************************************************************************
 * @brief Set boot mode in shared flash memory
 * @param None
 * @return
 ******************************************************************************/
_CRITICAL void LuosHAL_SetMode(uint8_t mode)
{
    uint64_t data_to_write = ~BOOT_MODE_MASK | (mode << BOOT_MODE_OFFSET);
    uint32_t page_error    = 0;
    FLASH_EraseInitTypeDef s_eraseinit;

    s_eraseinit.TypeErase = FLASH_TYPEERASE_PAGES;
    s_eraseinit.Banks     = FLASH_BANK_BOTH;
    s_eraseinit.Page      = SHARED_MEMORY_ADDRESS / (uint32_t)FLASH_PAGE_SIZE;
    s_eraseinit.NbPages   = 1;

    // Unlock flash
    HAL_FLASH_Unlock();
    // Erase Page
    HAL_FLASHEx_Erase(&s_eraseinit, &page_error);
    // ST hal flash program function write data by uint64_t raw data
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)SHARED_MEMORY_ADDRESS, data_to_write);
    // re-lock FLASH
    HAL_FLASH_Lock();
}

/******************************************************************************
 * @brief Save node ID in shared flash memory
 * @param node_id
 * @return
 ******************************************************************************/
_CRITICAL void LuosHAL_SaveNodeID(uint16_t node_id)
{
    uint32_t page_error = 0;
    FLASH_EraseInitTypeDef s_eraseinit;
    uint32_t *p_start = (uint32_t *)SHARED_MEMORY_ADDRESS;

    uint32_t saved_data    = *p_start;
    uint32_t data_tmp      = ~NODE_ID_MASK | (node_id << NODE_ID_OFFSET);
    uint32_t data_to_write = saved_data & data_tmp;

    s_eraseinit.TypeErase = FLASH_TYPEERASE_PAGES;
    s_eraseinit.Page      = SHARED_MEMORY_ADDRESS / (uint32_t)FLASH_PAGE_SIZE;
    s_eraseinit.NbPages   = 1;

    // Unlock flash
    HAL_FLASH_Unlock();
    // Erase Page
    HAL_FLASHEx_Erase(&s_eraseinit, &page_error);
    // ST hal flash program function write data by uint64_t raw data
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)SHARED_MEMORY_ADDRESS, (uint64_t)data_to_write);
    // re-lock FLASH
    HAL_FLASH_Lock();
}

/******************************************************************************
 * @brief Software reboot the microprocessor
 * @param None
 * @return
 ******************************************************************************/
void LuosHAL_Reboot(void)
{
    // DeInit RCC and HAL
    HAL_RCC_DeInit();
    HAL_DeInit();

    // reset systick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    // reset in bootloader mode
    NVIC_SystemReset();
}

/******************************************************************************
 * @brief DeInit Bootloader peripherals
 * @param None
 * @return
 ******************************************************************************/
typedef void (*pFunction)(void); /*!< Function pointer definition */

void LuosHAL_JumpToAddress(uint32_t addr)
{
    uint32_t JumpAddress = *(__IO uint32_t *)(addr + 4);
    pFunction Jump       = (pFunction)JumpAddress;

    __disable_irq();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    SCB->VTOR = addr;

    __set_MSP(*(__IO uint32_t *)addr);

    __enable_irq();

    Jump();
}

/******************************************************************************
 * @brief Return bootloader mode saved in flash
 * @param None
 * @return
 ******************************************************************************/
uint8_t LuosHAL_GetMode(void)
{
    uint32_t *p_start = (uint32_t *)SHARED_MEMORY_ADDRESS;
    uint32_t data     = (*p_start & BOOT_MODE_MASK) >> BOOT_MODE_OFFSET;

    return (uint8_t)data;
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
    uint32_t nb_sectors_to_erase = 0;
    uint32_t page_to_erase       = address / (uint32_t)FLASH_PAGE_SIZE;

    // compute number of sectors to erase
    nb_sectors_to_erase = (size / (uint32_t)FLASH_PAGE_SIZE) + 1;

    uint32_t page_error = 0;
    FLASH_EraseInitTypeDef s_eraseinit;
    s_eraseinit.Banks     = FLASH_BANK_1;
    s_eraseinit.TypeErase = FLASH_TYPEERASE_PAGES;
    s_eraseinit.NbPages   = 1;

    int i = 0;
    for (i = 0; i < nb_sectors_to_erase; i++)
    {
        s_eraseinit.Page = page_to_erase;

        // Unlock flash
        HAL_FLASH_Unlock();
        // Erase Page
        HAL_FLASHEx_Erase(&s_eraseinit, &page_error);
        // re-lock FLASH
        HAL_FLASH_Lock();

        // update page to erase
        page_to_erase += 1;
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
    for (uint32_t i = 0; i < size; i += sizeof(uint64_t))
    {
        while (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, i + address, *(uint64_t *)(&data[i])) != HAL_OK)
            ;
    }
    // re-lock FLASH
    HAL_FLASH_Lock();
}
#endif
