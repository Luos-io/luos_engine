/******************************************************************************
 * @file _pipe
 * @brief Private pipe functions
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "streaming.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef PIPE_SERIAL_BAUDRATE
    #define PIPE_SERIAL_BAUDRATE 1000000
#endif
#ifndef PIPE_TX_BUFFER_SIZE
    #define PIPE_TX_BUFFER_SIZE 1024
#endif
#ifndef PIPE_RX_BUFFER_SIZE
    #define PIPE_RX_BUFFER_SIZE 2048
#endif

/*******************************************************************************
 * Function
 ******************************************************************************/
streaming_channel_t *Pipe_GetTxStreamChannel(void);
streaming_channel_t *Pipe_GetRxStreamChannel(void);