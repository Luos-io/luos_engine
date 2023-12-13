/******************************************************************************
 * @file Stepper
 * @brief driver example a simple Stepper
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "main.h"
#include "stepper.h"
#include "math.h"
#include "tim.h"
#include <float.h>
#include "profile_servo_motor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
profile_servo_motor_t stepper_motor;

volatile uint8_t microstepping = 16;
volatile int target_step_nb    = 0;
volatile int current_step_nb   = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Stepper_MsgHandler(service_t *service, const msg_t *msg);
static void compute_speed(void);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Stepper_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};

    HAL_GPIO_WritePin(MS1_GPIO_Port, MS1_Pin, 1);
    HAL_GPIO_WritePin(MS2_GPIO_Port, MS2_Pin, 1);
    HAL_GPIO_WritePin(MS3_GPIO_Port, MS3_Pin, 1);

    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, 1);

    // ************** Default configuration settings *****************
    // motor mode by default
    stepper_motor.mode.mode_compliant        = 1;
    stepper_motor.mode.current               = 0;
    stepper_motor.mode.mode_power            = 1;
    stepper_motor.mode.mode_angular_position = 0;
    stepper_motor.mode.mode_angular_speed    = 0;
    stepper_motor.mode.mode_linear_position  = 0;
    stepper_motor.mode.mode_linear_speed     = 0;
    stepper_motor.mode.angular_position      = 1;
    stepper_motor.mode.angular_speed         = 0;
    stepper_motor.mode.linear_position       = 0;
    stepper_motor.mode.linear_speed          = 0;

    // default motor configuration
    stepper_motor.motor_reduction      = 131;
    stepper_motor.resolution           = 200;
    stepper_motor.wheel_diameter       = LinearOD_PositionFrom_m(0.100f);
    stepper_motor.target_angular_speed = AngularOD_SpeedFrom_deg_s(100.0);

    // default motor limits
    stepper_motor.limit_angular_position[MINI] = AngularOD_PositionFrom_deg(-FLT_MAX);
    stepper_motor.limit_angular_position[MAXI] = AngularOD_PositionFrom_deg(FLT_MAX);
    stepper_motor.motor.limit_current          = ElectricOD_CurrentFrom_A(6.0);

    // Control mode default values
    stepper_motor.control.unmap = 0; // PLAY and no REC

    compute_speed();

    // ************** Service creation *****************
    ProfileServo_CreateService(&stepper_motor, Stepper_MsgHandler, "stepper_motor", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Stepper_Loop(void)
{
    // compute values
    float degPerStep = 360.0 / (float)(stepper_motor.resolution * microstepping);
    target_step_nb   = (int)(AngularOD_PositionTo_deg(stepper_motor.target_angular_position) / degPerStep);
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Stepper_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == PARAMETERS)
    {
        HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, stepper_motor.mode.mode_compliant);
    }
}

static void compute_speed(void)
{
    if (fabs(AngularOD_SpeedTo_deg_s(stepper_motor.target_angular_speed)) > 0.1)
    {
        volatile float degPerStep  = 360.0 / (float)(stepper_motor.resolution * microstepping);
        volatile float timePerStep = 1.0 / (fabs(AngularOD_SpeedTo_deg_s(stepper_motor.target_angular_speed)) / degPerStep);
        htim3.Init.Period          = (uint32_t)(timePerStep * (float)(48000000 / htim3.Init.Prescaler));
        TIM3->CCR3                 = htim3.Init.Period / 2;
        HAL_TIM_Base_Init(&htim3);
        HAL_TIM_Base_Start(&htim3);
        HAL_TIM_Base_Start_IT(&htim3);
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    }
    else
    {
        TIM3->CCR3 = 0;
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static float last_speed = 100.0;
    if (AngularOD_SpeedTo_deg_s(stepper_motor.target_angular_speed) != last_speed)
    {
        last_speed = AngularOD_SpeedTo_deg_s(stepper_motor.target_angular_speed);
        compute_speed();
    }
    if (stepper_motor.mode.mode_compliant)
    {
        // stop pwm output
        TIM3->CCR3 = 0;
        return;
    }
    if (stepper_motor.mode.mode_angular_position || stepper_motor.mode.mode_linear_position)
    {
        if (current_step_nb < target_step_nb)
        {
            // start pwm output
            TIM3->CCR3 = htim3.Init.Period / 2;
            HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, 1);
            current_step_nb++;
        }
        else if (current_step_nb > target_step_nb)
        {
            // start pwm output
            TIM3->CCR3 = htim3.Init.Period / 2;
            HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, 0);
            current_step_nb--;
        }
        else
        {
            // stop pwm output
            TIM3->CCR3 = 0;
        }
    }
    else
    {
        if (AngularOD_SpeedTo_deg_s(stepper_motor.target_angular_speed) > 0.0)
        {
            // start pwm output
            TIM3->CCR3 = htim3.Init.Period / 2;
            HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, 1);
            current_step_nb++;
        }
        if (AngularOD_SpeedTo_deg_s(stepper_motor.target_angular_speed) < 0.0)
        {
            // start pwm output
            TIM3->CCR3 = htim3.Init.Period / 2;
            HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, 0);
            current_step_nb--;
        }
    }
}
