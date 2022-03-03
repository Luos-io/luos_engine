/******************************************************************************
 * @file low-level gpio
 * @brief driver example a simple gpio_dev
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "ll_gpio.h"
#include "analog.h"

#include "profile_state.h"
#include "profile_voltage.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief analog hardware initialization
 * @param None
 * @return None
 ******************************************************************************/
void ll_analog_init(void)
{
    // ******************* Analog measurement *******************
    // interesting tutorial about ADC : https://visualgdb.com/tutorials/arm/stm32/adc/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    ADC_ChannelConfTypeDef sConfig   = {0};
    // Enable  ADC Gpio clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC GPIO Configuration
     */
    // Configure analog input pin channel
    GPIO_InitStruct.Pin  = P1_Pin | P9_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin  = P8_Pin | P7_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Enable  ADC clocks
    __HAL_RCC_ADC1_CLK_ENABLE();
    // Setup Adc to loop on DMA continuously
    GpioDev_adc.Instance                   = ADC1;
    GpioDev_adc.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV1;
    GpioDev_adc.Init.Resolution            = ADC_RESOLUTION_12B;
    GpioDev_adc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    GpioDev_adc.Init.ScanConvMode          = ADC_SCAN_ENABLE;
    GpioDev_adc.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    GpioDev_adc.Init.LowPowerAutoWait      = DISABLE;
    GpioDev_adc.Init.LowPowerAutoPowerOff  = DISABLE;
    GpioDev_adc.Init.ContinuousConvMode    = ENABLE;
    GpioDev_adc.Init.DiscontinuousConvMode = DISABLE;
    GpioDev_adc.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    GpioDev_adc.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    GpioDev_adc.Init.DMAContinuousRequests = ENABLE;
    GpioDev_adc.Init.Overrun               = ADC_OVR_DATA_PRESERVED;
    if (HAL_ADC_Init(&GpioDev_adc) != HAL_OK)
    {
        // TODO: handle error
        // Error_Handler();
    }

    sConfig.Channel      = ADC_CHANNEL_0;
    sConfig.Rank         = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&GpioDev_adc, &sConfig) != HAL_OK)
    {
        // TODO: handle error
        // Error_Handler();
    }
    sConfig.Channel      = ADC_CHANNEL_9;
    sConfig.Rank         = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&GpioDev_adc, &sConfig) != HAL_OK)
    {
        // TODO: handle error
        // Error_Handler();
    }
    sConfig.Channel      = ADC_CHANNEL_8;
    sConfig.Rank         = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&GpioDev_adc, &sConfig) != HAL_OK)
    {
        // TODO: handle error
        // Error_Handler();
    }
    sConfig.Channel      = ADC_CHANNEL_1;
    sConfig.Rank         = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&GpioDev_adc, &sConfig) != HAL_OK)
    {
        // TODO: handle error
        // Error_Handler();
    }

    // Enable DMA1 clock
    __HAL_RCC_DMA1_CLK_ENABLE();
    /* ADC1 DMA Init */
    /* ADC Init */
    GpioDev_dma_adc.Instance                 = DMA1_Channel1;
    GpioDev_dma_adc.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    GpioDev_dma_adc.Init.PeriphInc           = DMA_PINC_DISABLE;
    GpioDev_dma_adc.Init.MemInc              = DMA_MINC_ENABLE;
    GpioDev_dma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    GpioDev_dma_adc.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    GpioDev_dma_adc.Init.Mode                = DMA_CIRCULAR;
    GpioDev_dma_adc.Init.Priority            = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&GpioDev_dma_adc) != HAL_OK)
    {
        // TODO: handle error
        // Error_Handler();
    }
    __HAL_LINKDMA(&GpioDev_adc, DMA_Handle, GpioDev_dma_adc);
    // disable DMA Irq
    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    // Start infinite ADC measurement
    HAL_ADC_Start_DMA(&GpioDev_adc, (uint32_t *)analog_input.unmap, sizeof(analog_input_t) / sizeof(uint32_t));
}

/******************************************************************************
 * @brief analog hardware initialization
 * @param None
 * @return None
 ******************************************************************************/
void ll_digital_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, P4_Pin | P3_Pin | P2_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pins : output */
    GPIO_InitStruct.Pin   = P4_Pin | P3_Pin | P2_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : input */
    GPIO_InitStruct.Pin  = P6_Pin | P5_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/******************************************************************************
 * @brief update digital GPIO
 * @param None
 * @return None
 ******************************************************************************/
uint8_t ll_digital_write(void *profile)
{
    profile_state_t *gpio = (profile_state_t *)profile;

    HAL_GPIO_WritePin(P2_GPIO_Port, P2_Pin, gpio[P2].state);
    HAL_GPIO_WritePin(P3_GPIO_Port, P3_Pin, gpio[P3].state);
    HAL_GPIO_WritePin(P4_GPIO_Port, P4_Pin, gpio[P4].state);

    return true;
}

/******************************************************************************
 * @brief update analog measures
 * @param None
 * @return None
 ******************************************************************************/
uint8_t ll_analog_read(void *profile)
{
    profile_voltage_t *analog = (profile_voltage_t *)profile;

    analog[P1].voltage = ((float)analog_input.p1 / 4096.0f) * 3.3f;
    analog[P7].voltage = ((float)analog_input.p7 / 4096.0f) * 3.3f;
    analog[P8].voltage = ((float)analog_input.p8 / 4096.0f) * 3.3f;
    analog[P9].voltage = ((float)analog_input.p9 / 4096.0f) * 3.3f;

    return true;
}

/******************************************************************************
 * @brief read digital GPIO
 * @param None
 * @return None
 ******************************************************************************/
uint8_t ll_digital_read(void *profile)
{
    profile_state_t *gpio = (profile_state_t *)profile;

    gpio[P5].state = (bool)(HAL_GPIO_ReadPin(P5_GPIO_Port, P5_Pin) > 0);
    gpio[P6].state = (bool)(HAL_GPIO_ReadPin(P6_GPIO_Port, P6_Pin) > 0);

    return true;
}
