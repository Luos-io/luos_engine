/******************************************************************************
 * @file galvo_config.h
 * @brief Galvo configuration
 * @MCU Family STM32L4
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _GALVO_HAL_CONFIG_H_
#define _GALVO_HAL_CONFIG_H_

#ifndef MCUFREQ
    #define MCUFREQ 80000000 // MCU frequence
#endif

// ********************* GPIO *********************
// Galvo need all the pins (except enable) to be on the same port
#ifndef GALVO_PORT
    #define GALVO_PORT GPIOC
#endif

#ifndef GALVO_PIN_CLOCK_ENABLE
    #define GALVO_PIN_CLOCK_ENABLE()      \
        do                                \
        {                                 \
            __HAL_RCC_GPIOC_CLK_ENABLE(); \
        } while (0U)
#endif

// CLOCK Pinout
#ifndef GALVO_CLOCK_PIN
    #define GALVO_CLOCK_PIN GPIO_PIN_0
#endif

// SYNC Pinout
#ifndef GALVO_SYNC_PIN
    #define GALVO_SYNC_PIN GPIO_PIN_1
#endif

// X Pinout
#ifndef GALVO_X_PIN
    #define GALVO_X_PIN GPIO_PIN_2
#endif

// Y Pinout
#ifndef GALVO_Y_PIN
    #define GALVO_Y_PIN GPIO_PIN_3
#endif

// ENABLE Pinout
#ifndef GALVO_ENABLE_PIN
    #define GALVO_ENABLE_PIN GPIO_PIN_5
#endif

// ********************* TIMER *********************
#ifndef GALVO_TIMER
    #define GALVO_TIMER TIM6
#endif

#ifndef GALVO_BAUDRATE
    #define GALVO_BAUDRATE 2000000
#endif

#ifndef GALVO_TIMER_CLOCK_ENABLE
    #define GALVO_TIMER_CLOCK_ENABLE() __HAL_RCC_TIM6_CLK_ENABLE()
#endif

// ********************* DMA *********************
#ifndef GALVO_DMA_CLOCK_ENABLE
    #define GALVO_DMA_CLOCK_ENABLE() __HAL_RCC_DMA1_CLK_ENABLE();
#endif

#ifndef GALVO_DMA
    #define GALVO_DMA DMA1
#endif

#ifndef GALVO_DMA_CHANNEL
    #define GALVO_DMA_CHANNEL DMA1_Channel3
#endif

#ifndef GALVO_DMA_REQUEST
    #define GALVO_DMA_REQUEST DMA_REQUEST_6
#endif

#ifndef GALVO_DMA_IRQ
    #define GALVO_DMA_IRQ DMA1_Channel3_IRQn
#endif

#ifndef GALVO_DMA_IRQHANDLER
    #define GALVO_DMA_IRQHANDLER() DMA1_Channel3_IRQHandler()
#endif

// ********************* BUFFER *********************
#ifndef GALVO_BUFFER_SIZE
    #define GALVO_BUFFER_SIZE 8000 // Buffer size need to be a multiple of 40
#endif

// ********************* DEFAULT VALUES *********************
#ifndef DEFAULT_SAMPLE_FREQUENCY
    #define DEFAULT_SAMPLE_FREQUENCY 10000.0
#endif

#endif /* _GALVO_HAL_CONFIG_H_ */
