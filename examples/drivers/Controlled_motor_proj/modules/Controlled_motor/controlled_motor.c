#include "main.h"
#include "controlled_motor.h"
#include "analog.h"
#include "tim.h"
#include "math.h"
#include <float.h>
#include "luos.h"

#define SPEED_PERIOD 20.0
#define SAMPLING_PERIOD_MS 10.0
#define BUFFER_SIZE 1000

// Pin configuration
#define FB_Pin GPIO_PIN_0
#define FB_GPIO_Port GPIOB

volatile motor_config_t motor;

asserv_pid_t position;
asserv_pid_t speed;
float errSpeedSum = 0.0;
float motion_target_position = 0.0;
volatile time_luos_t time;


// Position Asserv things
volatile float errAngleSum = 0.0;
volatile float lastErrAngle = 0.0;

// Speed Asserv things
volatile float lastErrSpeed = 0.0;

// Control management
volatile control_mode_t control;

// Trajectory management (can be position or speed)
volatile float trajectory_buf[BUFFER_SIZE];
streaming_channel_t trajectory;
volatile angular_position_t last_position = 0.0;

// measurement management (can be position or speed)
volatile float measurement_buf[BUFFER_SIZE];
streaming_channel_t measurement;

void HAL_SYSTICK_Motor_Callback(void) {
    // ************* motion planning *************
    // ****** recorder management *********
    static uint32_t last_rec_systick = 0;
    if (control.mode_rec && ((HAL_GetTick() - last_rec_systick) >= time_to_ms(time))) {
        // We have to save a sample of current position
        set_sample(&measurement, &motor.angular_position);
        last_rec_systick = HAL_GetTick();
    }
    // ****** trajectory management *********
    static uint32_t last_systick = 0;
    if (control.mode_control == STOP) {
        reset_streaming_channel(&trajectory);
    }
    if ((get_nb_available_samples(&trajectory) > 0) && ((HAL_GetTick() - last_systick) >= time_to_ms(time)) && (control.mode_control == PLAY))
    {
        if (motor.mode.mode_linear_position == 1)
        {
            linear_position_t linear_position_tmp;
            get_sample(&trajectory, &linear_position_tmp);
            motor.target_angular_position = (linear_position_tmp * 360.0) / (3.141592653589793 * motor.wheel_diameter);
        }
        else
        {
            get_sample(&trajectory, &motor.target_angular_position);
        }
        last_systick = HAL_GetTick();
    }
    // ****** Linear interpolation *********
    if ((motor.mode.mode_angular_position || motor.mode.mode_linear_position) &&
        (motor.mode.mode_angular_speed || motor.mode.mode_linear_speed)) {

        // speed control and position control are enabled
        // we need to move target position following target speed
        float increment = (fabs(motor.target_angular_speed) / 1000.0);
        if (fabs(motor.target_angular_position - last_position) <= increment) {
            // target_position is the final target position
            motion_target_position = motor.target_angular_position;
        }else if ((motor.target_angular_position - motor.angular_position) < 0.0) {
            motion_target_position = last_position - increment;
        } else {
            motion_target_position = last_position + increment;
        }
    } else {
        // target_position is the final target position
        motion_target_position = motor.target_angular_position;
    }
        last_position = motion_target_position;
}

void set_ratio(float ratio) {
    // limit power value
    if (ratio < -motor.limit_ratio) ratio = -motor.limit_ratio;
    if (ratio > motor.limit_ratio) ratio = motor.limit_ratio;
    // transform power ratio to timer value
    uint16_t pulse;
    if (ratio > 0.0) {
        pulse = (uint16_t)(ratio * 24.0);
        TIM3->CCR1 = pulse;
        TIM3->CCR2 = 0;
    } else {
        pulse = (uint16_t)(-ratio * 24.0);
        TIM3->CCR1 = 0;
        TIM3->CCR2 = pulse;
    }
}

void enable_motor(char state) {
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, state);
}

void rx_ctrl_mot_cb(module_t *module, msg_t *msg) {
    if (msg->header.cmd == ASK_PUB_CMD) {
        // Report management
        msg_t pub_msg;
        pub_msg.header.target_mode = ID;
        pub_msg.header.target = msg->header.source;
        if (motor.mode.angular_position) {
            if (control.mode_rec) {
                // send back a record stream
                pub_msg.header.cmd = ANGULAR_POSITION;
                luos_send_streaming(module, &pub_msg, &measurement);
            } else {
                angular_position_to_msg(&motor.angular_position, &pub_msg);
                luos_send(module, &pub_msg);
            }
        }
        if (motor.mode.angular_speed) {
            angular_speed_to_msg(&motor.angular_speed, &pub_msg);
            luos_send(module, &pub_msg);
        }
        if (motor.mode.linear_position) {
            linear_position_to_msg(&motor.linear_position, &pub_msg);
            luos_send(module, &pub_msg);
        }
        if (motor.mode.linear_speed) {
            linear_speed_to_msg(&motor.linear_speed, &pub_msg);
            luos_send(module, &pub_msg);
        }
        if (motor.mode.current) {
            current_to_msg(&motor.current, &pub_msg);
            luos_send(module, &pub_msg);
        }
        return;
    }
    if (msg->header.cmd == PID) {
            // check the message size
            if (msg->header.size == sizeof(asserv_pid_t)) {
                // fill the message infos
                if ((motor.mode.mode_angular_position || motor.mode.mode_linear_position) &&
                    !(motor.mode.mode_angular_speed || motor.mode.mode_linear_speed)) {
                    // only position control is enable, we can save PID for positioning
                    memcpy(&position, msg->data, msg->header.size);
                }
                if ((motor.mode.mode_angular_speed || motor.mode.mode_linear_speed) &&
                    !(motor.mode.mode_angular_position || motor.mode.mode_linear_position)) {
                    // only speed control is enable, we can save PID for speed
                    memcpy(&speed, msg->data, msg->header.size);
                }
            }
            return;
        }
    if (msg->header.cmd == PARAMETERS) {
        // check the message size
        if (msg->header.size == sizeof(motor_mode_t)) {
            // fill the message infos
            memcpy(&motor.mode, msg->data, msg->header.size);
            enable_motor(motor.mode.mode_compliant == 0);
            if (motor.mode.mode_compliant == 0){
                last_position = motor.angular_position;
                errAngleSum = 0.0;
                lastErrAngle = 0.0;
                motor.target_angular_position = motor.angular_position;
            }
        }
        return;
    }
    if (msg->header.cmd == CONTROL) {
        control.unmap = msg->data[0];
        if (control.mode_control == 3){
            // impossible value, go back to default values
            control.unmap = 0;
        }
        return;
    }
    if (msg->header.cmd == RESOLUTION) {
        // set the encoder resolution
        memcpy(&motor.resolution, msg->data, sizeof(float));
        return;
    }
    if (msg->header.cmd == REDUCTION) {
        // set the motor reduction
        memcpy(&motor.motor_reduction, msg->data, sizeof(float));
        return;
    }
    if (msg->header.cmd == REINIT) {
        // set state to 0
        motor.angular_position = 0.0;
        motor.target_angular_position = 0.0;
        errAngleSum = 0.0;
        lastErrAngle = 0.0;
        last_position = 0.0;
        return;
    }
    if (msg->header.cmd == DIMENSION) {
        // set the wheel diameter m
        linear_position_from_msg(&motor.wheel_diameter, msg);
        return;
    }
    if (msg->header.cmd == RATIO) {
        // set the motor power ratio (no asserv)
        ratio_from_msg(&motor.target_ratio, msg);
        return;
    }
    if (msg->header.cmd == ANGULAR_POSITION) {
        if (motor.mode.mode_angular_position) {
            // Check message size
            if (msg->header.size == sizeof(float)){
                // set the motor target angular position
                last_position = motor.angular_position;
                angular_position_from_msg(&motor.target_angular_position, msg);
            } else {
                // this is a trajectory, save it into streaming channel.
                luos_receive_streaming(module, msg, &trajectory);
            }
        }
        return;
    }
    if (msg->header.cmd == ANGULAR_SPEED) {
        // set the motor target angular position
        if (motor.mode.mode_angular_speed) {
            angular_speed_from_msg(&motor.target_angular_speed, msg);
            // reset the integral factor for speed
            errSpeedSum = 0.0;
        }
        return;
    }
    if (msg->header.cmd == LINEAR_POSITION) {
        // set the motor target linear position
        // Check message size
        if (msg->header.size == sizeof(float)){
            linear_position_t linear_position = 0.0;
            linear_position_from_msg(&linear_position, msg);
            motor.target_angular_position = (linear_position * 360.0) / (3.141592653589793 * motor.wheel_diameter);
        } else {
            // this is a trajectory, save it into ring buffer.
            luos_receive_streaming(module, msg, &trajectory);
            // values will be converted one by one during trajectory management.
        }
        return;
    }
    if (msg->header.cmd == LINEAR_SPEED) {
        // set the motor target linear speed
        if (motor.wheel_diameter > 0.0) {
            linear_speed_t linear_speed = 0.0;
            linear_speed_from_msg(&linear_speed, msg);
            motor.target_angular_speed = (linear_speed * 360.0) / (3.141592653589793 * motor.wheel_diameter);
        }
        return;
    }
    if (msg->header.cmd == ANGULAR_POSITION_LIMIT) {
        // set the motor limit anglular position
        memcpy(motor.limit_angular_position, msg->data, 2*sizeof(float));
        return;
    }
    if (msg->header.cmd == LINEAR_POSITION_LIMIT) {
        // set the motor target linear position
        if (motor.mode.mode_linear_position & (motor.wheel_diameter != 0)) {
            linear_position_t linear_position[2] = {0.0, 0.0};
            memcpy(linear_position, msg->data, 2*sizeof(linear_position_t));
            motor.limit_angular_position[0] = (linear_position[0] * 360.0) / (3.141592653589793 * motor.wheel_diameter);
            motor.limit_angular_position[1] = (linear_position[1] * 360.0) / (3.141592653589793 * motor.wheel_diameter);
        }
        return;
    }
    if (msg->header.cmd == RATIO_LIMIT) {
        // set the motor power ratio limit
        memcpy(&motor.limit_ratio, msg->data, sizeof(float));
        motor.limit_ratio = fabs(motor.limit_ratio);
        if (motor.limit_ratio > 100.0) motor.limit_ratio = 100.0;
        return;
    }
    if (msg->header.cmd == CURRENT_LIMIT) {
        // set the motor current limit
        current_from_msg(&motor.limit_current, msg);
        return;
    }
    if (msg->header.cmd == TIME) {
        // save time in ms
        time_from_msg(&time, msg);
        return;
    }
}

void controlled_motor_init(void)
{
    // ******************* Analog measurement *******************
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    ADC_ChannelConfTypeDef sConfig = {0};
    // Stop DMA
    HAL_ADC_Stop_DMA(&luos_adc);

    // Configure analog input pin channel
    GPIO_InitStruct.Pin = FB_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(FB_GPIO_Port, &GPIO_InitStruct);

    // Add ADC channel to Luos adc configuration.
    sConfig.Channel = ADC_CHANNEL_8;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&luos_adc, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    // relinik DMA
    __HAL_LINKDMA(&luos_adc, DMA_Handle, luos_dma_adc);

    // Restart DMA
    HAL_ADC_Start_DMA(&luos_adc, analog_input.unmap, sizeof(analog_input.unmap) / sizeof(uint32_t));

    // ************** Pwm settings *****************
    time = time_from_ms(SAMPLING_PERIOD_MS);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_1|TIM_CHANNEL_2);

    // ************** Default configuration settings *****************
    // motor mode by default
    enable_motor(0);
    motor.mode.mode_compliant = 1;
    motor.mode.current = 0;
    motor.mode.mode_ratio = 1;
    motor.mode.mode_angular_position = 0;
    motor.mode.mode_angular_speed = 0;
    motor.mode.mode_linear_position = 0;
    motor.mode.mode_linear_speed = 0;
    motor.mode.angular_position = 1;
    motor.mode.angular_speed = 0;
    motor.mode.linear_position = 0;
    motor.mode.linear_speed = 0;

    // default motor configuration
    motor.motor_reduction = 131;
    motor.resolution = 16;
    motor.wheel_diameter = 0.100f;

    // default motor limits
    motor.limit_ratio = 100.0;
    motor.limit_angular_position[MIN] = -FLT_MAX;
    motor.limit_angular_position[MAX] = FLT_MAX;
    motor.limit_current = 6.0;

    // Position PID default values
    position.p = 4.0;
    position.i = 0.02;
    position.d = 100.0;

    // Speed PID default values
    speed.p = 0.1;
    speed.i = 0.1;
    speed.d = 0.0;

    // Control mode default values
    control.unmap = 0; // PLAY and no REC

    // Init streaming channels
    trajectory = create_streaming_channel(trajectory_buf, BUFFER_SIZE, sizeof(float));
    measurement = create_streaming_channel(measurement_buf, BUFFER_SIZE, sizeof(float));

    // ************** Module creation *****************
    luos_module_create(rx_ctrl_mot_cb, CONTROLLED_MOTOR_MOD, "controlled_motor_mod");
}

void controlled_motor_loop(void) {
    // copy analog value to the L0 struct
    node_analog.temperature_sensor = analog_input.temperature_sensor;
    node_analog.voltage_sensor = analog_input.voltage_sensor;

    // Time management
    static uint32_t last_speed_systick = 0;

    unsigned long timestamp = HAL_GetTick();
    unsigned long deltatime = timestamp - last_speed_systick;

    // Speed measurement
    static float last_angular_position = 0.0;
    static linear_position_t last_linear_distance = 0.0;

    // ************* Values computation *************
    // angular_posistion => degree
    int32_t encoder_count = (int16_t)TIM2->CNT;
    TIM2->CNT = 0;
    motor.angular_position += (angular_position_t)((double)encoder_count / (double)(motor.motor_reduction * motor.resolution * 4)) * 360.0;
    // linear_distance => m
    motor.linear_position = (motor.angular_position/ 360.0) * M_PI * motor.wheel_diameter;
    // current => A
    motor.current = ((((float)analog_input.current) * 3.3f) / 4096.0f) / 0.525f;

    if (deltatime >= SPEED_PERIOD) {
        // angular_speed => degree/seconds
        motor.angular_speed = (motor.angular_position - last_angular_position) * 1000.0/SPEED_PERIOD;
        // linear_speed => m/seconds
        motor.linear_speed = (motor.linear_position - last_linear_distance) * 1000.0/SPEED_PERIOD;
        last_speed_systick = timestamp;
        last_angular_position = motor.angular_position;
        last_linear_distance = motor.linear_position;
        // ************* Limit clamping *************
        if (motion_target_position < motor.limit_angular_position[MIN]) {
            motion_target_position = motor.limit_angular_position[MIN];
        }
        if (motion_target_position > motor.limit_angular_position[MAX]) {
            motion_target_position = motor.limit_angular_position[MAX];
        }
        float currentfactor = 1.0f;
        currentfactor = motor.limit_current / (motor.current*2);
        static float surpCurrentSum = 0.0;
        float surpCurrent = motor.current - motor.limit_current;
        surpCurrentSum += surpCurrent;
        // If surpCurrentSum > 0 do a real coef
        if (surpCurrentSum > 0.0) {
            currentfactor = motor.limit_current / (motor.limit_current + (surpCurrentSum / 1.5));
        } else {
            surpCurrentSum = 0.0;
            currentfactor = 1.0f;
        }
        if (motor.mode.mode_compliant) {
            //Motor is compliant, only manage motor limits
            if (motor.angular_position < motor.limit_angular_position[MIN]) {
                //re-enable motor to avoid bypassing motors limits
                enable_motor(1);
                set_ratio(100.0 * (motor.limit_angular_position[MIN]-motor.angular_position));
            } else if (motor.angular_position > motor.limit_angular_position[MAX]) {
                enable_motor(1);
                set_ratio(-100.0 * (motor.angular_position - motor.limit_angular_position[MAX]));
            } else {
                enable_motor(0);
            }
        }
        else if (motor.mode.mode_ratio) {
            set_ratio(motor.target_ratio * currentfactor);
        }
        else if (!motor.mode.mode_ratio) {
            // ************* position asserv *************
            // Target Position is managed by the motion planning interrupt (systick interrupt)
            float errAngle = 0.0;
            float dErrAngle = 0.0;
            float anglePower = 0.0;
            if (motor.mode.mode_angular_position || motor.mode.mode_linear_position) {
                errAngle = motion_target_position - motor.angular_position;
                dErrAngle = (errAngle - lastErrAngle) / deltatime;
                if (fabs(errAngle) > 5.0) { // do not integrate if error is too big.
                    errAngleSum = 0.0;
                    anglePower = (errAngle * position.p) + (dErrAngle * position.d); // raw PD command
                } else {
                    errAngleSum += (errAngle  * (float)deltatime);
                    if (errAngleSum < -100.0) errAngleSum = -100.0;
                    if (errAngleSum > 100.0) errAngleSum = 100;
                    anglePower = (errAngle * position.p) + (errAngleSum * position.i) + (dErrAngle * position.d); // raw PID command
                }
                lastErrAngle = errAngle;
            }
            // ************* speed asserv *************
            float errSpeed = 0.0;
            float dErrSpeed = 0.0;
            float speedPower = 0.0;
            if (motor.mode.mode_angular_speed || motor.mode.mode_linear_speed) {
                errSpeed = motor.target_angular_speed - motor.angular_speed;
                dErrSpeed = (errSpeed - lastErrSpeed) / deltatime;
                errSpeedSum += errSpeed; // Integral
                speedPower = ((errSpeed * speed.p) + (errSpeedSum * speed.i) + (dErrSpeed * speed.d)); // raw PID command
                lastErrSpeed = errSpeed;
            }
            // ************* command merge *************
            if (!(motor.mode.mode_angular_position || motor.mode.mode_linear_position) &&
                (motor.mode.mode_angular_speed || motor.mode.mode_linear_speed)) {
                // Speed control only
                set_ratio(speedPower * currentfactor);
            } else {
                // we use position control by default
                set_ratio(anglePower * currentfactor);
            }
        }
    }

}
