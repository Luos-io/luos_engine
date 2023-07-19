/******************************************************************************
 * @file RobusHAL_Config
 * @brief This file allow you to configure RobusHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _RobusHAL_CONFIG_H_
#define _RobusHAL_CONFIG_H_

// include main sdk files relative to your MCU family

#define _CRITICAL
#define DISABLE 0x00

// If your MCU do not Have DMA for tx transmit define USE_TX_IT

// If your MCU have CRC polynome 16 #define USE_CRC_HW 1 else #define USE_CRC_HW 0
#ifndef USE_CRC_HW
    #define USE_CRC_HW 0
#endif

#ifndef TIMERDIV
    #define TIMERDIV // clock divider for timer clock chosen
#endif
/*******************************************************************************
 * PINOUT CONFIG
 ******************************************************************************/
#ifndef PORT_CLOCK_ENABLE
    #define PORT_CLOCK_ENABLE
#endif

// PTP pin definition
#ifndef PTPA_PIN
    #define PTPA_PIN
#endif
#ifndef PTPA_PORT
    #define PTPA_PORT
#endif
#ifndef PTPA_IRQ
    #define PTPA_IRQ
#endif

#ifndef PTPB_PIN
    #define PTPB_PIN
#endif
#ifndef PTPB_PORT
    #define PTPB_PORT
#endif
#ifndef PTPB_IRQ
    #define PTPB_IRQ
#endif

// COM pin definition
#ifndef TX_LOCK_DETECT_PIN
    #define TX_LOCK_DETECT_PIN
#endif
#ifndef TX_LOCK_DETECT_PORT
    #define TX_LOCK_DETECT_PORT
#endif
#ifndef TX_LOCK_DETECT_IRQ
    #define TX_LOCK_DETECT_IRQ
#endif

#ifndef RX_EN_PIN
    #define RX_EN_PIN
#endif
#ifndef RX_EN_PORT
    #define RX_EN_PORT
#endif

#ifndef TX_EN_PIN
    #define TX_EN_PIN
#endif
#ifndef TX_EN_PORT
    #define TX_EN_PORT
#endif

#ifndef COM_TX_PIN
    #define COM_TX_PIN
#endif
#ifndef COM_TX_PORT
    #define COM_TX_PORT
#endif
#ifndef COM_TX_AF
    #define COM_TX_AF
#endif

#ifndef COM_RX_PIN
    #define COM_RX_PIN
#endif
#ifndef COM_RX_PORT
    #define COM_RX_PORT
#endif
#ifndef COM_RX_AF
    #define COM_RX_AF
#endif

#ifndef PINOUT_IRQHANDLER
    #define PINOUT_IRQHANDLER(PIN)
#endif

/*******************************************************************************
 * COM CONFIG
 ******************************************************************************/
#ifndef ROBUS_COM_CLOCK_ENABLE
    #define ROBUS_COM_CLOCK_ENABLE
#endif
#ifndef ROBUS_COM
    #define ROBUS_COM
#endif
#ifndef ROBUS_COM_IRQ
    #define ROBUS_COM_IRQ
#endif
#ifndef ROBUS_COM_IRQHANDLER
    #define ROBUS_COM_IRQHANDLER()
#endif
/*******************************************************************************
 * DMA CONFIG
 ******************************************************************************/
#ifndef ROBUS_DMA_CLOCK_ENABLE
    #define ROBUS_DMA_CLOCK_ENABLE
#endif
#ifndef ROBUS_DMA
    #define ROBUS_DMA
#endif
#ifndef ROBUS_DMA_CHANNEL
    #define ROBUS_DMA_CHANNEL
#endif
/*******************************************************************************
 * COM TIMEOUT CONFIG
 ******************************************************************************/
#ifndef ROBUS_TIMER_CLOCK_ENABLE
    #define ROBUS_TIMER_CLOCK_ENABLE
#endif
#ifndef ROBUS_TIMER
    #define ROBUS_TIMER
#endif
#ifndef ROBUS_TIMER_IRQ
    #define ROBUS_TIMER_IRQ
#endif
#ifndef ROBUS_TIMER_IRQHANDLER
    #define ROBUS_TIMER_IRQHANDLER()
#endif

#endif /* _RobusHAL_CONFIG_H_ */
