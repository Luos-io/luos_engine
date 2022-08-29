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
#define PIPE_TX_CLK() __HAL_RCC_GPIOA_CLK_ENABLE()
#define PIPE_TX_PIN   GPIO_PIN_2
#define PIPE_TX_PORT  GPIOA
#define PIPE_TX_AF    GPIO_AF1_USART2

#define PIPE_RX_CLK() __HAL_RCC_GPIOA_CLK_ENABLE()
#define PIPE_RX_PIN   GPIO_PIN_3
#define PIPE_RX_PORT  GPIOA
#define PIPE_RX_AF    GPIO_AF1_USART2

#define PIPE_COM_CLOCK_ENABLE() __HAL_RCC_USART2_CLK_ENABLE()
#define PIPE_COM                USART2
#define PIPE_COM_IRQ            USART2_IRQn
#define PIPE_COM_IRQHANDLER()   USART2_IRQHandler()

#define PIPE_RX_DMA_CLOCK_ENABLE()        __HAL_RCC_DMA1_CLK_ENABLE()
#define PIPE_RX_DMA                       DMA1
#define PIPE_RX_DMA_CHANNEL               LL_DMA_CHANNEL_5
#define PIPE_RX_DMA_REQUEST               LL_SYSCFG_USART2_RMP_DMA1CH54
#define PIPE_RX_DMA_TC(PIPE_RX_DMA)       LL_DMA_IsActiveFlag_TC5(PIPE_RX_DMA)
#define PIPE_RX_DMA_CLEAR_TC(PIPE_RX_DMA) LL_DMA_ClearFlag_TC5(PIPE_RX_DMA)

#define PIPE_TX_DMA_CLOCK_ENABLE()        __HAL_RCC_DMA1_CLK_ENABLE()
#define PIPE_TX_DMA                       DMA1
#define PIPE_TX_DMA_CHANNEL               LL_DMA_CHANNEL_4
#define PIPE_TX_DMA_REQUEST               LL_SYSCFG_USART2_RMP_DMA1CH54
#define PIPE_TX_DMA_TC(PIPE_TX_DMA)       LL_DMA_IsActiveFlag_TC4(PIPE_TX_DMA)
#define PIPE_TX_DMA_CLEAR_TC(PIPE_TX_DMA) LL_DMA_ClearFlag_TC4(PIPE_TX_DMA)
#define PIPE_TX_DMA_IRQ                   DMA1_Channel4_5_6_7_IRQn
#define PIPE_TX_DMA_IRQHANDLER()          DMA1_Channel4_5_6_7_IRQHandler()

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
