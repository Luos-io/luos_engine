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

#include <Arduino.h>

#ifndef MCUFREQ
    #define MCUFREQ 48000000 // MCU frequence
#endif

/*******************************************************************************
 * BOOTLOADER CONFIG
 ******************************************************************************/
#define SHARED_MEMORY_ADDRESS 0x0800C000
#define SHARED_FLASH_PAGE     25
#define APP_ADDRESS           (uint32_t)0x0800C800
#define APP_FLASH_PAGE        26

#endif /* _LUOSHAL_CONFIG_H_ */
