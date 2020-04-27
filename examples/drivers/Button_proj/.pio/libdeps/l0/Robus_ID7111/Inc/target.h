/*
 * target.h
 *
 * Created: 14/02/2017 11:53:28
 *  Author: Nicolas Rabault
 *  Abstract: multicast address bank management header
 */
 #ifndef _TARGET_H_
#define _TARGET_H_

#include "context.h"

unsigned char multicast_target_bank(vm_t* vm, unsigned short val);
void add_multicast_target(vm_t* vm, unsigned short target);

#endif /* _TARGET_H_ */
