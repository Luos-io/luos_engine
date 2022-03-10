/*******************************************************************************
  Direct Memory Access Controller (DMAC) PLIB

  Company
    Microchip Technology Inc.

  File Name
    plib_dmac.c

  Summary
    Source for DMAC peripheral library interface Implementation.

  Description
    This file defines the interface to the DMAC peripheral library. This
    library provides access to and control of the DMAC controller.

  Remarks:
    None.

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

#include "plib_dmac.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

#define DMAC_CHANNELS_NUMBER        1

/* DMAC channels object configuration structure */
typedef struct
{
    uint8_t                inUse;

    DMAC_CHANNEL_CALLBACK  callback;

    uintptr_t              context;

    uint8_t                busyStatus;

} DMAC_CH_OBJECT ;

/* Initial write back memory section for DMAC */
 static  dmac_descriptor_registers_t _write_back_section[DMAC_CHANNELS_NUMBER]    __ALIGNED(16);

/* Descriptor section for DMAC */
 static  dmac_descriptor_registers_t  descriptor_section[DMAC_CHANNELS_NUMBER]    __ALIGNED(16);

/* DMAC Channels object information structure */
DMAC_CH_OBJECT dmacChannelObj[DMAC_CHANNELS_NUMBER];

// *****************************************************************************
// *****************************************************************************
// Section: DMAC PLib Interface Implementations
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
This function initializes the DMAC controller of the device.
********************************************************************************/

void DMAC_Initialize( void )
{
    DMAC_CH_OBJECT *dmacChObj = (DMAC_CH_OBJECT *)&dmacChannelObj[0];
    uint16_t channel = 0;

    /* Initialize DMAC Channel objects */
    for(channel = 0; channel < DMAC_CHANNELS_NUMBER; channel++)
    {
        dmacChObj->inUse = 0;
        dmacChObj->callback = NULL;
        dmacChObj->context = 0;
        dmacChObj->busyStatus = false;

        /* Point to next channel object */
        dmacChObj += 1;
    }

    /* Update the Base address and Write Back address register */
    DMAC_REGS->DMAC_BASEADDR = (uint32_t) descriptor_section;
    DMAC_REGS->DMAC_WRBADDR  = (uint32_t)_write_back_section;

    /* Update the Priority Control register */
    DMAC_REGS->DMAC_PRICTRL0 = DMAC_PRICTRL0_LVLPRI0(1) | DMAC_PRICTRL0_RRLVLEN0_Msk | DMAC_PRICTRL0_LVLPRI1(1) | DMAC_PRICTRL0_RRLVLEN1_Msk | DMAC_PRICTRL0_LVLPRI2(1) | DMAC_PRICTRL0_RRLVLEN2_Msk | DMAC_PRICTRL0_LVLPRI3(1) | DMAC_PRICTRL0_RRLVLEN3_Msk;

    /***************** Configure DMA channel 0 ********************/

    DMAC_REGS->DMAC_CHID = 0;

    DMAC_REGS->DMAC_CHCTRLB = DMAC_CHCTRLB_TRIGACT(2) | DMAC_CHCTRLB_TRIGSRC(12) | DMAC_CHCTRLB_LVL(0) ;

    descriptor_section[0].DMAC_BTCTRL = DMAC_BTCTRL_BLOCKACT_INT | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID_Msk | DMAC_BTCTRL_SRCINC_Msk ;

    dmacChannelObj[0].inUse = 1;

    DMAC_REGS->DMAC_CHINTENSET = (DMAC_CHINTENSET_TERR_Msk | DMAC_CHINTENSET_TCMPL_Msk);

    /* Enable the DMAC module & Priority Level x Enable */
    DMAC_REGS->DMAC_CTRL = DMAC_CTRL_DMAENABLE_Msk | DMAC_CTRL_LVLEN0_Msk | DMAC_CTRL_LVLEN1_Msk | DMAC_CTRL_LVLEN2_Msk | DMAC_CTRL_LVLEN3_Msk;
}

/*******************************************************************************
    This function schedules a DMA transfer on the specified DMA channel.
********************************************************************************/

bool DMAC_ChannelTransfer( DMAC_CHANNEL channel, const void *srcAddr, const void *destAddr, size_t blockSize )
{
    uint8_t beat_size = 0;
    uint8_t channelId = 0;
    bool returnStatus = false;

    if (dmacChannelObj[channel].busyStatus == false)
    {
        /* Get a pointer to the module hardware instance */
        dmac_descriptor_registers_t *const dmacDescReg = &descriptor_section[channel];

        dmacChannelObj[channel].busyStatus = true;

        /* Set source address */
        if (dmacDescReg->DMAC_BTCTRL & DMAC_BTCTRL_SRCINC_Msk)
        {
            dmacDescReg->DMAC_SRCADDR = (uint32_t) ((intptr_t)srcAddr + blockSize);
        }
        else
        {
            dmacDescReg->DMAC_SRCADDR = (uint32_t) (srcAddr);
        }

        /* Set destination address */
        if (dmacDescReg->DMAC_BTCTRL & DMAC_BTCTRL_DSTINC_Msk)
        {
            dmacDescReg->DMAC_DSTADDR = (uint32_t) ((intptr_t)destAddr + blockSize);
        }
        else
        {
            dmacDescReg->DMAC_DSTADDR = (uint32_t) (destAddr);
        }

        /* Calculate the beat size and then set the BTCNT value */
        beat_size = (dmacDescReg->DMAC_BTCTRL & DMAC_BTCTRL_BEATSIZE_Msk) >> DMAC_BTCTRL_BEATSIZE_Pos;

        /* Set Block Transfer Count */
        dmacDescReg->DMAC_BTCNT = blockSize / (1 << beat_size);

        /* Save channel ID */
        channelId = (uint8_t)DMAC_REGS->DMAC_CHID;

        /* Set the DMA channel */
        DMAC_REGS->DMAC_CHID = channel;

        /* Enable the channel */
        DMAC_REGS->DMAC_CHCTRLA |= DMAC_CHCTRLA_ENABLE_Msk;

        /* Verify if Trigger source is Software Trigger */
        if ((((DMAC_REGS->DMAC_CHCTRLB & DMAC_CHCTRLB_TRIGSRC_Msk) >> DMAC_CHCTRLB_TRIGSRC_Pos) == 0x00)
                                        && ((DMAC_REGS->DMAC_CHCTRLB & DMAC_CHCTRLB_EVIE_Msk) != DMAC_CHCTRLB_EVIE_Msk))
        {
            /* Trigger the DMA transfer */
            DMAC_REGS->DMAC_SWTRIGCTRL |= (1 << channel);
        }

        /* Restore channel ID */
        DMAC_REGS->DMAC_CHID = channelId;

        returnStatus = true;
    }

    return returnStatus;
}

/*******************************************************************************
    This function returns the status of the channel.
********************************************************************************/

bool DMAC_ChannelIsBusy ( DMAC_CHANNEL channel )
{
    return (bool)dmacChannelObj[channel].busyStatus;
}

/*******************************************************************************
    This function disables the specified DMAC channel.
********************************************************************************/

void DMAC_ChannelDisable ( DMAC_CHANNEL channel )
{
    uint8_t channelId = 0;

    /* Save channel ID */
    channelId = (uint8_t)DMAC_REGS->DMAC_CHID;

    /* Set the DMA Channel ID */
    DMAC_REGS->DMAC_CHID = channel;

    /* Disable the DMA channel */
    DMAC_REGS->DMAC_CHCTRLA &= (~DMAC_CHCTRLA_ENABLE_Msk);

    while((DMAC_REGS->DMAC_CHCTRLA & DMAC_CHCTRLA_ENABLE_Msk) != 0);

    dmacChannelObj[channel].busyStatus = false;

    /* Restore channel ID */
    DMAC_REGS->DMAC_CHID = channelId;
}

uint16_t DMAC_ChannelGetTransferredCount( DMAC_CHANNEL channel )
{
    return(descriptor_section[channel].DMAC_BTCNT - _write_back_section[channel].DMAC_BTCNT);
}


/*******************************************************************************
    This function function allows a DMAC PLIB client to set an event handler.
********************************************************************************/

void DMAC_ChannelCallbackRegister( DMAC_CHANNEL channel, const DMAC_CHANNEL_CALLBACK callback, const uintptr_t context )
{
    dmacChannelObj[channel].callback = callback;

    dmacChannelObj[channel].context  = context;
}

/*******************************************************************************
    This function returns the current channel settings for the specified DMAC Channel
********************************************************************************/

DMAC_CHANNEL_CONFIG DMAC_ChannelSettingsGet (DMAC_CHANNEL channel)
{
    /* Get a pointer to the module hardware instance */
    dmac_descriptor_registers_t *const dmacDescReg = &descriptor_section[0];

    return (dmacDescReg[channel].DMAC_BTCTRL);
}

/*******************************************************************************
    This function changes the current settings of the specified DMAC channel.
********************************************************************************/

bool DMAC_ChannelSettingsSet (DMAC_CHANNEL channel, DMAC_CHANNEL_CONFIG setting)
{
    uint8_t channelId = 0;

    /* Get a pointer to the module hardware instance */
    dmac_descriptor_registers_t *const dmacDescReg = &descriptor_section[0];

    /* Save channel ID */
    channelId = (uint8_t)DMAC_REGS->DMAC_CHID;

    /* Set the DMA Channel ID */
    DMAC_REGS->DMAC_CHID = channel;

    /* Disable the DMA channel */
    DMAC_REGS->DMAC_CHCTRLA &= (~DMAC_CHCTRLA_ENABLE_Msk);

    /* Wait for channel to be disabled */
    while((DMAC_REGS->DMAC_CHCTRLA & DMAC_CHCTRLA_ENABLE_Msk) != 0);

    /* Set the new settings */
    dmacDescReg[channel].DMAC_BTCTRL = setting;

    /* Restore channel ID */
    DMAC_REGS->DMAC_CHID = channelId;

    return true;
}

/*******************************************************************************
    This function handles the DMA interrupt events.
*/
void DMAC_InterruptHandler( void )
{
    DMAC_CH_OBJECT  *dmacChObj = NULL;
    uint8_t channel = 0;
    uint8_t channelId = 0;
    volatile uint32_t chanIntFlagStatus = 0;
    DMAC_TRANSFER_EVENT event = DMAC_TRANSFER_EVENT_ERROR;

    /* Get active channel number */
    channel = DMAC_REGS->DMAC_INTPEND & DMAC_INTPEND_ID_Msk;

    dmacChObj = (DMAC_CH_OBJECT *)&dmacChannelObj[channel];

    /* Save channel ID */
    channelId = (uint8_t)DMAC_REGS->DMAC_CHID;

    /* Update the DMAC channel ID */
    DMAC_REGS->DMAC_CHID = channel;

    /* Get the DMAC channel interrupt status */
    chanIntFlagStatus = DMAC_REGS->DMAC_CHINTFLAG;

    /* Verify if DMAC Channel Transfer complete flag is set */
    if (chanIntFlagStatus & DMAC_CHINTENCLR_TCMPL_Msk)
    {
        /* Clear the transfer complete flag */
        DMAC_REGS->DMAC_CHINTFLAG = DMAC_CHINTENCLR_TCMPL_Msk;

        event = DMAC_TRANSFER_EVENT_COMPLETE;

        dmacChObj->busyStatus = false;
    }

    /* Verify if DMAC Channel Error flag is set */
    if (chanIntFlagStatus & DMAC_CHINTENCLR_TERR_Msk)
    {
        /* Clear transfer error flag */
        DMAC_REGS->DMAC_CHINTFLAG = DMAC_CHINTENCLR_TERR_Msk;

        event = DMAC_TRANSFER_EVENT_ERROR;

        dmacChObj->busyStatus = false;
    }

    /* Execute the callback function */
    if (dmacChObj->callback != NULL)
    {
        dmacChObj->callback (event, dmacChObj->context);
    }

    /* Restore channel ID */
    DMAC_REGS->DMAC_CHID = channelId;
}
