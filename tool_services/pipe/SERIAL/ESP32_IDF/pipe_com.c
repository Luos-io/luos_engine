/******************************************************************************
 * @file pipe_com
 * @brief communication driver
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "pipe_com.h"
#include "../serial_protocol.h"

// ESP32
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "hal/uart_hal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uart_hal_context_t uart_hal_context_log = {
    .dev = UART_LL_GET_HW(UART_NUM_0)};
/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_Init(void)
{
    SerialProtocol_Init();
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_Loop(void)
{
    int size = uart_hal_get_rxfifo_len(&uart_hal_context_log);
    uint8_t data[256];
    if (size != 0)
    {
        uart_hal_read_rxfifo(&uart_hal_context_log, &data[0], &size);
        uart_hal_rxfifo_rst(&uart_hal_context_log);
        Streaming_PutSample(Pipe_GetRxStreamChannel(), &data[0], size);
    }
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
 * @brief PipeCom_Send
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_Send(void)
{
    uint32_t wr_size = 0;

    SerialProtocol_CreateTxMsg();
    volatile uint16_t size = SerialProtocol_GetSizeToSend();
    while (size > 0)
    {
        uart_hal_write_txfifo(&uart_hal_context_log, (uint8_t *)SerialProtocol_GetDataToSend(), size, &wr_size);
        Streaming_RmvAvailableSampleNB(Pipe_GetTxStreamChannel(), wr_size);
        size = SerialProtocol_GetSizeToSend();
    }
}
