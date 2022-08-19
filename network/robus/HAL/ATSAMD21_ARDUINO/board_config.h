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

#include <Arduino.h>

#if defined(ARDUINO_SAMD_ZERO)
    #define LUOS_COM_CLOCK_ENABLE()                                                                                                          \
        do                                                                                                                                   \
        {                                                                                                                                    \
            GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM0_CORE_Val) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN); \
            PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0;                                                                                         \
        } while (0U)
    #define LUOS_COM              SERCOM0
    #define LUOS_COM_IRQ          SERCOM0_IRQn
    #define LUOS_COM_IRQHANDLER() SERCOM0_Handler()
    #define LUOS_DMA_TRIGGER      2
#endif

#if (defined(ARDUINO_SAMD_MKR1000) || defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_MKRFox1200)      \
     || defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310) || defined(ARDUINO_SAMD_MKRGSM1400) \
     || defined(ARDUINO_SAMD_MKRNB1500) || defined(ARDUINO_SAMD_MKRZERO) || defined(ARDUINO_SAMD_NANO_33_IOT)    \
     || defined(SAMD_MKRVIDOR4000))
    #define LUOS_COM_CLOCK_ENABLE()                                                                                                          \
        do                                                                                                                                   \
        {                                                                                                                                    \
            GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM5_CORE_Val) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN); \
            PM->APBCMASK.reg |= PM_APBCMASK_SERCOM5;                                                                                         \
        } while (0U)
    #define LUOS_COM              SERCOM5
    #define LUOS_COM_IRQ          SERCOM5_IRQn
    #define LUOS_COM_IRQHANDLER() SERCOM5_Handler()
    #define LUOS_DMA_TRIGGER      12

#endif

#if defined(SEEED_XIAO_M0)
    #define LUOS_COM_CLOCK_ENABLE()                                                                                                          \
        do                                                                                                                                   \
        {                                                                                                                                    \
            GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM4_CORE_Val) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN); \
            PM->APBCMASK.reg |= PM_APBCMASK_SERCOM4;                                                                                         \
        } while (0U)
    #define LUOS_COM              SERCOM4
    #define LUOS_COM_IRQ          SERCOM4_IRQn
    #define LUOS_COM_IRQHANDLER() SERCOM4_Handler()
    #define LUOS_DMA_TRIGGER      SERCOM4_DMAC_ID_TX
    #define COM_TX_POS            0 // PAD0
    #define COM_RX_POS            1 // PAD1

    #define ARDUINO_PTPA_PIN 1
    #define ARDUINO_PTPB_PIN 2
    #define ARDUINO_RX_EN    3
    #define ARDUINO_TX_EN    4
#endif

#endif /* _BOARD_CONFIG_H_ */
