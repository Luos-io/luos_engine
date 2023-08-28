/******************************************************************************
 * @file serial_hal
 * @brief serial communication hardware abstraction layer
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <Arduino.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include "serial_network_hal.h"
#include "_serial_network.h"
#ifdef __cplusplus
}
#endif

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
    Serial.begin(SERIAL_NETWORK_BAUDRATE);
}

/******************************************************************************
 * @brief Loop of the Serial communication
 * @param None
 * @return None
 ******************************************************************************/
void SerialHAL_Loop(void)
{
    uint8_t data = 0;
    while (Serial.available() > 0)
    {
        data = Serial.read();
        Serial_ReceptionAdd(&data, 1);
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
    Serial.write(data, size);
    // Call the callback managing the end of the transmission
    Serial_TransmissionEnd();
}

uint8_t SerialHAL_GetPort(void)
{
    // We may return the serial port number here
    // For now let's just consider that the port is 0
    return 0;
}
