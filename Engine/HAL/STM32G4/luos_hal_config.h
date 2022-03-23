/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family STM32G4
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

#include "stm32g4xx_hal.h"
// clang-format off

#ifndef MCUFREQ
#define MCUFREQ 170000000 // MCU frequence
#endif
/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef PAGE_SIZE
#define PAGE_SIZE (uint32_t) FLASH_PAGE_SIZE
#endif
#ifndef ADDRESS_LAST_PAGE_FLASH
#define ADDRESS_LAST_PAGE_FLASH ((uint32_t)((FLASH_BASE + FLASH_SIZE) - FLASH_PAGE_SIZE))
#endif

/*******************************************************************************
 * BOOTLOADER CONFIG
 ******************************************************************************/
#define FLASH_END FLASH_SIZE - 1

#ifndef END_ERASE_BOOTLOADER
#define END_ERASE_BOOTLOADER (uint32_t)0x08020000
#endif
#ifndef SHARED_MEMORY_ADDRESS
#define SHARED_MEMORY_ADDRESS (uint32_t)0x0801F800
#endif
#ifndef APP_ADDRESS
#define APP_ADDRESS (uint32_t)0x0800C800
#endif

#endif /* _LUOSHAL_CONFIG_H_ */
