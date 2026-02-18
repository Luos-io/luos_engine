#include <mpu_configuration.h>
#include "invensense.h"
#include "invensense_adv.h"
#include "mpu.h"
#include "main.h"
#include <eMPL_outputs.h>

/* The sensors can be mounted onto the board in any orientation. The mounting
 * matrix seen below tells the MPL how to rotate the raw data from the
 * driver(s).
 * TODO: The following matrices refer to the configuration on internal test
 * boards at Invensense. If needed, please modify the matrices to match the
 * chip-to-body matrix for your particular set up.
 */
struct platform_data_s gyro_pdata = {
    .orientation = {1, 0, 0,
                    0, 1, 0,
                    0, 0, 1}};

struct platform_data_s compass_pdata = {
    .orientation = {0, 1, 0,
                    1, 0, 0,
                    0, 0, -1}};

/* Get data from MPL.
 * TODO: Add return values to the inv_get_sensor_type_xxx APIs to differentiate
 * between new and stale data.
 */
void read_from_mpl(service_t *service)
{
    long data[9];
    int8_t accuracy;
    unsigned long timestamp;
    float float_data[3] = {0};
    msg_t msg;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.target      = hal.source_id;

    if (inv_get_sensor_type_quat(data, &accuracy, (inv_time_t *)&timestamp))
    {
        /* Sends a quaternion packet to the PC. Since this is used by the Python
         * test app to visually represent a 3D quaternion, it's sent each time
         * the MPL has new data.
         */
        if (hal.report.quat)
        {
            msg.header.cmd  = QUATERNION;
            msg.header.size = 4 * sizeof(float);
            float quat[4]   = {(float)data[0] / (1 << 30), (float)data[1] / (1 << 30), (float)data[2] / (1 << 30), (float)data[3] / (1 << 30)};
            memcpy(msg.data, quat, msg.header.size);
            Luos_SendMsg(service, &msg);
        }
    }

    if (hal.report.accell)
    {
        if (inv_get_sensor_type_accel(data, &accuracy, (inv_time_t *)&timestamp))
        {
            msg.header.cmd  = ACCEL_3D;
            msg.header.size = 3 * sizeof(float);
            float accell[3] = {(float)data[0] / (1 << 16), (float)data[1] / (1 << 16), (float)data[2] / (1 << 16)};
            memcpy(msg.data, accell, msg.header.size);
            Luos_SendMsg(service, &msg);
        }
    }
    if (hal.report.gyro)
    {
        if (inv_get_sensor_type_gyro(data, &accuracy, (inv_time_t *)&timestamp))
        {
            msg.header.cmd  = GYRO_3D;
            msg.header.size = 3 * sizeof(float);
            float gyro[3]   = {(float)data[0] / (1 << 16), (float)data[1] / (1 << 16), (float)data[2] / (1 << 16)};
            memcpy(msg.data, gyro, msg.header.size);
            Luos_SendMsg(service, &msg);
        }
    }
#ifdef COMPASS_ENABLED
    if (hal.report.compass)
    {
        if (inv_get_sensor_type_compass(data, &accuracy, (inv_time_t *)&timestamp))
        {
            msg.header.cmd   = COMPASS_3D;
            msg.header.size  = 3 * sizeof(float);
            float compass[3] = {(float)data[0] / (1 << 16), (float)data[1] / (1 << 16), (float)data[2] / (1 << 16)};
            memcpy(msg.data, compass, msg.header.size);
            Luos_SendMsg(service, &msg);
        }
    }
#endif
    if (hal.report.euler)
    {
        if (inv_get_sensor_type_euler(data, &accuracy, (inv_time_t *)&timestamp))
        {
            msg.header.cmd  = EULER_3D;
            msg.header.size = 3 * sizeof(float);
            float euler[3]  = {(float)data[0] / (1 << 16), (float)data[1] / (1 << 16), (float)data[2] / (1 << 16)};
            memcpy(msg.data, euler, msg.header.size);
            Luos_SendMsg(service, &msg);
        }
    }
    if (hal.report.rot_mat)
    {
        if (inv_get_sensor_type_rot_mat(data, &accuracy, (inv_time_t *)&timestamp))
        {
            msg.header.cmd  = ROT_MAT;
            msg.header.size = 9 * sizeof(float);
            short tmp[9]    = {(float)data[0] / (1 << 14), (float)data[1] / (1 << 14), (float)data[2] / (1 << 14), (float)data[3] / (1 << 14), (float)data[4] / (1 << 14), (float)data[5] / (1 << 14), (float)data[6] / (1 << 14), (float)data[7] / (1 << 14), (float)data[8] / (1 << 14)};
            memcpy(msg.data, tmp, msg.header.size);
            Luos_SendMsg(service, &msg);
        }
    }
    if (hal.report.heading)
    {
        if (inv_get_sensor_type_heading(data, &accuracy, (inv_time_t *)&timestamp))
        {
            msg.header.cmd  = HEADING;
            msg.header.size = sizeof(float);
            float heading   = (float)data[0] / (1 << 16);
            memcpy(msg.data, &heading, msg.header.size);
            Luos_SendMsg(service, &msg);
        }
    }
    if (hal.report.linear_accel)
    {
        if (inv_get_sensor_type_linear_acceleration(float_data, &accuracy, (inv_time_t *)&timestamp))
        {
            msg.header.cmd  = LINEAR_ACCEL;
            msg.header.size = 3 * sizeof(float);
            memcpy(msg.data, float_data, msg.header.size);
            Luos_SendMsg(service, &msg);
        }
    }
    if (hal.report.gravity_vector)
    {
        if (inv_get_sensor_type_gravity(float_data, &accuracy,
                                        (inv_time_t *)&timestamp))
        {
            msg.header.cmd  = GRAVITY_VECTOR;
            msg.header.size = 3 * sizeof(float);
            memcpy(msg.data, float_data, msg.header.size);
            Luos_SendMsg(service, &msg);
        }
    }
    if (hal.report.pedo)
    {
        unsigned long timestamp;
        timestamp = Luos_GetSystick();
        if (timestamp > hal.next_pedo_ms)
        {
            hal.next_pedo_ms = timestamp + PEDO_READ_MS;
            unsigned long step_count, walk_time;
            dmp_get_pedometer_step_count(&step_count);
            dmp_get_pedometer_walk_time(&walk_time);
            unsigned long pedo[2] = {step_count, walk_time};
            msg.header.cmd        = PEDOMETER;
            msg.header.size       = 2 * sizeof(long);
            memcpy(msg.data, pedo, msg.header.size);
            Luos_SendMsg(service, &msg);
        }
    }
}

#ifdef COMPASS_ENABLED
void send_status_compass()
{
    long data[3]    = {0};
    int8_t accuracy = {0};
    unsigned long timestamp;
    inv_get_compass_set(data, &accuracy, (inv_time_t *)&timestamp);
    MPL_LOGI("Compass: %7.4f %7.4f %7.4f ",
             data[0] / 65536.f, data[1] / 65536.f, data[2] / 65536.f);
    MPL_LOGI("Accuracy= %d\r\n", accuracy);
}
#endif

static void tap_cb(unsigned char direction, unsigned char count)
{
    return;
    switch (direction)
    {
        case TAP_X_UP:
            // MPL_LOGI("Tap X+ ");
            while (1)
                ;
            break;
        case TAP_X_DOWN:
            // MPL_LOGI("Tap X- ");
            while (1)
                ;
            break;
        case TAP_Y_UP:
            // MPL_LOGI("Tap Y+ ");
            while (1)
                ;
            break;
        case TAP_Y_DOWN:
            // MPL_LOGI("Tap Y- ");
            while (1)
                ;
            break;
        case TAP_Z_UP:
            // MPL_LOGI("Tap Z+ ");
            while (1)
                ;
            break;
        case TAP_Z_DOWN:
            // MPL_LOGI("Tap Z- ");
            while (1)
                ;
            break;
        default:
            return;
    }
    // MPL_LOGI("x%d\n", count);
    while (1)
        ;
    return;
}

static void android_orient_cb(unsigned char orientation)
{
    return;
    switch (orientation)
    {
        case ANDROID_ORIENT_PORTRAIT:
            // MPL_LOGI("Portrait\n");
            while (1)
                ;
            break;
        case ANDROID_ORIENT_LANDSCAPE:
            // MPL_LOGI("Landscape\n");
            while (1)
                ;
            break;
        case ANDROID_ORIENT_REVERSE_PORTRAIT:
            // MPL_LOGI("Reverse Portrait\n");
            while (1)
                ;
            break;
        case ANDROID_ORIENT_REVERSE_LANDSCAPE:
            // MPL_LOGI("Reverse Landscape\n");
            while (1)
                ;
            break;
        default:
            return;
    }
}

static inline void run_self_test(void)
{
    long gyro[3], accel[3];

#if defined(MPU6500) || defined(MPU9250)

    mpu_run_6500_self_test(gyro, accel, 0);
#elif defined(MPU6050) || defined(MPU9150)
    mpu_run_self_test(gyro, accel);
#endif
    /* Test passed. We can trust the gyro data here, so now we need to update calibrated data*/

#ifdef USE_CAL_HW_REGISTERS
    /*
     * This portion of the code uses the HW offset registers that are in the MPUxxxx devices
     * instead of pushing the cal data to the MPL software library
     */
    unsigned char i = 0;

    for (i = 0; i < 3; i++)
    {
        gyro[i] = (long)(gyro[i] * 32.8f); // convert to +-1000dps
        accel[i] *= 2048.f;                // convert to +-16G
        accel[i] = accel[i] >> 16;
        gyro[i]  = (long)(gyro[i] >> 16);
    }

    mpu_set_gyro_bias_reg(gyro);

    #if defined(MPU6500) || defined(MPU9250)
    mpu_set_accel_bias_6500_reg(accel);
    #elif defined(MPU6050) || defined(MPU9150)
    mpu_set_accel_bias_6050_reg(accel);
    #endif
#else
    /* Push the calibrated data to the MPL library.
     *
     * MPL expects biases in hardware units << 16, but self test returns
     * biases in g's << 16.
     */
    unsigned short accel_sens;
    float gyro_sens;

    mpu_get_accel_sens(&accel_sens);
    accel[0] *= accel_sens;
    accel[1] *= accel_sens;
    accel[2] *= accel_sens;
    inv_set_accel_bias(accel, 3);
    mpu_get_gyro_sens(&gyro_sens);
    gyro[0] = (long)(gyro[0] * gyro_sens);
    gyro[1] = (long)(gyro[1] * gyro_sens);
    gyro[2] = (long)(gyro[2] * gyro_sens);
    inv_set_gyro_bias(gyro, 3);
#endif
}

void mpu_setup(void)
{
    inv_error_t result;
    unsigned char accel_fsr;
    unsigned short gyro_rate, gyro_fsr;
    struct int_param_s int_param;
#ifdef COMPASS_ENABLED
    unsigned short compass_fsr;
#endif
    result = mpu_init(&int_param);
    if (result)
    {
        // Could not initialize gyro.
        LUOS_ASSERT(0);
    }

    /* If you're not using an MPU9150 AND you're not using DMP features, this
     * function will place all slaves on the primary bus.
     * mpu_set_bypass(1);
     */

    result = inv_init_mpl();
    if (result)
    {
        // Could not initialize MPL.
        LUOS_ASSERT(0);
    }

    /* Compute 6-axis and 9-axis quaternions. */
    inv_enable_quaternion();
    inv_enable_9x_sensor_fusion();
    /* The MPL expects compass data at a constant rate (matching the rate
     * passed to inv_set_compass_sample_rate). If this is an issue for your
     * application, call this function, and the MPL will depend on the
     * timestamps passed to inv_build_compass instead.
     *
     * inv_9x_fusion_use_timestamps(1);
     */

    /* Update gyro biases when not in motion.
     * WARNING: These algorithms are mutually exclusive.
     */
    inv_enable_fast_nomot();
    /* inv_enable_motion_no_motion(); */
    /* inv_set_no_motion_time(1000); */

    /* Update gyro biases when temperature changes. */
    inv_enable_gyro_tc();

    /* This algorithm updates the accel biases when in motion. A more accurate
     * bias measurement can be made when running the self-test (see case 't' in
     * handle_input), but this algorithm can be enabled if the self-test can't
     * be executed in your application.
     *
     * inv_enable_in_use_auto_calibration();
     */

#ifdef COMPASS_ENABLED
    /* Compass calibration algorithms. */
    inv_enable_vector_compass_cal();
    inv_enable_magnetic_disturbance();
#endif
    /* If you need to estimate your heading before the compass is calibrated,
     * enable this algorithm. It becomes useless after a good figure-eight is
     * detected, so we'll just leave it out to save memory.
     * inv_enable_heading_from_gyro();
     */

    /* Allows use of the MPL APIs in read_from_mpl. */
    inv_enable_eMPL_outputs();

    result = inv_start_mpl();
    if (result == INV_ERROR_NOT_AUTHORIZED)
    {
        // Not authorized.
        while (1)
            ;
    }
    if (result)
    {
        // Could not start the MPL.
        while (1)
            ;
    }

    /* Get/set hardware configuration. Start gyro. */
    /* Wake up all sensors. */
#ifdef COMPASS_ENABLED
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
#else
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
#endif
    /* Push both gyro and accel data into the FIFO. */
    mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    mpu_set_sample_rate(DEFAULT_MPU_HZ);
#ifdef COMPASS_ENABLED
    /* The compass sampling rate can be less than the gyro/accel sampling rate.
     * Use this function for proper power management.
     */
    mpu_set_compass_sample_rate(1000 / COMPASS_READ_MS);
#endif
    /* Read back configuration in case it was set improperly. */
    mpu_get_sample_rate(&gyro_rate);
    mpu_get_gyro_fsr(&gyro_fsr);
    mpu_get_accel_fsr(&accel_fsr);
#ifdef COMPASS_ENABLED
    mpu_get_compass_fsr(&compass_fsr);
#endif
    /* Sync driver configuration with MPL. */
    /* Sample rate expected in microseconds. */
    inv_set_gyro_sample_rate(1000000L / gyro_rate);
    inv_set_accel_sample_rate(1000000L / gyro_rate);
#ifdef COMPASS_ENABLED
    /* The compass rate is independent of the gyro and accel rates. As long as
     * inv_set_compass_sample_rate is called with the correct value, the 9-axis
     * fusion algorithm's compass correction gain will work properly.
     */
    inv_set_compass_sample_rate(COMPASS_READ_MS * 1000L);
#endif
    /* Set chip-to-body orientation matrix.
     * Set hardware units to dps/g's/degrees scaling factor.
     */
    inv_set_gyro_orientation_and_scale(
        inv_orientation_matrix_to_scalar(gyro_pdata.orientation),
        (long)gyro_fsr << 15);
    inv_set_accel_orientation_and_scale(
        inv_orientation_matrix_to_scalar(gyro_pdata.orientation),
        (long)accel_fsr << 15);
#ifdef COMPASS_ENABLED
    inv_set_compass_orientation_and_scale(
        inv_orientation_matrix_to_scalar(compass_pdata.orientation),
        (long)compass_fsr << 15);
#endif
    /* Initialize HAL state variables. */
#ifdef COMPASS_ENABLED
    hal.sensors = ACCEL_ON | GYRO_ON | COMPASS_ON;
#else
    hal.sensors = ACCEL_ON | GYRO_ON;
#endif
    hal.dmp_on          = 0;
    hal.report.unmap[0] = 0;
    hal.report.unmap[1] = 0;
    hal.rx.cmd          = 0;
    hal.next_pedo_ms    = 0;
    hal.next_compass_ms = 0;
    hal.next_temp_ms    = 0;

    /* Compass reads are handled by scheduler. */

    /* To initialize the DMP:
     * 1. Call dmp_load_motion_driver_firmware(). This pushes the DMP image in
     *    inv_mpu_dmp_motion_driver.h into the MPU memory.
     * 2. Push the gyro and accel orientation matrix to the DMP.
     * 3. Register gesture callbacks. Don't worry, these callbacks won't be
     *    executed unless the corresponding feature is enabled.
     * 4. Call dmp_enable_feature(mask) to enable different features.
     * 5. Call dmp_set_fifo_rate(freq) to select a DMP output rate.
     * 6. Call any feature-specific control functions.
     *
     * To enable the DMP, just call mpu_set_dmp_state(1). This function can
     * be called repeatedly to enable and disable the DMP at runtime.
     *
     * The following is a short summary of the features supported in the DMP
     * image provided in inv_mpu_dmp_motion_driver.c:
     * DMP_FEATURE_LP_QUAT: Generate a gyro-only quaternion on the DMP at
     * 200Hz. Integrating the gyro data at higher rates reduces numerical
     * errors (compared to integration on the MCU at a lower sampling rate).
     * DMP_FEATURE_6X_LP_QUAT: Generate a gyro/accel quaternion on the DMP at
     * 200Hz. Cannot be used in combination with DMP_FEATURE_LP_QUAT.
     * DMP_FEATURE_TAP: Detect taps along the X, Y, and Z axes.
     * DMP_FEATURE_ANDROID_ORIENT: Google's screen rotation algorithm. Triggers
     * an event at the four orientations where the screen should rotate.
     * DMP_FEATURE_GYRO_CAL: Calibrates the gyro data after eight seconds of
     * no motion.
     * DMP_FEATURE_SEND_RAW_ACCEL: Add raw accelerometer data to the FIFO.
     * DMP_FEATURE_SEND_RAW_GYRO: Add raw gyro data to the FIFO.
     * DMP_FEATURE_SEND_CAL_GYRO: Add calibrated gyro data to the FIFO. Cannot
     * be used in combination with DMP_FEATURE_SEND_RAW_GYRO.
     */
    dmp_load_motion_driver_firmware();
    dmp_set_orientation(
        inv_orientation_matrix_to_scalar(gyro_pdata.orientation));
    dmp_register_tap_cb(tap_cb);
    dmp_register_android_orient_cb(android_orient_cb);
    /*
     * Known Bug -
     * DMP when enabled will sample sensor data at 200Hz and output to FIFO at the rate
     * specified in the dmp_set_fifo_rate API. The DMP will then sent an interrupt once
     * a sample has been put into the FIFO. Therefore if the dmp_set_fifo_rate is at 25Hz
     * there will be a 25Hz interrupt from the MPU device.
     *
     * There is a known issue in which if you do not enable DMP_FEATURE_TAP
     * then the interrupts will be at 200Hz even if fifo rate
     * is set at a different rate. To avoid this issue include the DMP_FEATURE_TAP
     *
     * DMP sensor fusion works only with gyro at +-2000dps and accel +-2G
     */
    hal.dmp_features = DMP_FEATURE_6X_LP_QUAT
                       | DMP_FEATURE_TAP
                       | DMP_FEATURE_ANDROID_ORIENT
                       | DMP_FEATURE_SEND_RAW_ACCEL
                       | DMP_FEATURE_SEND_CAL_GYRO
                       | DMP_FEATURE_GYRO_CAL;
    dmp_enable_feature(hal.dmp_features);
    dmp_set_fifo_rate(DEFAULT_MPU_HZ);
    mpu_set_dmp_state(1);
    hal.dmp_on = 1;
}
