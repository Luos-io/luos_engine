#include "main.h"
#include "dc_motor.h"
#include "tim.h"

#define MOTORNUMBER 2

module_t *module[MOTORNUMBER];

int find_id(module_t* my_module) {
    int i = 0;
    for (i = 0; i<=MOTORNUMBER; i++) {
        if ((int)my_module == (int)module[i]) return i;
    }
    return i;
}

void set_power(module_t* module, ratio_t power) {
    // limit power value
    if (power < -100.0) power = -100.0;
    if (power > 100.0) power = 100.0;
    // transform power ratio to timer value
    uint16_t pulse;
    if (power > 0.0) {
        pulse = (uint16_t)(power * 50.0);
    } else {
        pulse = (uint16_t)(-power * 50.0);
    }
    switch (find_id(module)) {
        case 0:
            if (power > 0.0) {
                TIM2->CCR1 = pulse;
                TIM2->CCR2 = 0;
            } else {
                TIM2->CCR1 = 0;
                TIM2->CCR2 = pulse;
            }
        break;
        case 1:
            if (power > 0.0) {
                TIM3->CCR1 = pulse;
                TIM3->CCR2 = 0;
            } else {
                TIM3->CCR1 = 0;
                TIM3->CCR2 = pulse;
            }
        break;
        default:
            break;
    }
}

void rx_dc_mot_cb(module_t *module, msg_t *msg) {
    if (msg->header.cmd == RATIO) {
        // set the motor position
        ratio_t power;
        ratio_from_msg(&power, msg);
        set_power(module, power);
        return;
    }
}

void dc_motor_init(void) {
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    module[0] = luos_module_create(rx_dc_mot_cb, DCMOTOR_MOD, "DC_motor1_mod");
    module[1] = luos_module_create(rx_dc_mot_cb, DCMOTOR_MOD, "DC_motor2_mod");
    luos_module_enable_rt(module[0]);
    luos_module_enable_rt(module[1]);
}

void dc_motor_loop(void) {
}
