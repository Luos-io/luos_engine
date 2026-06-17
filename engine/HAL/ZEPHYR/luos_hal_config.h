/******************************************************************************
 * @file luos_hal_config
 * @brief Luos HAL config for Zephyr. No bus peripheral is reserved here; the
 *        serial network's radio HAL owns the physical layer.
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

#ifndef MCUFREQ
    #define MCUFREQ 128000000 /* nRF54L15 application core max */
#endif

/*******************************************************************************
 * DEFINE THREAD MUTEX LOCKING AND UNLOCKING FUNCTIONS
 * Engine runs in a single cooperative loop here; no-op like the bare-metal HALs.
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

#endif /* _LUOSHAL_CONFIG_H_ */
