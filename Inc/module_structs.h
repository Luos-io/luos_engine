 #ifndef __MODULE_STRUCT_H
#define __MODULE_STRUCT_H
#include "main.h"

// This file contain struct of messages exchanged between gate and modules

// GPIO struct
typedef struct __attribute__((__packed__)){
     union {
         struct __attribute__((__packed__)){
             float p1;
             uint8_t p5;
             uint8_t p6;
             float p7;
             float p8;
             float p9;
         };
         uint8_t unmap[(4 * sizeof(float)) + 2];                /*!< Uncmaped form. */
     };
 }gpio_t;

 // Handy struct
 typedef struct __attribute__((__packed__)){
      union {
          struct __attribute__((__packed__)){
        	  uint8_t index;
        	  uint8_t middle;
        	  uint8_t ring;
        	  uint8_t pinky;
        	  uint8_t thumb;
          };
          uint8_t unmap[(5 * sizeof(uint8_t))];                /*!< Uncmaped form. */
      };
  }handy_t;

#endif /*__ __L0_H */
