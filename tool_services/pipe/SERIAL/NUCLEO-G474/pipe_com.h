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

#define P2L_DMA_CLOCK_ENABLE()    __HAL_RCC_DMA1_CLK_ENABLE();
#define P2L_DMA                   DMA1
#define P2L_DMA_CHANNEL           LL_DMA_CHANNEL_3
#define P2L_DMA_REQUEST           LL_DMAMUX_REQ_LPUART1_RX
#define P2L_DMA_TC(P2L_DMA)       LL_DMA_IsActiveFlag_TC3(P2L_DMA)
#define P2L_DMA_CLEAR_TC(P2L_DMA) LL_DMA_ClearFlag_TC3(P2L_DMA)

#define L2P_DMA_CLOCK_ENABLE()    __HAL_RCC_DMA1_CLK_ENABLE();
#define L2P_DMA                   DMA1
#define L2P_DMA_CHANNEL           LL_DMA_CHANNEL_4
#define L2P_DMA_REQUEST           LL_DMAMUX_REQ_LPUART1_TX
#define L2P_DMA_TC(L2P_DMA)       LL_DMA_IsActiveFlag_TC4(L2P_DMA)
#define L2P_DMA_CLEAR_TC(L2P_DMA) LL_DMA_ClearFlag_TC4(L2P_DMA)
#define L2P_DMA_IRQ               DMA1_Channel4_IRQn
#define L2P_DMA_IRQHANDLER()      DMA1_Channel4_IRQHandler()

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void PipeCom_Init(void);
void PipeCom_Loop(void);
void PipeCom_SendL2P(uint8_t *data, uint16_t size);
volatile uint8_t PipeCom_SendL2PPending(void);
streaming_channel_t *get_L2P_StreamChannel(void);
streaming_channel_t *get_P2L_StreamChannel(void);

#endif /* PIPE_H */
