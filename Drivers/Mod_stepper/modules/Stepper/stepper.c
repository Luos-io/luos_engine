#include "main.h"
#include "stepper.h"
#include "math.h"
#include "tim.h"

volatile motor_config_t motor;
volatile uint8_t microstepping = 16;
volatile int target_step_nb = 0;
volatile int current_step_nb = 0;

void compute_speed(void) {
    if (fabs(motor.target_angular_speed) > 0.1) {
        volatile float degPerStep = 360.0 / (float)(motor.resolution * microstepping);
        volatile float timePerStep = 1.0 / (fabs(motor.target_angular_speed) / degPerStep);
        htim3.Init.Period = (uint32_t)(timePerStep * (float)(48000000 / htim3.Init.Prescaler));
        TIM3->CCR3 = htim3.Init.Period / 2;
        HAL_TIM_Base_Init(&htim3);
        HAL_TIM_Base_Start(&htim3);
        HAL_TIM_Base_Start_IT(&htim3);
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    } else {
        TIM3->CCR3 = 0;
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    static float last_speed = 100.0;
    if (motor.target_angular_speed != last_speed) {
        last_speed = motor.target_angular_speed;
        compute_speed();
    }
    if (motor.mode.mode_compliant) {
        // stop pwm output
        TIM3->CCR3 = 0;
        return;
    }
    if (motor.mode.mode_angular_position || motor.mode.mode_linear_position) {
        if (current_step_nb < target_step_nb) {
            // start pwm output
            TIM3->CCR3 = htim3.Init.Period / 2;
            HAL_GPIO_WritePin(DIR_GPIO_Port,DIR_Pin, 1);
            current_step_nb++;
        } else if (current_step_nb > target_step_nb) {
            // start pwm output
            TIM3->CCR3 = htim3.Init.Period / 2;
            HAL_GPIO_WritePin(DIR_GPIO_Port,DIR_Pin, 0);
            current_step_nb--;
        } else {
            // stop pwm output
            TIM3->CCR3 = 0;
        }
    } else {
        if (motor.target_angular_speed > 0.0) {
            // start pwm output
            TIM3->CCR3 = htim3.Init.Period / 2;
            HAL_GPIO_WritePin(DIR_GPIO_Port,DIR_Pin, 1);
            current_step_nb++;
        }
        if (motor.target_angular_speed < 0.0) {
            // start pwm output
            TIM3->CCR3 = htim3.Init.Period / 2;
            HAL_GPIO_WritePin(DIR_GPIO_Port,DIR_Pin, 0);
            current_step_nb--;
        }
    }
}

void rx_stp_cb(module_t *module, msg_t *msg) {
    if (msg->header.cmd == PARAMETERS) {
        // check the message size
        if (msg->header.size == sizeof(motor_mode_t)) {
            // fill the message infos
            memcpy(&motor.mode, msg->data, msg->header.size);
            HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, motor.mode.mode_compliant);
        }
        return;
    }
    if (msg->header.cmd == RESOLUTION) {
        // set the encoder resolution
        memcpy(&motor.resolution, msg->data, sizeof(float));
        return;
    }
    if (msg->header.cmd == REINIT) {
        // set motor position to 0
        motor.angular_position = 0.0;
        motor.target_angular_position = 0.0;
        motor.target_angular_speed = 100.0;
        return;
    }
    if (msg->header.cmd == DIMENSION) {
        // set the wheel diameter m
        linear_position_from_msg(&motor.wheel_diameter, msg);
        return;
    }
    if (msg->header.cmd == ANGULAR_POSITION) {
        // set the motor target rotation position
        if (motor.mode.mode_angular_position) {
            angular_position_from_msg(&motor.target_angular_position, msg);
        }
        return;
    }
    if (msg->header.cmd == ANGULAR_SPEED) {
        // set the motor target rotation position
        angular_speed_from_msg(&motor.target_angular_speed, msg);
        return;
    }

    if (msg->header.cmd == LINEAR_POSITION) {
        // set the motor target translation position
        if (motor.mode.mode_linear_position & (motor.wheel_diameter != 0)) {
            linear_position_t linear_position = 0.0;
            linear_position_from_msg(&linear_position, msg);
            motor.target_angular_position = (linear_position * 360.0) / (M_PI * motor.wheel_diameter);
        }
        return;
    }
    if (msg->header.cmd == LINEAR_SPEED) {
        // set the motor target rotation position
        if (motor.wheel_diameter != 0) {
            linear_speed_t linear_speed = 0.0;
            linear_speed_from_msg(&linear_speed, msg);
            motor.target_angular_speed = (linear_speed * 360.0) / (M_PI * motor.wheel_diameter);
        }
        return;
    }
}

void stepper_init(void) {
    luos_module_enable_rt(luos_module_create(rx_stp_cb, STEPPER_MOD, "stepper_mod"));
    motor.resolution = 200.0;
    motor.wheel_diameter = 0.0;
    motor.target_angular_speed = 100.0;

    motor.mode.mode_compliant = 1;
    motor.mode.mode_angular_position = 1;
    motor.mode.mode_angular_speed = 0;
    motor.mode.mode_linear_position = 0;
    motor.mode.mode_linear_speed = 0;

    HAL_GPIO_WritePin(MS1_GPIO_Port,MS1_Pin, 1);
    HAL_GPIO_WritePin(MS2_GPIO_Port,MS2_Pin, 1);
    HAL_GPIO_WritePin(MS3_GPIO_Port,MS3_Pin, 1);

    HAL_GPIO_WritePin(EN_GPIO_Port,EN_Pin, 1);

    compute_speed();
}

void stepper_loop(void) {
    // compute values
    float degPerStep = 360.0 / (float)(motor.resolution * microstepping);
    target_step_nb = (int)(motor.target_angular_position / degPerStep);
}
