#include "main.h"
#include "servo.h"
#include "tim.h"

#define SERVONUMBER 4
typedef struct {
        angular_position_t angle;
        servo_parameters_t param;
}servo_t;

static module_t *module_serv[SERVONUMBER];
volatile servo_t servo[SERVONUMBER];


uint8_t find_id(module_t* my_module) {
    uint8_t i = 0;
    for (i = 0; i<=SERVONUMBER; i++) {
        if ((int)my_module == (int)module_serv[i]) return i;
    }
    return i;
}

void set_position(uint8_t motor_id) {
    static char chan1 = 0;
    static char chan2 = 0;
    static char chan3 = 0;
    static char chan4 = 0;
    // limit angle value
    if (servo[motor_id].angle < 0.0) servo[motor_id].angle = 0.0;
    if (servo[motor_id].angle > servo[motor_id].param.max_angle) servo[motor_id].angle = servo[motor_id].param.max_angle;
    // transform angle to timer value
    //const uint32_t min = 400; // min pulse to have 0,5ms
    //const uint32_t max = 800; // max pulse to have 1,5ms
    uint32_t pulse_min = (uint32_t)(servo[motor_id].param.min_pulse_time * (float)(48000000 / htim2.Init.Prescaler));
    uint32_t pulse_max = (uint32_t)(servo[motor_id].param.max_pulse_time * (float)(48000000 / htim2.Init.Prescaler));
    uint32_t pulse = pulse_min + (uint32_t)(servo[motor_id].angle/servo[motor_id].param.max_angle * (pulse_max - pulse_min));

    switch (motor_id) {
        case 0:
            if (!chan1){
                HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
                chan1 = 1;
            }
            TIM2->CCR1 = pulse;
        break;
        case 1:
            if (!chan2){
                HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
                chan2 = 1;
            }
            TIM2->CCR2 = pulse;
        break;
        case 2:
            if (!chan3){
                HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
                chan3 = 1;
            }
            TIM2->CCR3 = pulse;
        break;
        case 3:
            if (!chan4){
                HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
                chan4 = 1;
            }
            TIM2->CCR4 = pulse;
        break;
        default:
            break;
    }
}

void rx_servo_cb(module_t *module, msg_t *msg) {
    if (msg->header.cmd == ANGULAR_POSITION) {
        // set the motor position
        uint8_t motor_id = find_id(module);
        angular_position_from_msg(&servo[motor_id].angle, msg);
        set_position(motor_id);
        return;
    }
    if (msg->header.cmd == PARAMETERS) {
        // set the servo parameters
        uint8_t motor_id = find_id(module);
        memcpy(servo[motor_id].param.unmap, msg->data, sizeof(servo_parameters_t));
        set_position(motor_id);
        return;
    }
}

void servo_init(void) {
    module_serv[0] = luos_module_create(rx_servo_cb, SERVO_MOD, "servo1_mod");
    module_serv[1] = luos_module_create(rx_servo_cb, SERVO_MOD, "servo2_mod");
    module_serv[2] = luos_module_create(rx_servo_cb, SERVO_MOD, "servo3_mod");
    module_serv[3] = luos_module_create(rx_servo_cb, SERVO_MOD, "servo4_mod");
    luos_module_enable_rt(module_serv[0]);
    luos_module_enable_rt(module_serv[1]);
    luos_module_enable_rt(module_serv[2]);
    luos_module_enable_rt(module_serv[3]);
    servo_parameters_t param;
    param.max_angle = 180.0;
    param.max_pulse_time = 1.5 / 1000.0;
    param.min_pulse_time = 0.5 / 1000.0;
    servo[0].param = param;
    servo[0].angle = 0.0;
    servo[1].param = param;
    servo[1].angle = 0.0;
    servo[2].param = param;
    servo[2].angle = 0.0;
    servo[3].param = param;
    servo[3].angle = 0.0;
}

void servo_loop(void) {
}
