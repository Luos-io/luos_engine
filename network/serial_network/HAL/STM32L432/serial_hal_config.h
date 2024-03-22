/******************************************************************************
 * @file serial_hal_config
 * @brief This file allow you to configure serial_hal according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file.
 *        If you want to ovewrite some, overlap the defines in your node_config.h
 * @MCU Family STM32L4
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _SERIAL_HAL_CONFIG_H_
#define _SERIAL_HAL_CONFIG_H_

// TX Pinout
#ifndef SERIAL_TX_CLK
    #define SERIAL_TX_CLK() __HAL_RCC_GPIOA_CLK_ENABLE();
#endif
#ifndef SERIAL_TX_PIN
    #define SERIAL_TX_PIN GPIO_PIN_2
#endif
#ifndef SERIAL_TX_PORT
    #define SERIAL_TX_PORT GPIOA
#endif
#ifndef SERIAL_TX_AF
    #define SERIAL_TX_AF GPIO_AF7_USART2
#endif

// RX Pinout
#ifndef SERIAL_RX_CLK
    #define SERIAL_RX_CLK() __HAL_RCC_GPIOA_CLK_ENABLE();
#endif
#ifndef SERIAL_RX_PIN
    #define SERIAL_RX_PIN GPIO_PIN_15
#endif
#ifndef SERIAL_RX_PORT
    #define SERIAL_RX_PORT GPIOA
#endif
#ifndef SERIAL_RX_AF
    #define SERIAL_RX_AF GPIO_AF3_USART2
#endif

// USART configuration
#ifndef SERIAL_COM_CLOCK_ENABLE
    #define SERIAL_COM_CLOCK_ENABLE() __HAL_RCC_USART2_CLK_ENABLE()
#endif
#ifndef SERIAL_COM
    #define SERIAL_COM USART2
#endif

// RX DMA configuration
#ifndef SERIAL_RX_DMA_CLOCK_ENABLE
    #define SERIAL_RX_DMA_CLOCK_ENABLE() __HAL_RCC_DMA1_CLK_ENABLE()
#endif
#ifndef SERIAL_RX_DMA
    #define SERIAL_RX_DMA DMA1
#endif
#ifndef SERIAL_RX_DMA_CHANNEL
    #define SERIAL_RX_DMA_CHANNEL LL_DMA_CHANNEL_6
#endif
#ifndef SERIAL_RX_DMA_REQUEST
    #define SERIAL_RX_DMA_REQUEST LL_DMA_REQUEST_2
#endif
#ifndef SERIAL_RX_DMA_TC
    #define SERIAL_RX_DMA_TC(SERIAL_RX_DMA) LL_DMA_IsActiveFlag_TC6(SERIAL_RX_DMA)
#endif
#ifndef SERIAL_RX_DMA_CLEAR_TC
    #define SERIAL_RX_DMA_CLEAR_TC(SERIAL_RX_DMA) LL_DMA_ClearFlag_TC6(SERIAL_RX_DMA)
#endif

// TX DMA configuration
#ifndef SERIAL_TX_DMA_CLOCK_ENABLE
    #define SERIAL_TX_DMA_CLOCK_ENABLE() __HAL_RCC_DMA1_CLK_ENABLE()
#endif
#ifndef SERIAL_TX_DMA
    #define SERIAL_TX_DMA DMA1
#endif
#ifndef SERIAL_TX_DMA_CHANNEL
    #define SERIAL_TX_DMA_CHANNEL LL_DMA_CHANNEL_7
#endif
#ifndef SERIAL_TX_DMA_REQUEST
    #define SERIAL_TX_DMA_REQUEST LL_DMA_REQUEST_2
#endif
#ifndef SERIAL_TX_DMA_TC
    #define SERIAL_TX_DMA_TC(SERIAL_TX_DMA) LL_DMA_IsActiveFlag_TC7(SERIAL_TX_DMA)
#endif
#ifndef SERIAL_TX_DMA_CLEAR_TC
    #define SERIAL_TX_DMA_CLEAR_TC(SERIAL_TX_DMA) LL_DMA_ClearFlag_TC7(SERIAL_TX_DMA)
#endif
#ifndef SERIAL_TX_DMA_IRQ
    #define SERIAL_TX_DMA_IRQ DMA1_Channel7_IRQn
#endif
#ifndef SERIAL_TX_DMA_IRQHANDLER
    #define SERIAL_TX_DMA_IRQHANDLER() DMA1_Channel7_IRQHandler()
#endif

#endif /* _SERIAL_HAL_CONFIG_H_ */
