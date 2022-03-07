/******************************************************************************
 * @file RobusHAL_Config
 * @brief This file allow you to configure RobusHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family STM32F4
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _RobusHAL_CONFIG_H_
#define _RobusHAL_CONFIG_H_

#include "stm32f4xx_hal.h"

#define DISABLE 0x00

// If your MCU do not Have DMA for tx transmit define USE_TX_IT
// If your MCU have CRC polynome 16 #define USE_CRC_HW 1 else #define USE_CRC_HW 0
#ifndef USE_CRC_HW
#define USE_CRC_HW 0
#endif

#ifndef MCUFREQ
#define MCUFREQ 168000000 // MCU frequence 168000000
#endif
#ifndef TIMERDIV
#define TIMERDIV 2 // clock divider for timer clock chosen 2
#endif

/*******************************************************************************
 * PINOUT CONFIG
 ******************************************************************************/
#ifndef PORT_CLOCK_ENABLE
#define PORT_CLOCK_ENABLE()           \
    do                                \
    {                                 \
        __HAL_RCC_GPIOA_CLK_ENABLE(); \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0U)
#endif

// PTP pin definition
#ifndef PTPA_PIN
#define PTPA_PIN GPIO_PIN_5
#endif
#ifndef PTPA_PORT
#define PTPA_PORT GPIOB
#endif
#ifndef PTPA_IRQ
#define PTPA_IRQ EXTI9_5_IRQn
#endif

#ifndef PTPB_PIN
#define PTPB_PIN GPIO_PIN_4
#endif
#ifndef PTPB_PORT
#define PTPB_PORT GPIOB
#endif
#ifndef PTPB_IRQ
#define PTPB_IRQ EXTI4_IRQn
#endif

// COM pin definition
#ifndef TX_LOCK_DETECT_PIN
#define TX_LOCK_DETECT_PIN GPIO_PIN_10
#endif
#ifndef TX_LOCK_DETECT_PORT
#define TX_LOCK_DETECT_PORT GPIOB
#endif
#ifndef TX_LOCK_DETECT_IRQ
#define TX_LOCK_DETECT_IRQ EXTI15_10_IRQn
#endif

#ifndef RX_EN_PIN
#define RX_EN_PIN GPIO_PIN_9
#endif
#ifndef RX_EN_PORT
#define RX_EN_PORT GPIOB
#endif

#ifndef TX_EN_PIN
#define TX_EN_PIN GPIO_PIN_8
#endif
#ifndef TX_EN_PORT
#define TX_EN_PORT GPIOB
#endif

#ifndef COM_TX_PIN
#define COM_TX_PIN GPIO_PIN_6
#endif
#ifndef COM_TX_PORT
#define COM_TX_PORT GPIOB
#endif
#ifndef COM_TX_AF
#define COM_TX_AF GPIO_AF7_USART1
#endif

#ifndef COM_RX_PIN
#define COM_RX_PIN GPIO_PIN_7
#endif
#ifndef COM_RX_PORT
#define COM_RX_PORT GPIOB
#endif
#ifndef COM_RX_AF
#define COM_RX_AF GPIO_AF7_USART1
#endif

#ifndef PINOUT_IRQHANDLER
#define PINOUT_IRQHANDLER(PIN) HAL_GPIO_EXTI_Callback(PIN)
#endif

/*******************************************************************************
 * COM CONFIG
 ******************************************************************************/
#ifndef LUOS_COM_CLOCK_ENABLE
#define LUOS_COM_CLOCK_ENABLE() __HAL_RCC_USART1_CLK_ENABLE()
#endif
#ifndef LUOS_COM
#define LUOS_COM USART1
#endif
#ifndef LUOS_COM_IRQ
#define LUOS_COM_IRQ USART1_IRQn
#endif
#ifndef LUOS_COM_IRQHANDLER
#define LUOS_COM_IRQHANDLER() USART1_IRQHandler()
#endif
/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef LUOS_DMA_CLOCK_ENABLE
#define LUOS_DMA_CLOCK_ENABLE() __HAL_RCC_DMA2_CLK_ENABLE();
#endif
#ifndef LUOS_DMA
#define LUOS_DMA DMA2
#endif
#ifndef LUOS_DMA_STREAM
#define LUOS_DMA_STREAM LL_DMA_STREAM_7
#endif
#ifndef LUOS_DMA_CHANNEL
#define LUOS_DMA_CHANNEL LL_DMA_CHANNEL_4
#endif
#ifndef LUOS_DMA_REMAP
#define LUOS_DMA_REMAP 0
#endif
/*******************************************************************************
 * COM TIMEOUT CONFIG
 ******************************************************************************/
#ifndef LUOS_TIMER_CLOCK_ENABLE
#define LUOS_TIMER_CLOCK_ENABLE() __HAL_RCC_TIM5_CLK_ENABLE()
#endif
#ifndef LUOS_TIMER
#define LUOS_TIMER TIM5
#endif
#ifndef LUOS_TIMER_IRQ
#define LUOS_TIMER_IRQ TIM5_IRQn
#endif
#ifndef LUOS_TIMER_IRQHANDLER
#define LUOS_TIMER_IRQHANDLER() TIM5_IRQHandler()
#endif
/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef PAGE_SIZE
#define PAGE_SIZE (uint32_t)0x400
#endif
#ifndef FLASH_SECTOR
#define FLASH_SECTOR FLASH_SECTOR_7
#endif
#ifndef ADDRESS_LAST_PAGE_FLASH
#define ADDRESS_LAST_PAGE_FLASH ((uint32_t)(FLASH_END - PAGE_SIZE))
#endif

/*******************************************************************************
 * BOOTLOADER CONFIG
 ******************************************************************************/
#ifndef SHARED_MEMORY_ADDRESS
#define SHARED_MEMORY_ADDRESS 0x0800C000
#endif
#ifndef SHARED_MEMORY_SECTOR
#define SHARED_MEMORY_SECTOR FLASH_SECTOR_3
#endif
#ifndef APP_ADDRESS
#define APP_ADDRESS (uint32_t)0x08010000
#endif
#ifndef APP_ADRESS_SECTOR
#define APP_ADRESS_SECTOR FLASH_SECTOR_4
#endif

#endif /* _RobusHAL_CONFIG_H_ */