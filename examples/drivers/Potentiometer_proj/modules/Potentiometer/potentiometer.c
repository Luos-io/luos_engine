#include "main.h"
#include "potentiometer.h"
#include "analog.h"

// Pin configuration
#define POS_Pin GPIO_PIN_0
#define POS_GPIO_Port GPIOA

volatile angular_position_t angle = 0.0;

void rx_pot_cb(module_t *module, msg_t *msg) {
    if (msg->header.cmd == ASK_PUB_CMD) {
        msg_t pub_msg;
        // fill the message infos
        pub_msg.header.target_mode = ID;
        pub_msg.header.target = msg->header.source;
        angular_position_to_msg(&angle, &pub_msg);
        luos_send(module, &pub_msg);
        return;
    }
}

void potentiometer_init(void) {
    // ******************* Analog measurement *******************
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    ADC_ChannelConfTypeDef sConfig = {0};
    // Stop DMA
    HAL_ADC_Stop_DMA(&luos_adc);

    // Configure analog input pin channel
    GPIO_InitStruct.Pin = POS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(POS_GPIO_Port, &GPIO_InitStruct);

    // Add ADC channel to Luos adc configuration.
    sConfig.Channel = ADC_CHANNEL_0;
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
    luos_module_create(rx_pot_cb, ANGLE_MOD, "potentiometer_mod");
}

void potentiometer_loop(void) {
    node_analog.temperature_sensor = analog_input.temperature_sensor;
    node_analog.voltage_sensor = analog_input.voltage_sensor;
    angle = ((float)analog_input.pos / 4096.0) * 300.0;
}
