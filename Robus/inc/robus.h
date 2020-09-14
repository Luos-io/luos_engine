/**
 * \file robus.h
 * \brief Robus communication main include file.
 * \author Nicolas Rabault
 * \version 0.1
 * \date 18 Fevrier 2017
 *
 * Include this file to use the robus communication protocol.
 *
 */

#ifndef _ROBUS_H_
#define _ROBUS_H_

#include "robus_struct.h"

/**
 * \fn void robus_init(RX_CB)
 * \brief Initialisation of the Robus communication lib.
 *
 * \param Luos function pointer into the rx callback interrupt.
 */
void robus_init(RX_CB callback);

/**
 * \fn void robus_modules_clear(void)
 * \brief Completely reset the list of virtual modules.
 *
 */
void robus_modules_clear(void);

/**
 * \fn vm_t* robus_module_create(unsigned char type, unsigned char *alias)
 * \brief Initialisation of the Robus communication lib.
 *
 * \param type module type.
 * \param alias string (15 caracters max).
 *
 * \return virtual module pointer.
 *
 */
vm_t *robus_module_create(unsigned char type);

/**
 * \fn unsigned char robus_send(vm_t* vm, msg_t *msg)
 * \brief  Send message function.
 *
 * \param virtual module who send.
 * \param msg Message to send.
 *
 * \return sent or not
 */
unsigned char robus_send(vm_t *vm, msg_t *msg);

/**
 * \fn unsigned char robus_set_baudrate(vm_t* vm, unsigned int baudrate)
 * \brief  setup the robus network baudrate.
 *
 * \param virtual module who send.
 * \param baudrate baudrate value.
 *
 * \return sent or not
 */
unsigned char robus_set_baudrate(vm_t *vm, unsigned int baudrate);

/**
 * \fn unsigned short* robus_get_node_branches(unsigned char* size)
 * \brief  get back node branches connections to other nodes.
 *
 * \param table branch number
 *
 * \return detection branches table pointers
 */
unsigned short *robus_get_node_branches(unsigned char *size);

/**
 * \fn unsigned char robus_topology_detection(vm_t* vm)
 * \brief start the detection procedure
 *
 * \param *vm virtual module who start the detection
 * \return return the number of detected module
 */
unsigned char robus_topology_detection(vm_t *vm);

#endif /* _ROBUS_H_ */
