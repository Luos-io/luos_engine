/******************************************************************************
 * @file distance driver
 * @brief driver vl53l0x
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef VL53L0X_DRV_H
#define VL53L0X_DRV_H

#include "stdbool.h"
#include "stm32f0xx_hal.h"
#include "luos_engine.h"
#include "robus_network.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
// pinout init
#define PIN_CLK()                     \
    do                                \
    {                                 \
        __HAL_RCC_GPIOA_CLK_ENABLE(); \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0U)

#define SHUTDOWN_PIN  GPIO_PIN_5
#define SHUTDOWN_PORT GPIOB

#define INT_PIN  GPIO_PIN_0
#define INT_PORT GPIOA

#define SCL_PIN  GPIO_PIN_10
#define SCL_PORT GPIOB
#define SCL_AF   GPIO_AF1_I2C2

#define SDA_PIN  GPIO_PIN_11
#define SDA_PORT GPIOB
#define SDA_AF   GPIO_AF1_I2C2

#define ADDRESS_DEFAULT 0b0101001
#define I2C_CLK()                    \
    do                               \
    {                                \
        __HAL_RCC_I2C2_CLK_ENABLE(); \
    } while (0U)

#define VL53L0_I2C I2C2

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern I2C_HandleTypeDef VL53L0_I2CHandle;
/*******************************************************************************
 * Function
 ******************************************************************************/
void vl53l0x_DRVInit(void);
uint8_t vl53l0x_DrvRead(linear_position_t *position);

#endif /* VL53L0X_DRV_H */
