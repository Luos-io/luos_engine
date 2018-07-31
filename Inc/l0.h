 #ifndef __L0_H
#define __L0_H

#define TEMP110_CAL_VALUE                                           ((uint16_t*)((uint32_t)0x1FFFF7C2))
#define TEMP30_CAL_VALUE                                            ((uint16_t*)((uint32_t)0x1FFFF7B8))

 // This structure need to list all ADC configured in the good order determined by the
 // ADC_CHANEL number in increasing order
typedef struct __attribute__((__packed__)){
     union {
         struct __attribute__((__packed__)){
             uint32_t power_sensor;
             uint32_t temperature_sensor;
         };
         uint32_t unmap[2];                /*!< Uncmaped form. */
     };
 }l0_analog_t;

 volatile l0_analog_t L0_analog;

#endif /*__ __L0_H */
