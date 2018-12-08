#ifndef MODULE_LIST_H
#define MODULE_LIST_H

#include <robus.h>

#define GATE_ID 1

typedef enum {
    GATE,
    SERVO,
    RGB_LED,
    POTENTIOMETER,
    BUTTON,
    DISTANCE,
    RELAY,
    DYNAMIXEL,
    STEPPER,
    DCMOTOR,
    GPIO,
    HANDY,
    IMU,
    LIGHT,
    CONTROLLED_MOTOR
} module_type_t;

typedef enum {
    // Common register for all modules and gate
    IDENTIFY_CMD, // Gate asks a module to identify itself
    INTRODUCTION_CMD, // Module sends its alias and type to the gate

    ASK_PUB_CMD, // Gate asks a sensor module to publish its data
    REVISION, // Module sends its firmware revision

    // Generic data register
    COLOR, // char[3] (R, G, B)
    COMPLIANT, // char (True/False)
    IO_STATE, // char (True/False)
    POWER_RATIO, // float (-100.0% <-> 100.0%)
    PEDOMETER, // long[2] (step number and step time millisecond)
    LUX, // float (Lux)
    CURRENT, // float (Ampere)

    // Configuration commands
    REINIT, // char (True/False)
    STOP, // char (True/False)
    PID, // pid_t float[3] = {p, i, d}
    RESOLUTION, // resolution parameter for a sensor float
    REDUCTION, // reduction factor (mechanical for example) float
    DIMENSION, // dimention of an element mm float

    // L0 things
    L0_TEMPERATURE, // float (°c)
    L0_VOLTAGE, // float (V)
    L0_LED, // char (True/False)

    // Space positioning
    ROTATION_POSITION, // float (°)
    ROTATION_SPEED, // float (°/s)
    TRANSLATION_POSITION, // Float (mm)
    TRANSLATION_SPEED, // Float (mm/s)
    ACCEL_3D, // long[3](X, Y, Z axis linear acceleration data in Gees)
    GYRO_3D, // long[3](X, Y, Z axis rotational acceleration data in degrees per second)
    QUATERNION, // long[4] (sensor fused w, x, y, z rotational angles)
    COMPASS_3D, // long[3](magnetic field data in micro tesla on each axis)
    EULER_3D, // long[3](Pitch, roll, yaw based in degrees with frame reference)
    ROT_MAT, // short[9] (linear math 9 element matrix representation)
    LINEAR_ACCEL, // float[3] (linear acceleration in body frame coordinates)
    GRAVITY_VECTOR, // float[3] (Which access gravity effects)
    HEADING, // long (360 degrees from North with Y+ axis as the pointer)

    // Specific register
    DXL_WHEELMODE, // char (True/False) TODO => should be managed by MOTOR_REPORT
    GPIO_GET_STATE, // gpio_t
    GPIO_SET_STATE, // char[3] (p2, p3, p4)
    HANDY_SET_POSITION, // handy_t
    IMU_REPORT, // imu_report_t (enable and disable IMU value type reporting)
    MOTOR_REPORT, // motor_mode_t (enable and disable motor value type reporting and target mode)

    LUOS_PROTOCOL_NB,
} module_register_t;


#endif /* MODULE_LIST_H */
