/******************************************************************************
 * @file robusHAL_Config
 * @brief This file allow you to configure RobusHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @Family Linux (Raspberry Pi)
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _ROBUSHAL_CONFIG_H_
#define _ROBUSHAL_CONFIG_H_

#include "luos_hal.h"

// Critical section (no-op, handled by recursive mutex in luos_hal.c)
#define _CRITICAL

// No hardware peripherals
#define DISABLE 0x00

// Enable NORT -- required for Linux HAL (non-real-time platform)
#define NORT

// Number of network PHYs (Robus + Serial = 2, adjust if more networks are added)
#ifndef LOCAL_PHY_NB
    #define LOCAL_PHY_NB 2
#endif

// Serial port device
#ifndef ROBUS_COM_DEVICE
    #define ROBUS_COM_DEVICE "/dev/ttyAMA0"
#endif

// GPIO chip device
#ifndef ROBUS_GPIO_CHIP
    #define ROBUS_GPIO_CHIP "/dev/gpiochip0"
#endif

// RS485 TX_EN GPIO (mandatory -- controls RS485 transceiver direction)
#ifndef TX_EN_GPIO_LINE
    #define TX_EN_GPIO_LINE 4
#endif

// PTP GPIO configuration (mandatory -- required for topology detection)
#ifndef PTPA_GPIO_LINE
    #define PTPA_GPIO_LINE 17
#endif
#ifndef PTPB_GPIO_LINE
    #define PTPB_GPIO_LINE 27
#endif

// PTP stub values for compatibility
#define PTPA_PIN  PTPA_GPIO_LINE
#define PTPA_PORT NULL
#define PTPA_IRQ  DISABLE

#define PTPB_PIN  PTPB_GPIO_LINE
#define PTPB_PORT NULL
#define PTPB_IRQ  DISABLE

// Timer
#ifndef TIMERDIV
    #define TIMERDIV 1
#endif

// CRC in software
#ifndef USE_CRC_HW
    #define USE_CRC_HW 0
#endif

// PTP timing overrides for Linux scheduler latency
#define PTP_PUSH_DELAY_MS 20
#define PTP_READ_DELAY_MS 30

// IRQ handler stubs (not used on Linux -- RX thread replaces ISRs)
#define ROBUS_COM_IRQHANDLER   void RobusHAL_ComIrqStub
#define ROBUS_TIMER_IRQHANDLER void RobusHAL_TimerIrqStub
#define PINOUT_IRQHANDLER      RobusHAL_PinoutIrqStub

// Conditional debug macro
#ifdef LUOS_DEBUG_PRINT
    #include <stdio.h>
    #define ROBUS_DBG(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); } while(0)
#else
    #define ROBUS_DBG(fmt, ...) ((void)0)
#endif

#endif /* _ROBUSHAL_CONFIG_H_ */
