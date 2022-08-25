/******************************************************************************
 * @file pipe_com
 * @brief communication driver
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdbool.h>
#include "pipe_com.h"
#include "luos_utils.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint8_t is_sending               = false;
volatile uint16_t size_to_send            = 0;
volatile uint16_t P2L_PrevPointerPosition = 0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void PipeCom_DMAInit(void);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_Init(void)
{
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

    P2L_PrevPointerPosition = 0;
    PipeCom_DMAInit();
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
static void PipeCom_DMAInit(void)
{
    P2L_DMA_CLOCK_ENABLE();
    L2P_DMA_CLOCK_ENABLE();

    // Pipe to Luos
    LL_DMA_DisableChannel(P2L_DMA, P2L_DMA_CHANNEL);
    LL_DMA_SetDataTransferDirection(P2L_DMA, P2L_DMA_CHANNEL, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetChannelPriorityLevel(P2L_DMA, P2L_DMA_CHANNEL, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(P2L_DMA, P2L_DMA_CHANNEL, LL_DMA_MODE_CIRCULAR);
    LL_DMA_SetPeriphIncMode(P2L_DMA, P2L_DMA_CHANNEL, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(P2L_DMA, P2L_DMA_CHANNEL, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(P2L_DMA, P2L_DMA_CHANNEL, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(P2L_DMA, P2L_DMA_CHANNEL, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetPeriphRequest(P2L_DMA, P2L_DMA_CHANNEL, P2L_DMA_REQUEST);

    // Prepare buffer
    LL_DMA_SetPeriphAddress(P2L_DMA, P2L_DMA_CHANNEL, (uint32_t)&PIPE_COM->RDR);
    LL_DMA_SetDataLength(P2L_DMA, P2L_DMA_CHANNEL, PIPE_TO_LUOS_BUFFER_SIZE);
    LL_DMA_SetMemoryAddress(P2L_DMA, P2L_DMA_CHANNEL, (uint32_t)PipeBuffer_GetP2LBuffer());
    LL_LPUART_EnableDMAReq_RX(PIPE_COM);
    LL_DMA_EnableChannel(P2L_DMA, P2L_DMA_CHANNEL);

    // Luos to Pipe
    LL_DMA_SetDataTransferDirection(L2P_DMA, L2P_DMA_CHANNEL, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetChannelPriorityLevel(L2P_DMA, L2P_DMA_CHANNEL, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(L2P_DMA, L2P_DMA_CHANNEL, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(L2P_DMA, L2P_DMA_CHANNEL, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(L2P_DMA, L2P_DMA_CHANNEL, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(L2P_DMA, L2P_DMA_CHANNEL, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(L2P_DMA, L2P_DMA_CHANNEL, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetPeriphRequest(L2P_DMA, L2P_DMA_CHANNEL, L2P_DMA_REQUEST);

    // Prepare buffer
    LL_DMA_SetPeriphAddress(L2P_DMA, L2P_DMA_CHANNEL, (uint32_t)&PIPE_COM->TDR);
    LL_LPUART_EnableDMAReq_TX(PIPE_COM);
    HAL_NVIC_EnableIRQ(L2P_DMA_IRQ);
    HAL_NVIC_SetPriority(L2P_DMA_IRQ, 1, 1);

    LL_DMA_EnableIT_TC(L2P_DMA, L2P_DMA_CHANNEL);
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_SendL2P(uint8_t *data, uint16_t size)
{
    LUOS_ASSERT(size > 0);
    is_sending   = true;
    size_to_send = size;
    LL_DMA_DisableChannel(L2P_DMA, L2P_DMA_CHANNEL);
    LL_DMA_SetMemoryAddress(L2P_DMA, L2P_DMA_CHANNEL, (uint32_t)data);
    LL_DMA_SetDataLength(L2P_DMA, L2P_DMA_CHANNEL, size);
    LL_DMA_EnableChannel(L2P_DMA, L2P_DMA_CHANNEL);
} /******************************************************************************
   * @brief loop must be call in project loop
   * @param None
   * @return None
   ******************************************************************************/
void PipeCom_Loop(void)
{
}
/******************************************************************************
 * @brief check if pipe is sending
 * @param None
 * @return true/false
 ******************************************************************************/
volatile uint8_t PipeCom_SendL2PPending(void)
{
    return is_sending;
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
        if (LL_DMA_GetDataLength(P2L_DMA, P2L_DMA_CHANNEL) == 0)
        {
            return;
        }

        P2L_PointerPosition = PIPE_TO_LUOS_BUFFER_SIZE - LL_DMA_GetDataLength(P2L_DMA, P2L_DMA_CHANNEL);

        if (P2L_DMA_TC(P2L_DMA) != RESET) // DMA buffer overflow
        {
            P2L_DMA_CLEAR_TC(P2L_DMA);
            size = (PIPE_TO_LUOS_BUFFER_SIZE - P2L_PrevPointerPosition) + P2L_PointerPosition;
        }
        else
        {
            size = P2L_PointerPosition - P2L_PrevPointerPosition;
        }
        P2L_PrevPointerPosition = P2L_PointerPosition;
        if (size != 0)
        {
            Stream_AddAvailableSampleNB(get_P2L_StreamChannel(), size);
        }
    }
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void L2P_DMA_IRQHANDLER()
{
    uint16_t size = 0;
    // check if we receive an IDLE on usart3
    if ((L2P_DMA_TC(L2P_DMA) != RESET) && (LL_DMA_IsEnabledIT_TC(L2P_DMA, L2P_DMA_CHANNEL) != RESET))
    {
        L2P_DMA_CLEAR_TC(L2P_DMA);

        Stream_RmvAvailableSampleNB(get_L2P_StreamChannel(), size_to_send);
        size = Stream_GetAvailableSampleNBUntilEndBuffer(get_L2P_StreamChannel());
        if (size > 0)
        {
            PipeCom_SendL2P(get_L2P_StreamChannel()->sample_ptr, size);
        }
        else
        {
            is_sending = false;
        }
    }
}
