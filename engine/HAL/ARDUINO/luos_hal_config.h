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

/*******************************************************************************
 * DEFINE THREAD MUTEX LOCKING AND UNLOCKING FUNCTIONS
 ******************************************************************************/
#ifndef MSGALLOC_MUTEX_LOCK
    #define MSGALLOC_MUTEX_LOCK
#endif
#ifndef MSGALLOC_MUTEX_UNLOCK
    #define MSGALLOC_MUTEX_UNLOCK
#endif

#ifndef LUOS_MUTEX_LOCK
    #define LUOS_MUTEX_LOCK
#endif
#ifndef LUOS_MUTEX_UNLOCK
    #define LUOS_MUTEX_UNLOCK
#endif

/*******************************************************************************
 * BOOTLOADER CONFIG
 ******************************************************************************/

#endif /* _LUOSHAL_CONFIG_H_ */
