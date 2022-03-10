/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

// include file relative to your MCU family

// If your MCU do not Have DMA for tx transmit define USE_TX_IT

#define DISABLE 0x00

#ifndef MCUFREQ
#define MCUFREQ // MCU frequence
#endif
#ifndef TIMERDIV
#define TIMERDIV // clock divider for timer clock chosen
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
#ifndef LUOS_COM_CLOCK_ENABLE
#define LUOS_COM_CLOCK_ENABLE() \
    do                          \
    {
}
while (0U)
#endif
#ifndef LUOS_COM
#define LUOS_COM
#endif
#ifndef LUOS_COM_IRQ
#define LUOS_COM_IRQ
#endif
#ifndef LUOS_COM_IRQHANDLER
#define LUOS_COM_IRQHANDLER()
#endif
/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef LUOS_DMA_CLOCK_ENABLE
#define LUOS_DMA_CLOCK_ENABLE() \
    do                          \
    {
}
while (0U)
#endif
#ifndef LUOS_DMA
#define LUOS_DMA
#endif
#ifndef LUOS_DMA_CHANNEL
#define LUOS_DMA_CHANNEL
#endif
/*******************************************************************************
 * COM TIMEOUT CONFIG
 ******************************************************************************/
#ifndef LUOS_TIMER_LOCK_ENABLE
#define LUOS_TIMER_LOCK_ENABLE() \
    do                           \
    {
}
while (0U)
#endif
#ifndef LUOS_TIMER
#define LUOS_TIMER
#endif
#ifndef LUOS_TIMER_IRQ
#define LUOS_TIMER_IRQ
#endif
#ifndef LUOS_TIMER_IRQHANDLER
#define LUOS_TIMER_IRQHANDLER()
#endif
/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef PAGE_SIZE
#define PAGE_SIZE (uint32_t) FLASH_PAGE_SIZE
#endif
#ifndef ADDRESS_LAST_PAGE_FLASH
#define ADDRESS_LAST_PAGE_FLASH ((uint32_t)(FLASH_BANK1_END - FLASH_PAGE_SIZE))
#endif

#endif /* _LUOSHAL_CONFIG_H_ */
