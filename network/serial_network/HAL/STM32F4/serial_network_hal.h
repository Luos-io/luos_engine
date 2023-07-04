/******************************************************************************
 * @file serial_hal
 * @brief hardware abstraction layer of serial communication driver for luos framework
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#ifndef _SERIAL_HAL_H_
#define _SERIAL_HAL_H_

#include "stdint.h"

// SERIAL_CONFIG need to be define in your node_config.h if you want to redefine
// SERIAL configuration usart or DMA If you need change something in this configuration
// you must redefine all the configuration in you node_config.h
#ifndef SERIAL_CONFIG
    #define SERIAL_TX_CLK() __HAL_RCC_GPIOA_CLK_ENABLE();
    #define SERIAL_TX_PIN   GPIO_PIN_2
    #define SERIAL_TX_PORT  GPIOA
    #define SERIAL_TX_AF    GPIO_AF7_USART2

    #define SERIAL_RX_CLK() __HAL_RCC_GPIOA_CLK_ENABLE();
    #define SERIAL_RX_PIN   GPIO_PIN_3
    #define SERIAL_RX_PORT  GPIOA
    #define SERIAL_RX_AF    GPIO_AF7_USART2

    #define SERIAL_COM_CLOCK_ENABLE() __HAL_RCC_USART2_CLK_ENABLE()
    #define SERIAL_COM                USART2
    #define SERIAL_COM_IRQ            USART2_IRQn
    #define SERIAL_COM_IRQHANDLER()   USART2_IRQHandler()

    #define SERIAL_RX_DMA_CLOCK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE();
    #define SERIAL_RX_DMA                         DMA1
    #define SERIAL_RX_DMA_CHANNEL                 LL_DMA_STREAM_5
    #define SERIAL_RX_DMA_REQUEST                 LL_DMA_CHANNEL_4
    #define SERIAL_RX_DMA_TC(SERIAL_RX_DMA)       LL_DMA_IsActiveFlag_TC5(SERIAL_RX_DMA)
    #define SERIAL_RX_DMA_CLEAR_TC(SERIAL_RX_DMA) LL_DMA_ClearFlag_TC5(SERIAL_RX_DMA)

    #define SERIAL_TX_DMA_CLOCK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE();
    #define SERIAL_TX_DMA                         DMA1
    #define SERIAL_TX_DMA_CHANNEL                 LL_DMA_STREAM_6
    #define SERIAL_TX_DMA_REQUEST                 LL_DMA_CHANNEL_4
    #define SERIAL_TX_DMA_TC(SERIAL_TX_DMA)       LL_DMA_IsActiveFlag_TC6(SERIAL_TX_DMA)
    #define SERIAL_TX_DMA_CLEAR_TC(SERIAL_TX_DMA) LL_DMA_ClearFlag_TC6(SERIAL_TX_DMA)
    #define SERIAL_TX_DMA_IRQ                     DMA1_Stream6_IRQn
    #define SERIAL_TX_DMA_IRQHANDLER()            DMA1_Stream6_IRQHandler()
#endif

void SerialHAL_Init(uint8_t *rx_buffer, uint32_t buffer_size);
void SerialHAL_Loop(void);
void SerialHAL_Send(uint8_t *data, uint16_t size);
uint8_t SerialHAL_GetPort(void); // Return the port number of the serial communication

#endif /* _SERIAL_HAL_H_ */
