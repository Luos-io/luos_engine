/******************************************************************************
 * @file robus_HAL
 * @brief Robus Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family STM32FO
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "robus_hal.h"
#include "luos_hal.h"

#include <stdbool.h>
#include "reception.h"
#include "context.h"

// MCU dependencies this HAL is for family STM32FO you can find
// the HAL stm32cubef0 on ST web site
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_tim.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_system.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEFAULT_TIMEOUT 30
#define TIMEOUT_ACK     DEFAULT_TIMEOUT / 4
/*******************************************************************************
 * Variables
 ******************************************************************************/
#if (USE_CRC_HW == 1)
CRC_HandleTypeDef hcrc;
#endif
GPIO_InitTypeDef GPIO_InitStruct = {0};

uint32_t Timer_Prescaler = (MCUFREQ / DEFAULTBAUDRATE) / TIMERDIV; //(freq MCU/freq timer)/divider timer clock source

typedef struct
{
    uint16_t Pin;
    GPIO_TypeDef *Port;
    uint8_t IRQ;
} Port_t;

Port_t PTP[NBR_PORT];

volatile uint16_t data_size_to_transmit = 0;
volatile uint8_t *tx_data               = 0;

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
    LUOS_COM_CLOCK_ENABLE();

    LL_USART_InitTypeDef USART_InitStruct;

    // Initialise USART1
    LL_USART_Disable(LUOS_COM);
    USART_InitStruct.BaudRate            = Baudrate;
    USART_InitStruct.DataWidth           = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits            = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity              = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection   = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling        = LL_USART_OVERSAMPLING_16;
    while (LL_USART_Init(LUOS_COM, &USART_InitStruct) != SUCCESS)
        ;
    LL_USART_Enable(LUOS_COM);

    // Enable Reception interrupt
    LL_USART_EnableIT_RXNE(LUOS_COM);

    HAL_NVIC_EnableIRQ(LUOS_COM_IRQ);
    HAL_NVIC_SetPriority(LUOS_COM_IRQ, 0, 1);

    // Timeout Initialization
    Timer_Prescaler = (MCUFREQ / Baudrate) / TIMERDIV;
    RobusHAL_TimeoutInit();

#ifndef USE_TX_IT
    LUOS_DMA_CLOCK_ENABLE();

    LL_DMA_SetDataTransferDirection(LUOS_DMA, LUOS_DMA_CHANNEL, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetChannelPriorityLevel(LUOS_DMA, LUOS_DMA_CHANNEL, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(LUOS_DMA, LUOS_DMA_CHANNEL, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(LUOS_DMA, LUOS_DMA_CHANNEL, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(LUOS_DMA, LUOS_DMA_CHANNEL, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(LUOS_DMA, LUOS_DMA_CHANNEL, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(LUOS_DMA, LUOS_DMA_CHANNEL, LL_DMA_MDATAALIGN_BYTE);
    LL_SYSCFG_SetRemapDMA_USART(LUOS_DMA_REMAP);
    LL_DMA_SetPeriphAddress(LUOS_DMA, LUOS_DMA_CHANNEL, (uint32_t)&LUOS_COM->TDR);
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
        // Put Tx in push pull
        LL_GPIO_SetPinOutputType(COM_TX_PORT, COM_TX_PIN, LL_GPIO_OUTPUT_PUSHPULL);
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            LL_GPIO_SetOutputPin(TX_EN_PORT, TX_EN_PIN);
        }
    }
    else
    {
        // Put Tx in open drain
        LL_GPIO_SetPinOutputType(COM_TX_PORT, COM_TX_PIN, LL_GPIO_OUTPUT_OPENDRAIN);
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            LL_GPIO_ResetOutputPin(TX_EN_PORT, TX_EN_PIN);
        }
#ifdef USE_TX_IT
        // Stop current transmit operation
        data_size_to_transmit = 0;
        // Disable Transmission empty buffer interrupt
        LL_USART_DisableIT_TXE(LUOS_COM);
#else
        LL_USART_DisableDMAReq_TX(LUOS_COM);
        LL_DMA_DisableChannel(LUOS_DMA, LUOS_DMA_CHANNEL);
#endif
        // Disable Transmission complete interrupt
        LL_USART_DisableIT_TC(LUOS_COM);
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
        LL_USART_RequestRxDataFlush(LUOS_COM); // Clear data register
        LL_USART_EnableDirectionRx(LUOS_COM);
        LL_USART_EnableIT_RXNE(LUOS_COM); // Enable Rx IT
    }
    else
    {
        LL_USART_DisableDirectionRx(LUOS_COM);
        LL_USART_DisableIT_RXNE(LUOS_COM); // Disable Rx IT
    }
}
/******************************************************************************
 * @brief Process data send or receive
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void LUOS_COM_IRQHANDLER()
{
    // Reset timeout to it's default value
    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);

    // reception management
    if ((LL_USART_IsActiveFlag_RXNE(LUOS_COM) != RESET) && (LL_USART_IsEnabledIT_RXNE(LUOS_COM) != RESET))
    {
        // We receive a byte
        uint8_t data = LL_USART_ReceiveData8(LUOS_COM);
        Recep_data(&data); // send reception byte to state machine
        if (data_size_to_transmit == 0)
        {
            LUOS_COM->ICR = 0xFFFFFFFF;
            return;
        }
    }
    else if (LL_USART_IsActiveFlag_FE(LUOS_COM) != RESET)
    {
        // Framing ERROR
        ctx.rx.status.rx_framing_error = true;
    }

    // Transmission management
    if ((LL_USART_IsActiveFlag_TC(LUOS_COM) != RESET) && (LL_USART_IsEnabledIT_TC(LUOS_COM) != RESET))
    {
        // Transmission complete
        data_size_to_transmit = 0;
        // Switch to reception mode
        RobusHAL_SetTxState(false);
        RobusHAL_SetRxState(true);
        // Disable transmission complete IRQ
        LL_USART_ClearFlag_TC(LUOS_COM);
        LL_USART_DisableIT_TC(LUOS_COM);
    }
#ifdef USE_TX_IT
    else if ((LL_USART_IsActiveFlag_TXE(LUOS_COM) != RESET) && (LL_USART_IsEnabledIT_TXE(LUOS_COM) != RESET))
    {
        // Transmit buffer empty (this is a software DMA)
        data_size_to_transmit--;
        LL_USART_TransmitData8(LUOS_COM, *(tx_data++));
        if (data_size_to_transmit == 0)
        {
            // Transmission complete, stop loading data and watch for the end of transmission
            // Disable Transmission empty buffer interrupt
            LL_USART_DisableIT_TXE(LUOS_COM);
            // Enable Transmission complete interrupt
            LL_USART_EnableIT_TC(LUOS_COM);
        }
    }
#endif
    LUOS_COM->ICR = 0xFFFFFFFF;
}
/******************************************************************************
 * @brief Process data transmit
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_ComTransmit(uint8_t *data, uint16_t size)
{
    while (LL_USART_IsActiveFlag_TXE(LUOS_COM) == RESET)
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
        LL_USART_TransmitData8(LUOS_COM, *(tx_data++));
        // Enable Transmission empty buffer interrupt to transmit next datas
        LL_USART_EnableIT_TXE(LUOS_COM);
        // Disable Transmission complete interrupt
        LL_USART_DisableIT_TC(LUOS_COM);
#else
        data_size_to_transmit = 0; // Reset this value avoiding to check IT TC during collision
        // Disable DMA to load new length to be tranmitted
        LL_DMA_DisableChannel(LUOS_DMA, LUOS_DMA_CHANNEL);
        // configure address to be transmitted by DMA
        LL_DMA_SetMemoryAddress(LUOS_DMA, LUOS_DMA_CHANNEL, (uint32_t)data);
        // set length to be tranmitted
        LL_DMA_SetDataLength(LUOS_DMA, LUOS_DMA_CHANNEL, size);
        // set request DMA
        LL_USART_EnableDMAReq_TX(LUOS_COM);
        // Enable TX
        RobusHAL_SetTxState(true);
        // Enable DMA again
        LL_DMA_EnableChannel(LUOS_DMA, LUOS_DMA_CHANNEL);
        // enable transmit complete
        LL_USART_EnableIT_TC(LUOS_COM);
#endif
    }
    else
    {
        data_size_to_transmit = 1;
        // wait before send ack
        // this is a patch du to difference MCU frequency
        while (LL_TIM_GetCounter(LUOS_TIMER) < TIMEOUT_ACK)
            ;
        // Enable TX
        RobusHAL_SetTxState(true);
        // Transmit the only byte we have
        LL_USART_TransmitData8(LUOS_COM, *data);
        // Enable Transmission complete interrupt because we only have one.
        LL_USART_EnableIT_TC(LUOS_COM);
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
        __HAL_GPIO_EXTI_CLEAR_IT(TX_LOCK_DETECT_IRQ);
        if (Enable == true)
        {
            EXTI->IMR |= TX_LOCK_DETECT_PIN;
        }
        else
        {
            EXTI->IMR &= ~TX_LOCK_DETECT_PIN;
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

#ifdef USART_ISR_BUSY
    if (LL_USART_IsActiveFlag_BUSY(LUOS_COM) == true)
    {
        RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
        result = true;
    }
#else
    if ((TX_LOCK_DETECT_PIN != DISABLE) && (TX_LOCK_DETECT_PORT != DISABLE))
    {
        if (HAL_GPIO_ReadPin(TX_LOCK_DETECT_PORT, TX_LOCK_DETECT_PIN) == GPIO_PIN_RESET)
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
#endif
    return result;
}
/******************************************************************************
 * @brief Luos Timeout initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_TimeoutInit(void)
{
    LL_TIM_InitTypeDef TimerInit = {0};

    // initialize clock
    LUOS_TIMER_CLOCK_ENABLE();

    TimerInit.Autoreload        = DEFAULT_TIMEOUT;
    TimerInit.ClockDivision     = LL_TIM_CLOCKDIVISION_DIV1;
    TimerInit.CounterMode       = LL_TIM_COUNTERMODE_UP;
    TimerInit.Prescaler         = Timer_Prescaler - 1;
    TimerInit.RepetitionCounter = 0;
    while (LL_TIM_Init(LUOS_TIMER, &TimerInit) != SUCCESS)
        ;
    LL_TIM_EnableIT_UPDATE(LUOS_TIMER);
    HAL_NVIC_SetPriority(LUOS_TIMER_IRQ, 0, 2);
    HAL_NVIC_EnableIRQ(LUOS_TIMER_IRQ);
}
/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_ResetTimeout(uint16_t nbrbit)
{
    LL_TIM_DisableCounter(LUOS_TIMER);
    NVIC_ClearPendingIRQ(LUOS_TIMER_IRQ); // Clear IT pending NVIC
    LL_TIM_ClearFlag_UPDATE(LUOS_TIMER);
    LL_TIM_SetCounter(LUOS_TIMER, 0); // Reset counter
    if (nbrbit != 0)
    {
        LL_TIM_SetAutoReload(LUOS_TIMER, nbrbit); // reload value
        LL_TIM_EnableCounter(LUOS_TIMER);
    }
}
/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void LUOS_TIMER_IRQHANDLER()
{
    if (LL_TIM_IsActiveFlag_UPDATE(LUOS_TIMER) != RESET)
    {
        LL_TIM_ClearFlag_UPDATE(LUOS_TIMER);
        LL_TIM_DisableCounter(LUOS_TIMER);
        if ((ctx.tx.lock == true) && (RobusHAL_GetTxLockState() == false))
        {
            // Enable RX detection pin if needed
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
    // Activate Clock for PIN choosen in RobusHAL
    PORT_CLOCK_ENABLE();

    if ((RX_EN_PIN != DISABLE) || (RX_EN_PORT != DISABLE))
    {
        /*Configure GPIO pins : RxEN_Pin */
        GPIO_InitStruct.Pin   = RX_EN_PIN;
        GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(RX_EN_PORT, &GPIO_InitStruct);
    }

    if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
    {
        /*Configure GPIO pins : TxEN_Pin */
        GPIO_InitStruct.Pin   = TX_EN_PIN;
        GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(TX_EN_PORT, &GPIO_InitStruct);
    }

    /*Configure GPIO pin : TxPin */
    GPIO_InitStruct.Pin       = COM_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = COM_TX_AF;
    HAL_GPIO_Init(COM_TX_PORT, &GPIO_InitStruct);

    /*Configure GPIO pin : RxPin */
    GPIO_InitStruct.Pin       = COM_RX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = COM_RX_AF;
    HAL_GPIO_Init(COM_RX_PORT, &GPIO_InitStruct);

    // configure PTP
    RobusHAL_RegisterPTP();
    for (uint8_t i = 0; i < NBR_PORT; i++) /*Configure GPIO pins : PTP_Pin */
    {
        GPIO_InitStruct.Pin   = PTP[i].Pin;
        GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
        GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(PTP[i].Port, &GPIO_InitStruct);
        // Setup PTP lines
        RobusHAL_SetPTPDefaultState(i);
        // activate IT for PTP
        HAL_NVIC_SetPriority(PTP[i].IRQ, 1, 0);
        HAL_NVIC_EnableIRQ(PTP[i].IRQ);
    }

    if ((TX_LOCK_DETECT_PIN != DISABLE) || (TX_LOCK_DETECT_PORT != DISABLE))
    {
        /*Configure GPIO pins : TX_LOCK_DETECT_Pin */
        GPIO_InitStruct.Pin   = TX_LOCK_DETECT_PIN;
        GPIO_InitStruct.Pull  = GPIO_PULLUP;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        if (TX_LOCK_DETECT_IRQ != DISABLE)
        {
            GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
            HAL_NVIC_SetPriority(TX_LOCK_DETECT_IRQ, 1, 0);
            HAL_NVIC_EnableIRQ(TX_LOCK_DETECT_IRQ);
        }
        HAL_GPIO_Init(TX_LOCK_DETECT_PORT, &GPIO_InitStruct);
    }
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
    PTP[0].IRQ  = PTPA_IRQ;
#endif

#if (NBR_PORT >= 2)
    PTP[1].Pin  = PTPB_PIN;
    PTP[1].Port = PTPB_PORT;
    PTP[1].IRQ  = PTPB_IRQ;
#endif

#if (NBR_PORT >= 3)
    PTP[2].Pin  = PTPC_PIN;
    PTP[2].Port = PTPC_PORT;
    PTP[2].IRQ  = PTPC_IRQ;
#endif

#if (NBR_PORT >= 4)
    PTP[3].Pin  = PTPD_PIN;
    PTP[3].Port = PTPD_PORT;
    PTP[3].IRQ  = PTPD_IRQ;
#endif
}
/******************************************************************************
 * @brief callback for GPIO IT
 * @param GPIO IT line
 * @return None
 ******************************************************************************/
_CRITICAL void PINOUT_IRQHANDLER(uint16_t GPIO_Pin)
{
    ////Process for Tx Lock Detec
    if ((GPIO_Pin == TX_LOCK_DETECT_PIN) && (TX_LOCK_DETECT_IRQ != DISABLE))
    {
        ctx.tx.lock = true;
        RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
        EXTI->IMR &= ~TX_LOCK_DETECT_PIN;
    }
    else
    {
        for (uint8_t i = 0; i < NBR_PORT; i++)
        {
            if (GPIO_Pin == PTP[i].Pin)
            {
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
    __HAL_GPIO_EXTI_CLEAR_IT(PTP[PTPNbr].Pin);
    // Pull Down / IT mode / Rising Edge
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Pin  = PTP[PTPNbr].Pin;
    HAL_GPIO_Init(PTP[PTPNbr].Port, &GPIO_InitStruct);
}
/******************************************************************************
 * @brief Set PTP for reverse detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_SetPTPReverseState(uint8_t PTPNbr)
{
    __HAL_GPIO_EXTI_CLEAR_IT(PTP[PTPNbr].Pin);
    // Pull Down / IT mode / Falling Edge
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING; // reverse the detection edge
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Pin  = PTP[PTPNbr].Pin;
    HAL_GPIO_Init(PTP[PTPNbr].Port, &GPIO_InitStruct);
}
/******************************************************************************
 * @brief Set PTP line
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_PushPTP(uint8_t PTPNbr)
{
    // Pull Down / Output mode
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Clean edge/state detection and set the PTP pin as output
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Pin  = PTP[PTPNbr].Pin;
    HAL_GPIO_Init(PTP[PTPNbr].Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(PTP[PTPNbr].Port, PTP[PTPNbr].Pin, GPIO_PIN_SET);
}
/******************************************************************************
 * @brief Get PTP line
 * @param PTP branch
 * @return Line state
 ******************************************************************************/
uint8_t RobusHAL_GetPTPState(uint8_t PTPNbr)
{
    // Pull Down / Input mode
    return (HAL_GPIO_ReadPin(PTP[PTPNbr].Port, PTP[PTPNbr].Pin));
}
/******************************************************************************
 * @brief Initialize CRC Process
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_CRCInit(void)
{
#if (USE_CRC_HW == 1)
    __HAL_RCC_CRC_CLK_ENABLE();
    hcrc.Instance                     = CRC;
    hcrc.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_DISABLE;
    hcrc.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_ENABLE;
    hcrc.Init.GeneratingPolynomial    = 7;
    hcrc.Init.CRCLength               = CRC_POLYLENGTH_16B;
    hcrc.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE;
    hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
    hcrc.InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES;
    if (HAL_CRC_Init(&hcrc) != HAL_OK)
    {
        while (1)
            ;
    }
#endif
}
/******************************************************************************
 * @brief Compute CRC
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
{
#if (USE_CRC_HW == 1)
    hcrc.Instance->INIT = *(uint16_t *)crc;
    __HAL_CRC_DR_RESET(&hcrc);
    *(uint16_t *)crc = (uint16_t)HAL_CRC_Accumulate(&hcrc, (uint32_t *)data, 1);
#else
    uint16_t dbyte = *data;
    *(uint16_t *)crc ^= dbyte << 8;
    for (uint8_t j = 0; j < 8; ++j)
    {
        uint16_t mix = *(uint16_t *)crc & 0x8000;
        *(uint16_t *)crc = (*(uint16_t *)crc << 1);
        if (mix)
            *(uint16_t *)crc = *(uint16_t *)crc ^ 0x0007;
    }
#endif
}
