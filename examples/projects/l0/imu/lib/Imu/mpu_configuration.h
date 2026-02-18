#ifndef __MPU_SETUP_H__
#define __MPU_SETUP_H__

#include <inv_mpu_dmp_motion_driver.h>
#include <inv_mpu.h>
#include "luos_engine.h"

#define SEND_ACCEL          (0x01)
#define SEND_GYRO           (0x02)
#define SEND_QUAT           (0x04)
#define SEND_COMPASS        (0x08)
#define SEND_EULER          (0x10)
#define SEND_ROT_MAT        (0x20)
#define SEND_HEADING        (0x40)
#define SEND_PEDO           (0x80)
#define SEND_LINEAR_ACCEL   (0x100)
#define SEND_GRAVITY_VECTOR (0x200)

#define ACCEL_ON   (0x01)
#define GYRO_ON    (0x02)
#define COMPASS_ON (0x04)

#define MOTION    (0)
#define NO_MOTION (1)

/* Starting sampling rate. (default 20) */
#define DEFAULT_MPU_HZ (200)

// #define FLASH_SIZE      (512)
// #define FLASH_MEM_START ((void*)0x1800)

#define PEDO_READ_MS    (1000)
#define TEMP_READ_MS    (500)
#define COMPASS_READ_MS (100)

// Imu report struct
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
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
} imu_report_t;

struct rx_s
{
    unsigned char header[3];
    unsigned char cmd;
};

struct hal_s
{
    unsigned char lp_accel_mode;
    unsigned char sensors;
    unsigned char dmp_on;
    unsigned char wait_for_tap;
    volatile unsigned char new_gyro;
    unsigned char motion_int_mode;
    unsigned long no_dmp_hz;
    unsigned long next_pedo_ms;
    unsigned long next_temp_ms;
    unsigned long next_compass_ms;
    imu_report_t report;
    unsigned short source_id;
    unsigned short dmp_features;
    struct rx_s rx;
};
struct hal_s hal;

/* Platform-specific information. Kinda like a boardfile. */
struct platform_data_s
{
    signed char orientation[9];
};

#define COMPASS_ENABLED 1

void mpu_setup(void);
void read_from_mpl(service_t *service);

#endif /* __MPU_SETUP_H__ */
