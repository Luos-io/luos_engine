/*******************************************************************************
 Cortex-M device vectors file

  Company:
    Microchip Technology Inc.

  File Name:
    device_vectors.h

  Summary:
    Harmony3 device handler structure for cortex-M devices

  Description:
    This file contains Harmony3 device handler structure for cortex-M devices
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END

#ifndef DEVICE_VECTORS_H
#define DEVICE_VECTORS_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdint.h>

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

/* Function pointer type for vector handlers */
typedef void (*pfn_handler_t)(void);

/* Structure defining device vector types */
typedef struct _H3DeviceVectors
{
  /* Stack pointer */
  uint32_t* pvStack;

  /* CORTEX-M0PLUS handlers */ 
  pfn_handler_t pfnReset_Handler;                   /* -15 Reset Vector, invoked on Power up and warm reset */
  pfn_handler_t pfnNonMaskableInt_Handler;          /* -14 Non maskable Interrupt, cannot be stopped or preempted */
  pfn_handler_t pfnHardFault_Handler;               /* -13 Hard Fault, all classes of Fault */
  pfn_handler_t pfnReservedC12;
  pfn_handler_t pfnReservedC11;
  pfn_handler_t pfnReservedC10;
  pfn_handler_t pfnReservedC9;
  pfn_handler_t pfnReservedC8;
  pfn_handler_t pfnReservedC7;
  pfn_handler_t pfnReservedC6;
  pfn_handler_t pfnSVCall_Handler;                  /* -5 System Service Call via SVC instruction */
  pfn_handler_t pfnReservedC4;
  pfn_handler_t pfnReservedC3;
  pfn_handler_t pfnPendSV_Handler;                  /* -2 Pendable request for system service */
  pfn_handler_t pfnSysTick_Handler;                 /* -1 System Tick Timer */

  /* Peripheral handlers */
  pfn_handler_t pfnPM_Handler;                      /* 0 Power Manager */
  pfn_handler_t pfnSYSCTRL_Handler;                 /* 1 System Controller */
  pfn_handler_t pfnWDT_Handler;                     /* 2 Watchdog Timer */
  pfn_handler_t pfnRTC_Handler;                     /* 3 Real Time Counter */
  pfn_handler_t pfnEIC_Handler;                     /* 4 External Interrupt Controller */
  pfn_handler_t pfnNVMCTRL_Handler;                 /* 5 Non-Volatile Memory Controller */
  pfn_handler_t pfnDMAC_Handler;                    /* 6 Direct Memory Controller */
  pfn_handler_t pfnUSB_Handler;                     /* 7 Universal Serial Bus */
  pfn_handler_t pfnEVSYS_Handler;                   /* 8 Event Systems */
  pfn_handler_t pfnSERCOM0_Handler;                 /* 9 Serial Communication Interface 0 */
  pfn_handler_t pfnSERCOM1_Handler;                 /* 10 Serial Communication Interface 1 */
  pfn_handler_t pfnSERCOM2_Handler;                 /* 11 Serial Communication Interface 2 */
  pfn_handler_t pfnSERCOM3_Handler;                 /* 12 Serial Communication Interface 3 */
  pfn_handler_t pfnSERCOM4_Handler;                 /* 13 Serial Communication Interface 4 */
  pfn_handler_t pfnSERCOM5_Handler;                 /* 14 Serial Communication Interface 5 */
  pfn_handler_t pfnTCC0_Handler;                    /* 15 Timer/Counter for Control Applications 0 */
  pfn_handler_t pfnTCC1_Handler;                    /* 16 Timer/Counter for Control Applications 1 */
  pfn_handler_t pfnTCC2_Handler;                    /* 17 Timer/Counter for Control Applications 2 */
  pfn_handler_t pfnTC3_Handler;                     /* 18 Timer/Counter 3 */
  pfn_handler_t pfnTC4_Handler;                     /* 19 Timer/Counter 4 */
  pfn_handler_t pfnTC5_Handler;                     /* 20 Timer/Counter 5 */
  pfn_handler_t pfnTC6_Handler;                     /* 21 Timer/Counter 6 */
  pfn_handler_t pfnTC7_Handler;                     /* 22 Timer/Counter 7 */
  pfn_handler_t pfnADC_Handler;                     /* 23 Analog-to-Digital Converter */
  pfn_handler_t pfnAC_Handler;                      /* 24 Analog Comparators */
  pfn_handler_t pfnDAC_Handler;                     /* 25 Digital-to-Analog Converter */
  pfn_handler_t pfnPTC_Handler;                     /* 26 Peripheral Touch Controller */
  pfn_handler_t pfnI2S_Handler;                     /* 27 Inter-IC Sound Controller */
}H3DeviceVectors;

#endif //DEVICE_VECTORS_H
