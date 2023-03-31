/******************************************************************************
 * @file servo_drv
 * @brief driver example a simple servo motor
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "servo_drv.h"

#include "stm32f0xx_ll_tim.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
servo_t servo[SERVONUMBER];

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Servo_DRVHWInit(void);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Servo_DRVInit(void)
{
    Servo_DRVHWInit();

    servo_parameters_t param;
    param.max_angle      = AngularOD_PositionFrom_deg(180.0);
    param.max_pulse_time = 1.5 / 1000.0;
    param.min_pulse_time = 0.5 / 1000.0;

    servo[0].param           = param;
    servo[0].angle           = AngularOD_PositionFrom_deg(0.0);
    servo[0].control.Timer   = S1_TIMER;
    servo[0].control.Channel = S1_CHANNEL;

    servo[1].param           = param;
    servo[1].angle           = AngularOD_PositionFrom_deg(0.0);
    servo[1].control.Timer   = S2_TIMER;
    servo[1].control.Channel = S2_CHANNEL;

    servo[2].param           = param;
    servo[2].angle           = AngularOD_PositionFrom_deg(0.0);
    servo[2].control.Timer   = S3_TIMER;
    servo[2].control.Channel = S3_CHANNEL;

    servo[3].param           = param;
    servo[3].angle           = AngularOD_PositionFrom_deg(0.0);
    servo[3].control.Timer   = S4_TIMER;
    servo[3].control.Channel = S4_CHANNEL;
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
uint8_t Servo_DRVSetPosition(angular_position_t angle, uint8_t motor_id)
{
    servo[motor_id].angle = angle;
    // limit angle value
    if (AngularOD_PositionTo_deg(servo[motor_id].angle) < 0.0)
    {
        servo[motor_id].angle = AngularOD_PositionFrom_deg(0.0);
    }
    else if (AngularOD_PositionTo_deg(servo[motor_id].angle) > AngularOD_PositionTo_deg(servo[motor_id].param.max_angle))
    {
        servo[motor_id].angle = servo[motor_id].param.max_angle;
    }

    uint32_t pulse_min = (uint32_t)(servo[motor_id].param.min_pulse_time * (float)(MCUFREQ / DEFAULT_PRESACALER));
    uint32_t pulse_max = (uint32_t)(servo[motor_id].param.max_pulse_time * (float)(MCUFREQ / DEFAULT_PRESACALER));
    uint32_t pulse     = pulse_min + (uint32_t)(AngularOD_PositionTo_deg(servo[motor_id].angle) / AngularOD_PositionTo_deg(servo[motor_id].param.max_angle) * (pulse_max - pulse_min));

    LL_TIM_EnableCounter(servo[motor_id].control.Timer);
    LL_TIM_CC_EnableChannel(servo[motor_id].control.Timer, servo[motor_id].control.Channel);

    if (servo[motor_id].control.Channel == LL_TIM_CHANNEL_CH1)
    {
        LL_TIM_OC_SetCompareCH1(servo[motor_id].control.Timer, pulse);
    }
    else if (servo[motor_id].control.Channel == LL_TIM_CHANNEL_CH2)
    {
        LL_TIM_OC_SetCompareCH2(servo[motor_id].control.Timer, pulse);
    }
    else if (servo[motor_id].control.Channel == LL_TIM_CHANNEL_CH3)
    {
        LL_TIM_OC_SetCompareCH3(servo[motor_id].control.Timer, pulse);
    }
    else if (servo[motor_id].control.Channel == LL_TIM_CHANNEL_CH4)
    {
        LL_TIM_OC_SetCompareCH4(servo[motor_id].control.Timer, pulse);
    }

    return SUCCEED;
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
uint8_t Servo_DRVParameter(servo_parameters_t param, uint8_t MotorId)
{
    servo[MotorId].param.max_angle      = param.max_angle;
    servo[MotorId].param.min_pulse_time = param.min_pulse_time;
    servo[MotorId].param.max_pulse_time = param.max_pulse_time;
    return SUCCEED;
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
static void Servo_DRVHWInit(void)
{
    // pinout initialization
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    PWM_PIN_CLK();

    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin       = S1_PIN;
    GPIO_InitStruct.Alternate = S1_AF;
    HAL_GPIO_Init(S1_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = S2_PIN;
    GPIO_InitStruct.Alternate = S2_AF;
    HAL_GPIO_Init(S2_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = S3_PIN;
    GPIO_InitStruct.Alternate = S3_AF;
    HAL_GPIO_Init(S3_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = S4_PIN;
    GPIO_InitStruct.Alternate = S4_AF;
    HAL_GPIO_Init(S4_PORT, &GPIO_InitStruct);

    // pwm init

    LL_TIM_InitTypeDef TimerInit;
    LL_TIM_OC_InitTypeDef TimerConfigOC;
    LL_TIM_StructInit(&TimerInit);
    LL_TIM_OC_StructInit(&TimerConfigOC);

    // initialize clock
    PWM_TIMER_CLK();

    TimerInit.Autoreload        = 16000 - 1;
    TimerInit.ClockDivision     = LL_TIM_CLOCKDIVISION_DIV1;
    TimerInit.CounterMode       = LL_TIM_COUNTERMODE_UP;
    TimerInit.Prescaler         = DEFAULT_PRESACALER;
    TimerInit.RepetitionCounter = 0;

    while (LL_TIM_Init(S1_TIMER, &TimerInit) != SUCCESS)
        ;
    while (LL_TIM_Init(S2_TIMER, &TimerInit) != SUCCESS)
        ;
    while (LL_TIM_Init(S3_TIMER, &TimerInit) != SUCCESS)
        ;
    while (LL_TIM_Init(S4_TIMER, &TimerInit) != SUCCESS)
        ;

    TimerConfigOC.OCMode       = LL_TIM_OCMODE_PWM1;
    TimerConfigOC.CompareValue = 400;
    while (LL_TIM_OC_Init(S1_TIMER, S1_CHANNEL, &TimerConfigOC) != SUCCESS)
        ;
    while (LL_TIM_OC_Init(S2_TIMER, S2_CHANNEL, &TimerConfigOC) != SUCCESS)
        ;
    while (LL_TIM_OC_Init(S3_TIMER, S3_CHANNEL, &TimerConfigOC) != SUCCESS)
        ;
    while (LL_TIM_OC_Init(S4_TIMER, S4_CHANNEL, &TimerConfigOC) != SUCCESS)
        ;
}
