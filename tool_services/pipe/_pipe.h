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
    #define PIPE_SERIAL_BAUDRATE 3000000
#endif
#ifndef PIPE_TX_BUFFER_SIZE
    #define PIPE_TX_BUFFER_SIZE 1024
#endif
#ifndef PIPE_RX_BUFFER_SIZE
    #define PIPE_RX_BUFFER_SIZE 2048
#endif
#ifdef PIPE_TO_LUOS_BUFFER_SIZE
    #warning "PIPE_TO_LUOS_BUFFER_SIZE is deprecated, use PIPE_RX_BUFFER_SIZE instead"
#endif
#ifdef LUOS_TO_PIPE_BUFFER_SIZE
    #warning "LUOS_TO_PIPE_BUFFER_SIZE is deprecated, use PIPE_TX_BUFFER_SIZE instead"
#endif

/*******************************************************************************
 * Function
 ******************************************************************************/
streaming_channel_t *Pipe_GetTxStreamChannel(void);
streaming_channel_t *Pipe_GetRxStreamChannel(void);
