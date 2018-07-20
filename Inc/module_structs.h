 #ifndef __MODULE_STRUCT_H
#define __MODULE_STRUCT_H

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

#endif /*__ __L0_H */
