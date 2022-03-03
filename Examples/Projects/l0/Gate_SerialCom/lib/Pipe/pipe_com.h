/******************************************************************************
 * @file pipe_com
 * @brief communication driver
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef PIPE_COM_H
#define PIPE_COM_H

#include "pipe_buffer.h"
#include "streaming.h"

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

#define P2L_DMA_CLOCK_ENABLE()    __HAL_RCC_DMA1_CLK_ENABLE()
#define P2L_DMA                   DMA1
#define P2L_DMA_CHANNEL           LL_DMA_CHANNEL_6
#define P2L_DMA_REQUEST           LL_SYSCFG_USART3_RMP_DMA1CH67
#define P2L_DMA_TC(P2L_DMA)       LL_DMA_IsActiveFlag_TC6(P2L_DMA)
#define P2L_DMA_CLEAR_TC(P2L_DMA) LL_DMA_ClearFlag_TC6(P2L_DMA)

#define L2P_DMA_CLOCK_ENABLE()    __HAL_RCC_DMA1_CLK_ENABLE()
#define L2P_DMA                   DMA1
#define L2P_DMA_CHANNEL           LL_DMA_CHANNEL_7
#define L2P_DMA_REQUEST           LL_SYSCFG_USART3_RMP_DMA1CH67
#define L2P_DMA_TC(L2P_DMA)       LL_DMA_IsActiveFlag_TC7(L2P_DMA)
#define L2P_DMA_CLEAR_TC(L2P_DMA) LL_DMA_ClearFlag_TC7(L2P_DMA)
#define L2P_DMA_IRQ               DMA1_Channel4_5_6_7_IRQn
#define L2P_DMA_IRQHANDLER()      DMA1_Channel4_5_6_7_IRQHandler()

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void PipeCom_Init(void);
void PipeCom_SendL2P(uint8_t *data, uint16_t size);
volatile uint8_t PipeCom_SendL2PPending(void);
streaming_channel_t *get_L2P_StreamChannel(void);
streaming_channel_t *get_P2L_StreamChannel(void);

#endif /* PIPE_H */
