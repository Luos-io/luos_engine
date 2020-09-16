/*
 * cmd.h
 *
 * Created: 14/02/2017 11:53:28
 *  Author: Nicolas Rabault
 *  Abstract: Protocol level command list
 */
#ifndef _CMD_H_
#define _CMD_H_

/*
 * All applicativ side message should have a structure like :
 * HEADER - DATA[MAX_DATA_MSG_SIZE] - CHECKSUM (ACK)
 */
typedef enum
{
    GET_ID,              /*!< Reply with ID. */
    WRITE_ID,            /*!< Get and save a new given ID. */
    GET_MODULE_TYPE,     /*!< Reply with module_type number. */
    GET_STATUS,          /*!< Reply with a status register. */
    GET_FIRM_REVISION,   /*!< Reply with the actual firmware revision number. */
    GET_COM_REVISION,    /*!< Reply with the actual communication protocole version (1 default). */
    RESET_DETECTION,     /*!< Reset detection*/
    SET_BAUDRATE,        /*!< Set Robus baudrate*/
    DEAD_MODULE,         /*!< This module is dead*/
    PROTOCOL_CMD_NB = 30 /*!< WARNING: do not exceed this value. This is the minimum cmd value available for applicative side. */
} cmd_t;

#endif /* _CONTEXT_H_ */
