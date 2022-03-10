/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family ATSAMD21
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos_hal.h"

#include <stdbool.h>
#include <string.h>
#include "reception.h"
#include "context.h"

// MCU dependencies this HAL is for family Atmel ATSAMD21 you can find

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEFAULT_TIMEOUT 20
#define TIMEOUT_ACK     DEFAULT_TIMEOUT / 4
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t tick;

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
static DmacDescriptor write_back_section __attribute__((aligned(8)));
static DmacDescriptor descriptor_section __attribute__((aligned(8)));
#endif

// timestamp variable
static ll_timestamp_t ll_timestamp;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void LuosHAL_SystickInit(void);
static void LuosHAL_FlashInit(void);
static void LuosHAL_CRCInit(void);
static void LuosHAL_TimeoutInit(void);
static void LuosHAL_GPIOInit(void);
static void LuosHAL_FlashEraseLuosMemoryInfo(void);
static void LuosHAL_RegisterPTP(void);

/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_Init(void)
{
    // Systick Initialization
    LuosHAL_SystickInit();

    // IO Initialization
    LuosHAL_GPIOInit();

    // Flash Initialization
    LuosHAL_FlashInit();

    // CRC Initialization
    LuosHAL_CRCInit();

    // Com Initialization
    LuosHAL_ComInit(DEFAULTBAUDRATE);
}
/******************************************************************************
 * @brief Luos HAL general disable IRQ
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_SetIrqState(uint8_t Enable)
{
    if (Enable == true)
    {
        __enable_irq();
    }
    else
    {
        __disable_irq();
    }
}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms initialize
 * @param None
 * @return tick Counter
 ******************************************************************************/
static void LuosHAL_SystickInit(void)
{
    SysTick->CTRL = 0;
    SysTick->VAL  = 0;
    SysTick->LOAD = 0xbb80 - 1;
    SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms
 * @param None
 * @return tick Counter
 ******************************************************************************/
uint32_t LuosHAL_GetSystick(void)
{
    return millis();
}
/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void LuosHAL_ComInit(uint32_t Baudrate)
{
    uint32_t baud = 0;
    // initialize clock
    LUOS_COM_CLOCK_ENABLE();

    /* Disable the USART before configurations */
    LUOS_COM->USART.CTRLA.reg &= ~SERCOM_USART_CTRLA_ENABLE;

    /* Configure Baud Rate */
    baud                     = 65536 - ((uint64_t)65536 * 16 * Baudrate) / MCUFREQ;
    LUOS_COM->USART.BAUD.reg = SERCOM_USART_BAUD_BAUD(baud);

    // Configures USART Clock Mode/ TXPO and RXPO/ Data Order/ Standby Mode/ Sampling rate/ IBON
    LUOS_COM->USART.CTRLA.reg = SERCOM_USART_CTRLA_MODE_USART_INT_CLK | SERCOM_USART_CTRLA_RXPO(COM_RX_POS)
                                | SERCOM_USART_CTRLA_TXPO(COM_TX_POS) | SERCOM_USART_CTRLA_DORD
                                | SERCOM_USART_CTRLA_IBON | SERCOM_USART_CTRLA_FORM(0x0)
                                | SERCOM_USART_CTRLA_SAMPR(0);

    // Configures RXEN/ TXEN/ CHSIZE/ Parity/ Stop bits
    LUOS_COM->USART.CTRLB.reg = SERCOM_USART_CTRLB_CHSIZE(0) | SERCOM_USART_CTRLB_SBMODE
                                | SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN
                                | SERCOM_USART_CTRLB_SFDE;

    /* Enable the UART after the configurations */
    LUOS_COM->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;

    /* Wait for sync */
    while (LUOS_COM->USART.SYNCBUSY.bit.ENABLE)
        ;

    /* Clean IT */
    LUOS_COM->USART.INTENSET.reg = SERCOM_USART_INTENSET_RESETVALUE;

    /* Enable Receive Complete interrupt */
    LUOS_COM->USART.INTENSET.reg = SERCOM_USART_INTENSET_RXC;

    NVIC_SetPriority(LUOS_COM_IRQ, 3);
    NVIC_EnableIRQ(LUOS_COM_IRQ);

    // Timeout Initialization
    timoutclockcnt = MCUFREQ / Baudrate;
    LuosHAL_TimeoutInit();

#ifndef USE_TX_IT
    LUOS_DMA_CLOCK_ENABLE();

    LUOS_DMA->BASEADDR.reg        = (uint32_t)&descriptor_section;
    LUOS_DMA->WRBADDR.reg         = (uint32_t)&write_back_section;
    LUOS_DMA->PRICTRL0.reg        = DMAC_PRICTRL0_LVLPRI0(1UL) | DMAC_PRICTRL0_RRLVLEN0 | DMAC_PRICTRL0_LVLPRI1(1UL) | DMAC_PRICTRL0_RRLVLEN1 | DMAC_PRICTRL0_LVLPRI2(1UL) | DMAC_PRICTRL0_RRLVLEN2 | DMAC_PRICTRL0_LVLPRI3(1UL) | DMAC_PRICTRL0_RRLVLEN3;
    LUOS_DMA->CHID.reg            = LUOS_DMA_CHANNEL; // DMA Channel
    LUOS_DMA->CHCTRLB.reg         = DMAC_CHCTRLB_TRIGACT(2) | DMAC_CHCTRLB_TRIGSRC(LUOS_DMA_TRIGGER) | DMAC_CHCTRLB_LVL(0);
    descriptor_section.BTCTRL.reg = DMAC_BTCTRL_BLOCKACT_INT | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID | DMAC_BTCTRL_SRCINC;
    LUOS_DMA->CTRL.reg            = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN0 | DMAC_CTRL_LVLEN1 | DMAC_CTRL_LVLEN2 | DMAC_CTRL_LVLEN3;
#endif
}
/******************************************************************************
 * @brief Tx enable/disable relative to com
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_SetTxState(uint8_t Enable)
{
    if (Enable == true)
    {
        PORT->Group[COM_TX_PORT].PINCFG[COM_TX_PIN].reg = PORT_PINCFG_RESETVALUE; // no pin mux / no input /  no pull / low streght
        PORT->Group[COM_TX_PORT].PINCFG[COM_TX_PIN].reg |= PORT_PINCFG_PMUXEN;    // mux en
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            PORT->Group[TX_EN_PORT].OUTSET.reg = (1 << TX_EN_PIN); // enable Tx
        }
    }
    else
    {
        PORT->Group[COM_TX_PORT].PINCFG[COM_TX_PIN].reg = PORT_PINCFG_RESETVALUE; // no pin mux / no input /  no pull / low streght
        PORT->Group[COM_TX_PORT].PINCFG[COM_TX_PIN].reg |= PORT_PINCFG_PULLEN;    // Enable Pull
        PORT->Group[COM_TX_PORT].OUTSET.reg = (1 << COM_TX_PIN);                  // Pull up
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            PORT->Group[TX_EN_PORT].OUTCLR.reg = (1 << TX_EN_PIN); // disable Tx
        }
#ifdef USE_TX_IT
        // Stop current transmit operation
        data_size_to_transmit = 0;
        // Disable Transmission empty buffer interrupt
        LUOS_COM->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE; // disable IT
#else
        LUOS_DMA->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
#endif
        // Disable Transmission complete interrupt
        LUOS_COM->USART.INTENCLR.reg    = SERCOM_USART_INTENCLR_TXC; // disable IT
        LUOS_COM->USART.INTFLAG.bit.RXS = 1;                         // clear flag rx start
    }
}
/******************************************************************************
 * @brief Rx enable/disable relative to com
 * @param
 * @return
 ******************************************************************************/
void LuosHAL_SetRxState(uint8_t Enable)
{
    if (Enable == true)
    {
        while ((LUOS_COM->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_RXC) == SERCOM_USART_INTFLAG_RXC)
        {
            LUOS_COM->USART.DATA.reg;
        }
        LUOS_COM->USART.INTENSET.reg = SERCOM_USART_INTENSET_RXC;
    }
    else
    {
        LUOS_COM->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_RXC;
    }
}
/******************************************************************************
 * @brief Process data receive
 * @param None
 * @return None
 ******************************************************************************/
void LUOS_COM_IRQHANDLER()
{
    // Reset timeout to it's default value
    LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);

    // reception management
    if (((LUOS_COM->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_RXC) == SERCOM_USART_INTFLAG_RXC) && ((LUOS_COM->USART.INTENSET.reg & SERCOM_USART_INTENSET_RXC) == SERCOM_USART_INTENSET_RXC))
    {
        // clean start bit detection
        uint8_t data = LUOS_COM->USART.DATA.reg;
        ctx.rx.callback(&data);
        if (data_size_to_transmit == 0)
        {
            LUOS_COM->USART.STATUS.reg      = SERCOM_USART_STATUS_PERR | SERCOM_USART_STATUS_FERR | SERCOM_USART_STATUS_BUFOVF;
            LUOS_COM->USART.INTFLAG.bit.RXS = 1; // clear flag rx start
            return;
        }
    }
    else if ((LUOS_COM->USART.STATUS.reg & SERCOM_USART_STATUS_FERR) == SERCOM_USART_STATUS_FERR) // check error on ligne
    {
        ctx.rx.status.rx_framing_error = true;
    }

    // Transmission management
    if (((LUOS_COM->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_TXC) == SERCOM_USART_INTFLAG_TXC) && ((LUOS_COM->USART.INTENSET.reg & SERCOM_USART_INTENSET_TXC) == SERCOM_USART_INTENSET_TXC))
    {
        // Transmission complete
        // Switch to reception mode
        LuosHAL_SetTxState(false);
        LuosHAL_SetRxState(true);
        // Disable transmission complete IRQ
        LUOS_COM->USART.INTFLAG.reg  = SERCOM_USART_INTFLAG_TXC;  // clear flag
        LUOS_COM->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_TXC; // disable IT
    }
#ifdef USE_TX_IT
    else if (((LUOS_COM->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_DRE) == SERCOM_USART_INTFLAG_DRE) && ((LUOS_COM->USART.INTENSET.reg & SERCOM_USART_INTENSET_DRE) == SERCOM_USART_INTENSET_DRE))
    {
        // Transmit buffer empty (this is a software DMA)
        data_size_to_transmit--;
        LUOS_COM->USART.DATA.reg = *(tx_data++);
        if (data_size_to_transmit == 0)
        {
            // Transmission complete, stop loading data and watch for the end of transmission
            // Disable Transmission empty buffer interrupt
            LUOS_COM->USART.INTFLAG.reg  = SERCOM_USART_INTFLAG_DRE;  // clear flag
            LUOS_COM->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE; // disable IT
            // Enable Transmission complete interrupt
            LUOS_COM->USART.INTENSET.reg = SERCOM_USART_INTENSET_TXC; // disable IT
        }
    }
#endif
    LUOS_COM->USART.STATUS.reg      = SERCOM_USART_STATUS_PERR | SERCOM_USART_STATUS_FERR | SERCOM_USART_STATUS_BUFOVF;
    LUOS_COM->USART.INTFLAG.bit.RXS = 1; // clear flag rx start
}
/******************************************************************************
 * @brief Process data transmit
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_ComTransmit(uint8_t *data, uint16_t size)
{
    while ((LUOS_COM->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_DRE) != SERCOM_USART_INTFLAG_DRE)
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
        LUOS_COM->USART.DATA.reg = *(tx_data++);
        // Enable Transmission empty buffer interrupt to transmit next datas
        LUOS_COM->USART.INTENSET.reg = SERCOM_USART_INTENSET_DRE; // enable IT
        // Disable Transmission complete interrupt
        LUOS_COM->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_TXC; // disable IT
#else
        data_size_to_transmit          = 0; // to not check IT TC during collision
        descriptor_section.SRCADDR.reg = (uint32_t)(data + size);
        descriptor_section.DSTADDR.reg = (uint32_t)&LUOS_COM->USART.DATA.reg;
        descriptor_section.BTCNT.reg   = size;
        // Enable TX
        LuosHAL_SetTxState(true);
        LUOS_DMA->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
        LUOS_COM->USART.INTENSET.reg = SERCOM_USART_INTENSET_TXC; // enable IT
#endif
    }
    else
    {
        // Wait before send ack
        // This is a patch du to difference MCU frequency
        while (LUOS_TIMER->COUNT16.COUNT.reg < (0xFFFF - (timoutclockcnt * (DEFAULT_TIMEOUT - TIMEOUT_ACK))))
            ;
        // Enable TX
        LuosHAL_SetTxState(true);
        // Transmit the only byte we have
        LUOS_COM->USART.DATA.reg = *data;
        // Enable Transmission complete interrupt because we only have one.
        LUOS_COM->USART.INTENSET.reg = SERCOM_USART_INTENSET_TXC; // enable IT
    }
    LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
}
/******************************************************************************
 * @brief set state of Txlock detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
void LuosHAL_SetRxDetecPin(uint8_t Enable)
{
    if (TX_LOCK_DETECT_IRQ != DISABLE)
    {
        EIC->INTFLAG.reg = (1 << TX_LOCK_DETECT_IRQ); // clear IT flag
        if (Enable == true)
        {
            EIC->INTENSET.reg = (1 << TX_LOCK_DETECT_IRQ); // enable IT
        }
        else
        {
            EIC->INTENCLR.reg = (1 << TX_LOCK_DETECT_IRQ);
        }
    }
}
/******************************************************************************
 * @brief get Lock Com transmit status this is the HW that can generate lock TX
 * @param None
 * @return Lock status
 ******************************************************************************/
uint8_t LuosHAL_GetTxLockState(void)
{
    uint8_t result = false;
    if (LUOS_COM->USART.INTFLAG.bit.RXS == 1)
    {
        LUOS_COM->USART.INTFLAG.bit.RXS = 1; // clear flag rx start
        LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
        result = true;
    }
    else
    {
        if ((TX_LOCK_DETECT_PIN != DISABLE) && (TX_LOCK_DETECT_PORT != DISABLE))
        {
            if (((PORT->Group[TX_LOCK_DETECT_PORT].IN.reg >> TX_LOCK_DETECT_PIN) & 0x01) == 0x00)
            {
                result = true;
            }
            if (TX_LOCK_DETECT_IRQ == DISABLE)
            {
                if (result == true)
                {
                    LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
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
static void LuosHAL_TimeoutInit(void)
{
    // initialize clock
    LUOS_TIMER_LOCK_ENABLE();

    LUOS_TIMER->COUNT16.CTRLA.reg = TC_CTRLA_RESETVALUE;
    while ((LUOS_TIMER->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY))
        ;
    /* Configure counter mode & prescaler */
    LUOS_TIMER->COUNT16.CTRLA.reg            = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_PRESCALER_DIV1 | TC_CTRLA_WAVEGEN_MPWM;
    LUOS_TIMER->COUNT16.CTRLBSET.bit.ONESHOT = 1;

    LUOS_TIMER->COUNT16.COUNT.reg = 0xFFFF - (timoutclockcnt * DEFAULT_TIMEOUT);
    /* Clear all interrupt flags */
    LUOS_TIMER->COUNT16.INTENSET.reg = TC_INTENSET_RESETVALUE;
    LUOS_TIMER->COUNT16.INTENSET.reg = TC_INTENSET_OVF;

    NVIC_SetPriority(LUOS_TIMER_IRQ, 3);
    NVIC_EnableIRQ(LUOS_TIMER_IRQ);
}
/******************************************************************************
 * @brief Luos Timeout for Rx communication
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_ResetTimeout(uint16_t nbrbit)
{
    NVIC_ClearPendingIRQ(LUOS_TIMER_IRQ); // clear IT pending
    LUOS_TIMER->COUNT16.INTFLAG.bit.OVF = 1;
    LUOS_TIMER->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
    if (nbrbit != 0)
    {
        LUOS_TIMER->COUNT16.COUNT.reg = 0xFFFF - (timoutclockcnt * nbrbit);
        LUOS_TIMER->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
    }
}
/******************************************************************************
 * @brief Luos Timeout for Rx communication
 * @param None
 * @return None
 ******************************************************************************/
void LUOS_TIMER_IRQHANDLER()
{
    if (LUOS_TIMER->COUNT16.INTFLAG.bit.OVF == 1)
    {
        LUOS_TIMER->COUNT16.INTFLAG.bit.OVF = 1;
        if ((ctx.tx.lock == true) && (LuosHAL_GetTxLockState() == false))
        {
            LuosHAL_SetTxState(false);
            LuosHAL_SetRxState(true);
            Recep_Timeout();
        }
    }
}

/******************************************************************************
 * @brief Luos GetTimestamp
 * @param None
 * @return uint64_t
 ******************************************************************************/
uint64_t LuosHAL_GetTimestamp(void)
{
    ll_timestamp.lower_timestamp  = (SysTick->LOAD - SysTick->VAL) * (1000000000 / MCUFREQ);
    ll_timestamp.higher_timestamp = LuosHAL_GetSystick() - ll_timestamp.start_offset;

    return ll_timestamp.higher_timestamp * 1000000 + (uint64_t)ll_timestamp.lower_timestamp;
}

/******************************************************************************
 * @brief Luos start Timestamp
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_StartTimestamp(void)
{
    ll_timestamp.start_offset = LuosHAL_GetSystick();
}

/******************************************************************************
 * @brief Luos stop Timestamp
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_StopTimestamp(void)
{
    ll_timestamp.lower_timestamp  = 0;
    ll_timestamp.higher_timestamp = 0;
    ll_timestamp.start_offset     = 0;
}

/******************************************************************************
 * @brief Initialisation GPIO
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_GPIOInit(void)
{
    uint32_t Position = 0;
    uint32_t Config   = 0;

    // Activate Clock for PIN choosen in luosHAL
    PORT_CLOCK_ENABLE();

    if ((RX_EN_PIN != DISABLE) || (RX_EN_PORT != DISABLE))
    {
        /*Configure GPIO pins : RxEN_Pin */
        PORT->Group[RX_EN_PORT].PINCFG[RX_EN_PIN].reg = PORT_PINCFG_RESETVALUE; // no pin mux / no input /  no pull / low streght
        PORT->Group[RX_EN_PORT].PINCFG[RX_EN_PIN].reg |= PORT_PINCFG_DRVSTR;    // hight streght drive
        PORT->Group[RX_EN_PORT].DIRSET.reg = (1 << RX_EN_PIN);                  // Output
        PORT->Group[RX_EN_PORT].OUTCLR.reg = (1 << RX_EN_PIN);                  // disable Tx set output low
    }

    if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
    {
        /*Configure GPIO pins : TxEN_Pin */
        PORT->Group[TX_EN_PORT].PINCFG[TX_EN_PIN].reg = PORT_PINCFG_RESETVALUE; // no pin mux / no input /  no pull / low streght
        PORT->Group[TX_EN_PORT].PINCFG[TX_EN_PIN].reg |= PORT_PINCFG_DRVSTR;    // hight streght drive
        PORT->Group[TX_EN_PORT].DIRSET.reg = (1 << TX_EN_PIN);                  // Output
        PORT->Group[TX_EN_PORT].OUTCLR.reg = (1 << TX_EN_PIN);                  // disable Tx set output low
    }

    /*Configure GPIO pins : TX_LOCK_DETECT_Pin */
    if ((TX_LOCK_DETECT_PIN != DISABLE) || (TX_LOCK_DETECT_PORT != DISABLE))
    {
        PORT->Group[TX_LOCK_DETECT_PORT].PINCFG[TX_LOCK_DETECT_PIN].reg = PORT_PINCFG_RESETVALUE; // no pin mux / no input /  no pull / low streght
        PORT->Group[TX_LOCK_DETECT_PORT].PINCFG[TX_LOCK_DETECT_PIN].reg |= PORT_PINCFG_INEN;      // enable input
        PORT->Group[TX_LOCK_DETECT_PORT].OUTSET.reg = (1 << TX_LOCK_DETECT_PIN);                  // pull up
        if (TX_LOCK_DETECT_IRQ != DISABLE)
        {
            PORT->Group[TX_LOCK_DETECT_PORT].PMUX[TX_LOCK_DETECT_PIN >> 1].reg |= (0 << (4 * (TX_LOCK_DETECT_PIN % 2)));
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
            EIC->CONFIG[Config].reg &= ~(EIC_CONFIG_SENSE0_Msk << Position);   // reset sense mode
            EIC->CONFIG[Config].reg |= EIC_CONFIG_SENSE0_FALL_Val << Position; // Falling EDGE
            EIC->INTFLAG.reg  = (1 << TX_LOCK_DETECT_IRQ);                     // clear IT flag
            EIC->INTENCLR.reg = (1 << TX_LOCK_DETECT_IRQ);
        }
    }

    /*Configure GPIO pin : TxPin */
    PORT->Group[COM_TX_PORT].PINCFG[COM_TX_PIN].reg = PORT_PINCFG_RESETVALUE;                    // no pin mux / no input /  no pull / low streght
    PORT->Group[COM_TX_PORT].PINCFG[COM_TX_PIN].reg |= PORT_PINCFG_PULLEN;                       // Enable Pull
    PORT->Group[COM_TX_PORT].OUTSET.reg = (1 << COM_TX_PIN);                                     // Pull up
    PORT->Group[COM_TX_PORT].PMUX[COM_TX_PIN >> 1].reg |= (COM_TX_AF << (4 * (COM_TX_PIN % 2))); // mux to sercom

    /*Configure GPIO pin : RxPin */
    PORT->Group[COM_RX_PORT].PINCFG[COM_RX_PIN].reg = PORT_PINCFG_RESETVALUE;                    // no pin mux / no input /  no pull / low streght
    PORT->Group[COM_RX_PORT].PINCFG[COM_RX_PIN].reg |= PORT_PINCFG_PMUXEN;                       // mux en
    PORT->Group[COM_RX_PORT].PMUX[COM_RX_PIN >> 1].reg |= (COM_TX_AF << (4 * (COM_RX_PIN % 2))); // mux to sercom

    // configure PTP
    LuosHAL_RegisterPTP();
    for (uint8_t i = 0; i < NBR_PORT; i++) /*Configure GPIO pins : PTP_Pin */
    {
        // Setup PTP lines //Mux all PTP to EIC
        PORT->Group[PTP[NBR_PORT].Port].PMUX[PTP[NBR_PORT].Pin >> 1].reg |= (0 << (4 * (PTP[NBR_PORT].Pin % 2)));
        LuosHAL_SetPTPDefaultState(i);
    }

    NVIC_SetPriority(EIC_IRQn, 3);
    NVIC_EnableIRQ(EIC_IRQn);

    // Enable EIC interrupt
    EIC->CTRL.reg |= EIC_CTRL_ENABLE;
}
/******************************************************************************
 * @brief Register PTP
 * @param void
 * @return None
 ******************************************************************************/
static void LuosHAL_RegisterPTP(void)
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
void PINOUT_IRQHANDLER()
{
    uint32_t FlagIT = 0;
    ////Process for Tx Lock Detec
    if (((EIC->INTFLAG.reg & (1 << TX_LOCK_DETECT_IRQ))) && (TX_LOCK_DETECT_IRQ != DISABLE))
    {
        ctx.tx.lock = true;
        LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
        EIC->INTFLAG.reg  = (uint32_t)(1 << TX_LOCK_DETECT_IRQ);
        EIC->INTENCLR.reg = (1 << TX_LOCK_DETECT_IRQ);
    }
    else
    {
        for (uint8_t i = 0; i < NBR_PORT; i++)
        {
            FlagIT = (EIC->INTFLAG.reg & (1 << PTP[i].Irq));
            if (FlagIT)
            {
                EIC->INTFLAG.reg = (uint32_t)(1 << PTP[i].Irq);
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
void LuosHAL_SetPTPDefaultState(uint8_t PTPNbr)
{
    uint32_t Position = 0;
    uint32_t Config   = 0;

    // Pull Down / IT mode / Rising Edge
    PORT->Group[PTP[PTPNbr].Port].PINCFG[PTP[PTPNbr].Pin].reg = PORT_PINCFG_RESETVALUE; // no pin mux / no input /  no pull / low streght
    PORT->Group[PTP[PTPNbr].Port].PINCFG[PTP[PTPNbr].Pin].reg |= PORT_PINCFG_PMUXEN;    // mux en
    PORT->Group[PTP[PTPNbr].Port].PINCFG[PTP[PTPNbr].Pin].reg |= PORT_PINCFG_PULLEN;    // pull en
    PORT->Group[PTP[PTPNbr].Port].OUTCLR.reg = (1 << PTP[PTPNbr].Pin);                  // pull down
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
    EIC->CONFIG[Config].reg &= ~(EIC_CONFIG_SENSE0_Msk << Position);   // reset sense mode
    EIC->CONFIG[Config].reg |= EIC_CONFIG_SENSE0_RISE_Val << Position; // Rising EDGE
    EIC->INTFLAG.reg  = (1 << PTP[PTPNbr].Irq);                        // clear IT flag
    EIC->INTENSET.reg = (1 << PTP[PTPNbr].Irq);                        // enable IT
}
/******************************************************************************
 * @brief Set PTP for reverse detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void LuosHAL_SetPTPReverseState(uint8_t PTPNbr)
{
    uint32_t Position = 0;
    uint32_t Config   = 0;

    // Pull Down / IT mode / Falling Edge
    PORT->Group[PTP[PTPNbr].Port].PINCFG[PTP[PTPNbr].Pin].reg = PORT_PINCFG_RESETVALUE; // no pin mux / no input /  no pull / low streght
    PORT->Group[PTP[PTPNbr].Port].PINCFG[PTP[PTPNbr].Pin].reg |= PORT_PINCFG_PMUXEN;    // mux en
    PORT->Group[PTP[PTPNbr].Port].PINCFG[PTP[PTPNbr].Pin].reg |= PORT_PINCFG_PULLEN;    // pull en
    PORT->Group[PTP[PTPNbr].Port].OUTCLR.reg = (1 << PTP[PTPNbr].Pin);                  // pull down
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
    EIC->CONFIG[Config].reg &= ~(EIC_CONFIG_SENSE0_Msk << Position);   // reset sense mode
    EIC->CONFIG[Config].reg |= EIC_CONFIG_SENSE0_FALL_Val << Position; // Falling EDGE
    EIC->INTFLAG.reg  = (1 << PTP[PTPNbr].Irq);                        // clear IT flag
    EIC->INTENSET.reg = (1 << PTP[PTPNbr].Irq);                        // enable IT
}
/******************************************************************************
 * @brief Set PTP line
 * @param PTP branch
 * @return None
 ******************************************************************************/
void LuosHAL_PushPTP(uint8_t PTPNbr)
{
    // Pull Down / Output mode
    EIC->INTENCLR.reg                                         = (1 << PTP[PTPNbr].Irq); // disable IT
    EIC->INTFLAG.reg                                          = (1 << PTP[PTPNbr].Irq); // clear IT flag
    PORT->Group[PTP[PTPNbr].Port].PINCFG[PTP[PTPNbr].Pin].reg = PORT_PINCFG_RESETVALUE; // no pin mux / no input /  no pull / low streght
    // PORT->Group[PTP[PTPNbr].Port].PINCFG[PTP[PTPNbr].Pin] |= PORT_PINCFG_PULLEN; //pull en
    PORT->Group[PTP[PTPNbr].Port].DIRSET.reg = (1 << PTP[PTPNbr].Pin); // Output
    PORT->Group[PTP[PTPNbr].Port].OUTSET.reg = (1 << PTP[PTPNbr].Pin); // pull down
}
/******************************************************************************
 * @brief Get PTP line
 * @param PTP branch
 * @return Line state
 ******************************************************************************/
uint8_t LuosHAL_GetPTPState(uint8_t PTPNbr)
{
    // Pull Down / Input mode
    EIC->INTENCLR.reg                                         = (1 << PTP[PTPNbr].Irq); // disable IT
    EIC->INTFLAG.reg                                          = (1 << PTP[PTPNbr].Irq); // clear IT flag
    PORT->Group[PTP[PTPNbr].Port].PINCFG[PTP[PTPNbr].Pin].reg = PORT_PINCFG_RESETVALUE; // no pin mux / no input /  no pull / low streght
    PORT->Group[PTP[PTPNbr].Port].PINCFG[PTP[PTPNbr].Pin].reg |= PORT_PINCFG_INEN;      // input
    PORT->Group[PTP[PTPNbr].Port].PINCFG[PTP[PTPNbr].Pin].reg |= PORT_PINCFG_PULLEN;    // pull en
    PORT->Group[PTP[PTPNbr].Port].DIRCLR.reg = (1 << PTP[PTPNbr].Pin);                  // Output
    PORT->Group[PTP[PTPNbr].Port].OUTCLR.reg = (1 << PTP[PTPNbr].Pin);                  // pull down
    return (((PORT->Group[PTP[PTPNbr].Port].IN.reg >> PTP[PTPNbr].Pin)) & 0x01);
}
/******************************************************************************
 * @brief Initialize CRC Process
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_CRCInit(void)
{
}
/******************************************************************************
 * @brief Compute CRC
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
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
/******************************************************************************
 * @brief Flash Initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_FlashInit(void)
{
    NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_READMODE_NO_MISS_PENALTY | NVMCTRL_CTRLB_SLEEPPRM_WAKEONACCESS
                         | NVMCTRL_CTRLB_RWS(1) | NVMCTRL_CTRLB_MANW;
}
/******************************************************************************
 * @brief Erase flash page where Luos keep permanente information
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_FlashEraseLuosMemoryInfo(void)
{
    uint32_t address   = ADDRESS_ALIASES_FLASH;
    NVMCTRL->ADDR.reg  = address >> 1;
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMD_ER_Val | NVMCTRL_CTRLA_CMDEX_KEY;
    NVMCTRL->ADDR.reg  = (address + 256) >> 1;
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMD_ER_Val | NVMCTRL_CTRLA_CMDEX_KEY;
    NVMCTRL->ADDR.reg  = (address + 512) >> 1;
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMD_ER_Val | NVMCTRL_CTRLA_CMDEX_KEY;
    NVMCTRL->ADDR.reg  = (address + 768) >> 1;
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMD_ER_Val | NVMCTRL_CTRLA_CMDEX_KEY;
}
/******************************************************************************
 * @brief Write flash page where Luos keep permanente information
 * @param Address page / size to write / pointer to data to write
 * @return
 ******************************************************************************/
void LuosHAL_FlashWriteLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data)
{
    uint32_t i         = 0;
    uint32_t *paddress = (uint32_t *)addr;

    // Before writing we have to erase the entire page
    // to do that we have to backup current falues by copying it into RAM
    uint8_t page_backup[16 * PAGE_SIZE];
    memcpy(page_backup, (void *)ADDRESS_ALIASES_FLASH, 16 * PAGE_SIZE);

    // Now we can erase the page
    LuosHAL_FlashEraseLuosMemoryInfo();

    // Then add input data into backuped value on RAM
    uint32_t RAMaddr = (addr - ADDRESS_ALIASES_FLASH);
    memcpy(&page_backup[RAMaddr], data, size);

    /* writing 32-bit data into the given address */
    for (i = 0; i < (PAGE_SIZE / 4); i++)
    {
        *paddress++ = page_backup[i];
    }

    /* Set address and command */
    NVMCTRL->ADDR.reg = addr >> 1;

    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMD_WP_Val | NVMCTRL_CTRLA_CMDEX_KEY;
}
/******************************************************************************
 * @brief read information from page where Luos keep permanente information
 * @param Address info / size to read / pointer callback data to read
 * @return
 ******************************************************************************/
void LuosHAL_FlashReadLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data)
{
    memcpy(data, (void *)(addr), size);
}

/******************************************************************************
 * @brief Set boot mode in shared flash memory
 * @param
 * @return
 ******************************************************************************/
void LuosHAL_SetMode(uint8_t mode)
{
}

/******************************************************************************
 * @brief Save node ID in shared flash memory
 * @param Address, node_id
 * @return
 ******************************************************************************/
void LuosHAL_SaveNodeID(uint16_t node_id)
{
}

/******************************************************************************
 * @brief software reboot the microprocessor
 * @param
 * @return
 ******************************************************************************/
void LuosHAL_Reboot(void)
{
}

#ifdef BOOTLOADER_CONFIG
/******************************************************************************
 * @brief Get node id saved in flash memory
 * @param Address
 * @return node_id
 ******************************************************************************/
uint16_t LuosHAL_GetNodeID(void)
{
}

/******************************************************************************
 * @brief Save node ID in shared flash memory
 * @param Address, node_id
 * @return
 ******************************************************************************/
void LuosHAL_ProgramFlash(uint32_t address, uint8_t page, uint16_t size, uint8_t *data)
{
}

/******************************************************************************
 * @brief DeInit Bootloader peripherals
 * @param
 * @return
 ******************************************************************************/
void LuosHAL_DeInit(void)
{
}

/******************************************************************************
 * @brief DeInit Bootloader peripherals
 * @param
 * @return
 ******************************************************************************/
typedef void (*pFunction)(void); /*!< Function pointer definition */

void LuosHAL_JumpToApp(uint32_t app_addr)
{
}

/******************************************************************************
 * @brief Return bootloader mode saved in flash
 * @param
 * @return
 ******************************************************************************/
uint8_t LuosHAL_GetMode(void)
{
}
#endif