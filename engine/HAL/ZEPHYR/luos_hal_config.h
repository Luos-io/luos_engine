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

#endif /* _LUOSHAL_CONFIG_H_ */
