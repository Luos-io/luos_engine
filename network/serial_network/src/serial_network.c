/******************************************************************************
 * @file serial_network.c
 * @brief serial communication driver for Luos library
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

/******************************************************************************
 * # Serial Topology management algorythm:
 * +------------+--------------+          +--------------+------------+
 * |   1st node (master)       |          |        2nd node           |
 * +------------+--------------+          +--------------+------------+
 * |  luos_phy  |serial_network|          |serial_network|  luos_phy  |
 * +------------+--------------+          +--------------+------------+
 * |   ping  ---+--> ping      | -------> |    ping   ---+->phy_source|
 * |          <-+--------------+---msgs---+--------------+->          |
 * |            |              |          |              | ping others|
 * | topo_done <+-- de-ping    | <------- |    de-ping <-+--- ping    |
 * | ping others|              |          |       |------+->topo_done |
 * +------------+--------------+          +--------------+------------+
 *  The ping and de-ping are the exact same request and reply sequence. The only difference is the direction of the message.
 *  ping : SerialHeader_t + SERIAL_FOOTER --> answer : SerialHeader_t + 1 byte + SERIAL_FOOTER
 *
 * # Serial protocol:
 *  ----------------------------------------------------------------------
 * | SERIAL_HEADER | size    |           data             | SERIAL_FOOTER |
 *  ----------------------------------------------------------------------
 * |     SerialHeader_t      |
 ******************************************************************************/

#include "luos_phy.h"
#include "serial_network.h"
#include "_serial_network.h"
#include "serial_network_hal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SERIAL_HEADER 0x7E
#define SERIAL_FOOTER 0x81

// Phy callback definitions
static void Serial_JobHandler(luos_phy_t *phy_ptr, phy_job_t *job);
static error_return_t Serial_RunTopology(luos_phy_t *phy_ptr, uint8_t *portId);
static void Serial_Reset(luos_phy_t *phy_ptr);
static void Serial_Send(void);

typedef struct __attribute__((__packed__))
{
    uint8_t header;
    uint16_t size;
} SerialHeader_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
luos_phy_t *phy_serial;
volatile bool sending        = false; // This flag is true when TX is busy
volatile bool wait_reception = false; // This flag is true when we are waiting for a reply to a ping

uint8_t TX_data[sizeof(msg_t) + sizeof(SerialHeader_t) + 1]; // This buffer is used to prepare the message to send
uint8_t RX_data[SERIAL_RX_BUFFER_SIZE];                      // This buffer is used to store received bytes and used as ring buffer
uint32_t rx_size         = 0;
bool we_initiate_ping    = false;
bool next_ping_is_deping = false;

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Initialisation of the Serial communication
 * @param None
 * @return None
 ******************************************************************************/
void Serial_Init(void)
{
    // Instantiate the phy struct
    phy_serial = Phy_Create(Serial_JobHandler, Serial_RunTopology, Serial_Reset);
    LUOS_ASSERT(phy_serial);

    Serial_Reset(phy_serial);
    SerialHAL_Init(RX_data, SERIAL_RX_BUFFER_SIZE);

    phy_serial->rx_timestamp   = 0;
    phy_serial->rx_buffer_base = RX_data;
    phy_serial->rx_data        = RX_data; // In our case we don't need to use this pointer because we use DMA to receive complete messages in one time.
    phy_serial->rx_keep        = true;
}

/******************************************************************************
 * @brief Reset the serial communication variables
 * @return None
 ******************************************************************************/
void Serial_Reset(luos_phy_t *phy_ptr)
{
    // Init the serial communication variables
    sending             = false;
    wait_reception      = false;
    rx_size             = 0;
    we_initiate_ping    = false;
    next_ping_is_deping = false;
}

/******************************************************************************
 * @brief Function called to move the rx buffer pointer
 * @param size size to move the pointer
 * @return None
 ******************************************************************************/
static void Serial_MoveRxPtr(uint8_t size)
{
    // Move the rx buffer pointer
    LUOS_ASSERT(rx_size >= size);
    Phy_SetIrqState(false);
    phy_serial->rx_buffer_base += size;
    if (phy_serial->rx_buffer_base >= RX_data + sizeof(RX_data))
    {
        // We are at the end of the buffer, we need to go back to the beginning
        phy_serial->rx_buffer_base = RX_data + (phy_serial->rx_buffer_base - (RX_data + sizeof(RX_data)));
    }
    phy_serial->rx_data = phy_serial->rx_buffer_base;
    rx_size -= size;
    Phy_SetIrqState(true);
}

/******************************************************************************
 * @brief Loop of the Serial communication
 * @param None
 * @return None
 ******************************************************************************/
void Serial_Loop(void)
{
    static uint32_t timeout_systick = 0;
    SerialHAL_Loop();

    // Manage received data
    while (rx_size > 0)
    {
        if (*phy_serial->rx_buffer_base != SERIAL_HEADER)
        {
            Serial_MoveRxPtr(1);
            continue;
        }
        /***********************************************
         * 1 - Receive the header and check if the message is complete
         *********************************************/
        if (rx_size < sizeof(SerialHeader_t))
        {
            // We don't receive the complete header yet.
            return;
        }
        SerialHeader_t header;
        // Check if the header address is after the end of the buffer
        if (((uintptr_t)RX_data + sizeof(RX_data)) - ((uintptr_t)(phy_serial->rx_buffer_base)) < sizeof(SerialHeader_t))
        {
            // Header is cut by the end of the buffer
            // We need to go back to the beginning of the buffer
            uint8_t buffer_end_size = ((uintptr_t)RX_data + sizeof(RX_data)) - (uintptr_t)(phy_serial->rx_buffer_base);
            memcpy(&header, phy_serial->rx_buffer_base, buffer_end_size);
            memcpy(&header + buffer_end_size, RX_data, sizeof(SerialHeader_t) - buffer_end_size);
        }
        else
        {
            // Header is continuous
            memcpy(&header, phy_serial->rx_buffer_base, sizeof(SerialHeader_t));
        }
        if (header.size >= sizeof(msg_t))
        {
            // This data seems to be corrupted or at least we can't receive it with our buffer size, drop it.
            Serial_MoveRxPtr(1);
        }
        // Now we have the complete header, check if we receive the complete message
        if (rx_size < header.size + sizeof(SerialHeader_t) + 1)
        {
            // We don't receive the complete message yet.
            // Manage a timeout to be sure we are not looking for a wrong message
            if (timeout_systick == 0)
            {
                // Start the timeout counter
                timeout_systick = LuosHAL_GetSystick();
            }
            else
            {
                // We already start the timeout
                if ((LuosHAL_GetSystick() - timeout_systick) > 100)
                {
                    // We spend the 100ms timeout, remove the byte
                    Serial_MoveRxPtr(1);
                    timeout_systick = 0;
                }
            }
            return;
        }
        /***********************************************
         * 2 - Receive the message
         *********************************************/
        timeout_systick          = 0;
        uint8_t *footer_position = &phy_serial->rx_buffer_base[header.size + sizeof(SerialHeader_t)];
        // Check if the footer address is after the end of the buffer
        if ((uintptr_t)footer_position >= (uintptr_t)RX_data + sizeof(RX_data))
        {
            // We need to go back to the beginning of the buffer
            footer_position = (uint8_t *)((uintptr_t)RX_data + ((uintptr_t)footer_position - ((uintptr_t)RX_data + sizeof(RX_data))));
        }
        while ((header.header != SERIAL_HEADER) || (*footer_position != SERIAL_FOOTER))
        {
            // This is not a correct message, we need to go to the next byte
            Serial_MoveRxPtr(1);
            if (rx_size == 0)
            {
                // We don't have data to parse anymore
                return;
            }
        }
        if (rx_size < sizeof(SerialHeader_t))
        {
            // We don't receive the complete message yet.
            return;
        }
        /***********************************************
         * 3 - Process the message
         *********************************************/
        if ((header.size > sizeof(msg_t)) || (header.size < sizeof(header_t)))
        {
            // This is not a standars message.
            if (header.size == 0)
            {
                // This is a ping or a de-ping message
                // - The ping message indicate that a master node is looking for this one
                // - The de-ping message indicate that we are the master node. We already ping this node by calling Serial_RunTopology and we need to consider this branch as done

                // message reply to it
                sending = true;
                Phy_SetIrqState(true);
                // We can send the message
                // Add the encapsulation to the message
                SerialHeader_t header_rply;
                header_rply.header = SERIAL_HEADER;
                // By adding 1 data, this message will be received but not interpreted
                header_rply.size = 1;
                memcpy(TX_data, &header_rply, sizeof(SerialHeader_t));
                TX_data[sizeof(SerialHeader_t)]     = 0;
                TX_data[sizeof(SerialHeader_t) + 1] = SERIAL_FOOTER;

                // Send the message
                SerialHAL_Send(TX_data, sizeof(SerialHeader_t) + 2);

                // Did we receive this ping from a master node or are we the node that initiate the ping?
                if (we_initiate_ping == false)
                {
                    // We receive this ping from a master node
                    // This port become the topology source of this node
                    // Notify luos_phy about it
                    Phy_TopologySource(phy_serial, SerialHAL_GetPort());
                    // The next ping we send will be a deping
                    next_ping_is_deping = true;
                }
                else
                {
                    // We already ping this node, we need to consider this branch as done
                    Phy_TopologyDone(phy_serial);
                    we_initiate_ping = false;
                }
            }
            // This is not a correct message, we need to trash it
            Serial_MoveRxPtr(header.size + sizeof(SerialHeader_t) + 1);
        }
        else
        {
            // This message is correct and have actual data to parse, we can process it
            // First, we need to remove the header encapsulation
            Serial_MoveRxPtr(sizeof(SerialHeader_t));

            uint8_t continuous_data[header.size];
            uint8_t *rx_buffer_base_bkp = phy_serial->rx_buffer_base;
            // Check if the message is continuous in the buffer
            if (phy_serial->rx_buffer_base + header.size > RX_data + sizeof(RX_data))
            {
                // The message is not continuous in the buffer, we need to move it
                uint8_t size = RX_data + sizeof(RX_data) - phy_serial->rx_buffer_base;
                memcpy(continuous_data, phy_serial->rx_buffer_base, size);
                memcpy(continuous_data + size, RX_data, header.size - size);
                phy_serial->rx_buffer_base = continuous_data;
                phy_serial->rx_data        = continuous_data;
            }
            // The phy manager will need the data to be continuous in memory, so we need to move it

            // Give only the header to begin
            phy_serial->received_data = sizeof(header_t);
            Phy_ComputeHeader(phy_serial);
            if (phy_serial->rx_keep == true)
            {
                // Header compute ask us to keep this message, to give it to an other phy. We already have the complete message, we can give it
                phy_serial->received_data = header.size;
                // We want to keep this message, we can validate it
                Phy_ValidMsg(phy_serial);
                if (phy_serial->rx_data == NULL)
                {
                    // The message wasn't kept, there is no more space on the buffer, or the message is corrupted.
                    // This is a critical failure
                    LUOS_ASSERT(0);
                }
            }
            phy_serial->rx_buffer_base = rx_buffer_base_bkp;
            // The message have been consumed, we can move the rx buffer pointer
            Serial_MoveRxPtr(header.size + 1);
            // If we still have data in the buffer after this message we need to move the phy_serial->rx_timestamp accordingly allowing the next message to be correctly timed.
            if (rx_size > 0)
            {
                // Add to the original timestamp value the time needed to receive all the bytes of the current message.
                phy_serial->rx_timestamp += (header.size + sizeof(SerialHeader_t) + 1) * (uint32_t)10 * (uint32_t)1000000000 / (uint32_t)SERIAL_NETWORK_BAUDRATE;
            }
        }
    }
}

/******************************************************************************
 * @brief Serial job handler
 * @param phy_ptr
 * @param job
 * @return None
 ******************************************************************************/
void Serial_JobHandler(luos_phy_t *phy_ptr, phy_job_t *job)
{
    // Try to directly transmit... Who knows perhaps the line is free
    Serial_Send();
}

/******************************************************************************
 * @brief We finished to send the message, try to send another one
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void Serial_TransmissionEnd(void)
{
    sending = false;
    // We transmitted this message, we can remove it then send another one
    // We may had a reset during this transmission, so we need to check if we still have something to transmit
    if (Phy_GetJobNumber(phy_serial) > 0)
    {
        phy_job_t *job = Phy_GetJob(phy_serial);
        job->phy_data  = 0;
        Phy_RmJob(phy_serial, job);
        Serial_Send();
    }
}

/******************************************************************************
 * @brief Try to send a message
 * @param None
 * @return None
 * ****************************************************************************/
_CRITICAL void Serial_Send(void)
{
    phy_job_t *job = Phy_GetJob(phy_serial);
    if (job == NULL)
    {
        return;
    }
    Phy_SetIrqState(false);
    if (sending == false)
    {
        sending = true;
        Phy_SetIrqState(true);
        // We can send the message
        // Add the encapsulation to the message
        SerialHeader_t header;
        header.header = SERIAL_HEADER;
        header.size   = job->size;
        memcpy(TX_data, &header, sizeof(SerialHeader_t));
        memcpy(TX_data + sizeof(SerialHeader_t), job->msg_pt, sizeof(msg_t));
        if (job->timestamp)
        {
            // Convert date to a sendable timestamp and put it in the end of the message
            time_luos_t timestamp = Phy_ComputeMsgTimestamp(phy_serial, job);
            memcpy(&TX_data[sizeof(SerialHeader_t) + job->size - +sizeof(timestamp)], &timestamp, sizeof(time_luos_t));
            TX_data[sizeof(SerialHeader_t) + job->size] = SERIAL_FOOTER;
        }
        else
        {
            TX_data[sizeof(SerialHeader_t) + job->size] = SERIAL_FOOTER;
            // Send the message
        }
        SerialHAL_Send(TX_data, job->size + sizeof(SerialHeader_t) + 1);
    }
    Phy_SetIrqState(true);
}

/******************************************************************************
 * @brief Store received data in the rx buffer
 * @param data data to store
 * @param size size of the data
 * @return None
 *****************************************************************************/
_CRITICAL void Serial_ReceptionWrite(uint8_t *data, uint32_t size)
{
    LUOS_ASSERT((size < sizeof(RX_data)) && (data != NULL));
    if (rx_size == 0)
    {
        // This is probably the first data we received for this message, we need to timestamp the reception date.
        // Watch out, if the loop is executed very slowly we may receive multiple messages in the same loop. This could result in a wrong timestamp for the second message. Their is no way to avoid this problem, so we need to accept it. Anyway we even didn't have any way to store multiple timestamp...
        phy_serial->rx_timestamp = Phy_GetTimestamp() - (size * (uint32_t)10 * (uint32_t)1000000000 / (uint32_t)SERIAL_NETWORK_BAUDRATE); // now - (nbr_byte * 10bits * (1s in ns) / baudrate)
    }
    uint32_t copy_size = (uintptr_t)&RX_data + sizeof(RX_data) - (uintptr_t)phy_serial->rx_buffer_base + rx_size;
    if (copy_size > size)
    {
        copy_size = size;
    }
    memcpy(phy_serial->rx_buffer_base + rx_size, data, copy_size);
    if (copy_size < size)
    {
        memcpy(RX_data, data + copy_size, size - copy_size);
    }
    rx_size += size;
    LUOS_ASSERT(rx_size < sizeof(RX_data));
    if ((wait_reception == true) && (size >= sizeof(SerialHeader_t) + 1))
    {
        // We received the answer of a topology ping, just indicate that we receive it
        wait_reception = false;
    }
}

/******************************************************************************
 * @brief Reception end callback
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void Serial_ReceptionIncrease(uint32_t size)
{
    // Reception is finished, we can parse the message
    rx_size += size;
    // We consider this as the end of a complete message
    // If we received multiple messages in this call, this could result in a wrong timestamp for the second message. Their is no way to avoid this problem, so we need to accept it.
    phy_serial->rx_timestamp = Phy_GetTimestamp() - (rx_size * (uint32_t)10 * (uint32_t)1000000000 / (uint32_t)SERIAL_NETWORK_BAUDRATE); // now - (nbr_byte * 10bits * (1s in ns) / baudrate)
    LUOS_ASSERT(rx_size < sizeof(RX_data));
    if ((wait_reception == true) && (size >= sizeof(SerialHeader_t) + 1))
    {
        // We received the answer of a topology ping, just indicate that we receive it
        wait_reception = false;
    }
}

/******************************************************************************
 * @brief Find the next neighbour on this phy
 * @param None
 * @return error_return_t
 ******************************************************************************/
error_return_t Serial_RunTopology(luos_phy_t *phy_ptr, uint8_t *portId)
{
    // Send a ping message

    // While the transmission is not availble, we wait
    while (sending == true)
        ;
    Phy_SetIrqState(false);
    if (sending == false)
    {
        sending = true;
        Phy_SetIrqState(true);
        // We can send the message
        // Add the encapsulation to the message
        SerialHeader_t header;
        header.header = SERIAL_HEADER;
        header.size   = 0;
        memcpy(TX_data, &header, sizeof(SerialHeader_t));
        TX_data[sizeof(SerialHeader_t)] = SERIAL_FOOTER;

        // Send the message
        SerialHAL_Send(TX_data, sizeof(SerialHeader_t) + 1);
    }
    else
    {
        Phy_SetIrqState(true);
        LUOS_ASSERT(0);
    }

    // Now we have to wait for the answer during max 200 ms
    wait_reception   = true;
    uint32_t timeout = LuosHAL_GetSystick() + 200;
    while ((wait_reception == true) && (LuosHAL_GetSystick() < timeout))
    {
        // Treat potential received data
        Serial_Loop();
    }
    // Check if we received the answer
    if (wait_reception == false)
    {
        // We received the answer
        // We have to find out if this ping was a ping or a de-ping
        if (next_ping_is_deping == true)
        {
            // This was a de-ping, we need to consider this branch as done
            Phy_TopologyDone(phy_serial);
            next_ping_is_deping = false;
            return FAILED;
        }
        else
        {
            // We have to wait a de-ping to consider this branch as done
            // Save a flag to indicate that we initiate a ping
            we_initiate_ping = true;
            return SUCCEED;
        }
    }
    else
    {
        // We didn't receive the answer, consider this branch as done
        we_initiate_ping = false;
        Phy_TopologyDone(phy_serial);
        return FAILED;
    }
}
