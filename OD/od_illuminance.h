/*
 * od_illuminance.h
 *
 *  Created on: 7 déc. 2019
 *      Author: nicolasrabault
 */

#ifndef OD_OD_ILLUMINANCE_H_
#define OD_OD_ILLUMINANCE_H_

typedef float illuminance_t;

// illuminance are stored in lux (lx or lm/m2)
//******** Conversions ***********

// lx
static inline float illuminance_to_lx(illuminance_t self) {
    return self;
}

static inline illuminance_t illuminance_from_lx(float lx) {
    return lx;
}

//******** Messages management ***********
static inline void illuminance_to_msg(const illuminance_t* const self, msg_t* const msg) {
    msg->header.cmd = ILLUMINANCE;
    memcpy(msg->data, self, sizeof(illuminance_t));
    msg->header.size = sizeof(illuminance_t);
}

static inline void illuminance_from_msg(illuminance_t* const self, const msg_t* const msg) {
    memcpy(self, msg->data, msg->header.size);
}

// GPIO struct
typedef struct __attribute__((__packed__)){
    union {
        struct __attribute__((__packed__)){
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        uint8_t unmap[3];                /*!< Unmaped form. */
    };
}color_t;

 // color are stored in RGB
 //******** Messages management ***********
 static inline void color_to_msg(const color_t* const self, msg_t* const msg) {
     msg->header.cmd = COLOR;
     memcpy(msg->data, self, sizeof(color_t));
     msg->header.size = sizeof(color_t);
 }

 static inline void color_from_msg(color_t* const self, const msg_t* const msg) {
     memcpy(self, msg->data, msg->header.size);
 }




#endif /* OD_OD_ILLUMINANCE_H_ */
