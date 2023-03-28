/******************************************************************************
 * @file light sensor communication driver
 * @brief driver example a simple light sensor
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "light_sensor_drv.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void LightSensorADC_Init(void);
void LightSensorDMA_Init(void);

/******************************************************************************
 * @brief Initialization of communication, mus be called in service init
 * @param None
 * @return None
 ******************************************************************************/
void LightSensorDrv_Init(void)
{
    // ******************* Analog measurement *******************
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configure analog input pin channel
    GPIO_InitStruct.Pin  = LIGHT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(LIGHT_GPIO_Port, &GPIO_InitStruct);
    // Init ADC
    LightSensorADC_Init();
    // Init DMA
    LightSensorDMA_Init();
}
/******************************************************************************
 * @brief Initialization of analog to digital converter driver
 * @param None
 * @return None
 ******************************************************************************/
void LightSensorADC_Init()
{
    ADC_ChannelConfTypeDef sConfig = {0};
    // Enable  ADC clocks
    LIGHTSENSOR_ADC_CLK();
    // Setup Adc to loop on DMA continuously
    LightSensor_adc.Instance                   = LIGHTSENSOR_ADC;
    LightSensor_adc.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV1;
    LightSensor_adc.Init.Resolution            = ADC_RESOLUTION_12B;
    LightSensor_adc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    LightSensor_adc.Init.ScanConvMode          = ADC_SCAN_ENABLE;
    LightSensor_adc.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    LightSensor_adc.Init.LowPowerAutoWait      = DISABLE;
    LightSensor_adc.Init.LowPowerAutoPowerOff  = DISABLE;
    LightSensor_adc.Init.ContinuousConvMode    = ENABLE;
    LightSensor_adc.Init.DiscontinuousConvMode = DISABLE;
    LightSensor_adc.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    LightSensor_adc.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    LightSensor_adc.Init.DMAContinuousRequests = ENABLE;
    LightSensor_adc.Init.Overrun               = ADC_OVR_DATA_PRESERVED;
    HAL_ADC_Init(&LightSensor_adc);

    // Add ADC channel to Luos adc configuration.
    sConfig.Channel      = LIGHTSENSOR_ADC_CHANNEL;
    sConfig.Rank         = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&LightSensor_adc, &sConfig);
}

/******************************************************************************
 * @brief Initialization of dma driver
 * @param None
 * @return None
 ******************************************************************************/
void LightSensorDMA_Init(void)
{
    // Enable DMA clock
    LIGHTSENSOR_DMA_CLK();
    /* ADC1 DMA Init */
    /* ADC Init */
    LightSensor_dma_adc.Instance                 = LIGHTSENSOR_DMA_CHANNEL;
    LightSensor_dma_adc.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    LightSensor_dma_adc.Init.PeriphInc           = DMA_PINC_DISABLE;
    LightSensor_dma_adc.Init.MemInc              = DMA_MINC_ENABLE;
    LightSensor_dma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    LightSensor_dma_adc.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    LightSensor_dma_adc.Init.Mode                = DMA_CIRCULAR;
    LightSensor_dma_adc.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&LightSensor_dma_adc);
    // relinik DMA
    __HAL_LINKDMA(&LightSensor_adc, DMA_Handle, LightSensor_dma_adc);

    // disable DMA Irq
    HAL_NVIC_DisableIRQ(LIGHTSENSOR_DMA_IRQ);
    // Start infinite ADC measurement
    // Restart DMA
    HAL_ADC_Start_DMA(&LightSensor_adc, (uint32_t *)analog_input.unmap, sizeof(analog_input.unmap) / sizeof(uint32_t));
}
/******************************************************************************
 * @brief Illuminance read
 * @param illuminance
 * @return error value
 ******************************************************************************/
illuminance_t LightSensorDrv_Read(void)
{
    // get the value of the analog input
    illuminance_t lux = IlluminanceOD_IlluminanceFrom_Lux((((float)analog_input.light / 4096.0f) * 3.3f) * 1000.0f);
    // send a pointer of the scanned illuminance value
    return lux;
}