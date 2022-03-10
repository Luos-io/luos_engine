/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family STM32F4
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos_hal.h"

#include <stdbool.h>
#include <string.h>
#include "reception.h"
#include "context.h"

// MCU dependencies this HAL is for family STM32F4 you can find
// the HAL stm32cubeF4 on ST web site
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_system.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEFAULT_TIMEOUT 20
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

    // start timestamp
    LuosHAL_StartTimestamp();
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
}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms
 * @param None
 * @return tick Counter
 ******************************************************************************/
uint32_t LuosHAL_GetSystick(void)
{
    return HAL_GetTick();
}
/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void LuosHAL_ComInit(uint32_t Baudrate)
{
    LUOS_COM_CLOCK_ENABLE();

    LL_USART_InitTypeDef USART_InitStruct;

    // Initialise USART
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
    LuosHAL_TimeoutInit();

#ifndef USE_TX_IT
    LUOS_DMA_CLOCK_ENABLE();

    LL_DMA_SetChannelSelection(LUOS_DMA, LUOS_DMA_STREAM, LUOS_DMA_CHANNEL);
    LL_DMA_SetDataTransferDirection(LUOS_DMA, LUOS_DMA_STREAM, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetStreamPriorityLevel(LUOS_DMA, LUOS_DMA_STREAM, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(LUOS_DMA, LUOS_DMA_STREAM, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(LUOS_DMA, LUOS_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(LUOS_DMA, LUOS_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(LUOS_DMA, LUOS_DMA_STREAM, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(LUOS_DMA, LUOS_DMA_STREAM, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_DisableFifoMode(LUOS_DMA, LUOS_DMA_STREAM);
    LL_DMA_SetPeriphAddress(LUOS_DMA, LUOS_DMA_STREAM, (uint32_t)&LUOS_COM->DR);
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
        LL_DMA_DisableStream(LUOS_DMA, LUOS_DMA_STREAM);
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
void LuosHAL_SetRxState(uint8_t Enable)
{
    if (Enable == true)
    {
        LL_USART_ReceiveData8(LUOS_COM);  // empty buffer
        LL_USART_EnableIT_RXNE(LUOS_COM); // Enable Rx IT
    }
    else
    {
        LL_USART_DisableIT_RXNE(LUOS_COM); // Disable Rx IT
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
    if ((LL_USART_IsActiveFlag_RXNE(LUOS_COM) != RESET) && (LL_USART_IsEnabledIT_RXNE(LUOS_COM) != RESET))
    {
        // We receive a byte
        uint8_t data = LL_USART_ReceiveData8(LUOS_COM);
        ctx.rx.callback(&data); // send reception byte to state machine
        if (data_size_to_transmit == 0)
        {
            LUOS_COM->SR = 0xFFFFFFFF;
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
        // Switch to reception mode
        LuosHAL_SetTxState(false);
        LuosHAL_SetRxState(true);
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
    LUOS_COM->SR = 0xFFFFFFFF;
}
/******************************************************************************
 * @brief Process data transmit
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_ComTransmit(uint8_t *data, uint16_t size)
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
        data_size_to_transmit = 0; // to not check IT TC during collision
        // Disable DMA to load new length to be tranmitted
        LL_DMA_DisableStream(LUOS_DMA, LUOS_DMA_STREAM);
        // Configure address to be transmitted by DMA
        LL_DMA_SetMemoryAddress(LUOS_DMA, LUOS_DMA_STREAM, (uint32_t)data);
        // Set length to be tranmitted
        LL_DMA_SetDataLength(LUOS_DMA, LUOS_DMA_STREAM, size);
        // Set request DMA
        LL_USART_EnableDMAReq_TX(LUOS_COM);
        // clear flag shity way must be change
        LUOS_DMA->HIFCR = 0xFFFFFFFF;
        LUOS_DMA->LIFCR = 0xFFFFFFFF;
        // Enable TX
        LuosHAL_SetTxState(true);
        // Enable DMA again
        LL_DMA_EnableStream(LUOS_DMA, LUOS_DMA_STREAM);
        // Enable transmit complete
        LL_USART_EnableIT_TC(LUOS_COM);
#endif
    }
    else
    {
        // Wait before send ack
        // This is a patch du to difference MCU frequency
        while (LL_TIM_GetCounter(LUOS_TIMER) < TIMEOUT_ACK)
            ;
        // Enable TX
        LuosHAL_SetTxState(true);
        // Transmit the only byte we have
        LL_USART_TransmitData8(LUOS_COM, *data);
        // Enable Transmission complete interrupt because we only have one.
        LL_USART_EnableIT_TC(LUOS_COM);
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
uint8_t LuosHAL_GetTxLockState(void)
{
    uint8_t result = false;

#ifdef USART_ISR_BUSY
    if (LL_USART_IsActiveFlag_BUSY(LUOS_COM) == true)
    {
        LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
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
                LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
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
static void LuosHAL_TimeoutInit(void)
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
 * @brief Luos Timeout for Rx communication
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_ResetTimeout(uint16_t nbrbit)
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
 * @brief Luos communication Timeout
 * @param None
 * @return None
 ******************************************************************************/
void LUOS_TIMER_IRQHANDLER()
{
    if (LL_TIM_IsActiveFlag_UPDATE(LUOS_TIMER) != RESET)
    {
        LL_TIM_ClearFlag_UPDATE(LUOS_TIMER);
        LL_TIM_DisableCounter(LUOS_TIMER);
        if ((ctx.tx.lock == true) && (LuosHAL_GetTxLockState() == false))
        {
            // Enable RX detection pin if needed
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
    // Activate Clock for PIN choosen in luosHAL
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
    LuosHAL_RegisterPTP();
    for (uint8_t i = 0; i < NBR_PORT; i++) /*Configure GPIO pins : PTP_Pin */
    {
        GPIO_InitStruct.Pin   = PTP[i].Pin;
        GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
        GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(PTP[i].Port, &GPIO_InitStruct);
        // Setup PTP lines
        LuosHAL_SetPTPDefaultState(i);
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
static void LuosHAL_RegisterPTP(void)
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
void PINOUT_IRQHANDLER(uint16_t GPIO_Pin)
{
    ////Process for Tx Lock Detec
    if ((GPIO_Pin == TX_LOCK_DETECT_PIN) && (TX_LOCK_DETECT_IRQ != DISABLE))
    {
        ctx.tx.lock = true;
        LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
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
void LuosHAL_SetPTPDefaultState(uint8_t PTPNbr)
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
void LuosHAL_SetPTPReverseState(uint8_t PTPNbr)
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
void LuosHAL_PushPTP(uint8_t PTPNbr)
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
uint8_t LuosHAL_GetPTPState(uint8_t PTPNbr)
{
    // Pull Down / Input mode
    return (HAL_GPIO_ReadPin(PTP[PTPNbr].Port, PTP[PTPNbr].Pin));
}
/******************************************************************************
 * @brief Initialize CRC Process
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_CRCInit(void)
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
void LuosHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
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
/******************************************************************************
 * @brief Flash Initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_FlashInit(void)
{
}
/******************************************************************************
 * @brief Erase flash page where Luos keep permanente information
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_FlashEraseLuosMemoryInfo(void)
{
    uint32_t page_error = 0;
    FLASH_EraseInitTypeDef s_eraseinit;

    s_eraseinit.TypeErase = FLASH_TYPEERASE_SECTORS;
    s_eraseinit.NbSectors = 1;
    s_eraseinit.Sector    = FLASH_SECTOR;

    // Erase Page
    HAL_FLASH_Unlock();
    HAL_FLASHEx_Erase(&s_eraseinit, &page_error);
    HAL_FLASH_Lock();
}
/******************************************************************************
 * @brief Write flash page where Luos keep permanente information
 * @param Address page / size to write / pointer to data to write
 * @return
 ******************************************************************************/
void LuosHAL_FlashWriteLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data)
{
    // Before writing we have to erase the entire page
    // to do that we have to backup current falues by copying it into RAM
    uint8_t page_backup[PAGE_SIZE];
    memcpy(page_backup, (void *)ADDRESS_ALIASES_FLASH, PAGE_SIZE);

    // Now we can erase the page
    LuosHAL_FlashEraseLuosMemoryInfo();

    // Then add input data into backuped value on RAM
    uint32_t RAMaddr = (addr - ADDRESS_ALIASES_FLASH);
    memcpy(&page_backup[RAMaddr], data, size);

    // and copy it into flash
    HAL_FLASH_Unlock();

    // ST hal flash program function write data by uint64_t raw data
    for (uint32_t i = 0; i < PAGE_SIZE; i += sizeof(uint64_t))
    {
        while (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, i + ADDRESS_ALIASES_FLASH, *(uint64_t *)(&page_backup[i])) != HAL_OK)
            ;
    }
    HAL_FLASH_Lock();
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
    uint32_t data_to_write = ~BOOT_MODE_MASK | (mode << BOOT_MODE_OFFSET);

    uint32_t sector_error = 0;
    FLASH_EraseInitTypeDef s_eraseinit;

    s_eraseinit.TypeErase    = FLASH_TYPEERASE_SECTORS;
    s_eraseinit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    s_eraseinit.NbSectors    = 1;
    s_eraseinit.Sector       = SHARED_MEMORY_SECTOR;

    /****************************** WARNING ***************************************
     * when STRT bit in FLASH->CR register is called from the app (sector 4 in flash)
     * the application crashes, that's why we only erase the flash from the
     * bootloader
     ******************************* WARNING **************************************/
    if ((mode == 0x01) && (mode == 0x02))
    {
        // erase sector
        HAL_FLASH_Unlock();
        HAL_FLASHEx_Erase(&s_eraseinit, &sector_error);
        HAL_FLASH_Lock();
    }

    // write sector
    HAL_FLASH_Unlock();
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)SHARED_MEMORY_ADDRESS, data_to_write);
    HAL_FLASH_Lock();
}

/******************************************************************************
 * @brief Save node ID in shared flash memory
 * @param Address, node_id
 * @return
 ******************************************************************************/
void LuosHAL_SaveNodeID(uint16_t node_id)
{
    uint32_t *p_start      = (uint32_t *)SHARED_MEMORY_ADDRESS;
    uint32_t saved_data    = *p_start;
    uint32_t data_tmp      = ~NODE_ID_MASK | (node_id << NODE_ID_OFFSET);
    uint32_t data_to_write = saved_data & data_tmp;

    // write sector
    HAL_FLASH_Unlock();
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)SHARED_MEMORY_ADDRESS, data_to_write);
    HAL_FLASH_Lock();
}

/******************************************************************************
 * @brief software reboot the microprocessor
 * @param
 * @return
 ******************************************************************************/
void LuosHAL_Reboot(void)
{
    // DeInit RCC and HAL
    HAL_RCC_DeInit();
    HAL_DeInit();

    // reset systick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    // reset in bootloader mode
    NVIC_SystemReset();
}

#ifdef BOOTLOADER_CONFIG
/******************************************************************************
 * @brief DeInit Bootloader peripherals
 * @param
 * @return
 ******************************************************************************/
void LuosHAL_DeInit(void)
{
    HAL_RCC_DeInit();
    HAL_DeInit();
}

/******************************************************************************
 * @brief DeInit Bootloader peripherals
 * @param
 * @return
 ******************************************************************************/
typedef void (*pFunction)(void); /*!< Function pointer definition */

void LuosHAL_JumpToApp(uint32_t app_addr)
{
    uint32_t JumpAddress = *(__IO uint32_t *)(app_addr + 4);
    pFunction Jump       = (pFunction)JumpAddress;

    __disable_irq();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    SCB->VTOR = app_addr;

    __set_MSP(*(__IO uint32_t *)app_addr);

    __enable_irq();

    Jump();
}

/******************************************************************************
 * @brief Return bootloader mode saved in flash
 * @param
 * @return
 ******************************************************************************/
uint8_t LuosHAL_GetMode(void)
{
    uint32_t *p_start = (uint32_t *)SHARED_MEMORY_ADDRESS;
    uint32_t data     = (*p_start & BOOT_MODE_MASK) >> BOOT_MODE_OFFSET;

    return (uint8_t)data;
}

/******************************************************************************
 * @brief Get node id saved in flash memory
 * @param Address
 * @return node_id
 ******************************************************************************/
uint16_t LuosHAL_GetNodeID(void)
{
    uint32_t *p_start = (uint32_t *)SHARED_MEMORY_ADDRESS;
    uint32_t data     = *p_start & NODE_ID_MASK;
    uint16_t node_id  = (uint16_t)(data >> NODE_ID_OFFSET);

    return node_id;
}

/******************************************************************************
 * @brief erase sectors in flash memory
 * @param Address, size
 * @return
 ******************************************************************************/
void LuosHAL_EraseMemory(uint32_t address, uint16_t size)
{
    uint32_t nb_sectors_to_erase = FLASH_SECTOR_TOTAL - 1 - APP_ADRESS_SECTOR;
    uint32_t sector_to_erase     = APP_ADRESS_SECTOR;

    uint32_t sector_error = 0;
    FLASH_EraseInitTypeDef s_eraseinit;
    s_eraseinit.TypeErase    = FLASH_TYPEERASE_SECTORS;
    s_eraseinit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    s_eraseinit.NbSectors    = 1;

    int i = 0;
    for (i = 0; i < nb_sectors_to_erase; i++)
    {
        s_eraseinit.Sector = sector_to_erase;

        // Unlock flash
        HAL_FLASH_Unlock();
        // Erase Page
        HAL_FLASHEx_Erase(&s_eraseinit, &sector_error);
        // re-lock FLASH
        HAL_FLASH_Lock();

        // update page to erase
        sector_to_erase += 1;
    }
}

/******************************************************************************
 * @brief Save binary data in shared flash memory
 * @param Address, size, data[]
 * @return
 ******************************************************************************/
void LuosHAL_ProgramFlash(uint32_t address, uint16_t size, uint8_t *data)
{
    // Unlock flash
    HAL_FLASH_Unlock();
    // ST hal flash program function write data by uint64_t raw data
    for (uint32_t i = 0; i < size; i += sizeof(uint32_t))
    {
        while (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, i + address, *(uint32_t *)(&data[i])) != HAL_OK)
            ;
    }
    // re-lock FLASH
    HAL_FLASH_Lock();
}
#endif
