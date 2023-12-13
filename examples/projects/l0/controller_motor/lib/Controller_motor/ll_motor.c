/******************************************************************************
 * @file low level motor functions
 * @brief driver example a simple controller motor
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "ll_motor.h"
#include "analog.h"
#include "stm32f0xx_hal.h"
#include "math.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
motor_config_t motor_parameters;
/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief hardware peripherals init
 * @param None
 * @return None
 ******************************************************************************/
void ll_motor_init(void)
{
    // ******************* Analog measurement *******************
    // interesting tutorial about ADC : https://visualgdb.com/tutorials/arm/stm32/adc/
    ADC_ChannelConfTypeDef sConfig   = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // Enable  ADC Gpio clocks
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();

    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC          = {0};
    TIM_Encoder_InitTypeDef sConfigTim    = {0};

    htim2.Instance               = POSITION_TIMER;
    htim2.Init.Prescaler         = 0;
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.Period            = 16777215;
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    sConfigTim.EncoderMode       = TIM_ENCODERMODE_TI12;
    sConfigTim.IC1Polarity       = TIM_ICPOLARITY_RISING;
    sConfigTim.IC1Selection      = TIM_ICSELECTION_DIRECTTI;
    sConfigTim.IC1Prescaler      = TIM_ICPSC_DIV1;
    sConfigTim.IC1Filter         = 5;
    sConfigTim.IC2Polarity       = TIM_ICPOLARITY_RISING;
    sConfigTim.IC2Selection      = TIM_ICSELECTION_DIRECTTI;
    sConfigTim.IC2Prescaler      = TIM_ICPSC_DIV1;
    sConfigTim.IC2Filter         = 5;
    while (HAL_TIM_Encoder_Init(&htim2, &sConfigTim) != HAL_OK)
        ;

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    while (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
        ;

    htim3.Instance               = COMMAND_TIMER;
    htim3.Init.Prescaler         = 0;
    htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim3.Init.Period            = 2400 - 1;
    htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    while (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
        ;

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    while (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
        ;

    sConfigOC.OCMode     = TIM_OCMODE_PWM1;
    sConfigOC.Pulse      = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    while (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
        ;

    while (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
        ;

    GPIO_InitStruct.Pin       = B_Pin | A_Pin;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /**TIM3 GPIO Configuration
    PB4     ------> TIM3_CH1
    PB5     ------> TIM3_CH2
    */
    GPIO_InitStruct.Pin       = PWM2_Pin | PWM1_Pin;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /**ADC GPIO Configuration
     */
    GPIO_InitStruct.Pin  = FB_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(FB_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, EN_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin  = POWER_SENSOR_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(POWER_SENSOR_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : PBPin PBPin PBPin */
    GPIO_InitStruct.Pin   = EN_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Enable  ADC clocks
    __HAL_RCC_ADC1_CLK_ENABLE();
    // Setup Adc to loop on DMA continuously
    ControllerMotor_adc.Instance                   = ADC1;
    ControllerMotor_adc.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV1;
    ControllerMotor_adc.Init.Resolution            = ADC_RESOLUTION_12B;
    ControllerMotor_adc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    ControllerMotor_adc.Init.ScanConvMode          = ADC_SCAN_ENABLE;
    ControllerMotor_adc.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    ControllerMotor_adc.Init.LowPowerAutoWait      = DISABLE;
    ControllerMotor_adc.Init.LowPowerAutoPowerOff  = DISABLE;
    ControllerMotor_adc.Init.ContinuousConvMode    = ENABLE;
    ControllerMotor_adc.Init.DiscontinuousConvMode = DISABLE;
    ControllerMotor_adc.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    ControllerMotor_adc.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    ControllerMotor_adc.Init.DMAContinuousRequests = ENABLE;
    ControllerMotor_adc.Init.Overrun               = ADC_OVR_DATA_PRESERVED;
    while (HAL_ADC_Init(&ControllerMotor_adc) != HAL_OK)
        ;

    /** Configure voltage input channel. */
    sConfig.Channel      = ADC_CHANNEL_8;
    sConfig.Rank         = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    while (HAL_ADC_ConfigChannel(&ControllerMotor_adc, &sConfig) != HAL_OK)
        ;

    // Enable DMA1 clock
    __HAL_RCC_DMA1_CLK_ENABLE();
    /* ADC1 DMA Init */
    /* ADC Init */
    ControllerMotor_dma_adc.Instance                 = DMA1_Channel1;
    ControllerMotor_dma_adc.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    ControllerMotor_dma_adc.Init.PeriphInc           = DMA_PINC_DISABLE;
    ControllerMotor_dma_adc.Init.MemInc              = DMA_MINC_ENABLE;
    ControllerMotor_dma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    ControllerMotor_dma_adc.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    ControllerMotor_dma_adc.Init.Mode                = DMA_CIRCULAR;
    ControllerMotor_dma_adc.Init.Priority            = DMA_PRIORITY_LOW;
    while (HAL_DMA_Init(&ControllerMotor_dma_adc) != HAL_OK)
        ;

    __HAL_LINKDMA(&ControllerMotor_adc, DMA_Handle, ControllerMotor_dma_adc);
    // disable DMA Irq
    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    // Start infinite ADC measurement
    HAL_ADC_Start_DMA(&ControllerMotor_adc, (uint32_t *)analog_input.unmap, sizeof(analog_input_t) / sizeof(uint32_t));
    // ************** Pwm settings *****************
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_1 | TIM_CHANNEL_2);
}

/******************************************************************************
 * @brief enable motor
 * @param state
 * @return None
 ******************************************************************************/
void ll_motor_enable(char state)
{
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, state);
}

/******************************************************************************
 * @brief send command to motor
 * @param ratio
 * @return None
 ******************************************************************************/
void ll_motor_Command(uint16_t mode, float ratio)
{
    current_t current = ElectricOD_CurrentFrom_A(ll_motor_GetCurrent());
    float currentfactor;
    currentfactor               = motor_parameters.limit_current->raw / (current.raw * 2.0f);
    static float surpCurrentSum = 0.0;
    const float surpCurrent     = current.raw - motor_parameters.limit_current->raw;
    surpCurrentSum += surpCurrent;
    // If surpCurrentSum > 0 do a real coef
    if (surpCurrentSum > 0.0)
    {
        currentfactor = motor_parameters.limit_current->raw / (motor_parameters.limit_current->raw + (surpCurrentSum / 1.5));
    }
    else
    {
        surpCurrentSum = 0.0;
        currentfactor  = 1.0f;
    }

    if (!mode)
    {
        ratio = ratio * currentfactor;
    }

    // limit power value
    if (ratio < -motor_parameters.limit_ratio->raw)
        ratio = -motor_parameters.limit_ratio->raw;
    if (ratio > motor_parameters.limit_ratio->raw)
        ratio = motor_parameters.limit_ratio->raw;

    // transform power ratio to timer value
    uint16_t pulse;
    if (ratio > 0.0)
    {
        pulse               = (uint16_t)(ratio * 24.0);
        COMMAND_TIMER->CCR1 = pulse;
        COMMAND_TIMER->CCR2 = 0;
    }
    else
    {
        pulse               = (uint16_t)(-ratio * 24.0);
        COMMAND_TIMER->CCR1 = 0;
        COMMAND_TIMER->CCR2 = pulse;
    }
}

/******************************************************************************
 * @brief get current measure
 * @param none
 * @return current
 ******************************************************************************/
float ll_motor_GetCurrent(void)
{
    return (((float)analog_input.current * 3.3f) / 4096.0f) / 0.525f;
}

/******************************************************************************
 * @brief get motor angular position
 * @param none
 * @return position
 ******************************************************************************/
float ll_motor_GetAngularPosition(void)
{
    int32_t encoder_count = (int16_t)POSITION_TIMER->CNT;
    POSITION_TIMER->CNT   = 0;
    return ((double)encoder_count / (double)((*motor_parameters.motor_reduction) * (*motor_parameters.resolution) * 4)) * 360.0;
}

/******************************************************************************
 * @brief get motor linear position
 * @param angular position
 * @return position
 ******************************************************************************/
float ll_motor_GetLinearPosition(float angular_position)
{
    return angular_position / 360.0 * M_PI * LinearOD_PositionTo_m(*motor_parameters.wheel_diameter);
}

/******************************************************************************
 * @brief configure motor parameters
 * @param motor_configuration
 * @return none
 ******************************************************************************/
void ll_motor_config(motor_config_t motor_config)
{
    motor_parameters.motor_reduction = motor_config.motor_reduction;
    motor_parameters.resolution      = motor_config.resolution;
    motor_parameters.wheel_diameter  = motor_config.wheel_diameter;
    motor_parameters.limit_ratio     = motor_config.limit_ratio;
    motor_parameters.limit_current   = motor_config.limit_current;

    // Default motor configuration
    *motor_parameters.motor_reduction = 131;
    *motor_parameters.resolution      = 16;
    *motor_parameters.wheel_diameter  = LinearOD_PositionFrom_m(0.100f);
    // Default motor hardware limits
    *motor_parameters.limit_ratio   = RatioOD_RatioFrom_Percent(100.0f);
    *motor_parameters.limit_current = ElectricOD_CurrentFrom_A(6.0f);
}
