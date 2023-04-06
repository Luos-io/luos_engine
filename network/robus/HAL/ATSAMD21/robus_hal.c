/******************************************************************************
 * @file robus_HAL
 * @brief Robus Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family ATSAMD21
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "robus_hal.h"
#include "luos_hal.h"

#include <stdbool.h>
#include "reception.h"
#include "context.h"

// MCU dependencies this HAL is for family Atmel ATSAMD21 you can find

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEFAULT_TIMEOUT 30
#define TIMEOUT_ACK     DEFAULT_TIMEOUT / 4
/*******************************************************************************
 * Variables
 ******************************************************************************/

typedef struct
{
    uint8_t Pin;
    uint8_t Port;
    uint8_t Irq;
} Port_t;

Port_t PTP[NBR_PORT];

volatile uint16_t timoutclockcnt        = 0;
volatile uint16_t data_size_to_transmit = 0;
volatile uint8_t *tx_data               = 0;

#ifndef USE_TX_IT
static dmac_descriptor_registers_t write_back_section __ALIGNED(8);
static dmac_descriptor_registers_t descriptor_section __ALIGNED(8);
#endif

/*******************************************************************************
 * Function
 ******************************************************************************/
static void RobusHAL_CRCInit(void);
static void RobusHAL_TimeoutInit(void);
static void RobusHAL_GPIOInit(void);
static void RobusHAL_RegisterPTP(void);

/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_Init(void)
{
    // IO Initialization
    RobusHAL_GPIOInit();

    // CRC Initialization
    RobusHAL_CRCInit();

    // Com Initialization
    RobusHAL_ComInit(DEFAULTBAUDRATE);
}

/******************************************************************************
 * @brief Luos HAL general loop
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_Loop(void)
{
}
/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void RobusHAL_ComInit(uint32_t Baudrate)
{
    uint32_t baud = 0;
    // initialize clock
    LUOS_COM_CLOCK_ENABLE();

    /* Disable the USART before configurations */
    LUOS_COM->USART_INT.SERCOM_CTRLA &= ~SERCOM_USART_INT_CTRLA_ENABLE_Msk;

    /* Configure Baud Rate */
    baud                            = 65536 - ((uint64_t)65536 * 16 * Baudrate) / MCUFREQ;
    LUOS_COM->USART_INT.SERCOM_BAUD = SERCOM_USART_INT_BAUD_BAUD(baud);

    // Configures USART Clock Mode/ TXPO and RXPO/ Data Order/ Standby Mode/ Sampling rate/ IBON
    LUOS_COM->USART_INT.SERCOM_CTRLA = SERCOM_USART_INT_CTRLA_MODE_USART_INT_CLK | SERCOM_USART_INT_CTRLA_RXPO(COM_RX_POS)
                                       | SERCOM_USART_INT_CTRLA_TXPO(COM_TX_POS) | SERCOM_USART_INT_CTRLA_DORD_Msk
                                       | SERCOM_USART_INT_CTRLA_IBON_Msk | SERCOM_USART_INT_CTRLA_FORM(0x0)
                                       | SERCOM_USART_INT_CTRLA_SAMPR(0);

    // Configures RXEN/ TXEN/ CHSIZE/ Parity/ Stop bits
    LUOS_COM->USART_INT.SERCOM_CTRLB = SERCOM_USART_INT_CTRLB_CHSIZE_8_BIT | SERCOM_USART_INT_CTRLB_SBMODE_1_BIT
                                       | SERCOM_USART_INT_CTRLB_RXEN_Msk | SERCOM_USART_INT_CTRLB_TXEN_Msk
                                       | SERCOM_USART_INT_CTRLB_SFDE_Msk;

    /* Enable the UART after the configurations */
    LUOS_COM->USART_INT.SERCOM_CTRLA |= SERCOM_USART_INT_CTRLA_ENABLE_Msk;

    /* Wait for sync */
    while (LUOS_COM->USART_INT.SERCOM_SYNCBUSY)
        ;

    /* Clean IT */
    LUOS_COM->USART_INT.SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_RESETVALUE;

    /* Enable Receive Complete interrupt */
    LUOS_COM->USART_INT.SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_RXC_Msk;

    NVIC_SetPriority(LUOS_COM_IRQ, 3);
    NVIC_EnableIRQ(LUOS_COM_IRQ);

    // Timeout Initialization
    timoutclockcnt = MCUFREQ / Baudrate;
    RobusHAL_TimeoutInit();

#ifndef USE_TX_IT
    LUOS_DMA_CLOCK_ENABLE();

    LUOS_DMA->DMAC_BASEADDR        = (uint32_t)&descriptor_section;
    LUOS_DMA->DMAC_WRBADDR         = (uint32_t)&write_back_section;
    LUOS_DMA->DMAC_PRICTRL0        = DMAC_PRICTRL0_LVLPRI0(1UL) | DMAC_PRICTRL0_RRLVLEN0_Msk | DMAC_PRICTRL0_LVLPRI1(1UL) | DMAC_PRICTRL0_RRLVLEN1_Msk | DMAC_PRICTRL0_LVLPRI2(1UL) | DMAC_PRICTRL0_RRLVLEN2_Msk | DMAC_PRICTRL0_LVLPRI3(1UL) | DMAC_PRICTRL0_RRLVLEN3_Msk;
    LUOS_DMA->DMAC_CHID            = LUOS_DMA_CHANNEL; // DMA Channel
    LUOS_DMA->DMAC_CHCTRLB         = DMAC_CHCTRLB_TRIGACT(2) | DMAC_CHCTRLB_TRIGSRC(LUOS_DMA_TRIGGER) | DMAC_CHCTRLB_LVL(0);
    descriptor_section.DMAC_BTCTRL = DMAC_BTCTRL_BLOCKACT_INT | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID_Msk | DMAC_BTCTRL_SRCINC_Msk;
    LUOS_DMA->DMAC_CTRL            = DMAC_CTRL_DMAENABLE_Msk | DMAC_CTRL_LVLEN0_Msk | DMAC_CTRL_LVLEN1_Msk | DMAC_CTRL_LVLEN2_Msk | DMAC_CTRL_LVLEN3_Msk;
#endif
}
/******************************************************************************
 * @brief Tx enable/disable relative to com
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_SetTxState(uint8_t Enable)
{
    if (Enable == true)
    {
        PORT_REGS->GROUP[COM_TX_PORT].PORT_PINCFG[COM_TX_PIN] = PORT_PINCFG_RESETVALUE;  // no pin mux / no input /  no pull / low streght
        PORT_REGS->GROUP[COM_TX_PORT].PORT_PINCFG[COM_TX_PIN] |= PORT_PINCFG_PMUXEN_Msk; // mux en
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            PORT_REGS->GROUP[TX_EN_PORT].PORT_OUTSET = (1 << TX_EN_PIN); // enable Tx
        }
    }
    else
    {
        PORT_REGS->GROUP[COM_TX_PORT].PORT_PINCFG[COM_TX_PIN] = PORT_PINCFG_RESETVALUE;  // no pin mux / no input /  no pull / low streght
        PORT_REGS->GROUP[COM_TX_PORT].PORT_PINCFG[COM_TX_PIN] |= PORT_PINCFG_PULLEN_Msk; // Enable Pull
        PORT_REGS->GROUP[COM_TX_PORT].PORT_OUTSET = (1 << COM_TX_PIN);                   // Pull up
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            PORT_REGS->GROUP[TX_EN_PORT].PORT_OUTCLR = (1 << TX_EN_PIN); // disable Tx
        }
#ifdef USE_TX_IT
        // Stop current transmit operation
        data_size_to_transmit = 0;
        // Disable Transmission empty buffer interrupt
        LUOS_COM->USART_INT.SERCOM_INTENCLR = SERCOM_USART_INT_INTENCLR_DRE_Msk; // disable IT
#else
        LUOS_DMA->DMAC_CHCTRLA &= ~DMAC_CHCTRLA_ENABLE_Msk;
#endif
        // Disable Transmission complete interrupt
        LUOS_COM->USART_INT.SERCOM_INTENCLR = SERCOM_USART_INT_INTENCLR_TXC_Msk; // disable IT
        LUOS_COM->USART_INT.SERCOM_INTFLAG  = SERCOM_USART_INT_INTFLAG_RXS_Msk;
    }
}

/******************************************************************************
 * @brief Rx enable/disable relative to com
 * @param
 * @return
 ******************************************************************************/
_CRITICAL void RobusHAL_SetRxState(uint8_t Enable)
{
    if (Enable == true)
    {
        while ((LUOS_COM->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXC_Msk) == SERCOM_USART_INT_INTFLAG_RXC_Msk)
        {
            LUOS_COM->USART_INT.SERCOM_DATA; // clear data buffer
        }
        LUOS_COM->USART_INT.SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_RXC_Msk;
    }
    else
    {
        LUOS_COM->USART_INT.SERCOM_INTENCLR = SERCOM_USART_INT_INTENCLR_RXC_Msk;
    }
}
/******************************************************************************
 * @brief Process data receive
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void LUOS_COM_IRQHANDLER()
{
    // Reset timeout to it's default value
    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);

    // reception management
    if (((LUOS_COM->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXC_Msk) == SERCOM_USART_INT_INTFLAG_RXC_Msk) && ((LUOS_COM->USART_INT.SERCOM_INTENSET & SERCOM_USART_INT_INTENSET_RXC_Msk) == SERCOM_USART_INT_INTENSET_RXC_Msk))
    {
        // clean start bit detection
        uint8_t data = LUOS_COM->USART_INT.SERCOM_DATA;
        Recep_data(&data);
        if (data_size_to_transmit == 0)
        {
            LUOS_COM->USART_INT.SERCOM_STATUS  = SERCOM_USART_INT_STATUS_PERR_Msk | SERCOM_USART_INT_STATUS_FERR_Msk | SERCOM_USART_INT_STATUS_BUFOVF_Msk;
            LUOS_COM->USART_INT.SERCOM_INTFLAG = SERCOM_USART_INT_INTFLAG_RXS_Msk;
            return;
        }
    }
    else if ((LUOS_COM->USART_INT.SERCOM_STATUS & SERCOM_USART_INT_STATUS_FERR_Msk) == SERCOM_USART_INT_STATUS_FERR_Msk)
    {
        ctx.rx.status.rx_framing_error = true;
    }

    // Transmission management
    if (((LUOS_COM->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_TXC_Msk) == SERCOM_USART_INT_INTFLAG_TXC_Msk) && ((LUOS_COM->USART_INT.SERCOM_INTENSET & SERCOM_USART_INT_INTENSET_TXC_Msk) == SERCOM_USART_INT_INTENSET_TXC_Msk))
    {
        // Transmission complete
        data_size_to_transmit = 0;
        // Switch to reception mode
        RobusHAL_SetTxState(false);
        RobusHAL_SetRxState(true);
        // Disable transmission complete IRQ
        LUOS_COM->USART_INT.SERCOM_INTFLAG  = SERCOM_USART_INT_INTFLAG_TXC_Msk;  // clear flag
        LUOS_COM->USART_INT.SERCOM_INTENCLR = SERCOM_USART_INT_INTENCLR_TXC_Msk; // disable IT
    }
#ifdef USE_TX_IT
    else if (((LUOS_COM->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_DRE_Msk) == SERCOM_USART_INT_INTFLAG_DRE_Msk) && ((LUOS_COM->USART_INT.SERCOM_INTENSET & SERCOM_USART_INT_INTENSET_DRE_Msk) == SERCOM_USART_INT_INTENSET_DRE_Msk))
    {
        // Transmit buffer empty (this is a software DMA)
        data_size_to_transmit--;
        LUOS_COM->USART_INT.SERCOM_DATA = *(tx_data++);
        if (data_size_to_transmit == 0)
        {
            // Transmission complete, stop loading data and watch for the end of transmission
            // Disable Transmission empty buffer interrupt
            LUOS_COM->USART_INT.SERCOM_INTFLAG  = SERCOM_USART_INT_INTFLAG_DRE_Msk;  // clear flag
            LUOS_COM->USART_INT.SERCOM_INTENCLR = SERCOM_USART_INT_INTENCLR_DRE_Msk; // disable IT
            // Enable Transmission complete interrupt
            LUOS_COM->USART_INT.SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_TXC_Msk; // disable IT
        }
    }
#endif
    LUOS_COM->USART_INT.SERCOM_STATUS  = SERCOM_USART_INT_STATUS_PERR_Msk | SERCOM_USART_INT_STATUS_FERR_Msk | SERCOM_USART_INT_STATUS_BUFOVF_Msk;
    LUOS_COM->USART_INT.SERCOM_INTFLAG = SERCOM_USART_INT_INTFLAG_RXS_Msk;
}
/******************************************************************************
 * @brief Process data transmit
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_ComTransmit(uint8_t *data, uint16_t size)
{
    while ((LUOS_COM->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_DRE_Msk) != SERCOM_USART_INT_INTFLAG_DRE_Msk)
        ;
    // Disable RX detec pin if needed

    // Reduce size by one because we send one directly
    data_size_to_transmit = size - 1;
    if (size > 1)
    {
        // Start the data buffer transmission
        // **** NO DMA
        // Copy the data pointer globally alowing to keep it and run the transmission.
        tx_data = data;
#ifdef USE_TX_IT
        // Send the first byte
        LUOS_COM->USART_INT.SERCOM_DATA = *(tx_data++);
        // Enable Transmission empty buffer interrupt to transmit next datas
        LUOS_COM->USART_INT.SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_DRE_Msk; // enable IT
        // Disable Transmission complete interrupt
        LUOS_COM->USART_INT.SERCOM_INTENCLR = SERCOM_USART_INT_INTENCLR_TXC_Msk; // disable IT
#else
        data_size_to_transmit           = 0; // to not check IT TC during collision
        descriptor_section.DMAC_SRCADDR = (uint32_t)(data + size);
        descriptor_section.DMAC_DSTADDR = (uint32_t)&LUOS_COM->USART_INT.SERCOM_DATA;
        descriptor_section.DMAC_BTCNT   = size;
        // Enable TX
        RobusHAL_SetTxState(true);
        LUOS_DMA->DMAC_CHCTRLA |= DMAC_CHCTRLA_ENABLE_Msk;
        LUOS_COM->USART_INT.SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_TXC_Msk; // enable IT
#endif
    }
    else
    {
        // Wait before send ack
        data_size_to_transmit = 1;
        // This is a patch du to difference MCU frequency
        while (LUOS_TIMER->COUNT16.TC_COUNT < (0xFFFF - (timoutclockcnt * (DEFAULT_TIMEOUT - TIMEOUT_ACK))))
            ;
        // Enable TX
        RobusHAL_SetTxState(true);
        // Transmit the only byte we have
        LUOS_COM->USART_INT.SERCOM_DATA = *data;
        // Enable Transmission complete interrupt because we only have one.
        LUOS_COM->USART_INT.SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_TXC_Msk; // enable IT
    }
    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
}
/******************************************************************************
 * @brief set state of Txlock detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
_CRITICAL void RobusHAL_SetRxDetecPin(uint8_t Enable)
{
    if (TX_LOCK_DETECT_IRQ != DISABLE)
    {
        EIC_REGS->EIC_INTFLAG = (1 << TX_LOCK_DETECT_IRQ); // clear IT flag
        if (Enable == true)
        {
            EIC_REGS->EIC_INTENSET = (1 << TX_LOCK_DETECT_IRQ); // enable IT
        }
        else
        {
            EIC_REGS->EIC_INTENCLR = (1 << TX_LOCK_DETECT_IRQ);
        }
    }
}
/******************************************************************************
 * @brief get Lock Com transmit status this is the HW that can generate lock TX
 * @param None
 * @return Lock status
 ******************************************************************************/
_CRITICAL uint8_t RobusHAL_GetTxLockState(void)
{
    uint8_t result = false;
    if ((LUOS_COM->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXS_Msk) == SERCOM_USART_INT_INTFLAG_RXS_Msk)
    {
        LUOS_COM->USART_INT.SERCOM_INTFLAG = SERCOM_USART_INT_INTFLAG_RXS_Msk;
        RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
        result = true;
    }
    else
    {
        if ((TX_LOCK_DETECT_PIN != DISABLE) && (TX_LOCK_DETECT_PORT != DISABLE))
        {
            if (((PORT_REGS->GROUP[TX_LOCK_DETECT_PORT].PORT_IN >> TX_LOCK_DETECT_PIN) & 0x01) == 0x00)
            {
                result = true;
            }
            if (TX_LOCK_DETECT_IRQ == DISABLE)
            {
                if (result == true)
                {
                    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
                }
            }
        }
    }
    return result;
}
/******************************************************************************
 * @brief Luos Timeout initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_TimeoutInit(void)
{
    // initialize clock
    LUOS_TIMER_CLOCK_ENABLE();

    LUOS_TIMER->COUNT16.TC_CTRLA = TC_CTRLA_RESETVALUE;
    while ((LUOS_TIMER->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk))
        ;
    /* Configure counter mode & prescaler */
    LUOS_TIMER->COUNT16.TC_CTRLA    = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_PRESCALER_DIV1 | TC_CTRLA_WAVEGEN_MPWM;
    LUOS_TIMER->COUNT16.TC_CTRLBSET = TC_CTRLBSET_ONESHOT_Msk;

    LUOS_TIMER->COUNT16.TC_COUNT = 0xFFFF - (timoutclockcnt * DEFAULT_TIMEOUT);
    /* Clear all interrupt flags */
    LUOS_TIMER->COUNT16.TC_INTENSET = TC_INTENSET_RESETVALUE;
    LUOS_TIMER->COUNT16.TC_INTENSET = TC_INTENSET_OVF_Msk;

    NVIC_SetPriority(LUOS_TIMER_IRQ, 3);
    NVIC_EnableIRQ(LUOS_TIMER_IRQ);
}
/******************************************************************************
 * @brief Luos Timeout for Rx communication
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_ResetTimeout(uint16_t nbrbit)
{
    NVIC_ClearPendingIRQ(LUOS_TIMER_IRQ);                // clear IT pending
    LUOS_TIMER->COUNT16.TC_INTFLAG = TC_INTFLAG_OVF_Msk; // clear flag
    LUOS_TIMER->COUNT16.TC_CTRLA &= ~TC_CTRLA_ENABLE_Msk;
    if (nbrbit != 0)
    {
        LUOS_TIMER->COUNT16.TC_COUNT = 0xFFFF - (timoutclockcnt * nbrbit);
        LUOS_TIMER->COUNT16.TC_CTRLA |= TC_CTRLA_ENABLE_Msk;
    }
}
/******************************************************************************
 * @brief Luos Timeout for Rx communication
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void LUOS_TIMER_IRQHANDLER()
{
    if ((LUOS_TIMER->COUNT16.TC_INTFLAG & TC_INTFLAG_OVF_Msk) == TC_INTFLAG_OVF_Msk)
    {
        LUOS_TIMER->COUNT16.TC_INTFLAG = TC_INTFLAG_OVF_Msk; // clear
        if ((ctx.tx.lock == true) && (RobusHAL_GetTxLockState() == false))
        {
            RobusHAL_SetTxState(false);
            RobusHAL_SetRxState(true);
            Recep_Timeout();
        }
    }
}

/******************************************************************************
 * @brief Initialisation GPIO
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_GPIOInit(void)
{
    uint32_t Position = 0;
    uint32_t Config   = 0;

    // Activate Clock for PIN choosen in RobusHAL
    PORT_CLOCK_ENABLE();

    if ((RX_EN_PIN != DISABLE) || (RX_EN_PORT != DISABLE))
    {
        /*Configure GPIO pins : RxEN_Pin */
        PORT_REGS->GROUP[RX_EN_PORT].PORT_PINCFG[RX_EN_PIN] = PORT_PINCFG_RESETVALUE;  // no pin mux / no input /  no pull / low streght
        PORT_REGS->GROUP[RX_EN_PORT].PORT_PINCFG[RX_EN_PIN] |= PORT_PINCFG_DRVSTR_Msk; // hight streght drive
        PORT_REGS->GROUP[RX_EN_PORT].PORT_DIRSET = (1 << RX_EN_PIN);                   // Output
        PORT_REGS->GROUP[RX_EN_PORT].PORT_OUTCLR = (1 << RX_EN_PIN);                   // disable Tx set output low
    }

    if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
    {
        /*Configure GPIO pins : TxEN_Pin */
        PORT_REGS->GROUP[TX_EN_PORT].PORT_PINCFG[TX_EN_PIN] = PORT_PINCFG_RESETVALUE;  // no pin mux / no input /  no pull / low streght
        PORT_REGS->GROUP[TX_EN_PORT].PORT_PINCFG[TX_EN_PIN] |= PORT_PINCFG_DRVSTR_Msk; // hight streght drive
        PORT_REGS->GROUP[TX_EN_PORT].PORT_DIRSET = (1 << TX_EN_PIN);                   // Output
        PORT_REGS->GROUP[TX_EN_PORT].PORT_OUTCLR = (1 << TX_EN_PIN);                   // disable Tx set output low
    }

    /*Configure GPIO pins : TX_LOCK_DETECT_Pin */
    if ((TX_LOCK_DETECT_PIN != DISABLE) || (TX_LOCK_DETECT_PORT != DISABLE))
    {
        PORT_REGS->GROUP[TX_LOCK_DETECT_PORT].PORT_PINCFG[TX_LOCK_DETECT_PIN] = PORT_PINCFG_RESETVALUE; // no pin mux / no input /  no pull / low streght
        PORT_REGS->GROUP[TX_LOCK_DETECT_PORT].PORT_PINCFG[TX_LOCK_DETECT_PIN] |= PORT_PINCFG_INEN_Msk;  // enable input
        PORT_REGS->GROUP[TX_LOCK_DETECT_PORT].PORT_OUTSET = (1 << TX_LOCK_DETECT_PIN);                  // pull up
        if (TX_LOCK_DETECT_IRQ != DISABLE)
        {
            PORT_REGS->GROUP[TX_LOCK_DETECT_PORT].PORT_PMUX[TX_LOCK_DETECT_PIN >> 1] |= (0 << (4 * (TX_LOCK_DETECT_PIN % 2)));
            if (TX_LOCK_DETECT_IRQ < 8)
            {
                Config   = 0;
                Position = TX_LOCK_DETECT_IRQ << 2;
            }
            else
            {
                Config   = 1;
                Position = (TX_LOCK_DETECT_IRQ - 8) << 2;
            }
        }
        EIC_REGS->EIC_CONFIG[Config] &= ~(EIC_CONFIG_SENSE0_Msk << Position);   // reset sense mode
        EIC_REGS->EIC_CONFIG[Config] |= EIC_CONFIG_SENSE0_FALL_Val << Position; // Falling EDGE
        EIC_REGS->EIC_INTFLAG  = (1 << TX_LOCK_DETECT_IRQ);                     // clear IT flag
        EIC_REGS->EIC_INTENCLR = (1 << TX_LOCK_DETECT_IRQ);
    }

    /*Configure GPIO pin : TxPin */
    PORT_REGS->GROUP[COM_TX_PORT].PORT_PINCFG[COM_TX_PIN] = PORT_PINCFG_RESETVALUE;                    // no pin mux / no input /  no pull / low streght
    PORT_REGS->GROUP[COM_TX_PORT].PORT_PINCFG[COM_TX_PIN] |= PORT_PINCFG_PULLEN_Msk;                   // Enable Pull
    PORT_REGS->GROUP[COM_TX_PORT].PORT_OUTSET = (1 << COM_TX_PIN);                                     // Pull up
    PORT_REGS->GROUP[COM_TX_PORT].PORT_PMUX[COM_TX_PIN >> 1] |= (COM_TX_AF << (4 * (COM_TX_PIN % 2))); // mux to sercom

    /*Configure GPIO pin : RxPin */
    PORT_REGS->GROUP[COM_RX_PORT].PORT_PINCFG[COM_RX_PIN] = PORT_PINCFG_RESETVALUE;                    // no pin mux / no input /  no pull / low streght
    PORT_REGS->GROUP[COM_RX_PORT].PORT_PINCFG[COM_RX_PIN] |= PORT_PINCFG_PMUXEN_Msk;                   // mux en
    PORT_REGS->GROUP[COM_RX_PORT].PORT_PMUX[COM_RX_PIN >> 1] |= (COM_TX_AF << (4 * (COM_RX_PIN % 2))); // mux to sercom

    // configure PTP
    RobusHAL_RegisterPTP();
    for (uint8_t i = 0; i < NBR_PORT; i++) /*Configure GPIO pins : PTP_Pin */
    {
        // Setup PTP lines //Mux all PTP to EIC
        PORT_REGS->GROUP[PTP[NBR_PORT].Port].PORT_PMUX[PTP[NBR_PORT].Pin >> 1] |= (0 << (4 * (PTP[NBR_PORT].Pin % 2)));
        RobusHAL_SetPTPDefaultState(i);
    }

    NVIC_SetPriority(EIC_IRQn, 3);
    NVIC_EnableIRQ(EIC_IRQn);

    // Enable EIC interrupt
    EIC_REGS->EIC_CTRL |= EIC_CTRL_ENABLE_Msk;
}
/******************************************************************************
 * @brief Register PTP
 * @param void
 * @return None
 ******************************************************************************/
static void RobusHAL_RegisterPTP(void)
{
#if (NBR_PORT >= 1)
    PTP[0].Pin  = PTPA_PIN;
    PTP[0].Port = PTPA_PORT;
    PTP[0].Irq  = PTPA_IRQ;
#endif

#if (NBR_PORT >= 2)
    PTP[1].Pin  = PTPB_PIN;
    PTP[1].Port = PTPB_PORT;
    PTP[1].Irq  = PTPB_IRQ;
#endif

#if (NBR_PORT >= 3)
    PTP[2].Pin  = PTPC_PIN;
    PTP[2].Port = PTPC_PORT;
    PTP[2].Irq  = PTPC_PORT;
#endif

#if (NBR_PORT >= 4)
    PTP[3].Pin  = PTPD_PIN;
    PTP[3].Port = PTPD_PORT;
    PTP[3].Irq  = PTPD_PORT;
#endif
}
/******************************************************************************
 * @brief callback for GPIO IT
 * @param GPIO IT line
 * @return None
 ******************************************************************************/
_CRITICAL void PINOUT_IRQHANDLER()
{
    uint32_t FlagIT = 0;
    ////Process for Tx Lock Detec
    if (((EIC_REGS->EIC_INTFLAG & (1 << TX_LOCK_DETECT_IRQ))) && (TX_LOCK_DETECT_IRQ != DISABLE))
    {
        ctx.tx.lock = true;
        RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
        EIC_REGS->EIC_INTFLAG  = (uint32_t)(1 << TX_LOCK_DETECT_IRQ);
        EIC_REGS->EIC_INTENCLR = (1 << TX_LOCK_DETECT_IRQ);
    }
    else
    {
        for (uint8_t i = 0; i < NBR_PORT; i++)
        {
            FlagIT = (EIC_REGS->EIC_INTFLAG & (1 << PTP[i].Irq));
            if (FlagIT)
            {
                EIC_REGS->EIC_INTFLAG = (uint32_t)(1 << PTP[i].Irq);
                PortMng_PtpHandler(i);
                break;
            }
        }
    }
}
/******************************************************************************
 * @brief Set PTP for Detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_SetPTPDefaultState(uint8_t PTPNbr)
{
    uint32_t Position = 0;
    uint32_t Config   = 0;

    // Pull Down / IT mode / Rising Edge
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_PINCFG[PTP[PTPNbr].Pin] = PORT_PINCFG_RESETVALUE;  // no pin mux / no input /  no pull / low streght
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_PINCFG[PTP[PTPNbr].Pin] |= PORT_PINCFG_PMUXEN_Msk; // mux en
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_PINCFG[PTP[PTPNbr].Pin] |= PORT_PINCFG_PULLEN_Msk; // pull en
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_OUTCLR = (1 << PTP[PTPNbr].Pin);                   // pull down
    if (PTP[PTPNbr].Irq < 8)
    {
        Config   = 0;
        Position = PTP[PTPNbr].Irq << 2;
    }
    else
    {
        Config   = 1;
        Position = (PTP[PTPNbr].Irq - 8) << 2;
    }
    EIC_REGS->EIC_CONFIG[Config] &= ~(EIC_CONFIG_SENSE0_Msk << Position);   // reset sense mode
    EIC_REGS->EIC_CONFIG[Config] |= EIC_CONFIG_SENSE0_RISE_Val << Position; // Rising EDGE
    EIC_REGS->EIC_INTFLAG  = (1 << PTP[PTPNbr].Irq);                        // clear IT flag
    EIC_REGS->EIC_INTENSET = (1 << PTP[PTPNbr].Irq);                        // enable IT
}
/******************************************************************************
 * @brief Set PTP for reverse detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_SetPTPReverseState(uint8_t PTPNbr)
{
    uint32_t Position = 0;
    uint32_t Config   = 0;
    // Pull Down / IT mode / Falling Edge
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_PINCFG[PTP[PTPNbr].Pin] = PORT_PINCFG_RESETVALUE;  // no pin mux / no input /  no pull / low streght
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_PINCFG[PTP[PTPNbr].Pin] |= PORT_PINCFG_PMUXEN_Msk; // mux en
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_PINCFG[PTP[PTPNbr].Pin] |= PORT_PINCFG_PULLEN_Msk; // pull en
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_OUTCLR = (1 << PTP[PTPNbr].Pin);                   // pull down
    if (PTP[PTPNbr].Irq < 8)
    {
        Config   = 0;
        Position = PTP[PTPNbr].Irq << 2;
    }
    else
    {
        Config   = 1;
        Position = (PTP[PTPNbr].Irq - 8) << 2;
    }
    EIC_REGS->EIC_CONFIG[Config] &= ~(EIC_CONFIG_SENSE0_Msk << Position);   // reset sense mode
    EIC_REGS->EIC_CONFIG[Config] |= EIC_CONFIG_SENSE0_FALL_Val << Position; // Falling EDGE
    EIC_REGS->EIC_INTFLAG  = (1 << PTP[PTPNbr].Irq);                        // clear IT flag
    EIC_REGS->EIC_INTENSET = (1 << PTP[PTPNbr].Irq);                        // enable IT
}
/******************************************************************************
 * @brief Set PTP line
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_PushPTP(uint8_t PTPNbr)
{
    // Pull Down / Output mode
    EIC_REGS->EIC_INTENCLR                                          = (1 << PTP[PTPNbr].Irq); // disable IT
    EIC_REGS->EIC_INTFLAG                                           = (1 << PTP[PTPNbr].Irq); // clear IT flag
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_PINCFG[PTP[PTPNbr].Pin] = PORT_PINCFG_RESETVALUE; // no pin mux / no input /  no pull / low streght
    // PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_PINCFG[PTP[PTPNbr].Pin] |= PORT_PINCFG_PULLEN_Msk; //pull en
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_DIRSET = (1 << PTP[PTPNbr].Pin); // Output
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_OUTSET = (1 << PTP[PTPNbr].Pin); // pull down
}
/******************************************************************************
 * @brief Get PTP line
 * @param PTP branch
 * @return Line state
 ******************************************************************************/
uint8_t RobusHAL_GetPTPState(uint8_t PTPNbr)
{
    // Pull Down / Input mode
    EIC_REGS->EIC_INTENCLR                                          = (1 << PTP[PTPNbr].Irq);  // disable IT
    EIC_REGS->EIC_INTFLAG                                           = (1 << PTP[PTPNbr].Irq);  // clear IT flag
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_PINCFG[PTP[PTPNbr].Pin] = PORT_PINCFG_RESETVALUE;  // no pin mux / no input /  no pull / low streght
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_PINCFG[PTP[PTPNbr].Pin] |= PORT_PINCFG_INEN_Msk;   // input
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_PINCFG[PTP[PTPNbr].Pin] |= PORT_PINCFG_PULLEN_Msk; // pull en
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_DIRCLR = (1 << PTP[PTPNbr].Pin);                   // Output
    PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_OUTCLR = (1 << PTP[PTPNbr].Pin);                   // pull down
    return (((PORT_REGS->GROUP[PTP[PTPNbr].Port].PORT_IN >> PTP[PTPNbr].Pin)) & 0x01);
}
/******************************************************************************
 * @brief Initialize CRC Process
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_CRCInit(void)
{
}
/******************************************************************************
 * @brief Compute CRC
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
{
    uint16_t dbyte = *data;
    *(uint16_t *)crc ^= dbyte << 8;
    for (uint8_t j = 0; j < 8; ++j)
    {
        uint16_t mix     = *(uint16_t *)crc & 0x8000;
        *(uint16_t *)crc = (*(uint16_t *)crc << 1);
        if (mix)
            *(uint16_t *)crc = *(uint16_t *)crc ^ 0x0007;
    }
}
