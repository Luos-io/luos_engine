#include "Dynamixel_Servo.h"
#include "usart.h"

/*--------------------------------------------------------------------------------------------*/
#define HALF_DUPLEX_DIRECTION_OUTPUT GPIO_PIN_SET
#define HALF_DUPLEX_DIRECTION_INPUT  GPIO_PIN_RESET

/*--------------------------------------------------------------------------------------------*/
typedef enum servo_instruction_enum
{
    SERVO_INSTRUCTION_PING       = 0x01,
    SERVO_INSTRUCTION_READ_DATA  = 0x02,
    SERVO_INSTRUCTION_WRITE_DATA = 0x03,
    SERVO_INSTRUCTION_REG_WRITE  = 0x04,
    SERVO_INSTRUCTION_ACTION     = 0x05,
    SERVO_INSTRUCTION_RESET      = 0x06,
    SERVO_INSTRUCTION_REBOOT     = 0x08,
    SERVO_INSTRUCTION_CLEAR      = 0x10,
    SERVO_INSTRUCTION_STATUS     = 0x55,
    SERVO_INSTRUCTION_SYNC_READ  = 0x82,
    SERVO_INSTRUCTION_SYNC_WRITE = 0x83,
    SERVO_INSTRUCTION_BULK_READ  = 0x92,
    SERVO_INSTRUCTION_BULK_WRITE = 0x93,
} servo_instruction_t;

/*--------------------------------------------------------------------------------------------*/
servo_error_t _servo_set(uint8_t id, servo_register_t reg, float val, int timeout_ms, uint8_t do_now);
servo_error_t _servo_set_raw_byte(uint8_t id, servo_register_t reg, uint8_t value, int timeout_ms, uint8_t do_now);
servo_error_t _servo_set_raw_word(uint8_t id, servo_register_t reg, uint16_t value, int timeout_ms, uint8_t do_now);
servo_error_t _servo_set_raw_page(uint8_t id, servo_register_t reg, uint8_t values[], int num_bytes, int timeout_ms, uint8_t do_now);
servo_error_t servo_get_response(uint8_t id, uint8_t result[], int result_size, int timeout_ms);
servo_error_t servo_send_instruction(uint8_t id, servo_instruction_t instruction,
                                     uint8_t parameters[], uint8_t num_parameters,
                                     uint8_t result[], int num_results, int timeout_ms);
#ifdef V2
uint16_t update_crc(uint16_t crc_accum, uint8_t *data_blk_ptr, uint16_t data_blk_size);
#endif

/*--------------------------------------------------------------------------------------------*/
void servo_init(uint32_t baud)
{
    huart3.Init.BaudRate = baud;
    UART_SetConfig(&huart3);
    LL_USART_DisableIT_RXNE(USART3); // Disable Rx IT

    HAL_GPIO_WritePin(DXL_DIR_GPIO_Port, DXL_DIR_Pin, HALF_DUPLEX_DIRECTION_INPUT);
}

/*--------------------------------------------------------------------------------------------*/
uint8_t servo_register_is_word(servo_register_t reg)
{
    uint8_t result = 0;

    switch (reg)
    {
        case SERVO_REGISTER_MODEL_NUMBER:
        case SERVO_REGISTER_MAX_ANGLE:
        case SERVO_REGISTER_MIN_ANGLE:
        case SERVO_REGISTER_MAX_TORQUE:
        case SERVO_REGISTER_GOAL_ANGLE:
        case SERVO_REGISTER_MOVING_SPEED:
        case SERVO_REGISTER_TORQUE_LIMIT:
        case SERVO_REGISTER_PRESENT_ANGLE:
        case SERVO_REGISTER_PRESENT_SPEED:
        case SERVO_REGISTER_PRESENT_TORQUE:
        case SERVO_REGISTER_PUNCH:
        case SERVO_REGISTER_CURRENT_CONSUMPTION:
        case SERVO_REGISTER_GOAL_TORQUE:
            result = 1;
            break;
        default:
            break;
    }

    return result;
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_factory_reset(uint8_t id, int timeout_ms)
{
#ifdef V2
    uint8_t parameters[1];
    if (id == SERVO_BROADCAST_ID)
    {
        parameters[0] = 0x01;
    }
    else
    {
        parameters[0] = 0xFF;
    }
    return servo_send_instruction(id, SERVO_INSTRUCTION_RESET, parameters, 1, NULL, 0, timeout_ms);
#else
    return servo_send_instruction(id, SERVO_INSTRUCTION_RESET, NULL, 0, NULL, 0, timeout_ms);
#endif
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_ping(uint8_t id, int timeout_ms)
{
#ifdef V2
    uint8_t result[3];
    return servo_send_instruction(id, SERVO_INSTRUCTION_PING, NULL, 0, result, 3, timeout_ms);
#else
    return servo_send_instruction(id, SERVO_INSTRUCTION_PING, NULL, 0, NULL, 0, timeout_ms);
#endif
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_get(uint8_t id, servo_register_t reg, float *result, int timeout_ms)
{
    servo_error_t error;
    uint8_t is_word = servo_register_is_word(reg);

    if (is_word)
    {
        uint16_t raw = 0;
        error        = servo_get_raw_word(id, reg, &raw, timeout_ms);
        *result      = servo_raw_to_anything(reg, (int)raw);
    }
    else
    {
        uint8_t raw = 0;
        error       = servo_get_raw_byte(id, reg, &raw, timeout_ms);
        *result     = servo_raw_to_anything(reg, (int)raw);
    }

    return error;
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t _servo_set(uint8_t id, servo_register_t reg, float val, int timeout_ms, uint8_t do_now)
{
    servo_error_t error = SERVO_ERROR_RANGE;
    uint8_t is_word     = servo_register_is_word(reg);
    int raw             = servo_anything_to_raw(reg, val);

    if (is_word)
    {
        if ((raw >= 0) && (raw <= 0xFFFF))
            error = _servo_set_raw_word(id, reg, (uint16_t)raw, timeout_ms, do_now);
    }
    else
    {
        if ((raw >= 0) && (raw <= 0xFF))
            error = _servo_set_raw_byte(id, reg, (uint8_t)raw, timeout_ms, do_now);
    }

    return error;
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_set(uint8_t id, servo_register_t reg, float val, int timeout_ms)
{
    return _servo_set(id, reg, val, timeout_ms, 1);
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_set_multiple(uint8_t ids[], servo_register_t start_reg, float values[], int num_ids, int num_values_per_servo)
{
    int bytes_per_value[num_values_per_servo];
    int *b              = bytes_per_value;
    int bytes_per_servo = 0;
    int i, j, k;
    int addr = (int)start_reg;

    uint8_t raw_bytes[num_ids * 2]; // worst case scenario
    uint8_t *r = raw_bytes;
    int raw_anything;

    for (i = 0; i < num_values_per_servo; i++)
    {
        *b = (servo_register_is_word((servo_register_t)addr)) ? 2 : 1;
        bytes_per_servo += *b;
        addr += *b++;
    }

    for (i = 0; i < num_ids; i++)
    {
        addr = start_reg;
        b    = bytes_per_value;
        for (j = 0; j < num_values_per_servo; j++)
        {
            raw_anything = servo_anything_to_raw((servo_register_t)addr, *values++);

            for (k = 0; k < *b; k++)
            {
                *r++ = (raw_anything & 0xFF);
                raw_anything >>= 8;
            }
            addr += *b++;
        }
    }

    return servo_set_multiple_raw(ids, start_reg, raw_bytes, num_ids, bytes_per_servo);
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_prepare(uint8_t id, servo_register_t reg, float val, int timeout_ms)
{
    return _servo_set(id, reg, val, timeout_ms, 0);
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_do_prepared(uint8_t id, int timeout_ms)
{
    return servo_send_instruction(id, SERVO_INSTRUCTION_ACTION, NULL, 0, NULL, 0, timeout_ms);
}

/*--------------------------------------------------------------------------------------------*/
char *servo_errors_to_string(servo_error_t error)
{
    static const char *const_error_strings[] /*PROGMEM*/ = {
        "voltage",
        "angle",
        "overheat",
        "range",
        "checksum",
        "overload",
        "instruction",
        "unknown",
        "timeout",
        "invalid_response",
    };

    static char result[100];
    char *r = result;

    if (error != SERVO_NO_ERROR)
    {
        int i;
        for (i = 0; i < 10; i++)
            if ((1 << i) & error)
            {
                // this saves over 1K of code space
                // r += sprintf(r, "%s ", const_error_strings[i]);
                const char *s = const_error_strings[i];
                while (*s != '\0')
                    *r++ = *s++;
                *r++ = ' ';
            }
    }

    *r = '\0';
    return result;
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_get_raw_byte(uint8_t id, servo_register_t reg, uint8_t *result, int timeout_ms)
{
#ifdef V2
    uint8_t params[4] = {(reg & 0xFF), 0, 1, 0};
    return servo_send_instruction(id, SERVO_INSTRUCTION_READ_DATA, params, 4, result, 1, timeout_ms);
#else
    uint8_t params[2] = {reg, 1};
    return servo_send_instruction(id, SERVO_INSTRUCTION_READ_DATA, params, 2, result, 1, timeout_ms);
#endif
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_get_raw_word(uint8_t id, servo_register_t reg, uint16_t *result, int timeout_ms)
{
    uint8_t results[2] = {0, 0};
    servo_error_t error;
#ifdef V2
    uint8_t params[4] = {reg, 0, 2, 0};
    error             = servo_send_instruction(id, SERVO_INSTRUCTION_READ_DATA, params, 4, results, 2, timeout_ms);
#else
    uint8_t params[2] = {reg, 2};
    error             = servo_send_instruction(id, SERVO_INSTRUCTION_READ_DATA, params, 2, results, 2, timeout_ms);
#endif
    *result = ((uint16_t)results[1] << 8) | ((uint16_t)results[0]);

    return error;
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_get_raw_page(uint8_t id, servo_register_t reg, uint8_t result[], int num_bytes, int timeout_ms)
{
#ifdef V2
    uint8_t params[4] = {(reg & 0xFF), 0, (num_bytes & 0xFF), (num_bytes >> 8)};
    return servo_send_instruction(id, SERVO_INSTRUCTION_READ_DATA, params, 4, result, num_bytes, timeout_ms);
#else
    uint8_t params[2] = {reg, num_bytes};
    return servo_send_instruction(id, SERVO_INSTRUCTION_READ_DATA, params, 2, result, num_bytes, timeout_ms);
#endif
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t _servo_set_raw_byte(uint8_t id, servo_register_t reg, uint8_t value, int timeout_ms, uint8_t do_now)
{
    servo_instruction_t instruction = (do_now) ? SERVO_INSTRUCTION_WRITE_DATA : SERVO_INSTRUCTION_REG_WRITE;
#ifdef V2
    uint8_t params[3] = {reg, 0, value};
    return servo_send_instruction(id, instruction, params, 3, NULL, 0, timeout_ms);
#else
    uint8_t params[2] = {reg, value};
    return servo_send_instruction(id, instruction, params, 2, NULL, 0, timeout_ms);
#endif
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t _servo_set_raw_word(uint8_t id, servo_register_t reg, uint16_t value, int timeout_ms, uint8_t do_now)
{
    servo_instruction_t instruction = (do_now) ? SERVO_INSTRUCTION_WRITE_DATA : SERVO_INSTRUCTION_REG_WRITE;
#ifdef V2
    uint8_t params[4] = {reg, 0, (value & 0xFF), (value >> 8)};
    return servo_send_instruction(id, instruction, params, 4, NULL, 0, timeout_ms);
#else
    uint8_t params[3] = {reg, (value & 0xFF), (value >> 8)};
    return servo_send_instruction(id, instruction, params, 3, NULL, 0, timeout_ms);
#endif
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t _servo_set_raw_page(uint8_t id, servo_register_t reg, uint8_t values[], int num_bytes, int timeout_ms, uint8_t do_now)
{
    servo_instruction_t instruction = (do_now) ? SERVO_INSTRUCTION_WRITE_DATA : SERVO_INSTRUCTION_REG_WRITE;
#ifdef V2
    uint8_t params[num_bytes + 2];
#else
    uint8_t params[num_bytes + 1];
#endif
    uint8_t *p = params;
    int n      = num_bytes;
    *p++       = reg;
#ifdef V2
    *p++ = 0;
#endif
    while (n-- < 0)
    {
        *p++ = *values++;
    }

    return servo_send_instruction(id, instruction, params, num_bytes, NULL, 0, timeout_ms);
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_set_raw_byte(uint8_t id, servo_register_t reg, uint8_t value, int timeout_ms)
{
    const int retry       = 2;
    int retry_nb          = 0;
    servo_error_t mot_err = SERVO_ERROR_TIMEOUT;
    for (volatile int y = 0; y < 500; y++)
        ;
    while ((retry_nb < retry) & (mot_err == SERVO_ERROR_TIMEOUT))
    {
        mot_err = _servo_set_raw_byte(id, reg, value, timeout_ms, 1);
        retry_nb++;
    }
    return mot_err;
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_set_raw_word(uint8_t id, servo_register_t reg, uint16_t value, int timeout_ms)
{
    return _servo_set_raw_word(id, reg, value, timeout_ms, 1);
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_set_raw_page(uint8_t id, servo_register_t reg, uint8_t values[], int num_bytes, int timeout_ms)
{
    return _servo_set_raw_page(id, reg, values, num_bytes, timeout_ms, 1);
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_prepare_raw_byte(uint8_t id, servo_register_t reg, uint8_t value, int timeout_ms)
{
    return _servo_set_raw_byte(id, reg, value, timeout_ms, 0);
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_prepare_raw_word(uint8_t id, servo_register_t reg, uint16_t value, int timeout_ms)
{
    return _servo_set_raw_word(id, reg, value, timeout_ms, 0);
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_prepare_raw_page(uint8_t id, servo_register_t reg, uint8_t values[], int num_bytes, int timeout_ms)
{
    return _servo_set_raw_page(id, reg, values, num_bytes, timeout_ms, 0);
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_set_multiple_raw(uint8_t ids[], servo_register_t start_reg, uint8_t bytes[], int num_ids, int bytes_per_servo)
{
    int i, j;
    uint8_t num_params = num_ids * (bytes_per_servo + 1) + 2;
    uint8_t params[num_params];
    uint8_t *p = params;

    *p++ = start_reg;
    *p++ = bytes_per_servo;

    for (i = 0; i < num_ids; i++)
    {
        *p++ = *ids++;
        for (j = 0; j < bytes_per_servo; j++)
            *p++ = *bytes++;
    }

    return servo_send_instruction(SERVO_BROADCAST_ID, SERVO_INSTRUCTION_SYNC_WRITE, params, num_params, NULL, 0, 0);
}

/*--------------------------------------------------------------------------------------------*/
void *servo_get_conversion_function_for_register(servo_register_t reg, uint8_t to_raw)
{
    int (*result)() = NULL;

    switch (reg)
    {
        case SERVO_REGISTER_BAUD_RATE:
            result = (to_raw) ? (int (*)())servo_baud_bps_to_raw : (int (*)())servo_raw_to_baud_bps;
            break;

        case SERVO_REGISTER_RETURN_DELAY_TIME:
            result = (to_raw) ? (int (*)())servo_return_delay_usec_to_raw : (int (*)())servo_raw_to_return_delay_usec;
            break;

        case SERVO_REGISTER_MAX_ANGLE:
        case SERVO_REGISTER_MIN_ANGLE:
        case SERVO_REGISTER_GOAL_ANGLE:
        case SERVO_REGISTER_PRESENT_ANGLE:
            result = (to_raw) ? (int (*)())servo_radians_to_raw : (int (*)())servo_raw_to_radians;
            break;

        case SERVO_REGISTER_MIN_VOLTAGE:
        case SERVO_REGISTER_MAX_VOLTAGE:
        case SERVO_REGISTER_PRESENT_VOLTAGE:
            result = (to_raw) ? (int (*)())servo_volts_to_raw : (int (*)())servo_raw_to_volts;
            break;

        case SERVO_REGISTER_MAX_TORQUE:
        case SERVO_REGISTER_PRESENT_TORQUE:
        case SERVO_REGISTER_TORQUE_LIMIT:
        case SERVO_REGISTER_GOAL_TORQUE:
            result = (to_raw) ? (int (*)())servo_torque_percentage_to_raw : (int (*)())servo_raw_to_torque_percentage;
            break;

        case SERVO_REGISTER_D_GAIN:
            result = (to_raw) ? (int (*)())servo_d_gain_to_raw : (int (*)())servo_raw_to_d_gain;
            break;

        case SERVO_REGISTER_I_GAIN:
            result = (to_raw) ? (int (*)())servo_i_gain_to_raw : (int (*)())servo_raw_to_i_gain;
            break;

        case SERVO_REGISTER_P_GAIN:
            result = (to_raw) ? (int (*)())servo_p_gain_to_raw : (int (*)())servo_raw_to_p_gain;
            break;

        case SERVO_REGISTER_MOVING_SPEED:
        case SERVO_REGISTER_PRESENT_SPEED:
            result = (to_raw) ? (int (*)())servo_radians_per_sec_to_raw : (int (*)())servo_raw_to_radians_per_sec;
            break;

        case SERVO_REGISTER_CURRENT_CONSUMPTION:
            result = (to_raw) ? (int (*)())servo_amperes_to_raw : (int (*)())servo_raw_to_amperes;
            break;

        case SERVO_REGISTER_GOAL_ACCELERATION:
            result = (to_raw) ? (int (*)())servo_radians_per_sec_2_to_raw : (int (*)())servo_raw_to_radians_per_sec_2;
            break;

        default:
            result = NULL;
            break;
    }

    return (void *)result;
}

/*--------------------------------------------------------------------------------------------*/
char get_register_size(servo_register_t reg)
{
    if ((reg == SERVO_REGISTER_MODEL_NUMBER) || (reg == SERVO_REGISTER_MIN_ANGLE) || (reg == SERVO_REGISTER_MAX_ANGLE) || (reg == SERVO_REGISTER_MAX_TORQUE) || (reg == SERVO_REGISTER_GOAL_ANGLE) || (reg == SERVO_REGISTER_MOVING_SPEED) || (reg == SERVO_REGISTER_TORQUE_LIMIT) || (reg == SERVO_REGISTER_PRESENT_ANGLE) || (reg == SERVO_REGISTER_PRESENT_SPEED) || (reg == SERVO_REGISTER_PRESENT_TORQUE) || (reg == SERVO_REGISTER_PUNCH) || (reg == SERVO_REGISTER_CURRENT_CONSUMPTION) || (reg == SERVO_REGISTER_GOAL_TORQUE))
    {
        return 2;
    }
    return 1;
}

/*--------------------------------------------------------------------------------------------*/
int servo_anything_to_raw(servo_register_t reg, float anything)
{
    int (*conversion)(float) = (int (*)(float))servo_get_conversion_function_for_register(reg, 1);
    return (conversion == NULL) ? (int)(anything + 0.5) : conversion(anything);
}

/*--------------------------------------------------------------------------------------------*/
float servo_raw_to_anything(servo_register_t reg, int raw)
{
    float (*conversion)(int) = (float (*)(int))servo_get_conversion_function_for_register(reg, 0);
    return (conversion == NULL) ? (float)raw : conversion(raw);
}

/*--------------------------------------------------------------------------------------------*/
int servo_baud_bps_to_raw(float baud)
{
    uint8_t result = 0xFF;
    float actual_baud;

    if ((baud <= 2000000) && (baud >= 8000))
        result = (int)(((2000000.0 / (float)baud) - 1) + 0.5);

    else
        switch ((unsigned)baud)
        {
            case 2250000:
                result = 250;
                break;
            case 2500000:
                result = 251;
                break;
            case 3000000:
                result = 252;
                break;
            default:
                break;
        }

    actual_baud = servo_raw_to_baud_bps(result);
    if (result * 0.03 > fabs(result - actual_baud))
        result = 0xFF; // invalid baud value, servo will return error and baud will not change;

    return result;
}

/*--------------------------------------------------------------------------------------------*/
float servo_raw_to_baud_bps(int raw)
{
    uint32_t result = 0;

    if (raw <= 249)
        result = 2000000 / (raw + 1);
    else
        switch (raw)
        {
            case 250:
                result = 2250000;
                break;
            case 251:
                result = 2500000;
                break;
            case 252:
                result = 3000000;
                break;
            default:
                break;
        }

    return result;
}

/*as functions and not macros so pointers can be used */
/*--------------------------------------------------------------------------------------------*/
int servo_return_delay_usec_to_raw(float usec) { return usec * 0.5; }
/*--------------------------------------------------------------------------------------------*/
float servo_raw_to_return_delay_usec(int raw) { return raw * 2; }
/*--------------------------------------------------------------------------------------------*/
int servo_radians_to_raw(float rad)
{
    float two_pi = 2 * M_PI;
    while (rad >= two_pi)
        rad -= two_pi;
    while (rad < 0)
        rad += two_pi;

    return (int)(rad * (4095.0 / two_pi) + 0.5);
}
/*--------------------------------------------------------------------------------------------*/
float servo_raw_to_radians(int raw) { return raw * (6.28318531 / 4095.0); }
/*--------------------------------------------------------------------------------------------*/
int servo_torque_percentage_to_raw(float percent)
{
    int result;
    int is_counter_clockwise = (percent < 0);
    if (is_counter_clockwise)
        percent *= -1;
    result = percent * 10.23 + 0.5;
    if (is_counter_clockwise)
        result |= 1024;

    return result;
}
/*--------------------------------------------------------------------------------------------*/
float servo_raw_to_torque_percentage(int raw)
{
    float result;
    int is_negative = (raw >= 0x400); // B10000000000
    raw &= 0x3FF;                     // B01111111111
    result = raw * (100.0 / 1023.0);
    if (is_negative)
        result *= -1;

    return result;
}
/*--------------------------------------------------------------------------------------------*/
int servo_p_gain_to_raw(float gain) { return gain * 0.8 + 0.5; }
/*--------------------------------------------------------------------------------------------*/
float servo_raw_to_p_gain(int raw) { return raw * 1.25; }
/*--------------------------------------------------------------------------------------------*/
int servo_i_gain_to_raw(float gain) { return gain * (2048.0 / 1000.0) + 0.5; }
/*--------------------------------------------------------------------------------------------*/
float servo_raw_to_i_gain(int raw) { return raw * (1000.0 / 2048.0); }
/*--------------------------------------------------------------------------------------------*/
int servo_d_gain_to_raw(float gain) { return gain * 250; }
/*--------------------------------------------------------------------------------------------*/
float servo_raw_to_d_gain(int raw) { return raw * 0.004; }
/*--------------------------------------------------------------------------------------------*/
int servo_volts_to_raw(float volts) { return volts * 10; }
/*--------------------------------------------------------------------------------------------*/
float servo_raw_to_volts(int raw) { return raw * 0.1; }
/*--------------------------------------------------------------------------------------------*/
int servo_amperes_to_raw(float amps) { return amps * 222.22222 + 2048.5; }
/*--------------------------------------------------------------------------------------------*/
float servo_raw_to_amperes(int raw) { return 0.0045 * (raw - 2048); }
/*--------------------------------------------------------------------------------------------*/
int servo_radians_per_sec_to_raw(float rad)
{
    int is_counter_clockwise = (rad < 0);
    if (is_counter_clockwise)
        rad *= -1;
    int result = (rad * (60 / (0.114 * 6.28318531))) + 0.5;
    if (result > 1023)
        result = 1023;

    // wheel mode needs this
    // if(result == 0) result = 1024;

    // joint mode needs this
    if (result == 0)
        result = 1;

    if (is_counter_clockwise)
        result |= 0x400;

    return result;

    // for joint mode 0 should indicate the motor's max speed,
    // but this is not implemented because 1023 is twice as fast as
    //  the motor's max speed anyhow;
    // return (result < 1024) ? result : 0;
}

/*--------------------------------------------------------------------------------------------*/
float servo_raw_to_radians_per_sec(int raw)
{
    // return (raw > 0) ? (raw - 1024) * (SERVO_MAX_SPEED / 1024.0) : SERVO_MAX_SPEED;

    float SERVO_MAX_SPEED = 13;
    float result;
    int is_negative = raw >= 0x400;             // B10000000000
    raw &= 0x3FF;                               // B01111111111
    result = raw * (0.114 * 6.28318531 / 60.0); // rad / sec
    if (is_negative)
        result *= -1;

    return (raw > 0) ? result : SERVO_MAX_SPEED;
}

/*--------------------------------------------------------------------------------------------*/
int servo_radians_per_sec_2_to_raw(float rad)
{
    int result = rad * (1.0 / 0.14979598) + 0.5;
    return (result < 255) ? result : 0;
}

/*--------------------------------------------------------------------------------------------*/
float servo_raw_to_radians_per_sec_2(int raw)
{
    float SERVO_MAX_ACCEL = 39;
    return (raw > 0) ? raw * 0.14979598 : SERVO_MAX_ACCEL;
}

/*--------------------------------------------------------------------------------------------*/
servo_error_t servo_send_instruction(uint8_t id, servo_instruction_t instruction,
                                     uint8_t parameters[], uint8_t num_parameters,
                                     uint8_t result[], int num_results, int timeout_ms)
{
    int data_size;
#ifdef V2
    data_size = num_parameters + 10;
#else
    data_size = num_parameters + 6;
#endif
    uint8_t data[data_size];
    uint8_t *d          = data;
    uint16_t checksum   = 0;
    servo_error_t error = SERVO_NO_ERROR;

    // could check instruction and number of parameters here;

    *d++ = 0xFF;
    *d++ = 0xFF;
#ifdef V2
    *d++ = 0xFD;
    *d++ = 0x00;
#endif
    *d++ = id;
#ifdef V2
    *d++ = num_parameters + 3;
    *d++ = 0;
#else
    *d++      = num_parameters + 2;
#endif
    *d++ = (uint8_t)instruction;

#ifdef V2
    uint16_t num_parameters_cpy = num_parameters;
    while (num_parameters_cpy-- > 0)
    {
        *d++ = *parameters++;
    }
    checksum = update_crc(0, data, (num_parameters + 3) + 5);
    *d++     = (uint8_t)checksum;
    *d++     = (uint8_t)(checksum >> 8);
#else
    checksum  = data[2] + data[3] + data[4];

    while (num_parameters-- > 0)
    {
        checksum += *parameters;
        *d++ = *parameters++;
    }

    *d++      = ~(uint8_t)checksum;
#endif
    HAL_GPIO_WritePin(DXL_DIR_GPIO_Port, DXL_DIR_Pin, HALF_DUPLEX_DIRECTION_OUTPUT);
    // Delay was necessary when on solderless breadboard, but not on custom PCB (probably stray capacitance)
    // delayMicroseconds(10);
    HAL_UART_Transmit(&huart3, data, data_size, timeout_ms);

    // Datasheet says to disable and enable interrupts here but the given reason seems
    // unnecessary, and without interrupts, flush won't know that the last byte has been transmitted
    // servo_serial->write(data, data_size-1);
    HAL_GPIO_WritePin(DXL_DIR_GPIO_Port, DXL_DIR_Pin, HALF_DUPLEX_DIRECTION_INPUT);

    if (id != SERVO_BROADCAST_ID)
        error = servo_get_response(id, result, num_results, timeout_ms);

    return error;
}

/*--------------------------------------------------------------------------------------------*/
volatile char dxl_msg_complete = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        dxl_msg_complete = 1;
    }
}

servo_error_t servo_get_response(uint8_t id, uint8_t result[], int result_size, int timeout_ms)
{
    int data_size;
#ifdef V2
    data_size = result_size + 11;
#else
    data_size = result_size + 6;
#endif
    static uint8_t data[DXL_RX_BUFFER_SIZE];
    uint8_t *d          = data;
    servo_error_t error = SERVO_NO_ERROR;

    if (dxl_msg_complete > 0)
    {
        // ERROR
        dxl_msg_complete = 0;
        HAL_UART_DMAStop(&huart3);
    }
    HAL_UART_Receive_DMA(&huart3, data, data_size);
    unsigned long timeout_rx = HAL_GetTick();
    while ((dxl_msg_complete == 0))
    {
        // wait for a complete msg
        if ((HAL_GetTick() - timeout_rx) == timeout_ms)
        {
            // reset reception
            HAL_UART_DMAStop(&huart3);
            return SERVO_ERROR_TIMEOUT;
        }
    }
    dxl_msg_complete = 0;

#ifdef V2
    if ((data[0] != 0xFF) || (data[1] != 0xFF) || (data[2] != 0xFD) || (data[4] != id) || (data[5] != (uint8_t)(data_size - 7)) || (data[6] != (uint8_t)((data_size - 7) >> 8)) || (data[7] != 0x55))
        return SERVO_ERROR_INVALID_RESPONSE;
#else
    if ((data[0] != 0xFF) || (data[1] != 0xFF) || (data[2] != id) || (data[3] != data_size - 4))
        return SERVO_ERROR_INVALID_RESPONSE;
#endif

#ifdef V2
    error = (servo_error_t)data[8];
#else
    error               = (servo_error_t)data[4];
#endif
#ifdef V2
    d                        = data + 9;
    uint16_t result_size_cpy = result_size;
    while (result_size_cpy-- > 0)
    {
        *result++ = *d++;
    }
    uint16_t checksum    = update_crc(0, data, (result_size + 4) + 5);
    uint16_t rx_checksum = (uint16_t)data[data_size - 2] | ((uint16_t)data[data_size - 1] << 8);
    if (rx_checksum != checksum)
    {
        error = (servo_error_t)(error | SERVO_ERROR_INVALID_RESPONSE);
    }
#else
    uint8_t checksumlow = data[2] + data[3] + data[4];
    d                   = data + 5;
    while (result_size-- > 0)
    {
        checksumlow += *d;
        *result++ = *d++;
    }
    checksumlow = ~checksumlow;
    if (*d != checksumlow)
        error = (servo_error_t)(error | SERVO_ERROR_INVALID_RESPONSE);
#endif
    return error;
}

#ifdef V2
uint16_t update_crc(uint16_t crc_accum, uint8_t *data_blk_ptr, uint16_t data_blk_size)
{
    uint16_t i, j;
    uint16_t crc_table[256] = {
        0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
        0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
        0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
        0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
        0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
        0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
        0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
        0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
        0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
        0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
        0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
        0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
        0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
        0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
        0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
        0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
        0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
        0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
        0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
        0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
        0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
        0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
        0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
        0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
        0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
        0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
        0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
        0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
        0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
        0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
        0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
        0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202};

    for (j = 0; j < data_blk_size; j++)
    {
        i         = ((uint16_t)(crc_accum >> 8) ^ data_blk_ptr[j]) & 0xFF;
        crc_accum = (crc_accum << 8) ^ crc_table[i];
    }

    return crc_accum;
}
#endif
