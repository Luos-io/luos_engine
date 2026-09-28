/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction
 * @Family Linux (Raspberry Pi)
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos_hal.h"

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <sys/stat.h>
#include <libgen.h>

pthread_mutex_t luos_recursive_mutex;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void LuosHAL_SystickInit(void);
static void LuosHAL_FlashInit(void);

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_Init(void)
{
    // Initialize the recursive mutex FIRST, before anything else.
    // MsgAlloc_Init() and Phy_Init() call Phy_SetIrqState() during
    // initialization, so the mutex must be ready before they run.
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&luos_recursive_mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    LuosHAL_SystickInit();
    LuosHAL_FlashInit();
    LuosHAL_StartTimestamp();
}

/******************************************************************************
 * @brief Luos HAL IRQ state control via recursive mutex
 * @param Enable: false=lock, true=unlock
 * @return None
 ******************************************************************************/
void LuosHAL_SetIrqState(bool Enable)
{
    if (Enable == false)
    {
        pthread_mutex_lock(&luos_recursive_mutex);
    }
    else
    {
        pthread_mutex_unlock(&luos_recursive_mutex);
    }
}

/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms initialize
 * @param None
 * @return None
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
    uint32_t ms;
    time_t s;
    clock_gettime(CLOCK_MONOTONIC, &time);
    s  = time.tv_sec;
    ms = round(time.tv_nsec / 1.0e6);
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
    clock_gettime(CLOCK_MONOTONIC, &time);
    uint64_t timestamp = time.tv_nsec + time.tv_sec * 1000000000;
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
 * @brief Set boot mode -- no-op on Linux (no bootloader)
 * @param mode
 * @return
 ******************************************************************************/
void LuosHAL_SetMode(uint8_t mode)
{
}

/******************************************************************************
 * @brief Save node ID to persistent file
 * @param node_id
 * @return
 ******************************************************************************/
void LuosHAL_SaveNodeID(uint16_t node_id)
{
    // Ensure the parent directory exists
    char path_copy[256];
    strncpy(path_copy, LUOS_PERSIST_PATH, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';
    mkdir(dirname(path_copy), 0755);

    FILE *f = fopen(LUOS_PERSIST_PATH, "wb");
    if (f)
    {
        fwrite(&node_id, sizeof(uint16_t), 1, f);
        fclose(f);
    }
}

/******************************************************************************
 * @brief software reboot -- no-op on Linux
 * @param
 * @return
 ******************************************************************************/
void LuosHAL_Reboot(void)
{
}
