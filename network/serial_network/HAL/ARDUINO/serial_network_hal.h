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
    #define SERIAL_TX_CLK() __HAL_RCC_GPIOB_CLK_ENABLE()
    #define SERIAL_TX_PIN   GPIO_PIN_10
    #define SERIAL_TX_PORT  GPIOB
    #define SERIAL_TX_AF    GPIO_AF4_USART3

    #define SERIAL_RX_CLK() __HAL_RCC_GPIOB_CLK_ENABLE()
    #define SERIAL_RX_PIN   GPIO_PIN_11
    #define SERIAL_RX_PORT  GPIOB
    #define SERIAL_RX_AF    GPIO_AF4_USART3

    #define SERIAL_COM_CLOCK_ENABLE() __HAL_RCC_USART3_CLK_ENABLE()
    #define SERIAL_COM                USART3
    #define SERIAL_COM_IRQ            USART3_4_IRQn
    #define SERIAL_COM_IRQHANDLER()   USART3_4_IRQHandler()

    #define SERIAL_RX_DMA_CLOCK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE()
    #define SERIAL_RX_DMA                         DMA1
    #define SERIAL_RX_DMA_CHANNEL                 LL_DMA_CHANNEL_6
    #define SERIAL_RX_DMA_REQUEST                 LL_SYSCFG_USART3_RMP_DMA1CH67
    #define SERIAL_RX_DMA_TC(SERIAL_RX_DMA)       LL_DMA_IsActiveFlag_TC6(SERIAL_RX_DMA)
    #define SERIAL_RX_DMA_CLEAR_TC(SERIAL_RX_DMA) LL_DMA_ClearFlag_TC6(SERIAL_RX_DMA)

    #define SERIAL_TX_DMA_CLOCK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE()
    #define SERIAL_TX_DMA                         DMA1
    #define SERIAL_TX_DMA_CHANNEL                 LL_DMA_CHANNEL_7
    #define SERIAL_TX_DMA_REQUEST                 LL_SYSCFG_USART3_RMP_DMA1CH67
    #define SERIAL_TX_DMA_TC(SERIAL_TX_DMA)       LL_DMA_IsActiveFlag_TC7(SERIAL_TX_DMA)
    #define SERIAL_TX_DMA_CLEAR_TC(SERIAL_TX_DMA) LL_DMA_ClearFlag_TC7(SERIAL_TX_DMA)
    #define SERIAL_TX_DMA_IRQ                     DMA1_Channel4_5_6_7_IRQn
    #define SERIAL_TX_DMA_IRQHANDLER()            DMA1_Channel4_5_6_7_IRQHandler()
#endif

void SerialHAL_Init(uint8_t *rx_buffer, uint32_t buffer_size);
void SerialHAL_Loop(void);
void SerialHAL_Send(uint8_t *data, uint16_t size);
uint8_t SerialHAL_GetPort(void); // Return the port number of the serial communication

#endif /* _SERIAL_HAL_H_ */
