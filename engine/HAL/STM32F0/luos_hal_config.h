/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family STM32FO
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

#include "stm32f0xx_hal.h"
// clang-format off

#ifndef MCUFREQ
#define MCUFREQ 48000000 // MCU frequence
#endif

/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef PAGE_SIZE
#define PAGE_SIZE                   (uint32_t) FLASH_PAGE_SIZE
#endif
#ifndef ADDRESS_LAST_PAGE_FLASH
#define ADDRESS_LAST_PAGE_FLASH     ((uint32_t)(FLASH_BANK1_END - FLASH_PAGE_SIZE + 1))
#endif

/*******************************************************************************
 * BOOTLOADER CONFIG
 ******************************************************************************/
#define FLASH_END 0x0801FFFF

#define SHARED_MEMORY_ADDRESS ((uint32_t)(ADDRESS_LAST_PAGE_FLASH - FLASH_PAGE_SIZE))
#define APP_ADDRESS           (uint32_t)0x0800C800

#endif /* _LUOSHAL_CONFIG_H_ */