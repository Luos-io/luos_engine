/******************************************************************************
 * @file analog
 * @brief analog for this project
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __ANALOG_H
#define __ANALOG_H

#include "stm32f0xx_hal.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
ADC_HandleTypeDef ControllerMotor_adc;
DMA_HandleTypeDef ControllerMotor_dma_adc;

// This structure need to list all ADC configured in the good order determined by the
// ADC_CHANEL number in increasing order
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            // uint32_t voltage_sensor;
            uint32_t current;
            // uint32_t temperature_sensor;
        };
        uint32_t unmap[1]; /*!< Unmaped form. */
    };
} analog_input_t;

volatile analog_input_t analog_input;

#endif /*__ __ANALOG_H */
