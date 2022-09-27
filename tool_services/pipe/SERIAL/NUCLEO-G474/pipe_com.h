/******************************************************************************
 * @file pipe_com
 * @brief communication driver
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef PIPE_COM_H
#define PIPE_COM_H

#include "_pipe.h"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_ll_lpuart.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_hal.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
// PIPE_CONFIG need to be define in your node_config.h if you want to redefine
// Pipe configuration usart or DMA If you need change something in this configuration
// you must redefine all the configuration in you node_config.h
#ifndef PIPE_CONFIG
    #define PIPE_TX_CLK() __HAL_RCC_GPIOA_CLK_ENABLE();
    #define PIPE_TX_PIN   GPIO_PIN_2
    #define PIPE_TX_PORT  GPIOA
    #define PIPE_TX_AF    GPIO_AF12_LPUART1

    #define PIPE_RX_CLK() __HAL_RCC_GPIOA_CLK_ENABLE();
    #define PIPE_RX_PIN   GPIO_PIN_3
    #define PIPE_RX_PORT  GPIOA
    #define PIPE_RX_AF    GPIO_AF12_LPUART1

    #define PIPE_COM_CLOCK_ENABLE() __HAL_RCC_LPUART1_CLK_ENABLE()
    #define PIPE_COM                LPUART1
    #define PIPE_COM_IRQ            LPUART1_IRQn
    #define PIPE_COM_IRQHANDLER()   LPUART1_IRQHandler()

    #define PIPE_RX_DMA_CLOCK_ENABLE()        __HAL_RCC_DMA1_CLK_ENABLE();
    #define PIPE_RX_DMA                       DMA1
    #define PIPE_RX_DMA_CHANNEL               LL_DMA_CHANNEL_3
    #define PIPE_RX_DMA_REQUEST               LL_DMAMUX_REQ_LPUART1_RX
    #define PIPE_RX_DMA_TC(PIPE_RX_DMA)       LL_DMA_IsActiveFlag_TC3(PIPE_RX_DMA)
    #define PIPE_RX_DMA_CLEAR_TC(PIPE_RX_DMA) LL_DMA_ClearFlag_TC3(PIPE_RX_DMA)

    #define PIPE_TX_DMA_CLOCK_ENABLE()        __HAL_RCC_DMA1_CLK_ENABLE();
    #define PIPE_TX_DMA                       DMA1
    #define PIPE_TX_DMA_CHANNEL               LL_DMA_CHANNEL_4
    #define PIPE_TX_DMA_REQUEST               LL_DMAMUX_REQ_LPUART1_TX
    #define PIPE_TX_DMA_TC(PIPE_TX_DMA)       LL_DMA_IsActiveFlag_TC4(PIPE_TX_DMA)
    #define PIPE_TX_DMA_CLEAR_TC(PIPE_TX_DMA) LL_DMA_ClearFlag_TC4(PIPE_TX_DMA)
    #define PIPE_TX_DMA_IRQ                   DMA1_Channel4_IRQn
    #define PIPE_TX_DMA_IRQHANDLER()          DMA1_Channel4_IRQHandler()
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
