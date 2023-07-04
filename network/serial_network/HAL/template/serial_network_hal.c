/******************************************************************************
 * @file serial_hal
 * @brief serial communication hardware abstraction layer
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "serial_network_hal.h"
#include "_serial_network.h"
// TODO: Include the needed files

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef SERIAL_COM_DEFAULTBAUDRATE
    #define SERIAL_COM_DEFAULTBAUDRATE 1000000
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t rx_buffer_size;
// TODO: Declare the needed variables

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
    rx_buffer_size = buffer_size;
    // Init the serial peripheral
    // TODO: Init the used usart peripheral using the SERIAL_COM_DEFAULTBAUDRATE baudrate.
    // TODO: Init TX pin with the given pin and port and set it as tx for the usart. Select the appropriate alternate function.
    // TODO: Init the Serial TX DMA interface, use it in normal mode and make it raise an IRQ when finished.

    // TODO: Init RX pin with the given pin and port and set it as rx for the usart. Select the appropriate alternate function.
    // TODO: Init the serial RX DMA interface to move RX bytes in the rx_buffer, use it in circular mode and make it loop on buffer_size. Do not make it raise an IRQ when finished.
    // TODO: Enable the serial RX Idle IRQ to raise an interrupt when a the RX line is idle
}

/******************************************************************************
 * @brief Loop of the Serial communication
 * @param None
 * @return None
 ******************************************************************************/
void SerialHAL_Loop(void)
{
    // If you want to receive data here, you can.
}

/******************************************************************************
 * @brief Initialisation of the Serial communication
 * @param data pointer of the data to send
 * @param size size of the data to send
 * @return None
 ******************************************************************************/
void SerialHAL_Send(uint8_t *data, uint16_t size)
{
    // TODO: Start the DMA transfert to transmit data from data[0] to data[size]
}

/******************************************************************************
 * @brief RX Serial IRQ handler
 * @param None
 * @return None
 ******************************************************************************/
void SERIAL_COM_IRQHANDLER(void)
{
    // Check if this IRQ is due to RX Idle
    if (/*TODO: Check if this IRQ means that RX is Idle*/)
    {
        // TODO: Clear the RX idle IRQ flag
        uint32_t received_data = rx_buffer_size - /* TODO: Get the size of the data received by the DMA*/;
        // Send the received data and size to the serial stack to deencapsulate it and send it to luos_phy
        Serial_ReceptionEnd(received_data);
    }
}

/******************************************************************************
 * @brief TX DMA IRQ handler
 * @param None
 * @return None
 ******************************************************************************/
void SERIAL_TX_DMA_IRQHANDLER()
{
    // Check if TX DMA is finished
    if (/* TODO: Check if this irq means that TX DMA is finished*/)
    {
        // TODO: Clear the DMA irq flags
        // Call the callback managing the end of the transmission
        Serial_TransmissionEnd();
    }
}
