#ifndef __ANALOG_H
 #define __ANALOG_H

 #include "luos_board.h"

 // This structure need to list all ADC configured in the good order determined by the
 // ADC_CHANEL number in increasing order
 typedef struct __attribute__((__packed__)){
     union {
         struct __attribute__((__packed__)){
             uint32_t light;
             uint32_t voltage_sensor;
             uint32_t temperature_sensor;
         };
         uint32_t unmap[3];                /*!< Unmaped form. */
     };
 }analog_input_t;

 volatile analog_input_t analog_input;

#endif /*__ __ANALOG_H */
