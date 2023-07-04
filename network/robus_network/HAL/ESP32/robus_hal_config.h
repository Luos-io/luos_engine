/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family ATSAMD21
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _RobusHAL_CONFIG_H_
#define _RobusHAL_CONFIG_H_

#include "board_config.h"
#include "esp_attr.h"
#include <soc/soc.h>

#define _CRITICAL IRAM_ATTR
#define DISABLE   0x00

#ifndef MCUFREQ
    #define MCUFREQ 80000000 // MCU frequence
#endif

// If your MCU do not Have DMA for tx transmit #define USE_TX_IT
// If your MCU have CRC polynome 16 #define USE_CRC_HW 1 else #define USE_CRC_HW 0
#ifndef USE_CRC_HW
    #define USE_CRC_HW 0
#endif

#ifndef TIMERDIV
    #define TIMERDIV 1 // clock divider for timer clock chosen
#endif
/*******************************************************************************
 * PINOUT CONFIG
 ******************************************************************************/
// PTP pin definition
#ifndef PTPA_PIN
    #define PTPA_PIN GPIO_NUM_26
#endif

#ifndef PTPB_PIN
    #define PTPB_PIN GPIO_NUM_27
#endif

// COM pin definition
#ifndef TX_LOCK_DETECT_PIN
    #define TX_LOCK_DETECT_PIN DISABLE
#endif

#ifndef RX_EN_PIN
    #define RX_EN_PIN DISABLE
#endif

#ifndef TX_EN_PIN
    #define TX_EN_PIN GPIO_NUM_25
#endif

#ifndef COM_TX_PIN
    #define COM_TX_PIN GPIO_NUM_10
#endif

#ifndef COM_RX_PIN
    #define COM_RX_PIN GPIO_NUM_9 // this pin should pin pull up to vcc if no internal pull up
#endif

/*******************************************************************************
 * COM CONFIG
 ******************************************************************************/
#ifndef LUOS_COM
    #define LUOS_COM UART_NUM_1
#endif
/*******************************************************************************
 * COM TIMEOUT CONFIG
 ******************************************************************************/
#ifndef LUOS_TIMER_GROUP
    #define LUOS_TIMER_GROUP TIMER_GROUP_0
#endif
#ifndef LUOS_TIMER
    #define LUOS_TIMER TIMER_0
#endif

#endif /* _LUOSHAL_CONFIG_H_ */
