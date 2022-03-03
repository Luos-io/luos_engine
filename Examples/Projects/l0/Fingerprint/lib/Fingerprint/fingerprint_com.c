/******************************************************************************
 * @file fingerprint_com
 * @brief This is a driver example for a fingerprint sensor. It will work as is
 * but if you want to see it inside of a project, I am using it to create a 
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/STM32F0-Luos-Biometric-Security-System
 * @author MarieBidouille
 * @version 0.0.0
 ******************************************************************************/
#include "fingerprint_com.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
int n_byte = 0;
fingerprint_packet packet_received;
volatile uint16_t size_to_transmit = 0;
volatile uint8_t frame_to_transmit[128];
volatile uint8_t frame_received[128];
volatile uint8_t *fingerprint_tx_data = 0;

/*******************************************************************************
 * Functions
 ******************************************************************************/
void Fingerprint_ComTransmit(void);
void Fingerprint_SetTxState(uint8_t enable);
void Fingerprint_SetRxState(uint8_t enable);
uint8_t FingerprintCom_GetPacket(void);
uint8_t Fingerprint_SendCommand(uint8_t command, uint16_t length, uint8_t data[]);

/******************************************************************************
 * @brief Com init must be called in Drv init
 * @param baudrate
 * @return None
 ******************************************************************************/
void FingerprintCom_Init(uint32_t baudrate)
{
    FINGERPRINT_COM_CLOCK_ENABLE();

    LL_USART_InitTypeDef USART_InitStruct;

    // Initialise USART4
    LL_USART_Disable(FINGERPRINT_COM);
    USART_InitStruct.BaudRate            = baudrate;
    USART_InitStruct.DataWidth           = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits            = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity              = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection   = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling        = LL_USART_OVERSAMPLING_16;
    while (LL_USART_Init(FINGERPRINT_COM, &USART_InitStruct) != SUCCESS)
        ;
    LL_USART_Enable(FINGERPRINT_COM);

    // Enable Reception interrupt
    LL_USART_EnableIT_RXNE(FINGERPRINT_COM);

    HAL_NVIC_EnableIRQ(FINGERPRINT_COM_IRQ);
    HAL_NVIC_SetPriority(FINGERPRINT_COM_IRQ, 0, 1);

    frame_to_transmit[0] = (uint8_t)(FINGERPRINT_STARTCODE >> 8);
    frame_to_transmit[1] = (uint8_t)(FINGERPRINT_STARTCODE & 0xFF);
    frame_to_transmit[2] = (uint8_t)(FINGERPRINT_ADDRESS >> 24);
    frame_to_transmit[3] = (uint8_t)(FINGERPRINT_ADDRESS >> 16);
    frame_to_transmit[4] = (uint8_t)(FINGERPRINT_ADDRESS >> 8);
    frame_to_transmit[5] = (uint8_t)(FINGERPRINT_ADDRESS & 0xFF);
    frame_to_transmit[6] = FINGERPRINT_COMMANDPACKET;
}

/******************************************************************************
 * @brief Set Tx state for Fingerprint com
 * @param enable
 * @return None
 ******************************************************************************/
void Fingerprint_SetTxState(uint8_t enable)
{
    if (enable == true)
    {
        // Put Tx in push pull
        LL_GPIO_SetPinOutputType(FINGERPRINT_COM_TX_PORT, FINGERPRINT_COM_TX_PIN, LL_GPIO_OUTPUT_PUSHPULL);
    }
    else
    {
        // Put Tx in open drain
        LL_GPIO_SetPinOutputType(FINGERPRINT_COM_TX_PORT, FINGERPRINT_COM_TX_PIN, LL_GPIO_OUTPUT_OPENDRAIN);

        // Stop current transmit operation
        size_to_transmit = 0;
        // Disable Transmission empty buffer interrupt
        LL_USART_DisableIT_TXE(FINGERPRINT_COM);

        // Disable Transmission complete interrupt
        LL_USART_DisableIT_TC(FINGERPRINT_COM);
    }
}

/******************************************************************************
 * @brief Set Rx state for Fingerprint com
 * @param enable
 * @return None
 ******************************************************************************/
void Fingerprint_SetRxState(uint8_t enable)
{
    if (enable == true)
    {
        n_byte = 0;
        LL_USART_RequestRxDataFlush(FINGERPRINT_COM); // Clear data register
        LL_USART_EnableIT_RXNE(FINGERPRINT_COM);      // Enable Rx IT
    }
    else
    {
        LL_USART_DisableIT_RXNE(FINGERPRINT_COM); // Disable Rx IT
    }
}

/******************************************************************************
 * @brief Fingerprint com interruption handler
 * @param None
 * @return  None
 ******************************************************************************/
void FINGERPRINT_COM_IRQHANDLER()
{
    // reception management
    if ((LL_USART_IsActiveFlag_RXNE(FINGERPRINT_COM) != RESET) && (LL_USART_IsEnabledIT_RXNE(FINGERPRINT_COM) != RESET))
    {
        // We receive a byte
        frame_received[n_byte] = LL_USART_ReceiveData8(FINGERPRINT_COM);
        n_byte++;

        if (size_to_transmit == 0)
        {
            FINGERPRINT_COM->ICR = 0xFFFFFFFF;
            return;
        }
    }
    else if (LL_USART_IsActiveFlag_FE(FINGERPRINT_COM) != RESET)
    {
        // Framing ERROR
    }

    // Transmission management
    if ((LL_USART_IsActiveFlag_TC(FINGERPRINT_COM) != RESET) && (LL_USART_IsEnabledIT_TC(FINGERPRINT_COM) != RESET))
    {
        // Transmission complete
        // Switch to reception mode
        Fingerprint_SetTxState(false);
        Fingerprint_SetRxState(true);
        // Disable transmission complete IRQ
        LL_USART_ClearFlag_TC(FINGERPRINT_COM);
        LL_USART_DisableIT_TC(FINGERPRINT_COM);
    }

    else if ((LL_USART_IsActiveFlag_TXE(FINGERPRINT_COM) != RESET) && (LL_USART_IsEnabledIT_TXE(FINGERPRINT_COM) != RESET))
    {
        // Transmit buffer empty (this is a software DMA)
        size_to_transmit--;
        LL_USART_TransmitData8(FINGERPRINT_COM, *(fingerprint_tx_data++));

        if (size_to_transmit == 0)
        {
            // Transmission complete, stop loading data and watch for the end of transmission
            // Disable Transmission empty buffer interrupt
            LL_USART_DisableIT_TXE(FINGERPRINT_COM);
            // Enable Transmission complete interrupt
            LL_USART_EnableIT_TC(FINGERPRINT_COM);
            Fingerprint_SetTxState(false);
            Fingerprint_SetRxState(true);
        }
    }
    FINGERPRINT_COM->ICR = 0xFFFFFFFF;
}

/******************************************************************************
 * @brief Transmit the frame to the sensor
 * @param None
 * @return None
 ******************************************************************************/
void Fingerprint_ComTransmit(void)
{
    while (LL_USART_IsActiveFlag_TXE(FINGERPRINT_COM) == RESET)
        ;

    // Reduce size by one because we send one directly
    size_to_transmit--;
    fingerprint_tx_data = frame_to_transmit;

    // Send the first byte
    LL_USART_TransmitData8(FINGERPRINT_COM, *(fingerprint_tx_data++));
    // Enable Transmission empty buffer interrupt to transmit next datas
    LL_USART_EnableIT_TXE(FINGERPRINT_COM);
    // Disable Transmission complete interrupt
    LL_USART_DisableIT_TC(FINGERPRINT_COM);
}

/******************************************************************************
 * @brief Complete the frame to be sent to the sensor
 * @param command
 * @param length
 * @param data
 * @return FINGERPRINT_PACKETRECIEVEERR if response packet wasn't processed successfully
 ******************************************************************************/
uint8_t Fingerprint_SendCommand(uint8_t command, uint16_t length, uint8_t data[])
{
    length = (length < 64) ? length : 64; //capacity

    uint16_t wire_length = length + 2;

    frame_to_transmit[7] = (uint8_t)(wire_length >> 8);
    frame_to_transmit[8] = (uint8_t)(wire_length & 0xFF);
    frame_to_transmit[9] = command;

    uint16_t sum = frame_to_transmit[9] + frame_to_transmit[7] + frame_to_transmit[8] + frame_to_transmit[6];

    for (uint8_t i = 0; i < (length - 1); i++)
    {
        frame_to_transmit[10 + i] = data[i];
        sum += data[i];
    }

    frame_to_transmit[9 + length]  = (uint8_t)(sum >> 8);
    frame_to_transmit[10 + length] = (uint8_t)(sum & 0xFF);

    size_to_transmit = 11 + length;

    Fingerprint_ComTransmit();

    HAL_Delay(500);

    if ((LL_USART_IsActiveFlag_RXNE(FINGERPRINT_COM) == RESET) && (LL_USART_IsEnabledIT_RXNE(FINGERPRINT_COM) != RESET))
    {
        if (FingerprintCom_GetPacket() != FINGERPRINT_OK)
            return FINGERPRINT_PACKETRECIEVEERR;

        if (packet_received.type != FINGERPRINT_ACKPACKET)
            return FINGERPRINT_PACKETRECIEVEERR;
    }
    return FINGERPRINT_PACKETRECIEVEERR;
}

/******************************************************************************
 * @brief Process the frame sent by the sensor
 * @param None
 * @return FINGERPRINT_BADPACKET or FINGERPRINT_OK depending on wheter the packet 
 *  was sent and processed successfully or not 
 ******************************************************************************/
uint8_t FingerprintCom_GetPacket(void)
{
    uint8_t id_byte = 0;
    while (1)
    {
        uint8_t byte = frame_received[id_byte];
        switch (id_byte)
        {
            case 0:
                if (byte == (FINGERPRINT_STARTCODE >> 8))
                    packet_received.start_code = (uint16_t)byte << 8;
                break;

            case 1:
                packet_received.start_code |= byte;
                if (packet_received.start_code != FINGERPRINT_STARTCODE)
                {
                    id_byte = 0;
                    return FINGERPRINT_BADPACKET;
                }
                break;

            case 2:
            case 3:
            case 4:
            case 5:
                packet_received.address[id_byte - 2] = byte;
                break;

            case 6:
                packet_received.type = byte;
                break;

            case 7:
                packet_received.length = (uint16_t)byte << 8;
                break;

            case 8:
                packet_received.length |= byte;
                break;

            default:
                packet_received.data[id_byte - 9] = byte;
                if ((id_byte - 8) == packet_received.length)
                {
                    id_byte = 0;
                    return FINGERPRINT_OK;
                }
                break;
        }
        id_byte++;
    }
    return FINGERPRINT_BADPACKET;
}

/******************************************************************************
 * @brief Returns the number of templates stored in the sensor
 * @param None
 * @return template_count, the number of templates stored in the sensor
 ******************************************************************************/
uint16_t Fingerprint_GetTemplateCount(void)
{
    Fingerprint_SendCommand(FINGERPRINT_TEMPLATECOUNT, 1, 0);

    uint16_t template_count = packet_received.data[1];
    template_count <<= 8;
    template_count |= packet_received.data[2];
    return template_count;
}

/******************************************************************************
 * @brief Turn on or off the built-in led
 * @param on      
 * @return FINGERPRINT_OK or an error 
 ******************************************************************************/
uint8_t Fingerprint_LEDcontrol(uint8_t on)
{
    Fingerprint_SendCommand(((on) ? FINGERPRINT_LEDON : FINGERPRINT_LEDOFF), 1, 0);
    return packet_received.data[0];
}

/******************************************************************************
 * @brief Ask the sensor to take an image of the finger pressed on surface
 * @param None
 * @return FINGERPRINT_OK or an error 
 ******************************************************************************/
uint8_t Fingerprint_GetImage(void)
{
    Fingerprint_SendCommand(FINGERPRINT_GETIMAGE, 1, 0);
    return packet_received.data[0];
}

/******************************************************************************
 * @brief Ask the sensor to convert image to feature template
 * @param slot
 * @return FINGERPRINT_OK or an error 
 ******************************************************************************/
uint8_t Fingerprint_Image2Tz(uint8_t slot)
{
    uint8_t *data = {slot};
    Fingerprint_SendCommand(FINGERPRINT_IMAGE2TZ, 2, data);
    return packet_received.data[0];
}

/******************************************************************************
 * @brief Ask the sensor to take two print feature template and create a model
 * @param None
 * @return FINGERPRINT_OK or an error 
 ******************************************************************************/
uint8_t Fingerprint_CreateModel(void)
{
    Fingerprint_SendCommand(FINGERPRINT_REGMODEL, 1, 0);
    return packet_received.data[0];
}

/******************************************************************************
 * @brief Ask the sensor to store the calculated model for later matching
 * @param location
 * @return FINGERPRINT_OK or an error 
 ******************************************************************************/
uint8_t Fingerprint_StoreModel(uint16_t location)
{
    uint8_t *data = {0x01, (uint8_t)(location >> 8), (uint8_t)(location & 0xFF)};
    Fingerprint_SendCommand(FINGERPRINT_STORE, 4, data);
    return packet_received.data[0];
}

/******************************************************************************
 * @brief Ask the sensor to delete ALL models in memory
 * @param None
 * @return FINGERPRINT_OK or an error 
 ******************************************************************************/
uint8_t Fingerprint_EmptyDatabase(void)
{
    Fingerprint_SendCommand(FINGERPRINT_EMPTY, 1, 0);
    return packet_received.data[0];
}

/******************************************************************************
 * @brief Ask the sensor to search the current slot fingerprint features to match saved templates.
 * @param None
 * @return FINGERPRINT_OK or an error 
 ******************************************************************************/
uint8_t Fingerprint_SearchFinger(void)
{
    uint8_t *data = {0x01, 0x00, 0x00, (uint8_t)(FINGERPRINT_CAPACITY >> 8), (uint8_t)(FINGERPRINT_CAPACITY & 0xFF)};
    Fingerprint_SendCommand(FINGERPRINT_SEARCH, 6, data);
    return packet_received.data[0];
}