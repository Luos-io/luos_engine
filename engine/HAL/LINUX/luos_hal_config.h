/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @Family Linux (Raspberry Pi)
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

#include <pthread.h>

#ifndef MCUFREQ
    #define MCUFREQ 1500000000
#endif

/*******************************************************************************
 * DEFINE STUB FLASH FOR LINUX
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
 * SINGLE RECURSIVE MUTEX FOR ALL CRITICAL SECTIONS
 * Consolidates IRQ state, MSGALLOC_MUTEX, and LUOS_MUTEX into one
 * recursive mutex to prevent lock-ordering deadlocks.
 ******************************************************************************/
extern pthread_mutex_t luos_recursive_mutex;

#ifndef MSGALLOC_MUTEX_LOCK
    #define MSGALLOC_MUTEX_LOCK pthread_mutex_lock(&luos_recursive_mutex);
#endif
#ifndef MSGALLOC_MUTEX_UNLOCK
    #define MSGALLOC_MUTEX_UNLOCK pthread_mutex_unlock(&luos_recursive_mutex);
#endif

#ifndef LUOS_MUTEX_LOCK
    #define LUOS_MUTEX_LOCK pthread_mutex_lock(&luos_recursive_mutex);
#endif
#ifndef LUOS_MUTEX_UNLOCK
    #define LUOS_MUTEX_UNLOCK pthread_mutex_unlock(&luos_recursive_mutex);
#endif

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
 * NODE PERSISTENCE
 ******************************************************************************/
#ifndef LUOS_PERSIST_PATH
    #define LUOS_PERSIST_PATH "/var/lib/luos/node_config"
#endif

#endif /* _LUOSHAL_CONFIG_H_ */
