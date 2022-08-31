#ifndef __BAROMETER_DRIVER_H__
#define __BAROMETER_DRIVER_H__

#include "stm32f0xx_hal.h"

#define SPIx SPI1
/* Size of buffer */
#define BUFFERSIZE (COUNTOF(aTxBuffer) - 1)

/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__) (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* Exported functions ------------------------------------------------------- */

#define SPIx_CLK_ENABLE()           __HAL_RCC_SPI1_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPIx_MISO_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPIx_MOSI_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPIx_CS_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()

#define SPIx_FORCE_RESET()   __HAL_RCC_SPI1_FORCE_RESET()
#define SPIx_RELEASE_RESET() __HAL_RCC_SPI1_RELEASE_RESET()

/* Definition for SPIx Pins */
#define SPIx_SCK_PIN        GPIO_PIN_3
#define SPIx_SCK_GPIO_PORT  GPIOB
#define SPIx_SCK_AF         GPIO_AF0_SPI1
#define SPIx_MISO_PIN       GPIO_PIN_4
#define SPIx_MISO_GPIO_PORT GPIOB
#define SPIx_MISO_AF        GPIO_AF0_SPI1
#define SPIx_MOSI_PIN       GPIO_PIN_5
#define SPIx_MOSI_GPIO_PORT GPIOB
#define SPIx_MOSI_AF        GPIO_AF0_SPI1
#define SPIx_CS_PIN         GPIO_PIN_6
#define SPIx_CS_GPIO_PORT   GPIOA

/* Functions Definition */
void SPI_Driver_Init();
void SPI_Driver_Loop();
void SystemClock_Config(void);
void Send_SPI_Message(uint8_t *Message, uint8_t Message_Size);
void Receive_SPI_Message(uint8_t *Reception_Buffer, uint8_t Data_Size);

#endif