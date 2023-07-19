/******************************************************************************
 * @file serial_hal_config
 * @brief This file allow you to configure serial_hal according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file.
 *        If you want to ovewrite some, overlap the defines in your node_config.h
 * @MCU Family TEMPLATE
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _SERIAL_HAL_CONFIG_H_
#define _SERIAL_HAL_CONFIG_H_

// TX Pinout
#ifndef SERIAL_TX_CLK
    #define SERIAL_TX_CLK() DEFAULT_CLK_PORT_ENABLE();
#endif
#ifndef SERIAL_TX_PIN
    #define SERIAL_TX_PIN GPIO_PIN
#endif
#ifndef SERIAL_TX_PORT
    #define SERIAL_TX_PORT GPIO_PORT
#endif
#ifndef SERIAL_TX_AF
    #define SERIAL_TX_AF GPIO_AF
#endif

// RX Pinout
#ifndef SERIAL_RX_CLK
    #define SERIAL_RX_CLK() DEFAULT_CLK_PORT_ENABLE();
#endif
#ifndef SERIAL_RX_PIN
    #define SERIAL_RX_PIN GPIO_PIN
#endif
#ifndef SERIAL_RX_PORT
    #define SERIAL_RX_PORT GPIO_PORT
#endif
#ifndef SERIAL_RX_AF
    #define SERIAL_RX_AF GPIO_AF
#endif

// USART configuration
#ifndef SERIAL_COM_CLOCK_ENABLE
    #define SERIAL_COM_CLOCK_ENABLE() DEFAULT_CLK_SERIAL_ENABLE()
#endif
#ifndef SERIAL_COM
    #define SERIAL_COM USART
#endif
#ifndef SERIAL_COM_IRQ
    #define SERIAL_COM_IRQ USART_IRQn
#endif
#ifndef SERIAL_COM_IRQHANDLER
    #define SERIAL_COM_IRQHANDLER() USART_IRQHandler()
#endif

// RX DMA configuration
#ifndef SERIAL_RX_DMA_CLOCK_ENABLE
    #define SERIAL_RX_DMA_CLOCK_ENABLE() DEFAULT_CLK_DMA_ENABLE()
#endif
#ifndef SERIAL_RX_DMA
    #define SERIAL_RX_DMA DMA
#endif
#ifndef SERIAL_RX_DMA_CHANNEL
    #define SERIAL_RX_DMA_CHANNEL DMA_CHANNEL
#endif
#ifndef SERIAL_RX_DMA_REQUEST
    #define SERIAL_RX_DMA_REQUEST DMA_REQUEST
#endif
#ifndef SERIAL_RX_DMA_TC
    #define SERIAL_RX_DMA_TC(SERIAL_RX_DMA) TRANSFERT_COMPLETE(SERIAL_RX_DMA)
#endif
#ifndef SERIAL_RX_DMA_CLEAR_TC
    #define SERIAL_RX_DMA_CLEAR_TC(SERIAL_RX_DMA) TRANSFERT_COMPLETE_CLEAR(SERIAL_RX_DMA)
#endif

// TX DMA configuration
#ifndef SERIAL_TX_DMA_CLOCK_ENABLE
    #define SERIAL_TX_DMA_CLOCK_ENABLE() DEFAULT_CLK_DMA_ENABLE()
#endif
#ifndef SERIAL_TX_DMA
    #define SERIAL_TX_DMA DMA
#endif
#ifndef SERIAL_TX_DMA_CHANNEL
    #define SERIAL_TX_DMA_CHANNEL DMA_CHANNEL
#endif
#ifndef SERIAL_TX_DMA_REQUEST
    #define SERIAL_TX_DMA_REQUEST DMA_REQUEST
#endif
#ifndef SERIAL_TX_DMA_TC
    #define SERIAL_TX_DMA_TC(SERIAL_TX_DMA) TRANSFERT_COMPLETE(SERIAL_TX_DMA)
#endif
#ifndef SERIAL_TX_DMA_CLEAR_TC
    #define SERIAL_TX_DMA_CLEAR_TC(SERIAL_TX_DMA) TRANSFERT_COMPLETE_CLEAR(SERIAL_TX_DMA)
#endif
#ifndef SERIAL_TX_DMA_IRQ
    #define SERIAL_TX_DMA_IRQ DMA_IRQn
#endif
#ifndef SERIAL_TX_DMA_IRQHANDLER
    #define SERIAL_TX_DMA_IRQHANDLER() DMA_Channel_IRQHandler()
#endif

#endif /* _SERIAL_HAL_CONFIG_H_ */
