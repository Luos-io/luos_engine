/******************************************************************************
 * @file message_mngr
 * @brief function relative to message processing
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef MESSAGE_MNGR_H_
#define MESSAGE_MNGR_H_

#include "luos.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
typedef struct __attribute__((__packed__)) mngr_t
{
    module_t *module;
    msg_t *msg; /*!< msg ready to be read */
} mngr_t;
/*******************************************************************************
 * Function
 ******************************************************************************/
uint8_t Mngr_AvailableMessage(void);
void Mngr_SetIndexMsg(module_t *module, msg_t *msg);
void Mngr_GetIndexMsg(int module_index, int msg_index, mngr_t *chunk);
int8_t Mngr_GetNextCallbackID(void);
int8_t Mngr_GetNextModuleID(module_t *module);
int8_t Mngr_GetNextMsgID(int mngr_id, short msg_from);

#endif /* MESSAGE_MNGR_H_ */
