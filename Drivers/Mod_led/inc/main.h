/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_dma.h"

#include "stm32f0xx_ll_exti.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B_Pin GPIO_PIN_0
#define B_GPIO_Port GPIOA
#define ROBUS_POWER_SENSOR_Pin GPIO_PIN_2
#define ROBUS_POWER_SENSOR_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_3
#define LED_GPIO_Port GPIOA
#define RS485_LVL_DOWN_Pin GPIO_PIN_5
#define RS485_LVL_DOWN_GPIO_Port GPIOA
#define RS485_LVL_UP_Pin GPIO_PIN_6
#define RS485_LVL_UP_GPIO_Port GPIOA
#define ROBUS_PTPB_Pin GPIO_PIN_13
#define ROBUS_PTPB_GPIO_Port GPIOB
#define ROBUS_RE_Pin GPIO_PIN_14
#define ROBUS_RE_GPIO_Port GPIOB
#define ROBUS_DE_Pin GPIO_PIN_15
#define ROBUS_DE_GPIO_Port GPIOB
#define ROBUS_PTPA_Pin GPIO_PIN_8
#define ROBUS_PTPA_GPIO_Port GPIOA
#define ROBUS_TX_Pin GPIO_PIN_9
#define ROBUS_TX_GPIO_Port GPIOA
#define ROBUS_RX_Pin GPIO_PIN_10
#define ROBUS_RX_GPIO_Port GPIOA
#define R_Pin GPIO_PIN_4
#define R_GPIO_Port GPIOB
#define G_Pin GPIO_PIN_5
#define G_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
