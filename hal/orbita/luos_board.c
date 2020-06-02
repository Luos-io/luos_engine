#include <luos_board.h>
#include <string.h>
#include "luos_flash.h"
#include "main.h"
#include "node_config.h"

int node_msg_handler(module_t *module, msg_t *input, msg_t *output)
{
    if (input->header.cmd == NODE_LED)
    {
        if (input->data[0] < 2)
        {
            status_led(input->data[0]);
            return LUOS_PROTOCOL_NB;
        }
    }
    if ((input->header.cmd == NODE_TEMPERATURE) & (input->header.size == 0))
    {
        output->header.target_mode = ID;
        output->header.target = input->header.source;
        temperature_t temp;
        temp = (((float)node_analog.temperature_sensor * VREF / 3.0f) - (float)(*TEMP30_CAL_VALUE));
        temp = temp * (110.0f - 30.0f);
        temp = temp / (float)(*TEMP110_CAL_VALUE - *TEMP30_CAL_VALUE);
        temp = temp + 30.0f;
        temperature_to_msg(&temp, output);
        // overlap default TEPERATURE type
        output->header.cmd = NODE_TEMPERATURE;
        return NODE_TEMPERATURE;
    }
    if ((input->header.cmd == NODE_VOLTAGE) & (input->header.size == 0))
    {
        output->header.target_mode = ID;
        output->header.target = input->header.source;
        voltage_t volt = (((float)node_analog.voltage_sensor * VREF) / 4096.0f) * VOLTAGEFACTOR;
        voltage_to_msg(&volt, output);
        // overlap default VOLTAGE type
        output->header.cmd = NODE_VOLTAGE;
        return NODE_VOLTAGE;
    }
    return LUOS_PROTOCOL_NB;
}

void status_led(char state)
{
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, (state == 0));
}

void node_init(void)
{

    // ********************* led Gpio ****************************
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

    // ********************* ADC : power and temperature ****************************
    // interesting tutorial about ADC : https://visualgdb.com/tutorials/arm/stm32/adc/
    //ADC_ChannelConfTypeDef sConfig = {0};

    // Enable  ADC Gpio clocks
    //__HAL_RCC_GPIOA_CLK_ENABLE(); => already enabled previously
    /**ADC GPIO Configuration
    PA2     ------> ADC_IN2
    */
    GPIO_InitStruct.Pin = ROBUS_POWER_SENSOR_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ROBUS_POWER_SENSOR_GPIO_Port, &GPIO_InitStruct);

    // Setup Adc to work in one shot mode
    /*TODO:luos_adc.Instance = ADC1;
    luos_adc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    luos_adc.Init.Resolution = ADC_RESOLUTION_12B;
    luos_adc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    luos_adc.Init.ScanConvMode = ADC_SCAN_ENABLE;
    luos_adc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    luos_adc.Init.LowPowerAutoWait = DISABLE;
    luos_adc.Init.ContinuousConvMode = ENABLE;
    luos_adc.Init.DiscontinuousConvMode = DISABLE;
    luos_adc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    luos_adc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    luos_adc.Init.DMAContinuousRequests = ENABLE;
    luos_adc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    if (HAL_ADC_Init(&luos_adc) != HAL_OK)
    {
        Error_Handler();
    }*/
    /** Configure voltage input channel. */
    /*TODO:sConfig.Channel = ADC_CHANNEL_2;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&luos_adc, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }*/
    /** Configure temperature measurement channel. */
    /*TODO:sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&luos_adc, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }*/

    // Enable DMA1 clock
    //TODO:__HAL_RCC_DMA1_CLK_ENABLE();

    /* ADC1 DMA Init */
    /* ADC Init */
    /*TODO:luos_dma_adc.Instance = DMA1_Channel1;
    luos_dma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
    luos_dma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
    luos_dma_adc.Init.MemInc = DMA_MINC_ENABLE;
    luos_dma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    luos_dma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    luos_dma_adc.Init.Mode = DMA_CIRCULAR;
    luos_dma_adc.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&luos_dma_adc) != HAL_OK)
    {
        Error_Handler();
    }*/

    //TODO:__HAL_LINKDMA(&luos_adc, DMA_Handle, luos_dma_adc);
    // disable DMA Irq
    //TODO:HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    // Start infinite ADC measurement
    //TODO:HAL_ADC_Start_DMA(&luos_adc, node_analog.unmap, sizeof(node_analog_t) / sizeof(uint32_t));
}

void node_loop(void)
{
    static uint32_t last_analog_systick = 0;
    if (HAL_GetTick() - last_analog_systick > 20)
    {
        HAL_ADCEx_InjectedStart(&hadc1);
        if (HAL_ADCEx_InjectedPollForConversion(&hadc1, 1) == HAL_OK)
        {
            node_analog.temperature_sensor = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_REGULAR_RANK_4);
        }
        HAL_ADCEx_InjectedStop(&hadc1);
        HAL_ADCEx_InjectedStart(&hadc3);
        if (HAL_ADCEx_InjectedPollForConversion(&hadc3, 1) == HAL_OK)
        {
            node_analog.voltage_sensor = HAL_ADCEx_InjectedGetValue(&hadc3, ADC_REGULAR_RANK_1);
        }
        HAL_ADCEx_InjectedStop(&hadc3);
        last_analog_systick = HAL_GetTick();
    }
}

// ******** Alias management ****************
void write_alias(unsigned short local_id, char *alias)
{
    volatile const uint16_t addr = local_id * MAX_ALIAS_SIZE;

    luos_flash_write(addr, MAX_ALIAS_SIZE, alias);
}

char read_alias(unsigned short local_id, char *alias)
{
    const uint16_t addr = local_id * MAX_ALIAS_SIZE;
    char data[MAX_ALIAS_SIZE];
    luos_flash_read(addr, MAX_ALIAS_SIZE, data);
    // Check name integrity
    if ((((data[0] < 'A') | (data[0] > 'Z')) & ((data[0] < 'a') | (data[0] > 'z'))) | (data[0] == '\0'))
    {
        return 0;
    }
    else
    {
        for (uint8_t i = 0; i < MAX_ALIAS_SIZE; i++)
        {
            alias[i] = data[i];
        }
    }
    return 1;
}

/**
 * \fn void board_disable_irq(void)
 * \brief disable IRQ
 *
 * \return error
 */
void node_disable_irq(void)
{
    __disable_irq();
}

/**
 * \fn void board_enable_irq(void)
 * \brief enable IRQ
 *
 * \return error
 */
void node_enable_irq(void)
{
    __enable_irq();
}
