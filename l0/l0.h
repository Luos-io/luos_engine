 #ifndef __L0_H
#define __L0_H

#include "luos.h"

#define TEMP110_CAL_VALUE                                           ((uint16_t*)((uint32_t)0x1FFFF7C2))
#define TEMP30_CAL_VALUE                                            ((uint16_t*)((uint32_t)0x1FFFF7B8))
#define VOLTAGEFACTOR                                               (10.0f + 68.0f) / 10.0f
#define LUOS_UUID                                                   ((uint32_t *)0x1FFFF7AC)
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

 int l0_msg_handler(module_t* module, msg_t* input, msg_t* output);
 void status_led(char state);
 void board_init(void);
 void write_alias(unsigned short id, char* alias);
 char read_alias(unsigned short id, char* alias);

#endif /*__ __L0_H */
