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

#include "esp_attr.h"
#include <soc/soc.h>

#ifndef MCUFREQ
    #define MCUFREQ APB_CLK_FREQ // MCU frequence
#endif

/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef PAGE_SIZE
    #define PAGE_SIZE
#endif
#ifndef ERASE_SIZE
    #define ERASE_SIZE
#endif
#ifndef FLASH_SIZE
    #define FLASH_SIZE
#endif
#ifndef ADDRESS_LAST_PAGE_FLASH
    #define ADDRESS_LAST_PAGE_FLASH FLASH_SIZE - (16 * PAGE_SIZE)
#endif

/*******************************************************************************
 * BOOTLOADER CONFIG
 ******************************************************************************/
#define FLASH_END
#define SHARED_MEMORY_ADDRESS
#define APP_ADDRESS

#endif /* _LUOSHAL_CONFIG_H_ */
