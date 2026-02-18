/******************************************************************************
 * @file node_config.h
 * @brief This file allow you to use standard preprocessor definitions to
 *        configure your project, Luos and Luos HAL libraries
 *
 *   # Introduction
 *     This file is for the luos user. You may here configure your project and
 *     define your custom Luos service and custom Luos command for your product
 *
 *     Luos libraries offer a minimal standard configuration to optimize
 *     memory usage. In some case you have to modify standard value to fit
 *     with your need concerning among of data transiting through the network
 *     or network speed for example
 *
 *     Luos libraries can be use with a lot a MCU family. Luos compagny give you
 *     a default configuration, for specific MCU family, in robus_hal_config.h.
 *     This configuration can be modify here to fit with you design by
 *     preprocessor definitions of MCU Hardware needs
 *
 *   # Usage
 *      This file should be place a the root folder of your project and include
 *      where build flag preprocessor definitions are define in your IDE
 *      -include node_config.h
 *
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _NODE_CONFIG_H_
#define _NODE_CONFIG_H_

/*******************************************************************************
 * PROJECT DEFINITION
 *******************************************************************************/

/*******************************************************************************
 * LUOS LIBRARY DEFINITION
 *******************************************************************************
 *    Define                | Default Value              | Description
 *    :---------------------|------------------------------------------------------
 *    MAX_LOCAL_SERVICE_NUMBER    |              5             | Service number in the node
 *    MSG_BUFFER_SIZE       | 3*SIZE_MSG_MAX (405 Bytes) | Size in byte of the Luos buffer TX and RX
 *    MAX_MSG_NB            |   2*MAX_LOCAL_SERVICE_NUMBER     | Message number in Luos buffer
 *    MAX_NODE_NUMBER       |              20            | Node number in the device
 *    MAX_SERVICE_NUMBER    |              20            | Service number in the device
 *    NBR_PORT              |              2             | PTP Branch number Max 8
 *    NBR_RETRY             |             10             | Send Retry number in case of NACK or collision
 ******************************************************************************/

#define MAX_LOCAL_SERVICE_NUMBER 2
#define MSG_BUFFER_SIZE          1024
#define MAX_MSG_NB               40
/*******************************************************************************
 * LUOS HAL LIBRARY DEFINITION
*******************************************************************************
 *    Define                  | Description
 *    :-----------------------|-----------------------------------------------
 *    MCUFREQ                 | Put your the MCU frequency (value in Hz)
 *    TIMERDIV                | Timer divider clock (see your clock configuration)
 *    USE_CRC_HW              | define to 0 if there is no Module CRC in your MCU
 *    USE_TX_IT               | define to 1 to not use DMA transfers for Luos Tx
 *
 *    PORT_CLOCK_ENABLE       | Enable clock for port
 *    PTPx                    | A,B,C,D etc. PTP Branch Pin/Port/IRQ
 *    TX_LOCK_DETECT          | Disable by default use if not busy flag in USART Pin/Port/IRQ
 *    RX_EN                   | Rx enable for driver RS485 always on Pin/Port
 *    TX_EN                   | Tx enable for driver RS485 Pin/Port
 *    COM_TX                  | Tx USART Com Pin/Port/Alternate
 *    COM_RX                  | Rx USART Com Pin/Port/Alternate
 *    PINOUT_IRQHANDLER       | Callback function for Pin IRQ handler

 *    ROBUS_COM_CLOCK_ENABLE   | Enable clock for USART
 *    ROBUS_COM                | USART number
 *    ROBUS_COM_IRQ            | USART IRQ number
 *    ROBUS_COM_IRQHANDLER     | Callback function for USART IRQ handler

 *    ROBUS_DMA_CLOCK_ENABLE   | Enable clock for DMA
 *    ROBUS_DMA                | DMA number
 *    ROBUS_DMA_CHANNEL        | DMA channel (depending on MCU DMA may need special config)

 *    ROBUS_TIMER_CLOCK_ENABLE | Enable clock for Timer
 *    ROBUS_TIMER              | Timer number
 *    ROBUS_TIMER_IRQ          | Timer IRQ number
 *    ROBUS_TIMER_IRQHANDLER   | Callback function for Timer IRQ handler
******************************************************************************/

/*******************************************************************************
 * FLASH CONFIGURATION FOR APP WITH BOOTLOADER
 ********************************************************************************
 *    Define                | Default Value              | Description
 *    :---------------------|------------------------------------------------------
 *    BOOT_START_ADDRESS    | FLASH_BASE = 0x8000000     | Start address of Bootloader in flash
 *    SHARED_MEMORY_ADDRESS | 0x0800C000                 | Start address of shared memory to save boot flag
 *    APP_START_ADDRESS     | 0x0800C800                 | Start address of application with bootloader
 *    APP_END_ADDRESS       | FLASH_BANK1_END=0x0801FFFF | End address of application with bootloader
 ******************************************************************************/

/*******************************************************************************
 * GATE SERIAL COM DEFINITION
 *******************************************************************************
 *    Define                    | Default Value              | Description
 *    :-------------------------|------------------------------------------------------
 *    MAX_NODE_NUMBER       |              20            | Node number in the device
 *    MAX_SERVICE_NUMBER    |              20            | Service number in the device
 *    GATE_BUFF_SIZE            |             1024           | Json receive buffer size
 *    PIPE_TX_BUFFER_SIZE       |             1024           | Receive pipe buffer size
 *    PIPE_RX_BUFFER_SIZE       |             2048           | Transmit pipe buffer size
 *    INIT_TIME                 |              150           | Wait init time before first detection
 ******************************************************************************/
#define GATE_BUFF_SIZE      1024
#define PIPE_TX_BUFFER_SIZE 2048
#define PIPE_RX_BUFFER_SIZE 2048

/*******************************************************************************
 * OTHER GATE PARAMETERS
 *******************************************************************************
 *    Define         |   Default Value    | Description
 *    :--------------|------------------------------------------------------
 *    INIT_TIME      |      150           | Wait init time before first detection
 * ******************************************************************************/

/*******************************************************************************
 * OTHER PIPE PARAMETERS
 *******************************************************************************
 *    Define               |     Default Value       | Description
 *    :--------------------|----------------------------------------------------
 *    PIPE_CONFIG          |          none           | To Modify pipe config
 * ******************************************************************************/

#endif /* _NODE_CONFIG_H_ */
