#ifndef LUOS_H
#define LUOS_H

#include "module_list.h"
#include "module_structs.h"
#include "robus_struct.h"

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
 * \fn vm_t* luos_module_create(RX_CB rx_cb, unsigned char type, unsigned char *alias)
 * \brief Initialisation of the Robus communication lib.
 *
 * \param rx_cb function pointer into the rx callback.
 * \param type type reference of this module hardware.
 * \param alias string (15 caracters max).
 *
 * \return virtual module object pointer.
 *
 */
vm_t* luos_module_create(RX_CB mod_cb, unsigned char type, const char *alias);

/**
 * \fn unsigned char luos_send(vm_t* vm, msg_t *msg)
 * \brief  Send message function.
 *
 * \param virtual module who send.
 * \param msg Message to send to the slave.
 *
 * \return send or not
 */
unsigned char luos_send(vm_t* vm, msg_t *msg);


#endif /* LUOS_H */
