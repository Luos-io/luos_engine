/******************************************************************************
 * @file serial_hal
 * @brief serial communication hardware abstraction layer
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "serial_network_hal.h"
#include "_serial_network.h"
#include "luos_utils.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_ll_lpuart.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_hal.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t rx_buffer_size;
volatile uint16_t RX_PrevPointerPosition = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Initialisation of the Serial communication
 * @param None
 * @return None
 * ****************************************************************************/
void SerialHAL_Init(uint8_t *rx_buffer, uint32_t buffer_size)
{
    LUOS_ASSERT(rx_buffer);
    rx_buffer_size         = buffer_size;
    RX_PrevPointerPosition = 0;

    LL_LPUART_DeInit(SERIAL_COM);
    ///////////////////////////////
    // GPIO SERIAL Init
    ///////////////////////////////
    SERIAL_TX_CLK();
    SERIAL_RX_CLK();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // TX
    GPIO_InitStruct.Pin       = SERIAL_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = SERIAL_TX_AF;
    HAL_GPIO_Init(SERIAL_TX_PORT, &GPIO_InitStruct);
    // RX
    GPIO_InitStruct.Pin       = SERIAL_RX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = SERIAL_RX_AF;
    HAL_GPIO_Init(SERIAL_RX_PORT, &GPIO_InitStruct);

    ///////////////////////////////
    // USART SERIAL Init
    ///////////////////////////////
    SERIAL_COM_CLOCK_ENABLE();

    LL_LPUART_InitTypeDef LPUART_InitStruct;
    // Initialise USART3
    LL_LPUART_Disable(SERIAL_COM);
    LPUART_InitStruct.BaudRate            = SERIAL_NETWORK_BAUDRATE;
    LPUART_InitStruct.PrescalerValue      = LL_LPUART_PRESCALER_DIV1;
    LPUART_InitStruct.DataWidth           = LL_LPUART_DATAWIDTH_8B;
    LPUART_InitStruct.StopBits            = LL_LPUART_STOPBITS_1;
    LPUART_InitStruct.Parity              = LL_LPUART_PARITY_NONE;
    LPUART_InitStruct.TransferDirection   = LL_LPUART_DIRECTION_TX_RX;
    LPUART_InitStruct.HardwareFlowControl = LL_LPUART_HWCONTROL_NONE;
    // LPUART_InitStruct.OverSampling        = LL_LPUART_OVERSAMPLING_16;
    while (LL_LPUART_Init(SERIAL_COM, &LPUART_InitStruct) != SUCCESS)
        ;
    LL_LPUART_Enable(SERIAL_COM);

    ///////////////////////////////
    // DMA SERIAL Init
    ///////////////////////////////
    LL_DMA_DeInit(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL);
    LL_DMA_DeInit(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL);

    SERIAL_RX_DMA_CLOCK_ENABLE();
    SERIAL_TX_DMA_CLOCK_ENABLE();

    // RX DMA
    LL_DMA_DisableChannel(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL);
    LL_DMA_SetDataTransferDirection(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetChannelPriorityLevel(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL, LL_DMA_MODE_CIRCULAR);
    LL_DMA_SetPeriphIncMode(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetPeriphRequest(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL, SERIAL_RX_DMA_REQUEST);

    // Prepare buffer
    LL_DMA_SetPeriphAddress(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL, (uint32_t)&SERIAL_COM->RDR);
    LL_DMA_SetDataLength(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL, buffer_size);
    LL_DMA_SetMemoryAddress(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL, (uint32_t)rx_buffer);
    LL_LPUART_EnableDMAReq_RX(SERIAL_COM);
    LL_DMA_EnableChannel(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL);

    // TX DMA
    LL_DMA_SetDataTransferDirection(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetChannelPriorityLevel(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetPeriphRequest(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL, SERIAL_TX_DMA_REQUEST);

    // Prepare buffer
    LL_DMA_SetPeriphAddress(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL, (uint32_t)&SERIAL_COM->TDR);
    LL_LPUART_EnableDMAReq_TX(SERIAL_COM);
    HAL_NVIC_EnableIRQ(SERIAL_TX_DMA_IRQ);
    HAL_NVIC_SetPriority(SERIAL_TX_DMA_IRQ, 1, 1);

    LL_DMA_EnableIT_TC(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL);
}

/******************************************************************************
 * @brief Loop of the Serial communication
 * @param None
 * @return None
 ******************************************************************************/
void SerialHAL_Loop(void)
{
    uint16_t size               = 0;
    uint16_t RX_PointerPosition = 0;

    RX_PointerPosition = rx_buffer_size - LL_DMA_GetDataLength(SERIAL_RX_DMA, SERIAL_RX_DMA_CHANNEL);
    if (RX_PointerPosition != RX_PrevPointerPosition)
    {
        if (SERIAL_RX_DMA_TC(SERIAL_RX_DMA) != RESET) // DMA buffer overflow
        {
            SERIAL_RX_DMA_CLEAR_TC(SERIAL_RX_DMA);
            size = (rx_buffer_size - RX_PrevPointerPosition) + RX_PointerPosition;
        }
        else
        {
            size = RX_PointerPosition - RX_PrevPointerPosition;
        }
        RX_PrevPointerPosition = RX_PointerPosition;
        // Send the received data and size to the serial stack to deencapsulate it and send it to luos_phy
        Serial_ReceptionIncrease(size);
    }
}
/******************************************************************************
 * @brief Initialisation of the Serial communication
 * @param data pointer of the data to send
 * @param size size of the data to send
 * @return None
 ******************************************************************************/
void SerialHAL_Send(uint8_t *data, uint16_t size)
{
    LL_DMA_DisableChannel(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL);
    LL_DMA_SetMemoryAddress(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL, (uint32_t)data);
    LL_DMA_SetDataLength(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL, size);
    LL_DMA_EnableChannel(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL);
}

/******************************************************************************
 * @brief TX DMA IRQ handler
 * @param None
 * @return None
 ******************************************************************************/
void SERIAL_TX_DMA_IRQHANDLER()
{
    // Check if TX DMA is finished
    if ((SERIAL_TX_DMA_TC(SERIAL_TX_DMA) != RESET) && (LL_DMA_IsEnabledIT_TC(SERIAL_TX_DMA, SERIAL_TX_DMA_CHANNEL) != RESET))
    {
        // Clear the DMA irq flags
        SERIAL_TX_DMA_CLEAR_TC(SERIAL_TX_DMA);
        // Call the callback managing the end of the transmission
        Serial_TransmissionEnd();
    }
}

uint8_t SerialHAL_GetPort(void)
{
    // We may return the USARt peeripheral number as port number
    // For now let's just consider that the port is 0
    return 0;
}
