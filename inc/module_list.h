#ifndef MODULE_LIST_H
#define MODULE_LIST_H

#include <robus.h>

typedef enum
{
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
    LOAD_MOD,
    LUOS_LAST_TYPE
} module_type_t;

typedef enum
{
    // Common register for all modules and gate
    IDENTIFY_CMD,     // Gate asks a module to identify itself
    INTRODUCTION_CMD, // Module sends its alias and type to the gate
    WRITE_ALIAS,      // Get and save a new given alias.
    ASK_PUB_CMD,      // Gate asks a sensor module to publish its data
    UPDATE_PUB,       // Ask to update a sensor value each time duration to the sender

    // Generic data
    COLOR,       // color_t (R, G, B)
    COMPLIANT,   // char (True/False)
    IO_STATE,    // char (True/False)
    RATIO,       // ratio_t (percentage %)
    PEDOMETER,   // long[2] (step number and step time millisecond)
    ILLUMINANCE, // illuminance_t (lx)
    VOLTAGE,     // voltage_t (Volt)
    CURRENT,     // current_t (Ampere)
    POWER,       // power_t (Watt)
    TEMPERATURE, // temperature_t (°C)
    TIME,        // time Second (float)
    FORCE,       // force_t (Newton N)
    MOMENT,      // moment_t (Newton meter N.m)
    CONTROL,     // control_mode (control_mode_t)

    // Configuration commands
    REGISTER,   // a register data [reg_add, data[]]
    REINIT,     // char (True/False)
    PID,        // pid_t float[3] = {p, i, d}
    RESOLUTION, // resolution parameter for a sensor float
    REDUCTION,  // reduction factor (mechanical for example) float
    DIMENSION,  // dimention of an element m linear_position_t
    OFFSET,     // decay float
    SETID,      // Set Dynamixel ID

    // Node things
    NODE_TEMPERATURE, // temperature_t (°c)
    NODE_VOLTAGE,     // voltage_t (V)
    NODE_LED,         // char (True/False)
    NODE_UUID,        // luos_uuid_t

    // Space positioning
    ANGULAR_POSITION, // angular_position_t (deg)
    ANGULAR_SPEED,    // angular_speed_t (deg/s)
    LINEAR_POSITION,  // linear_position_t (m)
    LINEAR_SPEED,     // linear_speed_t (m/s)
    ACCEL_3D,         // long[3](X, Y, Z axis linear acceleration data in Gees)
    GYRO_3D,          // long[3](X, Y, Z axis rotational acceleration data in degrees per second)
    QUATERNION,       // long[4] (sensor fused w, x, y, z rotational angles)
    COMPASS_3D,       // long[3](magnetic field data in micro tesla on each axis)
    EULER_3D,         // long[3](Pitch, roll, yaw based in degrees with frame reference)
    ROT_MAT,          // short[9] (linear math 9 element matrix representation)
    LINEAR_ACCEL,     // float[3] (linear acceleration in body frame coordinates)
    GRAVITY_VECTOR,   // float[3] (Which access gravity effects)
    HEADING,          // long (360 degrees from North with Y+ axis as the pointer)

    // Space positioning limits
    ANGULAR_POSITION_LIMIT, // min angular_position_t (deg), max angular_position_t (deg)
    LINEAR_POSITION_LIMIT,  // min linear_position_t (m), max linear_position_t (m)
    RATIO_LIMIT,            // float(%)
    CURRENT_LIMIT,          // float(A)
    ANGULAR_SPEED_lIMIT,    // min angular_speed_t (deg/s), max angular_speed_t (deg/s)
    LINEAR_SPEED_LIMIT,     // min linear_speed_t (m/s), max linear_speed_t (m/s)
    TORQUE_LIMIT,           // max moment_t (Nm)

    // Specific register
    DXL_WHEELMODE,      // char (True/False) TODO => should be managed by MOTOR_REPORT the same way as controlled motor
    HANDY_SET_POSITION, // handy_t
    PARAMETERS,         // depend on the module, can be : servo_parameters_t, imu_report_t, motor_mode_t

    // Revision management
    REVISION,      // Module sends its firmware revision
    LUOS_REVISION, // Module sends its luos revision

    // compatibility area
    LUOS_PROTOCOL_NB,
} module_register_t;

#endif /* MODULE_LIST_H */
