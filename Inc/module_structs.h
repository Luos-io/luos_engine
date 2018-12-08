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

// Imu report struct
typedef struct __attribute__((__packed__)){
    union {
        struct __attribute__((__packed__)){
            unsigned short accell : 1;
            unsigned short gyro : 1;
            unsigned short quat : 1;
            unsigned short compass : 1;
            unsigned short euler : 1;
            unsigned short rot_mat : 1;
            unsigned short pedo : 1;
            unsigned short linear_accel : 1;
            unsigned short gravity_vector : 1;
            unsigned short heading : 1;
        };
        unsigned char unmap[2];
    };
}imu_report_t;

// Pid
typedef struct __attribute__((__packed__)){
    union {
        struct __attribute__((__packed__)){
            float p;
            float i;
            float d;
        };
        unsigned char unmap[3 * sizeof(float)];
    };
}asserv_pid_t;

// Motor
typedef struct __attribute__((__packed__)){
    union {
            struct __attribute__((__packed__)){
                // target modes
                uint16_t mode_compliant : 1;
                uint16_t mode_power : 1;
                uint16_t mode_torque : 1;
                uint16_t mode_rot_speed : 1;
                uint16_t mode_rot_position : 1;
                uint16_t mode_trans_speed : 1;
                uint16_t mode_trans_position : 1;

                // report modes
                uint16_t rot_position : 1;
                uint16_t rot_speed : 1;
                uint16_t trans_distance : 1;
                uint16_t trans_speed : 1;
                uint16_t current : 1;
            };
            uint8_t unmap[2];
    };
} motor_mode_t;

typedef struct __attribute__((__packed__)){
        float power;

        // targets
        motor_mode_t mode;
        float target_rot_position;
        float target_rot_speed;

        // measures
        float rot_position;
        float rot_speed;
        float trans_distance;
        float trans_speed;

        //configs
        float motor_reduction;
        float encoder_res;
        float wheel_diameter_mm;
}motor_config_t;


#endif /*__ __L0_H */
