/******************************************************************************
 * @file potentiometer driver
 * @brief driver example a simple potentiometer
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "main.h"
#include "analog.h"
#include "luos_engine.h"
#include "robus_network.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
// Gpio Pins configuration
#define POS_Pin       GPIO_PIN_0
#define POS_GPIO_Port GPIOA
// Analog pins configuration
#define POS_ADC_CLK()   __HAL_RCC_ADC_CLK_ENABLE()
#define POS_ADC         ADC1
#define POS_ADC_CHANNEL ADC_CHANNEL_1
// DMA configuration
#define POS_DMA_CLK()   __HAL_RCC_DMA2_CLK_ENABLE()
#define POS_DMA         DMA1
#define POS_DMA_REQUEST DMA_REQUEST_0
#define POS_DMA_CHANNEL DMA_CHANNEL_1
#define POS_DMA_IRQ     DMA2_Channel1_IRQn

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

void PotentiometerDrv_Init(void);
angular_position_t PotentiometerDrv_Read(void);
