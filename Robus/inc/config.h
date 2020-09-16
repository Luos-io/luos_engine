/*
 * config.h
 *
 * Created: 14/02/2017 11:53:28
 *  Author: Nicolas Rabault
 *  Abstract: default configuration of the project.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define TRUE 1
#define FALSE 0

#define DEFAULTID 0x00
#define PROTOCOL_REVISION 0
#define BROADCAST_VAL 0x0FFF
#define DEFAULTBAUDRATE 1000000
#define COLLISION_TIMER ((int)(0.000002 * MCUFREQ))

#define TIMEOUT_VAL 2
#define MAX_ALIAS_SIZE 16
#define MAX_DATA_MSG_SIZE 128
#define MAX_MULTICAST_ADDRESS 1

#ifndef NBR_NAK_RETRY
#define NBR_NAK_RETRY 10
#endif

#ifndef MAX_VM_NUMBER
#define MAX_VM_NUMBER 20
#endif

#ifndef MSG_BUFFER_SIZE
#define MSG_BUFFER_SIZE 5
#endif

#ifndef MODULETYPE
#define MODULETYPE DEV_BOARD
#endif

#endif /* _CONFIG_H_ */
