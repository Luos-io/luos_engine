/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

// include file relative to your MCU family

#ifndef MCUFREQ
    #define MCUFREQ // MCU frequence
#endif

/*******************************************************************************
 * BOOTLOADER CONFIG
 ******************************************************************************/
#define SHARED_MEMORY_ADDRESS ((uint32_t)0x0800C000)
#define APP_START_ADDRESS     (uint32_t)0x0800C800 // Begining of the app on flash
#define APP_END_ADDRESS       (uint32_t) FLASH_END

#endif /* _LUOSHAL_CONFIG_H_ */
