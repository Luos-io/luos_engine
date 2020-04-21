#include "main.h"
#include "light_sensor.h"
#include "analog.h"
#include "string.h"

// Pin configuration
#define LIGHT_Pin GPIO_PIN_1
#define LIGHT_GPIO_Port GPIOA

volatile illuminance_t lux = 0.0;

void rx_lgt_cb(module_t *module, msg_t *msg) {
    if (msg->header.cmd == ASK_PUB_CMD) {
        msg_t pub_msg;
        // fill the message infos
        pub_msg.header.target_mode = ID;
        pub_msg.header.target = msg->header.source;
        illuminance_to_msg(&lux, &pub_msg);
        luos_send(module, &pub_msg);
        return;
    }
}

void light_sensor_init(void) {
    // ******************* Analog measurement *******************
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    ADC_ChannelConfTypeDef sConfig = {0};
    // Stop DMA
    HAL_ADC_Stop_DMA(&luos_adc);

    // Configure analog input pin channel
    GPIO_InitStruct.Pin = LIGHT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(LIGHT_GPIO_Port, &GPIO_InitStruct);

    // Add ADC channel to Luos adc configuration.
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&luos_adc, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    // relinik DMA
    __HAL_LINKDMA(&luos_adc, DMA_Handle, luos_dma_adc);

    // Restart DMA
    HAL_ADC_Start_DMA(&luos_adc, analog_input.unmap, sizeof(analog_input.unmap) / sizeof(uint32_t));

    // ******************* module creation *******************
    luos_module_create(rx_lgt_cb, LIGHT_MOD, "light_sensor_mod");
}

void light_sensor_loop(void) {
    node_analog.temperature_sensor = analog_input.temperature_sensor;
    node_analog.voltage_sensor = analog_input.voltage_sensor;
    lux = (((float)analog_input.light/4096.0f)*3.3f) * 1000.0f;
}
