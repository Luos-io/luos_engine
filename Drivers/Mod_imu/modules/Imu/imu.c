#include "main.h"
#include "imu.h"
#include <mpu_configuration.h>

volatile uint32_t hal_timestamp = 0;
unsigned char *mpl_key = (unsigned char*)"eMPL 5.1";

module_t *module_pointer;
volatile msg_t pub_msg;
volatile int pub = LUOS_PROTOCOL_NB;


/* Every time new gyro data is available, this function is called in an
 * ISR context. In this example, it sets a flag protecting the FIFO read
 * function.
 */
void gyro_data_ready_cb(void)
{
    hal.new_gyro = 1;
}

void HAL_SYSTICK_Callback(void) {
    if (pub != LUOS_PROTOCOL_NB) {
        luos_send(module_pointer, &pub_msg);
        pub = LUOS_PROTOCOL_NB;
    }
}

void rx_imu_cb(module_t *module, msg_t *msg) {
    if (msg->header.cmd == ASK_PUB_CMD) {
        // fill the message infos
        hal.update_request = 1;
        module_pointer = module;
        hal.source_id = msg->header.source;
        pub = LUOS_PROTOCOL_NB;
        return;
    }
    if (msg->header.cmd == PARAMETERS) {
        // check the message size
        if (msg->header.size == sizeof(short)) {
            module_pointer = module;
            // fill the message infos
            memcpy(&hal.report, msg->data, msg->header.size);
        }
        pub = LUOS_PROTOCOL_NB;
        return;
    }
}


void imu_init(void) {
    mpu_setup();
    hal.report.quat = 1;
    luos_module_enable_rt(luos_module_create(rx_imu_cb, IMU_MOD, "Imu_mod"));
}


void imu_loop(void) {
    // *********************IMU management*******************************
    unsigned long sensor_timestamp;
    unsigned long timestamp;
    int new_data = 0;
    timestamp = HAL_GetTick();
    static unsigned char new_temp = 0;
#ifdef COMPASS_ENABLED
    static unsigned char new_compass = 0;
    /* We're not using a data ready interrupt for the compass, so we'll
     * make our compass reads timer-based instead.
     */
    if ((timestamp > hal.next_compass_ms) && !hal.lp_accel_mode &&
        hal.new_gyro && (hal.sensors & COMPASS_ON)) {
        hal.next_compass_ms = timestamp + COMPASS_READ_MS;
        new_compass = 1;
    }
#endif
    /* Temperature data doesn't need to be read with every gyro sample.
     * Let's make them timer-based like the compass reads.
     */
    if (timestamp > hal.next_temp_ms) {
        hal.next_temp_ms = timestamp + TEMP_READ_MS;
        new_temp = 1;
    }
    if (hal.motion_int_mode) {
        /* Enable motion interrupt. */
        mpu_lp_motion_interrupt(500, 1, 5);
        /* Notify the MPL that contiguity was broken. */
        inv_accel_was_turned_off();
        inv_gyro_was_turned_off();
        inv_compass_was_turned_off();
        inv_quaternion_sensor_was_turned_off();
        /* Wait for the MPU interrupt. */
        while (!hal.new_gyro) {}
        /* Restore the previous sensor configuration. */
        mpu_lp_motion_interrupt(0, 0, 0);
        hal.motion_int_mode = 0;
    }
    if (!hal.sensors || !hal.new_gyro) {
        // we don't need to continue this loop
        return;
    }
    if (hal.new_gyro && hal.lp_accel_mode) {
        short accel_short[3];
        long accel[3];
        mpu_get_accel_reg(accel_short, &sensor_timestamp);
        accel[0] = (long)accel_short[0];
        accel[1] = (long)accel_short[1];
        accel[2] = (long)accel_short[2];
        inv_build_accel(accel, 0, sensor_timestamp);
        new_data = 1;
        hal.new_gyro = 0;
    } else if (hal.new_gyro && hal.dmp_on) {
        short gyro[3], accel_short[3], sensors;
        unsigned char more;
        long accel[3], quat[4], temperature;
        /* This function gets new data from the FIFO when the DMP is in
         * use. The FIFO can contain any combination of gyro, accel,
         * quaternion, and gesture data. The sensors parameter tells the
         * caller which data fields were actually populated with new data.
         * For example, if sensors == (INV_XYZ_GYRO | INV_WXYZ_QUAT), then
         * the FIFO isn't being filled with accel data.
         * The driver parses the gesture data to determine if a gesture
         * event has occurred; on an event, the application will be notified
         * via a callback (assuming that a callback function was properly
         * registered). The more parameter is non-zero if there are
         * leftover packets in the FIFO.
         */
        dmp_read_fifo(gyro, accel_short, quat, &sensor_timestamp, &sensors, &more);
        if (!more)
            hal.new_gyro = 0;
        if (sensors & INV_XYZ_GYRO) {
            /* Push the new data to the MPL. */
            inv_build_gyro(gyro, sensor_timestamp);
            new_data = 1;
            if (new_temp) {
                new_temp = 0;
                /* Temperature only used for gyro temp comp. */
                mpu_get_temperature(&temperature, &sensor_timestamp);
                inv_build_temp(temperature, sensor_timestamp);
            }
        }
        if (sensors & INV_XYZ_ACCEL) {
            accel[0] = (long)accel_short[0];
            accel[1] = (long)accel_short[1];
            accel[2] = (long)accel_short[2];
            inv_build_accel(accel, 0, sensor_timestamp);
            new_data = 1;
        }
        if (sensors & INV_WXYZ_QUAT) {
            inv_build_quat(quat, 0, sensor_timestamp);
            new_data = 1;
        }
    } else if (hal.new_gyro) {
        short gyro[3], accel_short[3];
        unsigned char sensors, more;
        long accel[3], temperature;
        /* This function gets new data from the FIFO. The FIFO can contain
         * gyro, accel, both, or neither. The sensors parameter tells the
         * caller which data fields were actually populated with new data.
         * For example, if sensors == INV_XYZ_GYRO, then the FIFO isn't
         * being filled with accel data. The more parameter is non-zero if
         * there are leftover packets in the FIFO. The HAL can use this
         * information to increase the frequency at which this function is
         * called.
         */
        hal.new_gyro = 0;
        mpu_read_fifo(gyro, accel_short, &sensor_timestamp,
            &sensors, &more);
        if (more)
            hal.new_gyro = 1;
        if (sensors & INV_XYZ_GYRO) {
            /* Push the new data to the MPL. */
            inv_build_gyro(gyro, sensor_timestamp);
            new_data = 1;
            if (new_temp) {
                new_temp = 0;
                /* Temperature only used for gyro temp comp. */
                mpu_get_temperature(&temperature, &sensor_timestamp);
                inv_build_temp(temperature, sensor_timestamp);
            }
        }
        if (sensors & INV_XYZ_ACCEL) {
            accel[0] = (long)accel_short[0];
            accel[1] = (long)accel_short[1];
            accel[2] = (long)accel_short[2];
            inv_build_accel(accel, 0, sensor_timestamp);
            new_data = 1;
        }
    }
#ifdef COMPASS_ENABLED
    if (new_compass) {
        short compass_short[3];
        long compass[3];
        new_compass = 0;
        /* For any MPU device with an AKM on the auxiliary I2C bus, the raw
         * magnetometer registers are copied to special gyro registers.
         */
        if (!mpu_get_compass_reg(compass_short, &sensor_timestamp)) {
            compass[0] = (long)compass_short[0];
            compass[1] = (long)compass_short[1];
            compass[2] = (long)compass_short[2];
            /* NOTE: If using a third-party compass calibration library,
             * pass in the compass data in uT * 2^16 and set the second
             * parameter to INV_CALIBRATED | acc, where acc is the
             * accuracy from 0 to 3.
             */
            inv_build_compass(compass, 0, sensor_timestamp);
        }
        new_data = 1;
    }
#endif
    if (new_data) {
        inv_execute_on_data();
        /* This function reads bias-compensated sensor data and sensor
         * fusion outputs from the MPL. The outputs are formatted as seen
         * in eMPL_outputs.c. This function only needs to be called at the
         * rate requested by the host.
         */
    }
    if (hal.update_request == 1) {
        read_from_mpl(module_pointer);
        hal.update_request = 0;
    }
}
