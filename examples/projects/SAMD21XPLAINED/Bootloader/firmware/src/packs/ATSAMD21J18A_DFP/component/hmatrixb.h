/**
 * \brief Component description for HMATRIXB
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
#ifndef _SAMD21_HMATRIXB_COMPONENT_H_
#define _SAMD21_HMATRIXB_COMPONENT_H_

/* ************************************************************************** */
/*   SOFTWARE API DEFINITION FOR HMATRIXB                                     */
/* ************************************************************************** */

/* -------- HMATRIXB_PRAS : (HMATRIXB Offset: 0x00) (R/W 32) Priority A for Slave -------- */
#define HMATRIXB_PRAS_RESETVALUE              _U_(0x00)                                            /**<  (HMATRIXB_PRAS) Priority A for Slave  Reset Value */

#define HMATRIXB_PRAS_Msk                     _U_(0x00000000)                                      /**< (HMATRIXB_PRAS) Register Mask  */


/* -------- HMATRIXB_PRBS : (HMATRIXB Offset: 0x04) (R/W 32) Priority B for Slave -------- */
#define HMATRIXB_PRBS_RESETVALUE              _U_(0x00)                                            /**<  (HMATRIXB_PRBS) Priority B for Slave  Reset Value */

#define HMATRIXB_PRBS_Msk                     _U_(0x00000000)                                      /**< (HMATRIXB_PRBS) Register Mask  */


/* -------- HMATRIXB_SFR : (HMATRIXB Offset: 0x110) (R/W 32) Special Function -------- */
#define HMATRIXB_SFR_RESETVALUE               _U_(0x00)                                            /**<  (HMATRIXB_SFR) Special Function  Reset Value */

#define HMATRIXB_SFR_SFR_Pos                  _U_(0)                                               /**< (HMATRIXB_SFR) Special Function Register Position */
#define HMATRIXB_SFR_SFR_Msk                  (_U_(0xFFFFFFFF) << HMATRIXB_SFR_SFR_Pos)            /**< (HMATRIXB_SFR) Special Function Register Mask */
#define HMATRIXB_SFR_SFR(value)               (HMATRIXB_SFR_SFR_Msk & ((value) << HMATRIXB_SFR_SFR_Pos))
#define HMATRIXB_SFR_Msk                      _U_(0xFFFFFFFF)                                      /**< (HMATRIXB_SFR) Register Mask  */


/** \brief HMATRIXB register offsets definitions */
#define HMATRIXB_PRAS_REG_OFST         (0x00)              /**< (HMATRIXB_PRAS) Priority A for Slave Offset */
#define HMATRIXB_PRBS_REG_OFST         (0x04)              /**< (HMATRIXB_PRBS) Priority B for Slave Offset */
#define HMATRIXB_SFR_REG_OFST          (0x110)             /**< (HMATRIXB_SFR) Special Function Offset */
#define HMATRIXB_SFR0_REG_OFST         (0x110)             /**< (HMATRIXB_SFR0) Special Function Offset */
#define HMATRIXB_SFR1_REG_OFST         (0x114)             /**< (HMATRIXB_SFR1) Special Function Offset */
#define HMATRIXB_SFR2_REG_OFST         (0x118)             /**< (HMATRIXB_SFR2) Special Function Offset */
#define HMATRIXB_SFR3_REG_OFST         (0x11C)             /**< (HMATRIXB_SFR3) Special Function Offset */
#define HMATRIXB_SFR4_REG_OFST         (0x120)             /**< (HMATRIXB_SFR4) Special Function Offset */
#define HMATRIXB_SFR5_REG_OFST         (0x124)             /**< (HMATRIXB_SFR5) Special Function Offset */
#define HMATRIXB_SFR6_REG_OFST         (0x128)             /**< (HMATRIXB_SFR6) Special Function Offset */
#define HMATRIXB_SFR7_REG_OFST         (0x12C)             /**< (HMATRIXB_SFR7) Special Function Offset */
#define HMATRIXB_SFR8_REG_OFST         (0x130)             /**< (HMATRIXB_SFR8) Special Function Offset */
#define HMATRIXB_SFR9_REG_OFST         (0x134)             /**< (HMATRIXB_SFR9) Special Function Offset */
#define HMATRIXB_SFR10_REG_OFST        (0x138)             /**< (HMATRIXB_SFR10) Special Function Offset */
#define HMATRIXB_SFR11_REG_OFST        (0x13C)             /**< (HMATRIXB_SFR11) Special Function Offset */
#define HMATRIXB_SFR12_REG_OFST        (0x140)             /**< (HMATRIXB_SFR12) Special Function Offset */
#define HMATRIXB_SFR13_REG_OFST        (0x144)             /**< (HMATRIXB_SFR13) Special Function Offset */
#define HMATRIXB_SFR14_REG_OFST        (0x148)             /**< (HMATRIXB_SFR14) Special Function Offset */
#define HMATRIXB_SFR15_REG_OFST        (0x14C)             /**< (HMATRIXB_SFR15) Special Function Offset */

#if !(defined(__ASSEMBLER__) || defined(__IAR_SYSTEMS_ASM__))
/** \brief PRS register API structure */
typedef struct
{
  __IO  uint32_t                       HMATRIXB_PRAS;      /**< Offset: 0x00 (R/W  32) Priority A for Slave */
  __IO  uint32_t                       HMATRIXB_PRBS;      /**< Offset: 0x04 (R/W  32) Priority B for Slave */
} hmatrixb_prs_registers_t;

#define HMATRIXB_PRS_NUMBER _U_(16)

/** \brief HMATRIXB register API structure */
typedef struct
{  /* HSB Matrix */
  __I   uint8_t                        Reserved1[0x80];
        hmatrixb_prs_registers_t       PRS[HMATRIXB_PRS_NUMBER]; /**< Offset: 0x80  */
  __I   uint8_t                        Reserved2[0x10];
  __IO  uint32_t                       HMATRIXB_SFR[16];   /**< Offset: 0x110 (R/W  32) Special Function */
} hmatrixb_registers_t;


#endif /* !(defined(__ASSEMBLER__) || defined(__IAR_SYSTEMS_ASM__)) */
#endif /* _SAMD21_HMATRIXB_COMPONENT_H_ */
