/******************************************************************************
 * @file timestamp feature
 * @brief time stamp data
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_

#include "stdint.h"
#include "stdbool.h"
#include "config.h"
#include "service_structs.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct timestamp_token
{
    uint64_t timestamp;
    void *target;
    struct timestamp_token *next;
} timestamp_token_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

// functions to use on data to time stamp
void Timestamp_Tag(timestamp_token_t *token, void *target);
timestamp_token_t *Timestamp_GetToken(void *target);

// functions to use on luos messages
bool Timestamp_IsTimestampMsg(msg_t *msg);
void Timestamp_TagMsg(msg_t *msg);
void Timestamp_EncodeMsg(msg_t *msg, void *target);
void Timestamp_DecodeMsg(msg_t *msg, uint64_t *timestamp);

#endif /* _TIMESTAMP_H_ */
