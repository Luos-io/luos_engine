/******************************************************************************
 * @file robusHAL_Config
 * @brief This file allow you to configure RobusHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @Family x86
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _ROBUSHAL_CONFIG_H_
#define _ROBUSHAL_CONFIG_H_

// STUB Value for x86 stub only
#define X86_STUB 0x00

#define _CRITICAL
#define DISABLE 0x00

// If your MCU do not Have DMA for tx transmit define USE_TX_IT
#define USE_TX_IT

// If your MCU have CRC polynome 16 #define USE_CRC_HW 1 else #define USE_CRC_HW 0
#ifndef USE_CRC_HW
    #define USE_CRC_HW 0
#endif

#ifndef TIMERDIV
    #define TIMERDIV 1 // clock divider for timer clock chosen
#endif
/*******************************************************************************
 * PINOUT CONFIG
 ******************************************************************************/
#ifndef PORT_CLOCK_ENABLE
    #define PORT_CLOCK_ENABLE() X86_STUB
#endif

// PTP pin definition
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

#ifndef PINOUT_IRQHANDLER
//#define PINOUT_IRQHANDLER(PIN)
#endif

/*******************************************************************************
 * COM CONFIG
 ******************************************************************************/
#ifndef ROBUS_COM_CLOCK_ENABLE
    #define ROBUS_COM_CLOCK_ENABLE() X86_STUB
#endif
#ifndef ROBUS_COM
    #define ROBUS_COM X86_STUB // STUB
#endif
#ifndef ROBUS_COM_IRQ
    #define ROBUS_COM_IRQ X86_STUB // STUB
#endif
#ifndef ROBUS_COM_IRQHANDLER
    #define ROBUS_COM_IRQHANDLER() // STUB
#endif

/*******************************************************************************
 * DMA CONFIG
 ******************************************************************************/
#ifndef ROBUS_DMA_CLOCK_ENABLE
    #define ROBUS_DMA_CLOCK_ENABLE() X86_STUB
#endif
#ifndef ROBUS_DMA
    #define ROBUS_DMA X86_STUB // STUB
#endif
#ifndef ROBUS_DMA_CHANNEL
    #define ROBUS_DMA_CHANNEL X86_STUB // STUB
#endif
#ifndef ROBUS_DMA_REMAP
    #define ROBUS_DMA_REMAP X86_STUB // STUB
#endif

/*******************************************************************************
 * COM TIMEOUT CONFIG
 ******************************************************************************/
#ifndef ROBUS_TIMER_CLOCK_ENABLE
    #define ROBUS_TIMER_CLOCK_ENABLE() // STUB
#endif
#ifndef ROBUS_TIMER
    #define ROBUS_TIMER X86_STUB // STUB
#endif
#ifndef ROBUS_TIMER_IRQ
    #define ROBUS_TIMER_IRQ X86_STUB // STUB
#endif
#ifndef ROBUS_TIMER_IRQHANDLER
    #define ROBUS_TIMER_IRQHANDLER() x86_Timer_IRQHandler()
#endif

#endif /* _ROBUSHAL_CONFIG_H_ */
