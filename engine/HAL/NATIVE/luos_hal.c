/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction
 * @Family x86/Linux/Mac
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos_hal.h"

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

pthread_mutex_t mutex_msg_alloc = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_luos      = PTHREAD_MUTEX_INITIALIZER;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void LuosHAL_SystickInit(void);
static void LuosHAL_FlashInit(void);
static void LuosHAL_FlashEraseLuosMemoryInfo(void);

/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_Init(void)
{
    {
        // Systick Initialization
        LuosHAL_SystickInit();

        // Flash Initialization
        LuosHAL_FlashInit();

        // start timestamp
        LuosHAL_StartTimestamp();
    }
}

/******************************************************************************
 * @brief Luos HAL general disable IRQ
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_SetIrqState(bool Enable)
{
}

/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms initialize
 * @param None
 * @return tick Counter
 ******************************************************************************/
static void LuosHAL_SystickInit(void)
{
}

/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms
 * @param None
 * @return tick Counter
 ******************************************************************************/
uint32_t LuosHAL_GetSystick(void)
{
    struct timespec time;
    uint32_t ms; // Milliseconds
    time_t s;    // Seconds
#ifdef linux
    clock_gettime(CLOCK_BOOTTIME, &time);
#else
    clock_gettime(CLOCK_MONOTONIC, &time);
#endif
    s  = time.tv_sec;
    ms = round(time.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999)
    {
        s++;
        ms = 0;
    }
    ms += s * 1000;
    return ms;
}

/******************************************************************************
 * @brief Luos GetTimestamp
 * @param None
 * @return uint64_t
 ******************************************************************************/
uint64_t LuosHAL_GetTimestamp(void)
{
    struct timespec time;
#ifdef linux
    clock_gettime(CLOCK_BOOTTIME, &time);
#else
    clock_gettime(CLOCK_MONOTONIC, &time);
#endif
    return time.tv_nsec;
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
}

/******************************************************************************
 * @brief Flash Initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_FlashInit(void)
{
    for (uint16_t i = 0; i < FLASH_PAGE_NUMBER; i++)
    {
        for (uint16_t j = 0; j < FLASH_PAGE_SIZE; j++)
        {
            stub_flash_x86[i][j] = 0;
        }
    }
}

/******************************************************************************
 * @brief Erase flash page where Luos keep permanente information
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_FlashEraseLuosMemoryInfo(void)
{
}

/******************************************************************************
 * @brief Write flash page where Luos keep permanente information
 * @param Address page / size to write / pointer to data to write
 * @return
 ******************************************************************************/
void LuosHAL_FlashWriteLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data)
{
}

/******************************************************************************
 * @brief read information from page where Luos keep permanente information
 * @param Address info / size to read / pointer callback data to read
 * @return
 ******************************************************************************/
void LuosHAL_FlashReadLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data)
{
    memset(data, 0xFF, size);
}

/******************************************************************************
 * @brief Set boot mode in shared flash memory
 * @param
 * @return
 ******************************************************************************/
void LuosHAL_SetMode(uint8_t mode)
{
}

/******************************************************************************
 * @brief Save node ID in shared flash memory
 * @param Address, node_id
 * @return
 ******************************************************************************/
void LuosHAL_SaveNodeID(uint16_t node_id)
{
}

/******************************************************************************
 * @brief software reboot the microprocessor
 * @param
 * @return
 ******************************************************************************/
void LuosHAL_Reboot(void)
{
}

#ifdef BOOTLOADER_CONFIG
/******************************************************************************
 * @brief DeInit Bootloader peripherals
 * @param
 * @return
 ******************************************************************************/
void LuosHAL_DeInit(void)
{
}

/******************************************************************************
 * @brief DeInit Bootloader peripherals
 * @param
 * @return
 ******************************************************************************/
typedef void (*pFunction)(void); /*!< Function pointer definition */

void LuosHAL_JumpToApp(uint32_t app_addr)
{
}

/******************************************************************************
 * @brief Return bootloader mode saved in flash
 * @param
 * @return
 ******************************************************************************/
uint8_t LuosHAL_GetMode(void)
{
}

/******************************************************************************
 * @brief Get node id saved in flash memory
 * @param Address
 * @return node_id
 ******************************************************************************/
uint16_t LuosHAL_GetNodeID(void)
{
}

/******************************************************************************
 * @brief erase sectors in flash memory
 * @param Address, size
 * @return
 ******************************************************************************/
void LuosHAL_EraseMemory(uint32_t address, uint16_t size)
{
}

/******************************************************************************
 * @brief Save binary data in shared flash memory
 * @param Address, size, data[]
 * @return
 ******************************************************************************/
void LuosHAL_ProgramFlash(uint32_t address, uint16_t size, uint8_t *data)
{
}
#endif
