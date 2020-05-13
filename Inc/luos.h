#ifndef LUOS_H
#define LUOS_H

#include "module_list.h"
#include "module_structs.h"
#include "routingTable.h"
#include "luos_od.h"
#include "streaming.h"
#include <robus.h>

/**
 * \fn void luos_init(void)
 * \brief Initialisation of Luos.
 *
 */
void luos_init(void);

/**
 * \fn void luos_loop(void)
 * \brief loop of Luos.
 *
 */
void luos_loop(void);

/**
 * \fn void luos_modules_clear(void)
 * \brief Completely reset the list of virtual modules.
 *
 */
void luos_modules_clear(void);

/**
 * \fn module_t* luos_module_create(MOD_CB mod_cb, unsigned char type, unsigned char *alias)
 * \brief Initialisation of the module.
 *
 * \param mod_cb function pointer into the rx callback.
 * \param type type reference of this module hardware.
 * \param alias string (15 caracters max).
 *
 * \return module object pointer.
 *
 */
module_t* luos_module_create(MOD_CB mod_cb, unsigned char type, const char *alias);


/**
 * \fn void luos_module_enable_rt(module_t*module)
 * \brief Enable real time callback mode.
 *
 * \param module module to enable.
 *
 */
void luos_module_enable_rt(module_t*module);

/**
 * \fn unsigned char luos_send(module_t* module, msg_t *msg)
 * \brief  Send message function.
 *
 * \param virtual module who send.
 * \param msg Message to send to the slave.
 *
 * \return send or not
 */
unsigned char luos_send(module_t* module, msg_t *msg);

/**
 * \fn unsigned char luos_send_data(module_t* module, msg_t*msg, void* bin_data, unsigned int size)
 * \brief  Send message with big datas into multiple chunk.
 *
 * \param module who send.
 * \param msg Message to send to the slave with basic informations.
 * \param bin_data Pointer to the message data table
 * \param size Size of the data to transmit
 *
 * \return send or not
 */
unsigned char luos_send_data(module_t* module, msg_t*msg, void* bin_data, unsigned short size);

/**
 * \fn unsigned char luos_send_streaming(module_t *module, msg_t *msg, streaming_channel_t *streaming)
 * \brief  Send datas of a streaming channel.
 *
 * \param module who send.
 * \param msg Message to send to the slave with basic informations.
 * \param streaming streaming channel pointer
 *
 * \return send or not
 */
unsigned char luos_send_streaming(module_t *module, msg_t *msg, streaming_channel_t *stream);

/**
 * \fn unsigned char luos_receive_data(module_t* module, msg_t* msg, void* bin_data, unsigned int* size)
 * \brief  Retrieve a multi chunk data
 *
 * \param module who receive.
 * \param msg Message chunk received by the slave.
 * \param bin_data Pointer to the data table
 * \param size Size of the received data
 *
 * \return reception finish or not
 */
unsigned char luos_receive_data(module_t *module, msg_t *msg, void *bin_data);

/**
 * \fn nsigned char luos_receive_streaming(module_t *module, msg_t *msg, streaming_channel_t streaming)
 * \brief  Receive a streaming channel datas
 *
 * \param module who receive.
 * \param msg Message received by the slave.
 * \param streaming streaming channel pointer
 *
 * \return reception finish or not
 */
unsigned char luos_receive_streaming(module_t *module, msg_t *msg, streaming_channel_t *stream);

/**
 * \fn msg_t* luos_read(module_t* module)
 * \brief  get a received message from a specific module.
 *
 * \param module who receive.
 *
 * \return the received message pointer
 */
msg_t *luos_read(module_t *module);

/**
 * \fn msg_t* luos_read_from(module_t* module, short id)
 * \brief  get a received message from a specific id to a specific module.
 *
 * \param module who receive the message we are looking for.
 * \param id who sent the message we are looking for .
 *
 * \return the received message pointer
 */
msg_t* luos_read_from(module_t* module, short id);

/**
 * \fn char luos_message_available(void)
 * \brief how many messages are available
 *
 * \return the number of message received.
 */
char luos_message_available(void);

/**
 * \fn void luos_save_alias(module_t* module, char* alias)
 * \brief  Save Alias in EEprom.
 *
 * \param concerned virtual module.
 * \param name string.
 *
 */
void luos_save_alias(module_t* module, char* alias);

/**
 * \fn void luos_set_baudrate(module_t* module, uint32_t baudrate)
 * \brief  Setup the entire luos network baudrate .
 *
 * \param module sending this command.
 * \param new baudrate.
 *
 */
void luos_set_baudrate(module_t* module, uint32_t baudrate);


#endif /* LUOS_H */
