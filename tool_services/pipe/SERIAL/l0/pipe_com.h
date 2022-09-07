/******************************************************************************
 * @file pipe_com
 * @brief communication driver
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef PIPE_COM_H
#define PIPE_COM_H

#include "_pipe.h"

#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_hal.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

// PIPE_CONFIG need to be define in your node_config.h if you want to redefine
// Pipe configuration usart or DMA If you need change something in this configuration
// you must redefine all the configuration in you node_config.h
#ifndef PIPE_CONFIG
    #define PIPE_TX_CLK() __HAL_RCC_GPIOB_CLK_ENABLE()
    #define PIPE_TX_PIN   GPIO_PIN_10
    #define PIPE_TX_PORT  GPIOB
    #define PIPE_TX_AF    GPIO_AF4_USART3

    #define PIPE_RX_CLK() __HAL_RCC_GPIOB_CLK_ENABLE()
    #define PIPE_RX_PIN   GPIO_PIN_11
    #define PIPE_RX_PORT  GPIOB
    #define PIPE_RX_AF    GPIO_AF4_USART3

    #define PIPE_COM_CLOCK_ENABLE() __HAL_RCC_USART3_CLK_ENABLE()
    #define PIPE_COM                USART3
    #define PIPE_COM_IRQ            USART3_4_IRQn
    #define PIPE_COM_IRQHANDLER()   USART3_4_IRQHandler()

    #define PIPE_RX_DMA_CLOCK_ENABLE()        __HAL_RCC_DMA1_CLK_ENABLE()
    #define PIPE_RX_DMA                       DMA1
    #define PIPE_RX_DMA_CHANNEL               LL_DMA_CHANNEL_6
    #define PIPE_RX_DMA_REQUEST               LL_SYSCFG_USART3_RMP_DMA1CH67
    #define PIPE_RX_DMA_TC(PIPE_RX_DMA)       LL_DMA_IsActiveFlag_TC6(PIPE_RX_DMA)
    #define PIPE_RX_DMA_CLEAR_TC(PIPE_RX_DMA) LL_DMA_ClearFlag_TC6(PIPE_RX_DMA)

    #define PIPE_TX_DMA_CLOCK_ENABLE()        __HAL_RCC_DMA1_CLK_ENABLE()
    #define PIPE_TX_DMA                       DMA1
    #define PIPE_TX_DMA_CHANNEL               LL_DMA_CHANNEL_7
    #define PIPE_TX_DMA_REQUEST               LL_SYSCFG_USART3_RMP_DMA1CH67
    #define PIPE_TX_DMA_TC(PIPE_TX_DMA)       LL_DMA_IsActiveFlag_TC7(PIPE_TX_DMA)
    #define PIPE_TX_DMA_CLEAR_TC(PIPE_TX_DMA) LL_DMA_ClearFlag_TC7(PIPE_TX_DMA)
    #define PIPE_TX_DMA_IRQ                   DMA1_Channel4_5_6_7_IRQn
    #define PIPE_TX_DMA_IRQHANDLER()          DMA1_Channel4_5_6_7_IRQHandler()
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void PipeCom_Init(void);
void PipeCom_Loop(void);
uint8_t PipeCom_Receive(uint16_t *size);
void PipeCom_Send(void);

#endif /* PIPE_H */
