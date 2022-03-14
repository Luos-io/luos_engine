/**
 * \brief Instance header file for ATSAMD21J18A
 *
 * Copyright (c) 2021 Microchip Technology Inc. and its subsidiaries.
 *
 * Subject to your compliance with these terms, you may use Microchip software and any derivatives
 * exclusively with Microchip products. It is your responsibility to comply with third party license
 * terms applicable to your use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY,
 * APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
 * MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT
 * EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 */

/* file generated from device description version 2019-11-25T06:52:33Z */
#ifndef _SAMD21_SYSCTRL_INSTANCE_
#define _SAMD21_SYSCTRL_INSTANCE_


/* ========== Instance Parameter definitions for SYSCTRL peripheral ========== */
#define SYSCTRL_BGAP_CALIB_MSB                   _UL_(11)   
#define SYSCTRL_BOD33_CALIB_MSB                  _UL_(5)    
#define SYSCTRL_DFLL48M_COARSE_MSB               _UL_(5)    
#define SYSCTRL_DFLL48M_FINE_MSB                 _UL_(9)    
#define SYSCTRL_GCLK_ID_DFLL48                   _UL_(0)    /* Index of Generic Clock for DFLL48 */
#define SYSCTRL_GCLK_ID_FDPLL                    _UL_(1)    /* Index of Generic Clock for DPLL */
#define SYSCTRL_GCLK_ID_FDPLL32K                 _UL_(2)    /* Index of Generic Clock for DPLL 32K */
#define SYSCTRL_OSC32K_COARSE_CALIB_MSB          _UL_(6)    
#define SYSCTRL_POR33_ENTEST_MSB                 _UL_(1)    
#define SYSCTRL_SYSTEM_CLOCK                     _UL_(1000000) /* Initial system clock frequency */
#define SYSCTRL_ULPVREF_DIVLEV_MSB               _UL_(3)    
#define SYSCTRL_ULPVREG_FORCEGAIN_MSB            _UL_(1)    
#define SYSCTRL_ULPVREG_RAMREFSEL_MSB            _UL_(2)    
#define SYSCTRL_VREF_CONTROL_MSB                 _UL_(48)   
#define SYSCTRL_VREF_STATUS_MSB                  _UL_(7)    
#define SYSCTRL_VREG_LEVEL_MSB                   _UL_(2)    
#define SYSCTRL_BOD12_VERSION                    _UL_(0x111) 
#define SYSCTRL_BOD33_VERSION                    _UL_(0x111) 
#define SYSCTRL_DFLL48M_VERSION                  _UL_(0x301) 
#define SYSCTRL_FDPLL_VERSION                    _UL_(0x111) 
#define SYSCTRL_OSCULP32K_VERSION                _UL_(0x111) 
#define SYSCTRL_OSC8M_VERSION                    _UL_(0x120) 
#define SYSCTRL_OSC32K_VERSION                   _UL_(0x1101) 
#define SYSCTRL_VREF_VERSION                     _UL_(0x200) 
#define SYSCTRL_VREG_VERSION                     _UL_(0x201) 
#define SYSCTRL_XOSC_VERSION                     _UL_(0x1111) 
#define SYSCTRL_XOSC32K_VERSION                  _UL_(0x1111) 
#define SYSCTRL_INSTANCE_ID                      _UL_(2)    

#endif /* _SAMD21_SYSCTRL_INSTANCE_ */
