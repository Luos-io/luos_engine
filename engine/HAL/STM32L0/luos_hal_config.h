/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family STM32L0
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
/*
 *
 * WITH_BOOTLOADER MODE FLASH
 *
 *
 *             SHARED_MEMORY_ADDRESS
 *             0x0800C000
 *               |
 *               |   APP_START_ADDRESS                  APP_END_ADDRESS
 *               |   |0x0800C800                          |
 *               |   |                                    |
 *               v   v                                    v
 *   +-----------+---+------------------------------------+
 *   |   48Kb    |2Kb|           ...                      |
 *   +-----------+---+------------------------------------+
 *   ^
 *   |
 *   |
 *   |
 * BOOT_START_ADDRESS
 * 0x08000000
 */

#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

#include "stm32l0xx_hal.h"

#ifndef MCUFREQ
    #define MCUFREQ 32000000 // MCU frequence
#endif

/*******************************************************************************
 * DEFINE THREAD MUTEX LOCKING AND UNLOCKING FUNCTIONS
 ******************************************************************************/
#ifndef MSGALLOC_MUTEX_LOCK
    #define MSGALLOC_MUTEX_LOCK
#endif
#ifndef MSGALLOC_MUTEX_UNLOCK
    #define MSGALLOC_MUTEX_UNLOCK
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
// begining of application in flash - by default after bootloader & shared mem
#ifndef APP_START_ADDRESS
    #define APP_START_ADDRESS (uint32_t)0x0800C800
#endif
// last address of app in case of a bootloader
#ifndef APP_END_ADDRESS
    #define APP_END_ADDRESS (uint32_t) FLASH_END
#endif
// Remapping address of vector table in flash
#ifdef WITH_BOOTLOADER
    #define LUOS_VECT_TAB APP_START_ADDRESS
#else
    #define LUOS_VECT_TAB BOOT_START_ADDRESS
#endif

#endif /* _LUOSHAL_CONFIG_H_ */