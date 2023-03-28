/******************************************************************************
 * @file servo_drv
 * @brief This is a simple driver for a servo motor. It will work as is
 * but if you want to see it inside of a project, I am using it to create a
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/L432KC-Luos-Biometric-Security-System
 * @author MarieBidouille
 * @version 0.0.0
 ******************************************************************************/
#include "servo_drv.h"

#include "stm32l4xx_hal.h"
#include "stm32l4xx_ll_tim.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
servo_t servo;

/*******************************************************************************
 * Functions
 ******************************************************************************/
static void ServoDrv_HWInit(void);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void ServoDrv_Init(void)
{
    ServoDrv_HWInit();

    servo_parameters_t param;
    param.max_angle      = AngularOD_PositionFrom_deg(180.0);
    param.max_pulse_time = 1.5 / 1000;
    param.min_pulse_time = 0.5 / 1000;

    servo.param           = param;
    servo.angle           = AngularOD_PositionFrom_deg(0.0);
    servo.control.Timer   = SERVO_TIMER;
    servo.control.Channel = SERVO_CHANNEL;
}

/******************************************************************************
 * @brief init must be call in project init
 * @param angle
 * @return None
 ******************************************************************************/
uint8_t ServoDrv_SetPosition(angular_position_t angle)
{
    servo.angle = angle;
    // limit angle value
    if (AngularOD_PositionTo_deg(servo.angle) < 0.0)
    {
        servo.angle = AngularOD_PositionFrom_deg(0.0);
    }
    else if (AngularOD_PositionTo_deg(servo.angle) > AngularOD_PositionTo_deg(servo.param.max_angle))
    {
        servo.angle = servo.param.max_angle;
    }

    uint32_t pulse_min = (uint32_t)(servo.param.min_pulse_time * (float)(MCUFREQ / DEFAULT_PRESACALER));
    uint32_t pulse_max = (uint32_t)(servo.param.max_pulse_time * (float)(MCUFREQ / DEFAULT_PRESACALER));
    uint32_t pulse     = pulse_min + (uint32_t)(AngularOD_PositionTo_deg(servo.angle) / AngularOD_PositionTo_deg(servo.param.max_angle) * (pulse_max - pulse_min));

    LL_TIM_EnableCounter(servo.control.Timer);
    LL_TIM_CC_EnableChannel(servo.control.Timer, servo.control.Channel);

    if (servo.control.Channel == LL_TIM_CHANNEL_CH1)
    {
        LL_TIM_OC_SetCompareCH1(servo.control.Timer, pulse);
    }

    return SUCCEED;
}

/******************************************************************************
 * @brief set servo parameters
 * @param param
 * @return SUCCEED
 ******************************************************************************/
uint8_t ServoDrv_Parameter(servo_parameters_t param)
{
    servo.param.max_angle      = param.max_angle;
    servo.param.min_pulse_time = param.min_pulse_time;
    servo.param.max_pulse_time = param.max_pulse_time;
    return SUCCEED;
}

/******************************************************************************
 * @brief hw init must be call in package init
 * @param None
 * @return None
 ******************************************************************************/
static void ServoDrv_HWInit(void)
{
    // pinout initialization
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    PWM_PIN_CLK();

    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin       = SERVO_PIN;
    GPIO_InitStruct.Alternate = SERVO_AF;
    HAL_GPIO_Init(SERVO_PORT, &GPIO_InitStruct);

    // pwm init

    LL_TIM_InitTypeDef TimerInit;
    LL_TIM_OC_InitTypeDef TimerConfigOC;
    LL_TIM_StructInit(&TimerInit);
    LL_TIM_OC_StructInit(&TimerConfigOC);

    // initialize clock
    PWM_TIMER_CLK();

    TimerInit.Autoreload        = 32000 - 1;
    TimerInit.ClockDivision     = LL_TIM_CLOCKDIVISION_DIV1;
    TimerInit.CounterMode       = LL_TIM_COUNTERMODE_UP;
    TimerInit.Prescaler         = DEFAULT_PRESACALER;
    TimerInit.RepetitionCounter = 0;

    while (LL_TIM_Init(SERVO_TIMER, &TimerInit) != SUCCESS)
        ;

    TimerConfigOC.OCMode       = LL_TIM_OCMODE_PWM1;
    TimerConfigOC.CompareValue = 400;
    while (LL_TIM_OC_Init(SERVO_TIMER, SERVO_CHANNEL, &TimerConfigOC) != SUCCESS)
        ;
}