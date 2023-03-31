/******************************************************************************
 * @file potentiometer driver
 * @brief driver example a simple potentiometer
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "potentiometer_drv.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*******************************************************************************
 * Function
 ******************************************************************************/
void PotentiometerADC_Init(void);
void PotentiometerDMA_Init(void);

/******************************************************************************
 * @brief driver init must be call in package init
 * @param None
 * @return None
 ******************************************************************************/
void PotentiometerDrv_Init(void)
{
    // ******************* Analog measurement *******************
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configure analog input pin channel
    GPIO_InitStruct.Pin  = POS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(POS_GPIO_Port, &GPIO_InitStruct);
    // analog input config
    PotentiometerADC_Init();
    // dma config
    PotentiometerDMA_Init();
}

/******************************************************************************
 * @brief adc init
 * @param None
 * @return None
 ******************************************************************************/
void PotentiometerADC_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    // Enable  ADC clocks
    POS_ADC_CLK();
    // Setup Adc to loop on DMA continuously
    Potentiometer_adc.Instance                   = POS_ADC;
    Potentiometer_adc.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV2;
    Potentiometer_adc.Init.Resolution            = ADC_RESOLUTION_12B;
    Potentiometer_adc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    Potentiometer_adc.Init.ScanConvMode          = ENABLE;
    Potentiometer_adc.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    Potentiometer_adc.Init.ContinuousConvMode    = ENABLE;
    Potentiometer_adc.Init.NbrOfConversion       = 8;
    Potentiometer_adc.Init.DiscontinuousConvMode = DISABLE;
    Potentiometer_adc.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    Potentiometer_adc.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    Potentiometer_adc.Init.DMAContinuousRequests = ENABLE;
    if (HAL_ADC_Init(&Potentiometer_adc) != HAL_OK)
    {
        Error_Handler();
    }

    // Add ADC channel adc configuration.
    sConfig.Channel      = POS_ADC_CHANNEL;
    sConfig.Rank         = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
    if (HAL_ADC_ConfigChannel(&Potentiometer_adc, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/******************************************************************************
 * @brief dma init
 * @param None
 * @return None
 ******************************************************************************/
void PotentiometerDMA_Init(void)
{
    // Enable DMA1 clock
    POS_DMA_CLK();
    /* ADC1 DMA Init */
    /* ADC Init */
    Potentiometer_dma_adc.Instance                 = POS_DMA_STREAM;
    Potentiometer_dma_adc.Init.Channel             = POS_DMA_CHANNEL;
    Potentiometer_dma_adc.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    Potentiometer_dma_adc.Init.PeriphInc           = DMA_PINC_DISABLE;
    Potentiometer_dma_adc.Init.MemInc              = DMA_MINC_ENABLE;
    Potentiometer_dma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    Potentiometer_dma_adc.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    Potentiometer_dma_adc.Init.Mode                = DMA_CIRCULAR;
    Potentiometer_dma_adc.Init.Priority            = DMA_PRIORITY_LOW;

    if (HAL_DMA_Init(&Potentiometer_dma_adc) != HAL_OK)
    {
        Error_Handler();
    }
    // relinik DMA
    __HAL_LINKDMA(&Potentiometer_adc, DMA_Handle, Potentiometer_dma_adc);

    // disable DMA Irq
    HAL_NVIC_DisableIRQ(POS_DMA_IRQ);
    // Start infinite ADC measurement
    // Restart DMA
    HAL_ADC_Start_DMA(&Potentiometer_adc, (uint32_t *)analog_input.unmap, sizeof(analog_input.unmap) / sizeof(uint32_t));
}

/******************************************************************************
 * @brief reads the angular position value
 * @param angle
 * @return error value
 ******************************************************************************/

angular_position_t PotentiometerDrv_Read(void)
{
    // get the analog input angle value
    angular_position_t angle = AngularOD_PositionFrom_deg(((float)analog_input.pos / 4096.0) * 300.0);
    return angle;
}