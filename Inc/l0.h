 #ifndef __L0_H
#define __L0_H

#include "luos.h"

#define TEMP110_CAL_VALUE                                           ((uint16_t*)((uint32_t)0x1FFFF7C2))
#define TEMP30_CAL_VALUE                                            ((uint16_t*)((uint32_t)0x1FFFF7B8))
#define VOLTAGEFACTOR                                               (10.0f + 68.0f) / 10.0f
 // This structure need to list all ADC configured in the good order determined by the
 // ADC_CHANEL number in increasing order
typedef struct __attribute__((__packed__)){
     union {
         struct __attribute__((__packed__)){
             uint32_t voltage_sensor;
             uint32_t temperature_sensor;
         };
         uint32_t unmap[2];                /*!< Uncmaped form. */
     };
 }l0_analog_t;

 volatile l0_analog_t L0_analog;

 int l0_msg_handler(vm_t* vm, msg_t* input, msg_t* output);

#endif /*__ __L0_H */
