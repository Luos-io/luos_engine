/******************************************************************************
 * @file RobusHAL_Config
 * @brief This file allow you to configure RobusHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _RobusHAL_CONFIG_H_
#define _RobusHAL_CONFIG_H_

// include main sdk files relative to your MCU family
#include "nrf.h"
#define DISABLE 0x00

// If your MCU do not Have DMA for tx transmit define USE_TX_IT

// If your MCU have CRC polynome 16 #define USE_CRC_HW 1 else #define USE_CRC_HW 0
#ifndef USE_CRC_HW
    #define USE_CRC_HW 0
#endif

#ifndef TIMERDIV
    #define TIMERDIV 2 // clock divider for timer clock chosen
#endif
/*******************************************************************************
 * PINOUT CONFIG
 ******************************************************************************/
#ifndef PORT_CLOCK_ENABLE
    #define PORT_CLOCK_ENABLE
#endif

// PTP pin definition
#ifndef PTPA_PIN
    #define PTPA_PIN 22
#endif

/*#ifndef PTPA_IRQ
    #define PTPA_IRQ GPIOTE_IRQn
#endif*/

#ifndef PTPB_PIN
    #define PTPB_PIN 23
#endif

// COM pin definition
#ifndef TX_LOCK_DETECT_PORT
    #define TX_LOCK_DETECT_PIN DISABLE
#endif

#ifndef RX_EN_PIN
    #define RX_EN_PIN DISABLE
#endif

#ifndef TX_EN_PIN
    #define TX_EN_PIN 24
#endif

#ifndef COM_TX_PIN
    #define COM_TX_PIN 25
#endif

#ifndef COM_RX_PIN
    #define COM_RX_PIN 26
#endif

/*******************************************************************************
 * COM CONFIG
 ******************************************************************************/
/*#ifndef LUOS_COM_CLOCK_ENABLE
    #define LUOS_COM_CLOCK_ENABLE
#endif
#ifndef LUOS_COM
    #define LUOS_COM
#endif*/
#ifndef LUOS_COM_IRQ
    #define LUOS_COM_IRQ UARTE0_UART0_IRQn
#endif
#ifndef LUOS_COM_IRQHANDLER
    #define LUOS_COM_IRQHANDLER() nrfx_uart_0_irq_handler()
#endif
/*******************************************************************************
 * DMA CONFIG
 ******************************************************************************/
#ifndef LUOS_DMA_CLOCK_ENABLE
    #define LUOS_DMA_CLOCK_ENABLE
#endif
#ifndef LUOS_DMA
    #define LUOS_DMA
#endif
#ifndef LUOS_DMA_CHANNEL
    #define LUOS_DMA_CHANNEL
#endif
/*******************************************************************************
 * COM TIMEOUT CONFIG
 ******************************************************************************/
/*#ifndef LUOS_TIMER_CLOCK_ENABLE
    #define LUOS_TIMER_CLOCK_ENABLE nrfx_timer_enable
#endif
#ifndef LUOS_TIMER
    #define LUOS_TIMER NRF_DRV_TIMER_INSTANCE(0)
#endif*/
#ifndef LUOS_TIMER_IRQ
    #define LUOS_TIMER_IRQ TIMER1_IRQn
#endif
/*#ifndef LUOS_TIMER_IRQHANDLER
    #define LUOS_TIMER_IRQHANDLER() TIMER1_IRQHandler()
#endif*/

#endif /* _RobusHAL_CONFIG_H_ */
