/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family ATSAMD21
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

#include <board_config.h>
#include <Arduino.h>

/*
DEFAULT LUOSHAL ARDUINO PIN CONFIGURATION
 +------------+------------------+---------------+
 | Pin number |     Board pin    | function name |
 +------------+------------------+---------------+
 |      ~     |         RX       |       RX      |
 |      ~     |         TX       |       TX      |
 |      2     |         D2       |      RXEN     |
 |      3     |         D3       |      TXEN     |
 |      6     |         D6       |      PTPA     |
 |      7     |         D7       |      PTPB     |
 +------------+------------------+---------------+
*/
#define DISABLE 0x00

// If your MCU do not Have DMA for tx transmit #define USE_TX_IT
// If your MCU have CRC polynome 16 #define USE_CRC_HW 1 else #define USE_CRC_HW 0
#ifndef USE_CRC_HW
#define USE_CRC_HW 0
#endif

#ifndef MCUFREQ
#define MCUFREQ 48000000 // MCU frequence
#endif
#ifndef TIMERDIV
#define TIMERDIV 1 // clock divider for timer clock chosen
#endif
/*******************************************************************************
 * PINOUT CONFIG
 ******************************************************************************/
#ifndef PORT_CLOCK_ENABLE
#define PORT_CLOCK_ENABLE()                                                                                                     \
    do                                                                                                                          \
    {                                                                                                                           \
        GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_EIC_Val) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN); \
        PM->APBAMASK.reg |= PM_APBAMASK_EIC;                                                                                    \
    } while (0U)
#endif

// PTP pin definition
#ifndef PTPA_PIN
#define PTPA_PIN g_APinDescription[6].ulPin
#endif
#ifndef PTPA_PORT
#define PTPA_PORT g_APinDescription[6].ulPort
#endif
#ifndef PTPA_IRQ
#define PTPA_IRQ g_APinDescription[6].ulExtInt // see EXTINT
#endif

#ifndef PTPB_PIN
#define PTPB_PIN g_APinDescription[7].ulPin
#endif
#ifndef PTPB_PORT
#define PTPB_PORT g_APinDescription[7].ulPort
#endif
#ifndef PTPB_IRQ
#define PTPB_IRQ g_APinDescription[7].ulExtInt // see EXTINT
#endif

// COM pin definition
#ifndef TX_LOCK_DETECT_PIN
#define TX_LOCK_DETECT_PIN DISABLE
#endif
#ifndef TX_LOCK_DETECT_PORT
#define TX_LOCK_DETECT_PORT DISABLE
#endif
#ifndef TX_LOCK_DETECT_IRQ
#define TX_LOCK_DETECT_IRQ DISABLE
#endif

#ifndef RX_EN_PIN
#define RX_EN_PIN g_APinDescription[2].ulPin
#endif
#ifndef RX_EN_PORT
#define RX_EN_PORT g_APinDescription[2].ulPort
#endif

#ifndef TX_EN_PIN
#define TX_EN_PIN g_APinDescription[3].ulPin
#endif
#ifndef TX_EN_PORT
#define TX_EN_PORT g_APinDescription[3].ulPort
#endif

#ifndef COM_TX_PIN
#define COM_TX_PIN g_APinDescription[PIN_SERIAL1_TX].ulPin
#endif
#ifndef COM_TX_PORT
#define COM_TX_PORT g_APinDescription[PIN_SERIAL1_TX].ulPort
#endif
#ifndef COM_TX_AF
#define COM_TX_AF g_APinDescription[PIN_SERIAL1_TX].ulPinType
#endif
#ifndef COM_TX_POS
#define COM_TX_POS 1 // PAD2
#endif

#ifndef COM_RX_PIN
#define COM_RX_PIN g_APinDescription[PIN_SERIAL1_RX].ulPin // this pin should pin pull up to vcc if no internal pull up
#endif
#ifndef COM_RX_PORT
#define COM_RX_PORT g_APinDescription[PIN_SERIAL1_RX].ulPort
#endif
#ifndef COM_RX_AF
#define COM_RX_AF g_APinDescription[PIN_SERIAL1_RX].ulPinType
#endif
#ifndef COM_RX_POS
#define COM_RX_POS 3 // PAD3
#endif

#ifndef PINOUT_IRQHANDLER
#define PINOUT_IRQHANDLER() EIC_Handler()
#endif

/*******************************************************************************
 * COM CONFIG
 ******************************************************************************/
#ifndef LUOS_COM_CLOCK_ENABLE
#define LUOS_COM_CLOCK_ENABLE()                                                                                                          \
    do                                                                                                                                   \
    {                                                                                                                                    \
        GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM0_CORE_Val) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN); \
        PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0;                                                                                         \
    } while (0U)
#endif
#ifndef LUOS_COM
#define LUOS_COM SERCOM0
#endif
#ifndef LUOS_COM_IRQ
#define LUOS_COM_IRQ SERCOM0_IRQn
#endif
#ifndef LUOS_COM_IRQHANDLER
#define LUOS_COM_IRQHANDLER() SERCOM0_Handler()
#endif
/*******************************************************************************
 * DMA CONFIG
 ******************************************************************************/
#ifndef LUOS_DMA_CLOCK_ENABLE
#define LUOS_DMA_CLOCK_ENABLE()              \
    do                                       \
    {                                        \
        PM->APBCMASK.reg |= PM_AHBMASK_DMAC; \
    } while (0U)
#endif
#ifndef LUOS_DMA
#define LUOS_DMA DMAC
#endif
#ifndef LUOS_DMA_TRIGGER
#define LUOS_DMA_TRIGGER 2
#endif
#ifndef LUOS_DMA_CHANNEL
#define LUOS_DMA_CHANNEL 0
#endif
/*******************************************************************************
 * COM TIMEOUT CONFIG
 ******************************************************************************/
#ifndef LUOS_TIMER_LOCK_ENABLE
#define LUOS_TIMER_LOCK_ENABLE()                                                                                                     \
    do                                                                                                                               \
    {                                                                                                                                \
        GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TCC2_TC3_Val) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN); \
        PM->APBCMASK.reg |= PM_APBCMASK_TC3;                                                                                         \
    } while (0U)
#endif
#ifndef LUOS_TIMER
#define LUOS_TIMER TC3
#endif
#ifndef LUOS_TIMER_IRQ
#define LUOS_TIMER_IRQ TC3_IRQn
#endif
#ifndef LUOS_TIMER_IRQHANDLER
#define LUOS_TIMER_IRQHANDLER() TC3_Handler()
#endif
/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef PAGE_SIZE
#define PAGE_SIZE 64
#endif
#ifndef ERASE_SIZE
#define ERASE_SIZE 256
#endif
#ifndef FLASH_SIZE
#define FLASH_SIZE 0x40000
#endif
#ifndef ADDRESS_LAST_PAGE_FLASH
#define ADDRESS_LAST_PAGE_FLASH FLASH_SIZE - (16 * PAGE_SIZE)
#endif

/*******************************************************************************
 * BOOTLOADER CONFIG
 ******************************************************************************/
#define SHARED_MEMORY_ADDRESS 0x0800C000
#define SHARED_FLASH_PAGE     25
#define APP_ADDRESS           (uint32_t)0x0800C800
#define APP_FLASH_PAGE        26

#endif /* _LUOSHAL_CONFIG_H_ */
