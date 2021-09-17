/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
  * @Family x86
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

//include file relative to your MCU family

//If your MCU do not Have DMA for tx transmit define USE_TX_IT
#define USE_TX_IT

//If your MCU have CRC polynome 16 #define USE_CRC_HW 1 else #define USE_CRC_HW 0
#ifndef USE_CRC_HW
#define USE_CRC_HW 0
#endif

//STUB Value for x86 stub only
#define X86_STUB 0x00

#define DISABLE 0x00

#ifndef MCUFREQ
#define MCUFREQ 100000000 //MCU frequence
#endif
#ifndef TIMERDIV
#define TIMERDIV 1 //clock divider for timer clock chosen
#endif
/*******************************************************************************
 * PINOUT CONFIG
 ******************************************************************************/
#ifndef PORT_CLOCK_ENABLE
#define PORT_CLOCK_ENABLE() \
    do                      \
    {                       \
    } while (0U)
#endif

//PTP pin definition
#ifndef PTPA_PIN
#define PTPA_PIN X86_STUB
#endif
#ifndef PTPA_PORT
#define PTPA_PORT X86_STUB
#endif
#ifndef PTPA_IRQ
#define PTPA_IRQ X86_STUB
#endif

#ifndef PTPB_PIN
#define PTPB_PIN X86_STUB
#endif
#ifndef PTPB_PORT
#define PTPB_PORT X86_STUB
#endif
#ifndef PTPB_IRQ
#define PTPB_IRQ X86_STUB
#endif

//COM pin definition
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
#define RX_EN_PIN X86_STUB
#endif
#ifndef RX_EN_PORT
#define RX_EN_PORT X86_STUB
#endif

#ifndef TX_EN_PIN
#define TX_EN_PIN X86_STUB
#endif
#ifndef TX_EN_PORT
#define TX_EN_PORT X86_STUB
#endif

#ifndef COM_TX_PIN
#define COM_TX_PIN X86_STUB
#endif
#ifndef COM_TX_PORT
#define COM_TX_PORT X86_STUB
#endif
#ifndef COM_TX_AF
#define COM_TX_AF X86_STUB
#endif

#ifndef COM_RX_PIN
#define COM_RX_PIN X86_STUB
#endif
#ifndef COM_RX_PORT
#define COM_RX_PORT X86_STUB
#endif
#ifndef COM_RX_AF
#define COM_RX_AF X86_STUB
#endif

/*
#ifndef PINOUT_IRQHANDLER
#define PINOUT_IRQHANDLER(PIN)
#endif
*/

/*******************************************************************************
 * COM CONFIG
 ******************************************************************************/
#ifndef LUOS_COM_CLOCK_ENABLE
#define LUOS_COM_CLOCK_ENABLE() \
    do                          \
    {                           \
        ;                       \
    } while (0U)
#endif
#ifndef LUOS_COM
#define LUOS_COM X86_STUB //STUB
#endif
#ifndef LUOS_COM_IRQ
#define LUOS_COM_IRQ X86_STUB //STUB
#endif
/*#ifndef LUOS_COM_IRQHANDLER
#define LUOS_COM_IRQHANDLER() //STUB
#endif*/

/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef LUOS_DMA_CLOCK_ENABLE
#define LUOS_DMA_CLOCK_ENABLE() \
    do                          \
    {                           \
        ;                       \
    } while (0U)
#endif
#ifndef LUOS_DMA
#define LUOS_DMA X86_STUB //STUB
#endif
#ifndef LUOS_DMA_CHANNEL
#define LUOS_DMA_CHANNEL X86_STUB //STUB
#endif
#ifndef LUOS_DMA_REMAP
#define LUOS_DMA_REMAP X86_STUB //STUB
#endif

/*******************************************************************************
 * COM TIMEOUT CONFIG
 ******************************************************************************/
/*#ifndef LUOS_TIMER_LOCK_ENABLE
#define LUOS_TIMER_LOCK_ENABLE()      \
    do                                \
    {                                 \
        ;                             \
    } while (0U)
#endif*/
#ifndef LUOS_TIMER_CLOCK_ENABLE
//#define LUOS_TIMER_LOCK_ENABLE() //STUB
#define LUOS_TIMER_CLOCK_ENABLE() \
    do                            \
    {                             \
    } while (0U) //STUB
#endif
#ifndef LUOS_TIMER
#define LUOS_TIMER X86_STUB //STUB
#endif
#ifndef LUOS_TIMER_IRQ
#define LUOS_TIMER_IRQ X86_STUB //STUB
#endif
/*#ifndef LUOS_TIMER_IRQHANDLER
#define LUOS_TIMER_IRQHANDLER() //STUB
#endif*/

/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef PAGE_SIZE
//#define PAGE_SIZE (uint32_t) FLASH_PAGE_SIZE
#define PAGE_SIZE (uint32_t) X86_STUB //STUB
#endif
#ifndef ADDRESS_LAST_PAGE_FLASH
//#define ADDRESS_LAST_PAGE_FLASH ((uint32_t)(FLASH_BANK1_END - FLASH_PAGE_SIZE))
#define ADDRESS_LAST_PAGE_FLASH X86_STUB //STUB
#endif

#endif /* _LUOSHAL_CONFIG_H_ */
