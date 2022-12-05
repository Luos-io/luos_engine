/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family STM32F4
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
/*
 * WITH_BOOTLOADER MODE FLASH
 *
 *             SHARED_MEMORY_ADDRESS
 * BOOT_START  0x0800C000
 * 0x08000000  |
 * |           |   APP_START_ADDRESS                    APP_END_ADDRESS
 * |           |   |0x08010000                          |FLASH_END
 * |           |   |                                    |
 * |           v   v                                    v
 * +-----------+---+------------------------------------+
 * |   48K     |16K|                                    |
 * +---+---+---+---+---+----------------------------+---+
 * | 0 | 1 | 2 | 3 | 4 |. . .                       |   |
 * +---+---+---+---+---+----------------------------+---+
 * ^           ^   ^                                ^
 * |           |   |                                |
 * |           |   APP_START_SECTOR               APP_END_SECTOR
 * |          SHARED_MEMORY_SECTOR                LAST_FLASH_SECTOR
 * FLASH_SECTOR_0
 */
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

#include "stm32f4xx_hal.h"

#ifndef MCUFREQ
    #define MCUFREQ 168000000 // MCU frequence 168000000
#endif

/*******************************************************************************
 * DEFINE THREAD MUTEX LOCKING AND UNLOCKING FUNCTIONS
 ******************************************************************************/
#ifndef THREAD_LOCK
    #define THREAD_LOCK
#endif
#ifndef THREAD_UNLOCK
    #define THREAD_UNLOCK
#endif

/*******************************************************************************
 * BOOTLOADER CONFIG
 ******************************************************************************/
// The beginning of the bootloader code in flash
#ifndef BOOT_START_ADDRESS
    #define BOOT_START_ADDRESS (uint32_t) FLASH_BASE
#endif
// Shared memory to store bootmode
#ifndef SHARED_MEMORY_ADDRESS
    #define SHARED_MEMORY_ADDRESS (uint32_t)0x0800C000
#endif
#ifndef SHARED_MEMORY_SECTOR
    #define SHARED_MEMORY_SECTOR FLASH_SECTOR_3
#endif
// begining of application in flash - by default after bootloader & shared mem
#ifndef APP_START_ADDRESS
    #define APP_START_ADDRESS (uint32_t)0x08010000
#endif
// last address of app in case of a bootloader
#ifndef APP_END_ADDRESS
    #define APP_END_ADDRESS (uint32_t) FLASH_END
#endif
// first sector of the app
#ifndef APP_START_SECTOR
    #define APP_START_SECTOR FLASH_SECTOR_4
#endif
// last sector of the app
#ifndef APP_END_SECTOR
    #define APP_END_SECTOR FLASH_SECTOR_TOTAL - 1
#endif
// Remapping address of vector table in flash
#ifdef WITH_BOOTLOADER
    #define LUOS_VECT_TAB APP_START_ADDRESS
#else
    #define LUOS_VECT_TAB BOOT_START_ADDRESS
#endif

#endif /* _LUOSHAL_CONFIG_H_ */