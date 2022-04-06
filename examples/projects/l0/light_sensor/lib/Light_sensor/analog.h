#ifndef __ANALOG_H
#define __ANALOG_H

ADC_HandleTypeDef LightSensor_adc;
DMA_HandleTypeDef LightSensor_dma_adc;

// This structure need to list all ADC configured in the good order determined by the
// ADC_CHANEL number in increasing order
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint32_t light;
        };
        uint32_t unmap[1]; /*!< Unmaped form. */
    };
} analog_input_t;

volatile analog_input_t analog_input;

#endif /*__ __ANALOG_H */
