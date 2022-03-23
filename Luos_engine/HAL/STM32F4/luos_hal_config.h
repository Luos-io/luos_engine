/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family STM32F4
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

#include "stm32f4xx_hal.h"
// clang-format off

#ifndef MCUFREQ
#define MCUFREQ 168000000 // MCU frequence 168000000
#endif

/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef PAGE_SIZE
#define PAGE_SIZE (uint32_t)0x400
#endif
#ifndef FLASH_SECTOR
#define FLASH_SECTOR FLASH_SECTOR_7
#endif
#ifndef ADDRESS_LAST_PAGE_FLASH
#define ADDRESS_LAST_PAGE_FLASH ((uint32_t)(FLASH_END - PAGE_SIZE))
#endif

/*******************************************************************************
 * BOOTLOADER CONFIG
 ******************************************************************************/
#ifndef SHARED_MEMORY_ADDRESS
#define SHARED_MEMORY_ADDRESS 0x0800C000
#endif
#ifndef SHARED_MEMORY_SECTOR
#define SHARED_MEMORY_SECTOR FLASH_SECTOR_3
#endif
#ifndef APP_ADDRESS
#define APP_ADDRESS (uint32_t)0x08010000
#endif
#ifndef APP_ADRESS_SECTOR
#define APP_ADRESS_SECTOR FLASH_SECTOR_4
#endif

#endif /* _LUOSHAL_CONFIG_H_ */