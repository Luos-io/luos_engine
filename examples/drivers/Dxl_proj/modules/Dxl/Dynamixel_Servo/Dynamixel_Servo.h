/*!
 * @header Dynamixel_Servo
 * @abstract <span style="font-family:monospace"><pre>
 *   ____                              _          _   ____
 *  |  _ \ _   _ _ __   __ _ _ __ ___ (_)_  _____| | / ___|  ___ _ ____   _____
 *  | | | | | | | '_ \ / _` | '_ ` _ \| \ \/ / _ \ | \___ \ / _ \ '__\ \ / / _ \
 *  | |_| | |_| | | | | (_| | | | | | | |\  \  __/ |  ___) |  __/ |   \ V / (_) |
 *  |____/ \__, |_| |_|\__,_|_| |_| |_|_/_/\_\___|_| |____/ \___|_|    \_/ \___/
 *         |___/
 *  </pre></span>
 *
 *  Dynamixel_Servo is an Arduino compatible library for controlling Dynamixel
 *  MX and RX series servos. The servo will have to be connected to the Arduino
 *  using the appropriate circuit as shown here:
 *
 *  http://support.robotis.com/en/product/dynamixel/dxl_mx_main.htm
 *
 *  It is also helpful to use an Arduino with 2 serial ports: one to communicate
 *  with your laptop and one with the servo (the 32u4 - based Arduini are pretty
 *  handy for this). Note that this library is fully compatible with the existing
 *  Arduino library, and does not require the user to change anything in the
 *  Arduino source.
 *
 * @version
 *  2.1 (Sept 8, 2015)
 *
 * @copyright
 *  This code is open-source and free for all. Enjoy!
 *
 * @discussion
 *  This module presents a high-level interface and a low-level interface for
 *  manipulating the servo's registers. The high-level interface looks up
 *  the width (one or two bytes) of the specified value and manipulates all
 *  relevant registers (low byte and high byte) as a single value. The high-
 *  level interface also performs automatic conversions between the raw values
 *  defined in the servo's datasheet, and physical values such as radians,
 *  radians-per-second, Amperes, Volts, baud, microseconds, degrees-Celcius,
 *  percent (of the motor's maximum torque) etc... as appropriate to the value.
 *  The down-side is that the high-level interface hogs several kilobytes of
 *  code space, as the result of using floating-point math and having some
 *  large-ish lookup tables. Therefore a low-level interface is provided that
 *  allows more straightforward access to the servo's registers using raw values,
 *  which may optionally be converted to and from physical values using the
 *  provided functions. Using the low-level interface and raw values may save
 *  several kilobytes of code space (The compiler won't compile functions
 *  that aren't used). The low-level interface also provides the capability
 *  to access several of the servo's values in a single command using the
 *  xxx_raw_page functions. More information on how to use the servos can be
 *  found here:<br />
 *
 *  http://support.robotis.com/en/product/dynamixel/mx_series/mx-64.htm <br />
 *  http://support.robotis.com/en/product/dynamixel/communication/dxl_packet.htm <br />
 *  http://support.robotis.com/en/product/dynamixel/communication/dxl_instruction.htm <br />
 *
 *  Change Log
 *    2.0 adds support for SYNC_WRITE, and fixes bug with 8000 buad.
 *    2.1 fixed some long-standing workarounds for shoddy hardware.
 *
 *  @bug
 *   When the motor is powered on, the value returned servo_get(SERVO_REGISTER_MOVING_SPEED)
 *   will be incorrect (0) until it is set to some other value.
 *  @bug
 *   This module only supports one Dynamixel bus.
 *  @bug
 *   This module has only been tested with MX 64-T. It is unknown whether it works with others.
 *
 *
 * @author
 *  Written by Michael Krzyzaniak at Arizona State University's
 *  School of Arts, Media + Engineering in Spring of 2014.
 *
 * mkrzyzan@asu.edu
 *
 * @unsorted
 * @process-everything
 */

#ifndef DYNAMIXEL_SERVO
#define DYNAMIXEL_SERVO

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "math.h"

/*--------------------------------------------------------------------------------------------*/
#define SERVO_BROADCAST_ID               0xFE
#define SERVO_DEFAULT_ID                 1
#define SERVO_DEFAULT_BAUD               57600

#define SERVO_MAXIMUM_MOVING_SPEED       6.60   // rad / sec

/*--------------------------------------------------------------------------------------------*/
typedef enum servo_error_bit_mask_enum
{
  /* defined by dynamixel */
  SERVO_ERROR_VOLTAGE                  = 0x0001,
  SERVO_ERROR_ANGLE                    = 0x0002,
  SERVO_ERROR_OVERHEAT                 = 0x0004,
  SERVO_ERROR_RANGE                    = 0x0008,
  SERVO_ERROR_CHECKSUM                 = 0x0010,
  SERVO_ERROR_OVERLOAD                 = 0x0020,
  SERVO_ERROR_INSTRUCTION              = 0x0040,
  SERVO_ERROR_UNKNOWN                  = 0x0080,

  /* defined by this module */
  SERVO_ERROR_TIMEOUT                  = 0x0100,
  SERVO_ERROR_INVALID_RESPONSE         = 0x0200,

  /* raw value, not bit mask */
  SERVO_NO_ERROR                       = 0x0000,
}servo_error_t;

/*--------------------------------------------------------------------------------------------*/
#ifdef V2
typedef enum servo_register_enum
{
  /* EEPROM */
  SERVO_REGISTER_MODEL_NUMBER            = 0x00,
  //SERVO_REGISTER_MODEL_NUMBER_L        = 0x00,
  //SERVO_REGISTER_MODEL_NUMBER_H        = 0x01,
  SERVO_REGISTER_FIRMWARE_VERSION        = 0x02,
  SERVO_REGISTER_ID                      = 0x03,
  SERVO_REGISTER_BAUD_RATE               = 0x04,
  SERVO_REGISTER_RETURN_DELAY_TIME       = 0x05,
  SERVO_REGISTER_MIN_ANGLE               = 0x06,
  //SERVO_REGISTER_MIN_ANGLE_L           = 0x06,
  //SERVO_REGISTER_MIN_ANGLE_H           = 0x07,
  SERVO_REGISTER_MAX_ANGLE               = 0x08,
  //SERVO_REGISTER_MAX_ANGLE_L           = 0x08,
  //SERVO_REGISTER_MAX_ANGLE_H           = 0x09,
  SERVO_REGISTER_CONTROL_MODE            = 0x0B,
  SERVO_REGISTER_MAX_TEMPERATURE         = 0x0C,
  SERVO_REGISTER_MIN_VOLTAGE             = 0x0D,
  SERVO_REGISTER_MAX_VOLTAGE             = 0x0E,
  SERVO_REGISTER_MAX_TORQUE              = 0x0F,
  //SERVO_REGISTER_MAX_TORQUE_L          = 0x0F,
  //SERVO_REGISTER_MAX_TORQUE_H          = 0x10,
  SERVO_REGISTER_RETURN_LEVEL            = 0x11,
  SERVO_REGISTER_SHUTDOWN_ERROR          = 0x12,

  /* RAM */
  SERVO_REGISTER_TORQUE_ENABLE           = 0x18,
  SERVO_REGISTER_LED_IS_ON               = 0x19,
  SERVO_REGISTER_D_GAIN                  = 0x1B,
  SERVO_REGISTER_I_GAIN                  = 0x1C,
  SERVO_REGISTER_P_GAIN                  = 0x1D,
  SERVO_REGISTER_GOAL_ANGLE              = 0x1E,
  //SERVO_REGISTER_GOAL_ANGLE_L          = 0x1E,
  //SERVO_REGISTER_GOAL_ANGLE_H          = 0x1F,
  SERVO_REGISTER_MOVING_SPEED            = 0x20,
  //SERVO_REGISTER_MOVING_SPEED_L        = 0x20,
  //SERVO_REGISTER_MOVING_SPEED_H        = 0x21,
  SERVO_REGISTER_TORQUE_LIMIT            = 0x23,
  //SERVO_REGISTER_TORQUE_LIMIT_L        = 0x23,
  //SERVO_REGISTER_TORQUE_LIMIT_H        = 0x24,
  SERVO_REGISTER_PRESENT_ANGLE           = 0x25,
  //SERVO_REGISTER_PRESENT_ANGLE_L       = 0x25,
  //SERVO_REGISTER_PRESENT_ANGLE_H       = 0x26,
  SERVO_REGISTER_PRESENT_SPEED           = 0x27,
  //SERVO_REGISTER_PRESENT_SPEED_L       = 0x27,
  //SERVO_REGISTER_PRESENT_SPEED_H       = 0x28,
  SERVO_REGISTER_PRESENT_TORQUE          = 0x29,
  //SERVO_REGISTER_PRESENT_TORQUE_L      = 0x29,
  //SERVO_REGISTER_PRESENT_TORQUE_H      = 0x2A,
  SERVO_REGISTER_PRESENT_VOLTAGE         = 0x2D,
  SERVO_REGISTER_PRESENT_TEMPERATURE     = 0x2E,
  SERVO_REGISTER_PENDING_INSTRUCTION     = 0x2F,
  SERVO_REGISTER_IS_MOVING               = 0x31,
  SERVO_REGISTER_ERROR_STATUS            = 0x32,
  SERVO_REGISTER_PUNCH                   = 0x33,
  //SERVO_REGISTER_PUNCH_L               = 0x33,
  //SERVO_REGISTER_PUNCH_H               = 0x34,

  SERVO_REGISTER_CURRENT_CONSUMPTION     = 0x44,
  //SERVO_REGISTER_CURRENT_CONSUMPTION_L = 0x44,
  //SERVO_REGISTER_CURRENT_CONSUMPTION_H = 0x45,
  SERVO_REGISTER_TORQUE_CTRL_ENABLE      = 0x46,
  SERVO_REGISTER_GOAL_TORQUE             = 0x47,
  //SERVO_REGISTER_GOAL_TORQUE_L         = 0x47,
  //SERVO_REGISTER_GOAL_TORQUE_H         = 0x48,
  SERVO_REGISTER_GOAL_ACCELERATION       = 0x49,
}servo_register_t;
#else
typedef enum servo_register_enum
{
  /* EEPROM */
  SERVO_REGISTER_MODEL_NUMBER            = 0x00,
  //SERVO_REGISTER_MODEL_NUMBER_L        = 0x00,
  //SERVO_REGISTER_MODEL_NUMBER_H        = 0x01,
  SERVO_REGISTER_FIRMWARE_VERSION        = 0x02,
  SERVO_REGISTER_ID                      = 0x03,
  SERVO_REGISTER_BAUD_RATE               = 0x04,
  SERVO_REGISTER_RETURN_DELAY_TIME       = 0x05,
  SERVO_REGISTER_MIN_ANGLE               = 0x06,
  //SERVO_REGISTER_MIN_ANGLE_L           = 0x06,
  //SERVO_REGISTER_MIN_ANGLE_H           = 0x07,
  SERVO_REGISTER_MAX_ANGLE               = 0x08,
  //SERVO_REGISTER_MAX_ANGLE_L           = 0x08,
  //SERVO_REGISTER_MAX_ANGLE_H           = 0x09,
  SERVO_REGISTER_MAX_TEMPERATURE         = 0x0B,
  SERVO_REGISTER_MIN_VOLTAGE             = 0x0C,
  SERVO_REGISTER_MAX_VOLTAGE             = 0x0D,
  SERVO_REGISTER_MAX_TORQUE              = 0x0E,
  //SERVO_REGISTER_MAX_TORQUE_L          = 0x0E,
  //SERVO_REGISTER_MAX_TORQUE_H          = 0x0F,
  SERVO_REGISTER_RETURN_LEVEL            = 0x10,
  SERVO_REGISTER_LED_ERROR               = 0x11,
  SERVO_REGISTER_SHUTDOWN_ERROR          = 0x12,

  /* RAM */
  SERVO_REGISTER_TORQUE_ENABLE           = 0x18,
  SERVO_REGISTER_LED_IS_ON               = 0x19,
  SERVO_REGISTER_D_GAIN                  = 0x1A,
  SERVO_REGISTER_I_GAIN                  = 0x1B,
  SERVO_REGISTER_P_GAIN                  = 0x1C,
  SERVO_REGISTER_GOAL_ANGLE              = 0x1E,
  //SERVO_REGISTER_GOAL_ANGLE_L          = 0x1E,
  //SERVO_REGISTER_GOAL_ANGLE_H          = 0x1F,
  SERVO_REGISTER_MOVING_SPEED            = 0x20,
  //SERVO_REGISTER_MOVING_SPEED_L        = 0x20,
  //SERVO_REGISTER_MOVING_SPEED_H        = 0x21,
  SERVO_REGISTER_TORQUE_LIMIT            = 0x22,
  //SERVO_REGISTER_TORQUE_LIMIT_L        = 0x22,
  //SERVO_REGISTER_TORQUE_LIMIT_H        = 0x23,
  SERVO_REGISTER_PRESENT_ANGLE           = 0x24,
  //SERVO_REGISTER_PRESENT_ANGLE_L       = 0x24,
  //SERVO_REGISTER_PRESENT_ANGLE_H       = 0x25,
  SERVO_REGISTER_PRESENT_SPEED           = 0x26,
  //SERVO_REGISTER_PRESENT_SPEED_L       = 0x26,
  //SERVO_REGISTER_PRESENT_SPEED_H       = 0x27,
  SERVO_REGISTER_PRESENT_TORQUE          = 0x28,
  //SERVO_REGISTER_PRESENT_TORQUE_L      = 0x28,
  //SERVO_REGISTER_PRESENT_TORQUE_H      = 0x29,
  SERVO_REGISTER_PRESENT_VOLTAGE         = 0x2A,
  SERVO_REGISTER_PRESENT_TEMPERATURE     = 0x2B,

  SERVO_REGISTER_PENDING_INSTRUCTION     = 0x2C,
  SERVO_REGISTER_IS_MOVING               = 0x2E,
  SERVO_REGISTER_LOCK_EEPROM             = 0x2F,
  SERVO_REGISTER_PUNCH                   = 0x30,
  //SERVO_REGISTER_PUNCH_L               = 0x30,
  //SERVO_REGISTER_PUNCH_H               = 0x31,
  SERVO_REGISTER_CURRENT_CONSUMPTION     = 0x44,
  //SERVO_REGISTER_CURRENT_CONSUMPTION_L = 0x44,
  //SERVO_REGISTER_CURRENT_CONSUMPTION_H = 0x45,
  SERVO_REGISTER_TORQUE_CTRL_ENABLE      = 0x46,
  SERVO_REGISTER_GOAL_TORQUE             = 0x47,
  //SERVO_REGISTER_GOAL_TORQUE_L         = 0x47,
  //SERVO_REGISTER_GOAL_TORQUE_H         = 0x48,
  SERVO_REGISTER_GOAL_ACCELERATION       = 0x49,
}servo_register_t;
#endif

/*!
 * @functiongroup High-Level Interface
*/

/*--------------------------------------------------------------------------------------------*/
/*!
 * @function    servo_init
 * @abstract      Initialize this module.
 * @param       baud
 *                The baud rate that the servo wants to use to communicate, as defined in the
 *                servo's SERVO_REGISTER_BAUD_RATE. If you have not changed it, it should be
 *                SERVO_DEFAULT_BAUD.
 */
void          servo_init             (uint32_t baud);

/*--------------------------------------------------------------------------------------------*/
/*!
 * @function    servo_factory_reset
 * @abstract      Sets all of the servo's values to their factory default values.
 * @param       id
 *                Either the id of the servo with which you are trying to communicate (as defined
 *                in that servo's SERVO_REGISTER_ID), or SERVO_BROADCAST_ID, to which any servo
 *                will respond. If you have not changed it, the id of the motor will be
 *                SERVO_DEFAULT_ID.
 * @param       timeout_ms
 *                The number of milliseconds that this module should wait for a response
 *                before timing out. If you don't expect a response (ie because you changed
 *                the servo's SERVO_REGISTER_RETURN_LEVEL) or don't care about it, you may
 *                pass 0.
 * @result        SERVO_NO_ERROR or one or several error codes "or'd" together. To check for
 *                a specific error, use the "&" operator rather than "==", as in:
 *
 *                if(error & SERVO_ERROR_VOLTAGE) { handle volatge error }
 *
 *                In order to obtain a string that contains a human-readable description
 *                of all errors, pass the result to servo_errors_to_string();
 */
servo_error_t servo_factory_reset    (uint8_t id, int timeout_ms);

/*--------------------------------------------------------------------------------------------*/
/*!
 * @function    servo_ping
 * @abstract      Ping the servo to see if it is alive without accessing any of its registers.
 * @param       id
 *                Either the id of the servo with which you are trying to communicate (as defined
 *                in that servo's SERVO_REGISTER_ID), or SERVO_BROADCAST_ID, to which any servo
 *                will respond. If you have not changed it, the id of the motor will be
 *                SERVO_DEFAULT_ID.
 * @param       timeout_ms
 *                The number of milliseconds that this module should wait for a response
 *                before timing out.
 * @result        SERVO_NO_ERROR or one or several error codes "or'd" together. To check for
 *                a specific error, use the "&" operator rather than "==", as in:
 *
 *                if(error & SERVO_ERROR_VOLTAGE) { handle voltage error }
 *
 *                In order to obtain a string that contains a human-readable description
 *                of all errors, pass the result to servo_errors_to_string();
 */
servo_error_t servo_ping             (uint8_t id, int timeout_ms);

/*--------------------------------------------------------------------------------------------*/
/*!
 * @function    servo_get
 * @abstract      Read the servo's registers using physical units.
 * @param       id
 *                Either the id of the servo with which you are trying to communicate (as defined
 *                in that servo's SERVO_REGISTER_ID), or SERVO_BROADCAST_ID, to which any servo
 *                will respond. If you have not changed it, the id of the motor will be
 *                SERVO_DEFAULT_ID.
 * @param       reg
 *                The servo's register that you are trying to access. For 16-bit registers,
 *                it is not necessary to access the high and low bytes separately, as this
 *                module 'knows' which registers are 16 bit, will access both of them in a
 *                single operation, and treat them as a single value. For instance,
 *                rather than using SERVO_REGISTER_GOAL_ANGLE_L and SERVO_REGISTER_GOAL_ANGLE_H,
 *                you should just use SERVO_REGISTER_GOAL_ANGLE.
 * @param       result
 *                Upon sucessful return (SERVO_NO_ERROR), the value read from the servo's
 *                register(s) will have been written here. The value will be in physical units,
 *                rather than the raw values defined in the servo's datasheet. The units will
 *                be one of the following: bits-per-second, microseconds, radians, degrees-celcius,
 *                percent-of-the-motor's-maximum-torque, gain-coefficient (for PID controller),
 *                Volts, Amperes, radians-per-second, or radians-per-square-second, according to
 *                the type of quantity being read.
 * @param       timeout_ms
 *                The number of milliseconds that this module should wait for a response
 *                before timing out. If you don't expect a response (ie because you changed
 *                the servo's SERVO_REGISTER_RETURN_LEVEL) or don't care about it, you may
 *                pass 0.
 * @result        SERVO_NO_ERROR or one or several error codes "or'd" together. To check for
 *                a specific error, use the "&" operator rather than "==", as in:
 *
 *                if(error & SERVO_ERROR_VOLTAGE) { handle volatge error }
 *
 *                In order to obtain a string that contains a human-readable description
 *                of all errors, pass the result to servo_errors_to_string();
 */
servo_error_t servo_get              (uint8_t id, servo_register_t reg, float* result, int timeout_ms);

/*--------------------------------------------------------------------------------------------*/
/*!
 * @function    servo_set
 * @abstract      Write to the servo's registers using physical units.
 * @param       id
 *                Either the id of the servo with which you are trying to communicate (as defined
 *                in that servo's SERVO_REGISTER_ID), or SERVO_BROADCAST_ID, to which any servo
 *                will respond. If you have not changed it, the id of the motor will be
 *                SERVO_DEFAULT_ID.
 * @param       reg
 *                The servo's register that you are trying to access. For 16-bit registers,
 *                it is not necessary to access the high and low bytes separately, as this
 *                module 'knows' which registers are 16 bit, will access both of them in a
 *                single operation, and treat them as a single value. For instance,
 *                rather than using SERVO_REGISTER_GOAL_ANGLE_L and SERVO_REGISTER_GOAL_ANGLE_H,
 *                you should just use SERVO_REGISTER_GOAL_ANGLE.
 * @param       val
 *                The desired value of the specified register in physical units
 *                (rather than the raw values defined in the servo's datasheet). The units must
 *                be one of the following: bits-per-second, microseconds, radians, degrees-celcius,
 *                percent-of-the-motor's-maximum-torque, gain-coefficient (for PID controller),
 *                Volts, Amperes, radians-per-second, or radians-per-square-second, according to
 *                the type of quantity being read. This metohod converts this to a raw value,
 *                and  attempts to write it to reg.
 * @param       timeout_ms
 *                The number of milliseconds that this module should wait for a response
 *                before timing out. If you don't expect a response (ie because you changed
 *                the servo's SERVO_REGISTER_RETURN_LEVEL) or don't care about it, you may
 *                pass 0.
 * @result        SERVO_NO_ERROR or one or several error codes "or'd" together. To check for
 *                a specific error, use the "&" operator rather than "==", as in:
 *
 *                if(error & SERVO_ERROR_VOLTAGE) { handle volatge error }
 *
 *                In order to obtain a string that contains a human-readable description
 *                of all errors, pass the result to servo_errors_to_string();
 */
servo_error_t servo_set              (uint8_t id, servo_register_t reg, float  val,    int timeout_ms);

/*--------------------------------------------------------------------------------------------*/
/*!
 * @function    servo_set_multiple
 * @abstract      Set the value of one or more registers in several servos at once. This is supported
 *                by the hardware (SYNC_WRITE in the documentation) and is faster than setting
 *                them one at a time. You must be setting the same registers in all of the servos,
 *                and the registers must be sequential.
 * @param       ids
 *                an array of the IDs of the servos whose registers you are trying to access.
 * @param       start_reg
 *                The lowest ot the servo's register that you are trying to access.
 *                The registers must be sequential. For 16-bit registers,
 *                it is not necessary to access the high and low bytes separately, as this
 *                module 'knows' which registers are 16 bit, will access both of them in a
 *                single operation, and treat them as a single value. For instance,
 *                if start_reg is SERVO_REGISTER_GOAL_ANGLE and num_values_per_servo is 2,
 *                this module will write 4 1-byte registers: 2 for the goal angle, and
 *                2 for the moving speed.
 * @param       values
 *                An array containing the desired values of the specified registers in physical units
 *                (rather than the raw values defined in the servo's datasheet). The units must
 *                be the following: bits-per-second, microseconds, radians, degrees-celcius,
 *                percent-of-the-motor's-maximum-torque, gain-coefficient (for PID controller),
 *                Volts, Amperes, radians-per-second, or radians-per-square-second, according to
 *                the type of quantity being read. This metohod converts this to a raw value,
 *                and  attempts to write it to regs. The values should be in the following order:
 *                first value for ids[0] ... nth value for ids[0],first value for ids[1] ...
 *                nth value for ids[1], etc.
 * @param       num_ids
 *                the number of items in ids[].
 * @param       num_values_per_servo
 *                the number of items in values[] for each servo. num_values_per_servo * num_ids
 *                must equal the total number of items in values.
 * @result        The servos do not send a response packet, for this instruction
 *                and this function returns SERVO_NO_ERROR, regardless of success;
 */
servo_error_t servo_set_multiple     (uint8_t ids[], servo_register_t start_reg, float values[], int num_ids, int num_values_per_servo);

/*--------------------------------------------------------------------------------------------*/
/*!
 * @function    servo_prepare
 * @abstract      Prepare to write the servo's registers. The value will be sent to the servo,
 *                where it will be held in memory, but it will not be written to the appropriate
 *                register, and thus will not go into effect until you call servo_do_prepared();
 *                You may check whether there is a value that has been "prepared" but not "done"
 *                by reading the servo's SERVO_REGISTER_PENDING_INSTRUCTION. Only one value may
 *                be pending at a time, so a call to this function overwrites any currently
 *                pending value. The use of this function is identical to servo_set();
 */
servo_error_t servo_prepare          (uint8_t id, servo_register_t reg, float  val,    int timeout_ms);

/*--------------------------------------------------------------------------------------------*/
/*!
 * @function    servo_do_prepared
 * @abstract      If there is a value waiting to be written to the servo's registers (ie
 *                you called servo_prepare), this function causes it to be written, and thus
 *                take effect. If you have several servos on the same bus and wanted to move
 *                them all at the same time, you would individually prepare them to be set
 *                to the new angle, and then broadcast servo_do_prepared. This minimizes
 *                communication delays.
 * @param       id
 *                Either the id of the servo with which you are trying to communicate (as defined
 *                in that servo's SERVO_REGISTER_ID), or SERVO_BROADCAST_ID, to which any servo
 *                will respond. If you have not changed it, the id of the motor will be
 *                SERVO_DEFAULT_ID.
 * @param       timeout_ms
 *                The number of milliseconds that this module should wait for a response
 *                before timing out. If you don't expect a response (ie because you changed
 *                the servo's SERVO_REGISTER_RETURN_LEVEL) or don't care about it, you may
 *                pass 0.
 * @result        SERVO_NO_ERROR or one or several error codes "or'd" together. To check for
 *                a specific error, use the "&" operator rather than "==", as in:
 *
 *                if(error & SERVO_ERROR_VOLTAGE) { handle error }
 *
 *                In order to obtain a string that contains a human-readable description
 *                of all errors, pass the result to servo_errors_to_string();
 */
servo_error_t servo_do_prepared      (uint8_t id, int timeout_ms);

/*--------------------------------------------------------------------------------------------*/
/*!
 * @function    servo_errors_to_string
 * @abstract      Convert an error code to a human-readable string.
 * @param       error
 *                An error code returned by another function in this module.
 * @result        The human readable string. The memory for this is owned by this module and
 *                should not be freed.
 */
char*         servo_errors_to_string (servo_error_t error);

/*!
 * @functiongroup Low-Level Interface
*/

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_get_raw_byte     (uint8_t id, servo_register_t reg, uint8_t*  result,   int timeout_ms);
servo_error_t servo_get_raw_word     (uint8_t id, servo_register_t reg, uint16_t* result,   int timeout_ms);
servo_error_t servo_get_raw_page     (uint8_t id, servo_register_t reg, uint8_t   result[], int num_bytes, int timeout_ms);

servo_error_t servo_set_raw_byte     (uint8_t id, servo_register_t reg, uint8_t   value,    int timeout_ms);
servo_error_t servo_set_raw_word     (uint8_t id, servo_register_t reg, uint16_t  value,    int timeout_ms);
servo_error_t servo_set_raw_page     (uint8_t id, servo_register_t reg, uint8_t   values[], int num_bytes, int timeout_ms);

servo_error_t servo_prepare_raw_byte (uint8_t id, servo_register_t reg, uint8_t   value,    int timeout_ms);
servo_error_t servo_prepare_raw_word (uint8_t id, servo_register_t reg, uint16_t  value,    int timeout_ms);
servo_error_t servo_prepare_raw_page (uint8_t id, servo_register_t reg, uint8_t   values[], int num_bytes, int timeout_ms);

servo_error_t servo_set_multiple_raw(uint8_t ids[], servo_register_t start_reg, uint8_t values[], int num_ids, int num_bytes_per_servo);
servo_error_t servo_set_positions_and_speeds_raw(uint8_t ids[], servo_register_t start_reg, uint8_t values[], int num_ids, int num_bytes_per_servo);

/*!
 * @functiongroup Raw to Physical Conversions
 * @discussion done automatically by the high level interface
*/

/*--------------------------------------------------------------------------------------------*/
/*  return size of a given register */
char get_register_size(servo_register_t reg);

/*  call one of the following conversion functions appropriate to the type of value in reg  */
int   servo_anything_to_raw          (servo_register_t reg, float anything);
float servo_raw_to_anything          (servo_register_t reg, int raw);

/*    0 ~ 2M, 2.25M, 2.5M or 3M. unususal resolution... */
int   servo_baud_bps_to_raw          (float baud);
float servo_raw_to_baud_bps          (int  raw);

/*    0 ~ 508 usec. resolution 2 usec */
int   servo_return_delay_usec_to_raw (float usec);
float servo_raw_to_return_delay_usec (int raw);

/*    0 ~ 2PI. resolution 0.0015 radians*/
int   servo_radians_to_raw           (float rad);
float servo_raw_to_radians           (int raw);

/*    10 ~ 99 deg, do not change default */
int   servo_celcius_to_raw           (float celcius);
float servo_raw_to_celcius           (int raw);

/*    0 ~ 100 percent of motor maximum. */
int   servo_torque_percentage_to_raw (float percent);
float servo_raw_to_torque_percentage (int raw);

/*    0 ~ 31.75. resolution 0.125 */
int   servo_p_gain_to_raw            (float gain);
float servo_raw_to_p_gain            (int raw);

/*    0 ~ 124. resolution 0.488 */
int   servo_i_gain_to_raw            (float gain);
float servo_raw_to_i_gain            (int raw);

/*    0 ~ 1.016. resolution 0.004 */
int   servo_d_gain_to_raw            (float gain);
float servo_raw_to_d_gain            (int raw);

/*    5 ~ 25 V, resolution 0.1 V */
int   servo_volts_to_raw             (float volts);
float servo_raw_to_volts             (int raw);

/* -15 ~ 15 amps */
int   servo_amperes_to_raw           (float amps);
float servo_raw_to_amperes           (int raw);

/*  */
int   servo_radians_per_sec_to_raw   (float rad);
float servo_raw_to_radians_per_sec   (int raw);

/*  */
int   servo_radians_per_sec_2_to_raw (float rad);
float servo_raw_to_radians_per_sec_2 (int raw);

#ifdef __cplusplus
}
#endif

#endif //DYNAMIXEL_SERVO
