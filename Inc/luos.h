#ifndef LUOS_H
#define LUOS_H

#include "module_list.h"
#include "module_structs.h"
#include "robus_struct.h"
#include "routingTable.h"

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
 * \fn msg_t* luos_read(module_t* module)
 * \brief  get a received message from a specific module.
 *
 * \param module who receive.
 *
 * \return the received message pointer
 */
msg_t* luos_read(module_t* module);

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
 * \fn unsigned char luos_send_alias(module_t* module, msg_t *msg)
 * \brief  Send alias function.
 *
 * \param virtual module who send.
 * \param msg Message to send to the slave. (don't need any header.cmd)
 *
 * \return send or not
 */
unsigned char luos_send_alias(module_t* module, msg_t *msg);

#endif /* LUOS_H */
