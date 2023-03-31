/******************************************************************************
 * @file pipe_com
 * @brief communication driver
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdbool.h>
#include "pipe_com.h"
#include "luos_utils.h"
#include "../serial_protocol.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint8_t is_sending              = false;
volatile uint16_t size_to_send           = 0;
volatile uint16_t RX_PrevPointerPosition = 0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void PipeCom_DMAInit(void);
static void PipeCom_SerialSend(void);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_Init(void)
{
    LL_USART_DeInit(PIPE_COM);
    ///////////////////////////////
    // GPIO PIPE Init
    ///////////////////////////////
    PIPE_TX_CLK();
    PIPE_RX_CLK();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // TX
    GPIO_InitStruct.Pin       = PIPE_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = PIPE_TX_AF;
    HAL_GPIO_Init(PIPE_TX_PORT, &GPIO_InitStruct);
    // RX
    GPIO_InitStruct.Pin       = PIPE_RX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = PIPE_RX_AF;
    HAL_GPIO_Init(PIPE_RX_PORT, &GPIO_InitStruct);

    ///////////////////////////////
    // LPUART PIPE Init
    ///////////////////////////////
    PIPE_COM_CLOCK_ENABLE();

    LL_LPUART_InitTypeDef LPUART_InitStruct;
    // Initialise USART3
    LL_LPUART_Disable(PIPE_COM);
    LPUART_InitStruct.BaudRate            = PIPE_SERIAL_BAUDRATE;
    LPUART_InitStruct.PrescalerValue      = LL_LPUART_PRESCALER_DIV1;
    LPUART_InitStruct.DataWidth           = LL_LPUART_DATAWIDTH_8B;
    LPUART_InitStruct.StopBits            = LL_LPUART_STOPBITS_1;
    LPUART_InitStruct.Parity              = LL_LPUART_PARITY_NONE;
    LPUART_InitStruct.TransferDirection   = LL_LPUART_DIRECTION_TX_RX;
    LPUART_InitStruct.HardwareFlowControl = LL_LPUART_HWCONTROL_NONE;
    while (LL_LPUART_Init(PIPE_COM, &LPUART_InitStruct) != SUCCESS)
        ;
    LL_LPUART_Enable(PIPE_COM);

    LL_LPUART_ClearFlag_IDLE(PIPE_COM);
    LL_LPUART_EnableIT_IDLE(PIPE_COM);

    HAL_NVIC_EnableIRQ(PIPE_COM_IRQ);
    HAL_NVIC_SetPriority(PIPE_COM_IRQ, 1, 1);

    RX_PrevPointerPosition = 0;
    PipeCom_DMAInit();
    SerialProtocol_Init();
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
static void PipeCom_DMAInit(void)
{
    LL_DMA_DeInit(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL);
    LL_DMA_DeInit(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL);

    PIPE_RX_DMA_CLOCK_ENABLE();
    PIPE_TX_DMA_CLOCK_ENABLE();

    // Pipe to Luos
    LL_DMA_DisableChannel(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL);
    LL_DMA_SetDataTransferDirection(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetChannelPriorityLevel(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL, LL_DMA_MODE_CIRCULAR);
    LL_DMA_SetPeriphIncMode(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetPeriphRequest(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL, PIPE_RX_DMA_REQUEST);

    // Prepare buffer
    LL_DMA_SetPeriphAddress(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL, (uint32_t)&PIPE_COM->RDR);
    LL_DMA_SetDataLength(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL, PIPE_RX_BUFFER_SIZE);
    LL_DMA_SetMemoryAddress(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL, (uint32_t)Pipe_GetRxStreamChannel()->ring_buffer);
    LL_LPUART_EnableDMAReq_RX(PIPE_COM);
    LL_DMA_EnableChannel(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL);

    // Luos to Pipe
    LL_DMA_SetDataTransferDirection(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetChannelPriorityLevel(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetPeriphRequest(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL, PIPE_TX_DMA_REQUEST);

    // Prepare buffer
    LL_DMA_SetPeriphAddress(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL, (uint32_t)&PIPE_COM->TDR);
    LL_LPUART_EnableDMAReq_TX(PIPE_COM);
    HAL_NVIC_EnableIRQ(PIPE_TX_DMA_IRQ);
    HAL_NVIC_SetPriority(PIPE_TX_DMA_IRQ, 1, 1);

    LL_DMA_EnableIT_TC(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_Loop(void)
{
}
/******************************************************************************
 * @brief Check if a message is available
 * @param None
 * @return None
 ******************************************************************************/
uint8_t PipeCom_Receive(uint16_t *size)
{
    return SerialProtocol_IsMsgComplete(size);
}
/******************************************************************************
 * @brief Create msg and send it
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_Send(void)
{
    SerialProtocol_CreateTxMsg();
    PipeCom_SerialSend();
}
/******************************************************************************
 * @brief Send msg on serial Pipe
 * @param None
 * @return None
 ******************************************************************************/
static void PipeCom_SerialSend(void)
{
    if (is_sending == false)
    {
        is_sending   = true;
        size_to_send = SerialProtocol_GetSizeToSend();
        LL_DMA_DisableChannel(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL);
        LL_DMA_SetMemoryAddress(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL, (uint32_t)SerialProtocol_GetDataToSend());
        LL_DMA_SetDataLength(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL, size_to_send);
        LL_DMA_EnableChannel(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL);
    }
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PIPE_COM_IRQHANDLER()
{
    uint16_t size                = 0;
    uint16_t P2L_PointerPosition = 0;

    // check if we receive an IDLE on usart3
    if (LL_LPUART_IsActiveFlag_IDLE(PIPE_COM))
    {
        LL_LPUART_ClearFlag_IDLE(PIPE_COM);
        if (LL_DMA_GetDataLength(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL) == 0)
        {
            return;
        }

        P2L_PointerPosition = PIPE_RX_BUFFER_SIZE - LL_DMA_GetDataLength(PIPE_RX_DMA, PIPE_RX_DMA_CHANNEL);

        if (PIPE_RX_DMA_TC(PIPE_RX_DMA) != RESET) // DMA buffer overflow
        {
            PIPE_RX_DMA_CLEAR_TC(PIPE_RX_DMA);
            size = (PIPE_RX_BUFFER_SIZE - RX_PrevPointerPosition) + P2L_PointerPosition;
        }
        else
        {
            size = P2L_PointerPosition - RX_PrevPointerPosition;
        }
        RX_PrevPointerPosition = P2L_PointerPosition;
        if (size != 0)
        {
            Streaming_AddAvailableSampleNB(Pipe_GetRxStreamChannel(), size);
        }
    }
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PIPE_TX_DMA_IRQHANDLER()
{
    uint16_t size = 0;
    // check if we receive an IDLE on usart3
    if ((PIPE_TX_DMA_TC(PIPE_TX_DMA) != RESET) && (LL_DMA_IsEnabledIT_TC(PIPE_TX_DMA, PIPE_TX_DMA_CHANNEL) != RESET))
    {
        PIPE_TX_DMA_CLEAR_TC(PIPE_TX_DMA);

        Streaming_RmvAvailableSampleNB(Pipe_GetTxStreamChannel(), size_to_send);
        size       = SerialProtocol_GetSizeToSend();
        is_sending = false;
        if (size > 0)
        {
            PipeCom_SerialSend();
        }
    }
}
