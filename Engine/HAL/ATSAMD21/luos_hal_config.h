/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family ATSAMD21
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

#include "samd21.h"
// clang-format off

#ifndef MCUFREQ
#define MCUFREQ 48000000 // MCU frequence
#endif

/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef PAGE_SIZE
#define PAGE_SIZE 64
#endif
#ifndef ERASE_SIZE
#define ERASE_SIZE 256
#endif
#ifndef FLASH_SIZE
#define FLASH_SIZE 0x40000
#endif
#ifndef ADDRESS_LAST_PAGE_FLASH
#define ADDRESS_LAST_PAGE_FLASH FLASH_SIZE - (16 * PAGE_SIZE)
#endif

/*******************************************************************************
 * BOOTLOADER CONFIG
 ******************************************************************************/
#define FLASH_END FLASH_SIZE - 1

#define SHARED_MEMORY_ADDRESS 0x0003F000
#define APP_ADDRESS           0x0000A200

#endif /* _LUOSHAL_CONFIG_H_ */
