/******************************************************************************
 * @file RobusHAL_Config
 * @brief This file allow you to configure RobusHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family STM32FO
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _RobusHAL_CONFIG_H_
#define _RobusHAL_CONFIG_H_

#include "stm32l0xx_hal.h"

#define _CRITICAL
#define DISABLE 0x00

// If your MCU do not Have DMA for tx transmit define USE_TX_IT
// If your MCU have CRC polynome 16 #define USE_CRC_HW 1 else #define USE_CRC_HW 0
#ifndef USE_CRC_HW
    #define USE_CRC_HW 1
#endif

#ifndef TIMERDIV
    #define TIMERDIV 1 // clock divider for timer clock chosen
#endif
/*******************************************************************************
 * PINOUT CONFIG
 ******************************************************************************/
#ifndef PORT_CLOCK_ENABLE
    #define PORT_CLOCK_ENABLE()           \
        do                                \
        {                                 \
            __HAL_RCC_GPIOA_CLK_ENABLE(); \
            __HAL_RCC_GPIOB_CLK_ENABLE(); \
        } while (0U)
#endif

// PTP pin definition
#ifndef PTPA_PIN
    #define PTPA_PIN GPIO_PIN_8
#endif
#ifndef PTPA_PORT
    #define PTPA_PORT GPIOA
#endif
#ifndef PTPA_IRQ
    #define PTPA_IRQ EXTI4_15_IRQn
#endif

#ifndef PTPB_PIN
    #define PTPB_PIN GPIO_PIN_13
#endif
#ifndef PTPB_PORT
    #define PTPB_PORT GPIOB
#endif
#ifndef PTPB_IRQ
    #define PTPB_IRQ EXTI4_15_IRQn
#endif

// COM pin definition
#ifndef TX_LOCK_DETECT_PIN
    #define TX_LOCK_DETECT_PIN DISABLE
#endif
#ifndef TX_LOCK_DETECT_PORT
    #define TX_LOCK_DETECT_PORT DISABLE
#endif
#ifndef TX_LOCK_DETECT_IRQ
    #define TX_LOCK_DETECT_IRQ DISABLE
#endif

#ifndef RX_EN_PIN
    #define RX_EN_PIN GPIO_PIN_14
#endif
#ifndef RX_EN_PORT
    #define RX_EN_PORT GPIOB
#endif

#ifndef TX_EN_PIN
    #define TX_EN_PIN GPIO_PIN_15
#endif
#ifndef TX_EN_PORT
    #define TX_EN_PORT GPIOB
#endif

#ifndef COM_TX_PIN
    #define COM_TX_PIN GPIO_PIN_9
#endif
#ifndef COM_TX_PORT
    #define COM_TX_PORT GPIOA
#endif
#ifndef COM_TX_AF
    #define COM_TX_AF GPIO_AF4_USART1
#endif

#ifndef COM_RX_PIN
    #define COM_RX_PIN GPIO_PIN_10
#endif
#ifndef COM_RX_PORT
    #define COM_RX_PORT GPIOA
#endif
#ifndef COM_RX_AF
    #define COM_RX_AF GPIO_AF4_USART1
#endif

#ifndef PINOUT_IRQHANDLER
    #define PINOUT_IRQHANDLER(PIN) HAL_GPIO_EXTI_Callback(PIN)
#endif

/*******************************************************************************
 * COM CONFIG
 ******************************************************************************/
#ifndef ROBUS_COM_CLOCK_ENABLE
    #define ROBUS_COM_CLOCK_ENABLE() __HAL_RCC_USART1_CLK_ENABLE()
#endif
#ifndef ROBUS_COM
    #define ROBUS_COM USART1
#endif
#ifndef ROBUS_COM_IRQ
    #define ROBUS_COM_IRQ USART1_IRQn
#endif
#ifndef ROBUS_COM_IRQHANDLER
    #define ROBUS_COM_IRQHANDLER() USART1_IRQHandler()
#endif
/*******************************************************************************
 * DMA CONFIG
 ******************************************************************************/
#ifndef ROBUS_DMA_CLOCK_ENABLE
    #define ROBUS_DMA_CLOCK_ENABLE() __HAL_RCC_DMA1_CLK_ENABLE();
#endif
#ifndef ROBUS_DMA
    #define ROBUS_DMA DMA1
#endif
#ifndef ROBUS_DMA_CHANNEL
    #define ROBUS_DMA_CHANNEL LL_DMA_CHANNEL_2
#endif
#ifndef ROBUS_DMA_REQUEST
    #define ROBUS_DMA_REQUEST LL_DMA_REQUEST_3
#endif
#ifndef ROBUS_DMA_REMAP
    #define ROBUS_DMA_REMAP LL_SYSCFG_USART1TX_RMP_DMA1CH2
#endif

/*******************************************************************************
 * COM TIMEOUT CONFIG
 ******************************************************************************/
#ifndef ROBUS_TIMER_CLOCK_ENABLE
    #define ROBUS_TIMER_CLOCK_ENABLE() __HAL_RCC_TIM6_CLK_ENABLE()
#endif
#ifndef ROBUS_TIMER
    #define ROBUS_TIMER TIM6
#endif
#ifndef ROBUS_TIMER_IRQ
    #define ROBUS_TIMER_IRQ TIM6_IRQn
#endif
#ifndef ROBUS_TIMER_IRQHANDLER
    #define ROBUS_TIMER_IRQHANDLER() TIM6_DAC_IRQHandler()
#endif

#endif /* _RobusHAL_CONFIG_H_ */
