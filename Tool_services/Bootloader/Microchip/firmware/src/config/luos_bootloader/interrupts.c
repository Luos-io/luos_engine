/*******************************************************************************
 System Interrupts File

  Company:
    Microchip Technology Inc.

  File Name:
    interrupt.c

  Summary:
    Interrupt vectors mapping

  Description:
    This file maps all the interrupt vectors to their corresponding
    implementations. If a particular module interrupt is used, then its ISR
    definition can be found in corresponding PLIB source file. If a module
    interrupt is not used, then its ISR implementation is mapped to dummy
    handler.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "device_vectors.h"
#include "interrupts.h"
#include "definitions.h"


// *****************************************************************************
// *****************************************************************************
// Section: System Interrupt Vector Functions
// *****************************************************************************
// *****************************************************************************

extern uint32_t _stack;
extern const H3DeviceVectors exception_table;

extern void Dummy_Handler(void);

/* Brief default interrupt handler for unused IRQs.*/
void __attribute__((optimize("-O1"),section(".text.Dummy_Handler"),long_call, noreturn))Dummy_Handler(void)
{
    while (true)
    {
    }
}
/* Device vectors list dummy definition*/
extern void SVCall_Handler             ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void PendSV_Handler             ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void SysTick_Handler            ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void PM_Handler                 ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void SYSCTRL_Handler            ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void WDT_Handler                ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void RTC_Handler                ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void EIC_Handler                ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void NVMCTRL_Handler            ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void DMAC_Handler               ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void USB_Handler                ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void EVSYS_Handler              ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void SERCOM0_Handler            ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void SERCOM1_Handler            ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void SERCOM2_Handler            ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void SERCOM3_Handler            ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void SERCOM4_Handler            ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void SERCOM5_Handler            ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void TCC0_Handler               ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void TCC1_Handler               ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void TCC2_Handler               ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void TC3_Handler                ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void TC4_Handler                ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void TC5_Handler                ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void TC6_Handler                ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void TC7_Handler                ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void ADC_Handler                ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void AC_Handler                 ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void DAC_Handler                ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void PTC_Handler                ( void ) __attribute__((weak, alias("Dummy_Handler")));
extern void I2S_Handler                ( void ) __attribute__((weak, alias("Dummy_Handler")));



/* Multiple handlers for vector */



__attribute__ ((section(".vectors")))
const H3DeviceVectors exception_table=
{
    /* Configure Initial Stack Pointer, using linker-generated symbols */
    .pvStack = &_stack,

    .pfnReset_Handler              = Reset_Handler,
    .pfnNonMaskableInt_Handler     = NonMaskableInt_Handler,
    .pfnHardFault_Handler          = HardFault_Handler,
    .pfnSVCall_Handler             = SVCall_Handler,
    .pfnPendSV_Handler             = PendSV_Handler,
    .pfnSysTick_Handler            = SysTick_Handler,
    .pfnPM_Handler                 = PM_Handler,
    .pfnSYSCTRL_Handler            = SYSCTRL_Handler,
    .pfnWDT_Handler                = WDT_Handler,
    .pfnRTC_Handler                = RTC_Handler,
    .pfnEIC_Handler                = EIC_Handler,
    .pfnNVMCTRL_Handler            = NVMCTRL_Handler,
    .pfnDMAC_Handler               = DMAC_Handler,
    .pfnUSB_Handler                = USB_Handler,
    .pfnEVSYS_Handler              = EVSYS_Handler,
    .pfnSERCOM0_Handler            = SERCOM0_Handler,
    .pfnSERCOM1_Handler            = SERCOM1_Handler,
    .pfnSERCOM2_Handler            = SERCOM2_Handler,
    .pfnSERCOM3_Handler            = SERCOM3_Handler,
    .pfnSERCOM4_Handler            = SERCOM4_Handler,
    .pfnSERCOM5_Handler            = SERCOM5_Handler,
    .pfnTCC0_Handler               = TCC0_Handler,
    .pfnTCC1_Handler               = TCC1_Handler,
    .pfnTCC2_Handler               = TCC2_Handler,
    .pfnTC3_Handler                = TC3_Handler,
    .pfnTC4_Handler                = TC4_Handler,
    .pfnTC5_Handler                = TC5_Handler,
    .pfnTC6_Handler                = TC6_Handler,
    .pfnTC7_Handler                = TC7_Handler,
    .pfnADC_Handler                = ADC_Handler,
    .pfnAC_Handler                 = AC_Handler,
    .pfnDAC_Handler                = DAC_Handler,
    .pfnPTC_Handler                = PTC_Handler,
    .pfnI2S_Handler                = I2S_Handler,


};

/*******************************************************************************
 End of File
*/
