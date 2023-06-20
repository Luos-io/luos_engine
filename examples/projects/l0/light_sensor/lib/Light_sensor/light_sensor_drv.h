/******************************************************************************
 * @file light sensor communication driver
 * @brief driver example a simple light sensor
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef LIGHT_SENSOR_DRV
#define LIGHT_SENSOR_DRV

#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_system.h"
#include "analog.h"
#include "main.h"
#include "luos_engine.h"
#include "robus.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
// sensor gpio port definition
#define LIGHT_Pin       GPIO_PIN_1
#define LIGHT_GPIO_Port GPIOA

// adc port definition
#define LIGHTSENSOR_ADC_CLK()   __HAL_RCC_ADC1_CLK_ENABLE()
#define LIGHTSENSOR_ADC         ADC1
#define LIGHTSENSOR_ADC_CHANNEL ADC_CHANNEL_1

// DMA port definition
#define LIGHTSENSOR_DMA_CLK()   __HAL_RCC_DMA1_CLK_ENABLE()
#define LIGHTSENSOR_DMA         DMA1
#define LIGHTSENSOR_DMA_CHANNEL DMA1_Channel1
#define LIGHTSENSOR_DMA_IRQ     DMA1_Channel1_IRQn
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

void LightSensorDrv_Init(void);
illuminance_t LightSensorDrv_Read(void);

#endif /* LIGHT_SENSOR_DRV */
