#ifndef MODULE_LIST_H
#define MODULE_LIST_H

#include <robus.h>

#define GATE_ID 1

typedef enum {
    GATE_MOD,
    SERVO_MOD,
    COLOR_MOD,
    ANGLE_MOD,
    STATE_MOD,
    DISTANCE_MOD,
    VOLTAGE_MOD,
    DYNAMIXEL_MOD,
    STEPPER_MOD,
    DCMOTOR_MOD,
    Compatibility_line,
    HANDY_MOD,
    IMU_MOD,
    LIGHT_MOD,
    CONTROLLED_MOTOR_MOD,
    VOID_MOD,
    LOAD_MOD
} module_type_t;

typedef enum {
    // Common register for all modules and gate
    IDENTIFY_CMD, // Gate asks a module to identify itself
    INTRODUCTION_CMD, // Module sends its alias and type to the gate

    ASK_PUB_CMD, // Gate asks a sensor module to publish its data
    REVISION, // Module sends its firmware revision

    // Generic data
    COLOR, // char[3] (R, G, B)
    COMPLIANT, // char (True/False)
    IO_STATE, // char (True/False)
    POWER_RATIO, // float (-100.0% <-> 100.0%)
    PEDOMETER, // long[2] (step number and step time millisecond)
    LUX, // float (Lux)
    CURRENT, // float (Ampere)
    TEMPERATURE, // float (°C)
    TIME, // time Second (float)
    FORCE, // force or moment (float)

    // Configuration commands
    REGISTER, // a register data [reg_add, data[]]
    REINIT, // char (True/False)
    STOP, // char (True/False)
    PID, // pid_t float[3] = {p, i, d}
    RESOLUTION, // resolution parameter for a sensor float
    REDUCTION, // reduction factor (mechanical for example) float
    DIMENSION, // dimention of an element mm float
    OFFSET, // decay float
    SETID, // Set Dynamixel ID

    // L0 things
    L0_TEMPERATURE, // float (°c)
    L0_VOLTAGE, // float (V)
    L0_LED, // char (True/False)
    UUID, // luos_uuid_t

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
    DXL_WHEELMODE, // char (True/False) TODO => should be managed by MOTOR_REPORT the same way as controlled motor
    VOLT, // float (Volt) TODO : move it in the right place for rev 6
    REFRESH_PUB, // Ask to send a sensor value at a given max frequency TODO : move it in the right place for rev 6
    HANDY_SET_POSITION, // handy_t
    PARAMETERS, // depend on the module, can be : servo_parameters_t, imu_report_t, motor_mode_t

    // compatibility area
    // Space positioning limits
    ROTATION_POSITION_LIMIT, // min float (°), max float (°)
    TRANSLATION_POSITION_LIMIT, // min float (mm), max float (mm)
    POWER_LIMIT, // float(%)
    CURRENT_LIMIT, // float(A)
    WRITE_ALIAS, // Get and save a new given alias.

    LUOS_PROTOCOL_NB,
} module_register_t;


#endif /* MODULE_LIST_H */
