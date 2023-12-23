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
#define SHARED_MEMORY_ADDRESS // Begining of the shared on flash after bootloader
#define APP_START_ADDRESS     // Begining of the app on flash
#define APP_END_ADDRESS       // end of the app on flash

#endif /* _LUOSHAL_CONFIG_H_ */
