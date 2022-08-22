/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @Family x86
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

#ifndef MCUFREQ
#define MCUFREQ 100000000 // MCU frequence
#endif

/*******************************************************************************
 * DEFINE STUB FLASH FOR X86
 ******************************************************************************/
#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE 0x100
#endif
#ifndef FLASH_PAGE_NUMBER
#define FLASH_PAGE_NUMBER 8
#endif
static uint32_t stub_flash_x86[FLASH_PAGE_NUMBER][FLASH_PAGE_SIZE];
static uint32_t *last_page_stub_flash_x86 = &stub_flash_x86[FLASH_PAGE_NUMBER - 1][FLASH_PAGE_SIZE];

/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef PAGE_SIZE
#define PAGE_SIZE (uint32_t) FLASH_PAGE_SIZE
#endif
#ifndef ADDRESS_LAST_PAGE_FLASH
#define ADDRESS_LAST_PAGE_FLASH (uint32_t) last_page_stub_flash_x86
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
