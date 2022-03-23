/******************************************************************************
 * @file analog
 * @brief analog for this project
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __ANALOG_H
#define __ANALOG_H

#include "main.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
ADC_HandleTypeDef GpioDev_adc;
DMA_HandleTypeDef GpioDev_dma_adc;

// This structure need to list all ADC configured in the good order determined by the
// ADC_CHANEL number in increasing order
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint32_t p1;
            uint32_t p9;
            uint32_t p8;
            uint32_t p7;
        };
        uint32_t unmap[4]; /*!< Unmaped form. */
    };
} analog_input_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile analog_input_t analog_input;

/*******************************************************************************
 * Function
 ******************************************************************************/

#endif /*__ __ANALOG_H */
