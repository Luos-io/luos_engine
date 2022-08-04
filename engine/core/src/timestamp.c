/******************************************************************************
 * @file timestamp feature
 * @brief time stamp data
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "_timestamp.h"
#include "luos_hal.h"
#include "string.h"
#include "service_structs.h"
/******************************* Description of Timestamp process ************************************
 *
 * Timestamp is a mechanism which enables to track events in the system. To use it users have to get and save a timestamp in
 * a time_luos_t variable.
 * You can get a timestamp by calling the Timestamp_now() function. Then you can deal with the returned value as you want, allowing
 * you to tag a date in the past or in the future.
 *
 * Then you can send a message with a data and it's associated timestamp, luos can handle this by slighly modifiyng its
 * protocol. To send a message with it's timestamp, use Luos_SendTimestampMsg(service_t *service, msg_t *msg, time_luos_t timestamp)
 * function. This function will enable te message in timestamp mode and slightly modify it before sending it.
 *
 * An example is shown beneath: the sended data is a color, the command saved in the header is COLOR_TYPE. To support timstamp
 * Luos engine is modifying the protocol field. This allows the receiving service to process the message
 * in a dedicated way and extract the timestamp information. The timestamp value is placed at the end of the message,
 * after data. Also the Size header field is modified to include the timestamp size in the data field.
 *
          Timestamp protocol                           Updated size
                  │                                          │
                  ▼                                          ▼
           ┌────────────┬─────────┬───────┬─────────┬─────┬──────┬─────────────────────────────────────────┬─────────────────┐
           │  Protocol  │  Target │  Mode │  Source │ Cmd │ Size │ Payload                                 │     Timestamp   │
           └────────────┴─────────┴───────┴─────────┴─────┴──────┴─────────────────────────────────────────┴─────────────────┘
 *
 * At reception the timestamp message will be decoded allowing you to get the timestamp associated to the data saved in the message. It also recovers the
 * original message before its transposition in the Timestamp protocol with the correct size, as shown here after. Timestamp still stored at the end of message,
 * but users don't see it.
 * User can check if message are timestamped using the function Luos_IsTimestampMsg(msg_t *msg).
 * User can get back the timestamp value using the function Luos_GetTimestamp(msg_t *msg).
 *
                                                        Color size      Color value
                                                             │              │
                                                             ▼              ▼
           ┌────────────┬─────────┬───────┬─────────┬─────┬──────┬─────────────────────────────────────────┬-----------------┐
           │  Protocol  │  Target │  Mode │  Source │ Cmd │ Size │ Payload                                 │                 │
           └────────────┴─────────┴───────┴─────────┴─────┴──────┴─────────────────────────────────────────┴-----------------┘
*
* Timestamp values are transformed from date to latency at message send and from latency to date at message reception.
* This allow to track events in the system and put them at a representative date for the node using it.
*
 ***************************************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Get the present timestamp
 * @return time_luos_t
 ******************************************************************************/
time_luos_t Timestamp_now(void)
{
    return TimeOD_TimeFrom_ns((double)LuosHAL_GetTimestamp());
}
/******************************************************************************
 * @brief Check if the message is a timestamp message
 * @param msg Message to check
 * @return bool
 ******************************************************************************/
_CRITICAL inline bool Timestamp_IsTimestampMsg(msg_t *msg)
{
    return (msg->header.config == TIMESTAMP_PROTOCOL);
}
/******************************************************************************
 * @brief Get the timestamp associated to a message
 * @param msg Message to get the timestamp from
 * @return time_luos_t
 ******************************************************************************/
time_luos_t Timestamp_GetTimestamp(msg_t *msg)
{
    if (Timestamp_IsTimestampMsg(msg))
    {
        time_luos_t timestamp = 0.0f;
        // Timestamp is at the end of the message
        memcpy(&timestamp, (msg->data + msg->header.size), sizeof(time_luos_t));
        return timestamp;
    }
    else
    {
        return 0.0f;
    }
}

//************************* Private functions *********************************/

/******************************************************************************
 * @brief modifies luos message according to timestamp payload
 * @param message to modify
 * @return None
 ******************************************************************************/
void Timestamp_EncodeMsg(msg_t *msg, time_luos_t timestamp)
{
    // Update message header protocol
    msg->header.config = TIMESTAMP_PROTOCOL;
    // Timestamp is at the end of the message copy it
    memcpy(&msg->data[msg->header.size], &timestamp, sizeof(time_luos_t));
}

/******************************************************************************
 * @brief Compute and write the latency in the message
 * @param msg Message that will be sent
 * @return None
 ******************************************************************************/
_CRITICAL void Timestamp_ConvertToLatency(msg_t *msg)
{
    static time_luos_t timestamp_date = 0.0f;
    static msg_t *last_msg            = NULL;

    if (last_msg != msg)
    {
        // This is a new message, backup the timestamp date
        memcpy(&timestamp_date, &msg->data[msg->header.size], sizeof(time_luos_t));
        // Keep the message pointer to know if we already manage this one or not.
        last_msg = msg;
    }
    // Compute the latency from date
    time_luos_t latency = timestamp_date - Timestamp_now();
    // Write latency on the message
    memcpy(&msg->data[msg->header.size], &latency, sizeof(time_luos_t));
}

/******************************************************************************
 * @brief Compute and write the date in the message
 * @param msg Received message
 * @param reception_date Date of reception in ns
 * @return None
 ******************************************************************************/
_CRITICAL inline void Timestamp_ConvertToDate(msg_t *msg, uint64_t reception_date)
{
    time_luos_t timestamp_latency = 0.0f;
    // Get latency
    memcpy(&timestamp_latency, &msg->data[msg->header.size], sizeof(time_luos_t));
    // Compute the date from latency
    time_luos_t timestamp_date = timestamp_latency + TimeOD_TimeFrom_ns(reception_date);
    // Write the date on the message
    memcpy(&msg->data[msg->header.size], &timestamp_date, sizeof(time_luos_t));
}
