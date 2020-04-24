#include <gpio_dev.h>
#include "main.h"
#include "analog.h"

// Pin configuration
#define P1_Pin GPIO_PIN_0
#define P1_GPIO_Port GPIOA
#define P9_Pin GPIO_PIN_1
#define P9_GPIO_Port GPIOA
#define P8_Pin GPIO_PIN_0
#define P8_GPIO_Port GPIOB
#define P7_Pin GPIO_PIN_1
#define P7_GPIO_Port GPIOB

enum {
    P1,
    P2,
    P3,
    P4,
    P5,
    P6,
    P7,
    P8,
    P9
};

module_t* pin[9];

void rx_digit_read_cb(module_t *module, msg_t *msg) {
    if (msg->header.cmd == ASK_PUB_CMD) {
        msg_t pub_msg;
        // fill the message infos
        pub_msg.header.cmd = IO_STATE;
        pub_msg.header.target_mode = ID;
        pub_msg.header.target = msg->header.source;
        pub_msg.header.size = sizeof(char);

        if (module == pin[P5]) {
            pub_msg.data[0] = (char)(HAL_GPIO_ReadPin(P5_GPIO_Port, P5_Pin) > 0);
        }
        else if (module == pin[P6]) {
            pub_msg.data[0] = (char)(HAL_GPIO_ReadPin(P6_GPIO_Port, P6_Pin) > 0);
        } else {
            return;
        }
        luos_send(module, &pub_msg);
        return;
    }
}

void rx_digit_write_cb(module_t *module, msg_t *msg) {
    if (msg->header.cmd == IO_STATE) {
        // we have to update pin state
        if (module == pin[P2]) {
            HAL_GPIO_WritePin(P2_GPIO_Port,P2_Pin,msg->data[0]);
        }
        if (module == pin[P3]) {
            HAL_GPIO_WritePin(P3_GPIO_Port,P3_Pin,msg->data[0]);
        }
        if (module == pin[P4]) {
            HAL_GPIO_WritePin(P4_GPIO_Port,P4_Pin,msg->data[0]);
        }
    }
}

void rx_analog_read_cb(module_t *module, msg_t *msg) {
    if (msg->header.cmd == ASK_PUB_CMD) {
        msg_t pub_msg;
        voltage_t volt;
        if (module == pin[P1]) {
            volt = ((float)analog_input.p1/4096.0f)*3.3f;
        } else if (module == pin[P7]) {
            volt = ((float)analog_input.p7/4096.0f)*3.3f;
        } else if (module == pin[P8]) {
            volt = ((float)analog_input.p8/4096.0f)*3.3f;
        } else if (module == pin[P9]) {
            volt = ((float)analog_input.p9/4096.0f)*3.3f;
        } else {
            return;
        }
        // fill the message infos
        pub_msg.header.target_mode = ID;
        pub_msg.header.target = msg->header.source;
        voltage_to_msg(&volt, &pub_msg);
        luos_send(module, &pub_msg);
        return;
    }
}

void gpio_dev_init(void) {
    // ******************* Analog measurement *******************
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    ADC_ChannelConfTypeDef sConfig = {0};
    // Stop DMA
    HAL_ADC_Stop_DMA(&luos_adc);

    // Configure analog input pin channel
    GPIO_InitStruct.Pin = P1_Pin | P9_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = P8_Pin | P7_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Add ADC channel to Luos adc configuration.
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&luos_adc, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfig.Channel = ADC_CHANNEL_9;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&luos_adc, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfig.Channel = ADC_CHANNEL_8;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&luos_adc, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
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
    // ************* modules creation *******************
    pin[P1] = luos_module_create(rx_analog_read_cb, VOLTAGE_MOD, "analog_read_P1");
    pin[P7] = luos_module_create(rx_analog_read_cb, VOLTAGE_MOD, "analog_read_P7");
    pin[P8] = luos_module_create(rx_analog_read_cb, VOLTAGE_MOD, "analog_read_P8");
    pin[P9] = luos_module_create(rx_analog_read_cb, VOLTAGE_MOD, "analog_read_P9");
    pin[P5] = luos_module_create(rx_digit_read_cb, STATE_MOD, "digit_read_P5");
    pin[P6] = luos_module_create(rx_digit_read_cb, STATE_MOD, "digit_read_P6");
    pin[P2] = luos_module_create(rx_digit_write_cb, STATE_MOD, "digit_write_P2");
    pin[P3] = luos_module_create(rx_digit_write_cb, STATE_MOD, "digit_write_P3");
    pin[P4] = luos_module_create(rx_digit_write_cb, STATE_MOD, "digit_write_P4");
    luos_module_enable_rt(pin[P2]);
    luos_module_enable_rt(pin[P3]);
    luos_module_enable_rt(pin[P4]); 
}

void gpio_dev_loop(void) {
    // Copy analog value to the board struct
    node_analog.temperature_sensor = analog_input.temperature_sensor;
    node_analog.voltage_sensor = analog_input.voltage_sensor;
}
