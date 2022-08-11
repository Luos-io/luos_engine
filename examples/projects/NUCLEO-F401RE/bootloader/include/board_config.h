/******************************************************************************
 * @file NUCLEO F410RET6 Board configuration
 * @brief This file allow you to configure LuosHAL according to your design
 * @MCU Family STM32F4
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#define FLASH_SECTOR FLASH_SECTOR_7

/*******************************************************************************
 * TIMER CONFIG
 ******************************************************************************/
#define LUOS_TIMER_CLOCK_ENABLE() __HAL_RCC_TIM5_CLK_ENABLE()
#define LUOS_TIMER                TIM5
#define LUOS_TIMER_IRQ            TIM5_IRQn
#define LUOS_TIMER_IRQHANDLER()   TIM5_IRQHandler()

#endif /*__BOARD_CONFIG_H__ */