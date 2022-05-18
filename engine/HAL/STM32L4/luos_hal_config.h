/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family STM32L4
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

#include "stm32l4xx_hal.h"
// clang-format off

#ifndef MCUFREQ
#define MCUFREQ 80000000 // MCU frequence
#endif

/*******************************************************************************
 * BOOTLOADER CONFIG
 ******************************************************************************/
#define SHARED_MEMORY_ADDRESS ((uint32_t)(ADDRESS_LAST_PAGE_FLASH - FLASH_PAGE_SIZE))
#define APP_ADDRESS           (uint32_t)0x0800C800

#endif /* _LUOSHAL_CONFIG_H_ */
