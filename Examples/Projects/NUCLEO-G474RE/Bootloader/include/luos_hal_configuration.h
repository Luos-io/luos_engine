/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family STM32G4
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIGURATION_H_
#define _LUOSHAL_CONFIGURATION_H_

/*******************************************************************************
 * PINOUT CONFIG
 ******************************************************************************/
#ifndef PORT_CLOCK_ENABLE
#define PORT_CLOCK_ENABLE()           \
    do                                \
    {                                 \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
        __HAL_RCC_GPIOC_CLK_ENABLE(); \
    } while (0U)
#endif

// PTP pin definition
#define PTPA_PIN  GPIO_PIN_8
#define PTPA_PORT GPIOB
#define PTPA_IRQ  EXTI9_5_IRQn

#define PTPB_PIN  GPIO_PIN_13
#define PTPB_PORT GPIOC
#define PTPB_IRQ  EXTI15_10_IRQn

// COM pin definition
#define RX_EN_PIN  GPIO_PIN_6
#define RX_EN_PORT GPIOC

#define TX_EN_PIN  GPIO_PIN_14
#define TX_EN_PORT GPIOB

#define COM_TX_PIN  GPIO_PIN_10
#define COM_TX_PORT GPIOB
#define COM_TX_AF   GPIO_AF7_USART3

#define COM_RX_PIN  GPIO_PIN_11
#define COM_RX_PORT GPIOB
#define COM_RX_AF   GPIO_AF7_USART3

/*******************************************************************************
 * COM CONFIG
 ******************************************************************************/
#define LUOS_COM_CLOCK_ENABLE() __HAL_RCC_USART3_CLK_ENABLE()
#define LUOS_COM                USART3
#define LUOS_COM_IRQ            USART3_IRQn
#define LUOS_COM_IRQHANDLER()   USART3_IRQHandler()

/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#define LUOS_DMA_REQUEST LL_DMAMUX_REQ_USART3_TX

#endif /* _LUOSHAL_CONFIGURATION_H_ */