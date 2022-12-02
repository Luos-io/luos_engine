/******************************************************************************
 * @file RobusHAL_Config
 * @brief This file allow you to configure RobusHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family ATSAMD21
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include "sdkconfig.h"
#include "hal/gpio_hal.h"
#include "hal/timer_hal.h"
#include "hal/uart_hal.h"

#ifdef CONFIG_IDF_TARGET_ESP32
    #define PTPA_PIN   GPIO_NUM_26
    #define PTPB_PIN   GPIO_NUM_27
    #define TX_EN_PIN  GPIO_NUM_25
    #define COM_TX_PIN GPIO_NUM_10
    #define COM_RX_PIN GPIO_NUM_9 // this pin should pin pull up to vcc if no internal pull up

    #define LUOS_TIMER_GROUP TIMER_GROUP_1

#elif CONFIG_IDF_TARGET_ESP32C3
    #define PTPA_PIN   GPIO_NUM_6
    #define PTPB_PIN   GPIO_NUM_7
    #define TX_EN_PIN  GPIO_NUM_9
    #define COM_TX_PIN GPIO_NUM_4
    #define COM_RX_PIN GPIO_NUM_5

#else
    #define PTPA_PIN   GPIO_NUM_12
    #define PTPB_PIN   GPIO_NUM_13
    #define TX_EN_PIN  GPIO_NUM_14
    #define COM_TX_PIN GPIO_NUM_17
    #define COM_RX_PIN GPIO_NUM_18

#endif

#endif /* _BOARD_CONFIG_H_ */
