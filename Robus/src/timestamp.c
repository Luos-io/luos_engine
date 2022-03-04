/******************************************************************************
 * @file timestamp feature
 * @brief time stamp data
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "timestamp.h"
#include "luos_hal.h"
#include "string.h"

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
 * message is encoded and save the timestamp in it.
 * An example is shown beneath: the sended data is a color, the the command saved in the header is COLOR_TYPE. What
 * Timestamp_EncodeMsg() does is copying the COLOR_TYPE in a dedicated section after data payload and replace it in the
 * header by a TIMESTAMP command. The timestamp value is placed at the end of the message, after data and data_cmd sections.
 *
                                          Timestamp            Color value             Color type
 ┌─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬───────────────────────┬─────────────┬────────────────┐
 │Protocol │ Target  │  Mode   │ Source  │   Cmd   │  Size   │         Data          │  Data cmd   │   Timestamp    │
 └─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴───────────────────────┴─────────────┴────────────────┘
                                              │                                             ▲
                                              │                                             │
                                              └─────────────────────────────────────────────┘
 *
 * Timestamp_DecodeMsg allows you to get the timestamp associated to a data saved in the message. It also recovers the
 * original message before its transposition in the Timestamp protocol, as shown here after.
 *
                                          Color type           Color value
 ┌─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬────────────────────────┐
 │Protocol │ Target  │  Mode   │ Source  │   Cmd   │  Size   │         Data           │
 └─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴────────────────────────┘

 ***************************************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
// initialize a global variable to track token list
timestamp_token_t *token_list_head = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/
static inline void Timestamp_AddToken(timestamp_token_t *);
static int64_t Timestamp_GetTimeFromToken(void *);
static bool Timestamp_DeleteTokenFromList(void *);

/******************************************************************************
 * @brief link a token to a target until its comsumption in a message
 * @brief and save the associated timestamp with the relative event date
 * @param relative_date of the event
 * @param token instance
 * @param target to link to the token
 * @return None
 ******************************************************************************/
void Timestamp_CreateEvent(int64_t relative_date, timestamp_token_t *token, void *target)
{
    // update token only if it's not locked
    token->target    = target;
    token->timestamp = (((int64_t)LuosHAL_GetTimestamp()) + relative_date > 0) ? ((int64_t)LuosHAL_GetTimestamp()) + relative_date : 0;

    // if it's a new token, add it to the list
    if (!Timestamp_GetToken(target))
    {
        Timestamp_AddToken(token);
    }
}

/******************************************************************************
 * @brief link a token to a target until its comsumption in a message
 * @brief and save the associated timestamp
 * @param token instance
 * @param target to link to the token
 * @return None
 ******************************************************************************/
void Timestamp_Tag(timestamp_token_t *token, void *target)
{
    Timestamp_CreateEvent(0, token, target);
}

/******************************************************************************
 * @brief Add a token to the chained list
 * @param token instance
 * @return None
 ******************************************************************************/
void Timestamp_AddToken(timestamp_token_t *token)
{
    timestamp_token_t *current_token;

    if (!token_list_head)
    {
        // first token in the list
        token_list_head = token;
    }
    else
    {
        current_token = token_list_head;
        while (current_token->next)
        {
            // go to the next element of the list
            current_token = current_token->next;
        }
        // we found an available token
        current_token->next = token;
    }
}

/******************************************************************************
 * @brief find the timestamp linked to the target
 * @param target associated with the timestamp to find
 * @return timestamp linked to the target
 ******************************************************************************/
int64_t Timestamp_GetTimeFromToken(void *target)
{
    timestamp_token_t *token = Timestamp_GetToken(target);
    int64_t timestamp        = 0;
    if (token)
    {
        // get the timestamp from the token
        timestamp = token->timestamp;
        // delete the token from the list
        Timestamp_DeleteTokenFromList(target);
        // reset the token
        token->timestamp = 0;
        token->target    = 0;

        return timestamp;
    }
    else
    {
        // cannot find the token
        return 0;
    }
}

/******************************************************************************
 * @brief delete a token from the linked list
 * @param target
 * @return true / false
 ******************************************************************************/
bool Timestamp_DeleteTokenFromList(void *target)
{
    timestamp_token_t *token      = token_list_head;
    timestamp_token_t *prev_token = token_list_head;
    // find token linked to the target if it exists
    while (token != 0)
    {
        if (token->target != target)
        {
            // go to the next token
            prev_token = token;
            token      = token->next;
        }
        else
        {
            // we found the token
            prev_token->next = token->next;
            return true;
        }
    }
    // we didn't find the token
    return false;
}

/******************************************************************************
 * @brief find a token linked to the target
 * @param target
 * @return Timestamp_token
 ******************************************************************************/
timestamp_token_t *Timestamp_GetToken(void *target)
{
    timestamp_token_t *token = token_list_head;
    // find token linked to the target if it exists
    while (token != 0)
    {
        if (token->target != target)
        {
            // go to the next token
            token = token->next;
        }
        else
        {
            // we found the token
            return token;
        }
    }
    // we didn't find the token
    return 0;
}

/******************************************************************************
 * @brief check if a message needs a timestamp
 * @param data in the message
 * @return true / false
 ******************************************************************************/
bool Timestamp_IsTimestampMsg(msg_t *msg)
{
    if (msg->header.cmd == TIMESTAMP_CMD)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/******************************************************************************
 * @brief check if a message needs a timestamp
 * @param data in the message
 * @return None
 ******************************************************************************/
void Timestamp_TagMsg(msg_t *msg)
{
    uint16_t full_size     = sizeof(header_t) + msg->header.size + sizeof(uint16_t);
    int64_t data_timestamp = 0;
    int64_t latency        = 0;

    // get timestamp in message stream
    memcpy(&data_timestamp, &msg->stream[full_size - sizeof(uint16_t) - sizeof(int64_t)], sizeof(int64_t));
    // update timestamp
    latency = data_timestamp - (int64_t)LuosHAL_GetTimestamp();
    // copy timestamp in message
    memcpy(&msg->stream[full_size - sizeof(int16_t) - sizeof(int64_t)], &latency, sizeof(int64_t));
}

/******************************************************************************
 * @brief modifies luos message according to timestamp payload
 * @param message to modify
 * @return None
 ******************************************************************************/
void Timestamp_EncodeMsg(msg_t *msg, void *target)
{
    int64_t timestamp = Timestamp_GetTimeFromToken(target);
    uint8_t cmd       = msg->header.cmd;

    // copy timestamp command
    msg->header.cmd = TIMESTAMP_CMD;
    // copy subcommand
    memcpy(&msg->data[msg->header.size], &cmd, sizeof(uint8_t));
    // copy timestamp
    memcpy(&msg->data[msg->header.size + sizeof(uint8_t)], &timestamp, sizeof(int64_t));
    // update msg size
    msg->header.size = msg->header.size + sizeof(int64_t) + sizeof(uint8_t);
}

/******************************************************************************
 * @brief modifies luos message according to timestamp payload
 * @param message to modify
 * @return None
 ******************************************************************************/
void Timestamp_DecodeMsg(msg_t *msg, int64_t *timestamp)
{
    // deserialize data
    msg->header.size = msg->header.size - sizeof(int64_t) - sizeof(uint8_t);
    memcpy(&msg->header.cmd, &msg->data[msg->header.size], sizeof(uint8_t));
    memcpy(timestamp, &msg->data[msg->header.size + sizeof(uint8_t)], sizeof(int64_t));
}