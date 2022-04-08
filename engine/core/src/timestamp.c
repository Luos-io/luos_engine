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
 * Timestamp is a mechanism which enables to track events in the system. The feature is build around
 * the concept of tokens. A token is an object wich can be linked to an event and can store a
 * timestamp. Each time a token is created and linked to an event, it's placed in a list with other
 * tokens to form a linked list with all tokens presents in the system. The token is timestamped at
 * the same moment.
 *
 * When you need to read the timestamp associated with an event, you can use the Timestamp_GetToken()
 * function. This function will search the token linked to the event in the list and return the
 * timestamp saved in it.
 *

                                              Event_A         Event_B

                                                 │               │
                                                 ▼               ▼
                                            ───────────────────────────────────────────────►  Time
                                                 |               |
                                                 │               │
               Token list                        │               │
                                                 │               │
               ┌───────┐                         │               │
    Timestamp  │ Token │ ◄───────────────────────┘               │
               ├───────┤                                         │
    Timestamp  │ Token │ ◄───────────────────────────────────────┘
               ├───────┤
    Timestamp  │ Token │          Timestamp_Tag(token, Event_B)
               ├───────┤
    Timestamp  │ Token │
               └───────┘
                   .
                   .
                   .

               ┌───────┐
    Timestamp  │ Token │ ────────────────► Timestamp_GetToken(Event_B)
               ├───────┤
    Timestamp  │ Token │
               ├───────┤
    Timestamp  │ Token │
               └───────┘

 *
 * You can also send a message with a data and it's associated timestamp, luos can handle this by slighly modifiyng its
 * protocol. To encode a message with the timestamp protocol, use Timestamp_EncodeMsg() function, it will change the
 * message and save the timestamp in it.
 * An example is shown beneath: the sended data is a color, the command saved in the header is COLOR_TYPE. What
 * Timestamp_EncodeMsg() does is modifying the protocol field. This allows the receiving service to process the message
 * in a dedicated way and extract the timestamp information. The timestamp value is placed at the end of the message,
 * after data and data_cmd sections.
 *


          Timestamp protocol

                  │
                  │
                  │
                  │
                  ▼
           ┌────────────┬─────────┬───────┬─────────┬─────┬──────┬─────────────────────────────────────────┬─────────────────┐
           │  Protocol  │  Target │  Mode │  Source │ Cmd │ Size │ Payload                                 │     Timestamp   │
           └────────────┴─────────┴───────┴─────────┴─────┴──────┴─────────────────────────────────────────┴─────────────────┘

 *
 * Timestamp_DecodeMsg allows you to get the timestamp associated to a data saved in the message. It also recovers the
 * original message before its transposition in the Timestamp protocol, as shown here after.
 *
                                          Color type           Color value
 ┌─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬────────────────────────┐
 │Protocol │ Target  │  Mode   │ Source  │   Cmd   │  Size   │         Data           │
 └─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴────────────────────────┘

*
* Timestamp API also allows you to tag an event instead of a data. The idea is to create a command, attach a timestamp
* information to it and send it to a service. The service which receives this information can **execute** the command
* at the date specified by the timestamp. This is a really interesting feature if you want to execute actions at precise
* dates. It also releases the constraints on messages exchanged on the network as you can send them by batch: each message
* carries a timestamped event which will be processed at the defined date. In the API, **Timestamp_CreateEvent** enables
* you to use this feature. You have to give to this function same arguments as you give to **timestamp_tag** plus an
* argument called **relative_date**. This last one carries information about when the event has to be processed, its unit
* is **nanoseconds**. If you want your event to be processed in 1 second, put 1000000000 in this argument.
*

             ┌────────────────┐                                              ┌──────────────────┐
             │                │                                              │                  │
             │   Service A    │                                              │    Service B     ├────────┐
             │                │                                              │                  │        │
             └────────────────┘                                              └──────────────────┘        │
                                                                                                         │
                                                                                                         │    Service B uses the
                                                                                                         │
                                                                                                         │    timestamp to process
                                                                                                         │
                                                                                                         │    the event in the message
         ┌────────────────────────┐                                         ┌────────────────────┐       │
         │                        │                                         │                    │       │    at a precise date
         │   Timestamped message  ├────────────────────────────────────────►│  Received message  │       │
         │                        │                                         │                    │       │
         └────────────────────────┘                                         └────────────────────┘       │
                                                                                                         │
                                                                                  Timestamp  ◄───────────┘


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
inline bool Timestamp_IsTimestampMsg(msg_t *msg)
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
        memcpy(&timestamp, (msg->data + 1), sizeof(time_luos_t));
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
    memcpy(&msg->data[1], &timestamp, sizeof(time_luos_t));
    // Add timestamp size to message size
    msg->header.size += sizeof(time_luos_t);
}

/******************************************************************************
 * @brief Compute and write the latency in the message
 * @param msg Message that will be sent
 * @return None
 ******************************************************************************/
void Timestamp_ConvertToLatency(msg_t *msg)
{
    static time_luos_t timestamp_date = 0.0f;
    static msg_t *last_msg            = NULL;

    if (last_msg != msg)
    {
        // This is a new message, backup the timestamp date
        memcpy(&timestamp_date, &msg->data[msg->header.size], sizeof(time_luos_t));
        // Increase the size of the message to consider the latency
        msg->header.size = msg->header.size + sizeof(time_luos_t);
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
inline void Timestamp_ConvertToDate(msg_t *msg, uint64_t reception_date)
{
    time_luos_t timestamp_latency = 0.0f;
    // Update msg size
    msg->header.size = msg->header.size - sizeof(time_luos_t);
    // Get latency
    memcpy(&timestamp_latency, &msg->data[msg->header.size], sizeof(time_luos_t));
    // Compute the date from latency
    time_luos_t timestamp_date = timestamp_latency + TimeOD_TimeFrom_ns(reception_date);
    // Write the date on the message
    memcpy(&msg->data[msg->header.size], &timestamp_date, sizeof(time_luos_t));
}
