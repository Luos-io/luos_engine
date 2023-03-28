/******************************************************************************
 * @file controle_dc_motor
 * @brief pwm generation for dc motor control
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "stdbool.h"
#include "dc_motor_drv.h"

#include "stm32g4xx_ll_tim.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    TIM_TypeDef *CLKWiseTimer;
    uint32_t CLKWiseChannel;
    TIM_TypeDef *CounterCLKWiseTimer;
    uint32_t CounterCLKWiseChannel;
} dc_motor_command_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/
dc_motor_command_t MotorDCCommand[MOTOR_NUMBER];
TIM_HandleTypeDef Timerhandle;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void DRV_DCMotorHWInit(void);
/******************************************************************************
 * @brief Control_DCMotor1_Init
 * @param None
 * @return None
 ******************************************************************************/
void DRV_DCMotorInit(void)
{
    DRV_DCMotorHWInit();

    // assign to motor pwm
    MotorDCCommand[MOTOR_DC_1].CLKWiseTimer          = PWM_1_TIMER;
    MotorDCCommand[MOTOR_DC_1].CLKWiseChannel        = PWM_1_CHANNEL;
    MotorDCCommand[MOTOR_DC_1].CounterCLKWiseTimer   = PWM_2_TIMER;
    MotorDCCommand[MOTOR_DC_1].CounterCLKWiseChannel = PWM_2_CHANNEL;

    MotorDCCommand[MOTOR_DC_2].CLKWiseTimer          = PWM_3_TIMER;
    MotorDCCommand[MOTOR_DC_2].CLKWiseChannel        = PWM_3_CHANNEL;
    MotorDCCommand[MOTOR_DC_2].CounterCLKWiseTimer   = PWM_4_TIMER;
    MotorDCCommand[MOTOR_DC_2].CounterCLKWiseChannel = PWM_4_CHANNEL;

    DRV_DCMotorEnable(true);
}
/******************************************************************************
 * @brief Set Power to DC motor
 * @param None
 * @return None
 ******************************************************************************/
uint8_t DRV_DCMotorSetPower(uint8_t Motor, ratio_t power)
{
    uint16_t CLKWpulse;
    uint16_t CCLKWpulse;
    if (RatioOD_RatioTo_Percent(power) > 0.0)
    {
        // limit power value
        if (RatioOD_RatioTo_Percent(power) > 100.0)
        {
            power = RatioOD_RatioFrom_Percent(100.0);
        }
        // transform power ratio to timer value
        CLKWpulse  = (uint16_t)(RatioOD_RatioTo_Percent(power) * 50.0);
        CCLKWpulse = 0;
    }
    else
    {
        // limit power value
        if (RatioOD_RatioTo_Percent(power) < -100.0)
        {
            power = RatioOD_RatioFrom_Percent(-100.0);
        }
        // transform power ratio to timer value
        CCLKWpulse = (uint16_t)(-RatioOD_RatioTo_Percent(power) * 50.0);
        CLKWpulse  = 0;
    }

    if (MotorDCCommand[Motor].CLKWiseChannel == LL_TIM_CHANNEL_CH1)
    {
        LL_TIM_OC_SetCompareCH1(MotorDCCommand[Motor].CLKWiseTimer, CLKWpulse);
    }
    else if (MotorDCCommand[Motor].CLKWiseChannel == LL_TIM_CHANNEL_CH2)
    {
        LL_TIM_OC_SetCompareCH2(MotorDCCommand[Motor].CLKWiseTimer, CLKWpulse);
    }

    if (MotorDCCommand[Motor].CounterCLKWiseChannel == LL_TIM_CHANNEL_CH1)
    {
        LL_TIM_OC_SetCompareCH1(MotorDCCommand[Motor].CounterCLKWiseTimer, CCLKWpulse);
    }
    else if (MotorDCCommand[Motor].CounterCLKWiseChannel == LL_TIM_CHANNEL_CH2)
    {
        LL_TIM_OC_SetCompareCH2(MotorDCCommand[Motor].CounterCLKWiseTimer, CCLKWpulse);
    }
    return SUCCEED;
}
/******************************************************************************
 * @brief DRV_DCMotorHWEnable
 * @param None
 * @return None
 ******************************************************************************/
void DRV_DCMotorEnable(uint8_t enable)
{
    if (enable == true)
    {
        HAL_GPIO_WritePin(SLEEP_PORT, SLEEP_PIN, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(SLEEP_PORT, SLEEP_PIN, GPIO_PIN_RESET);
    }
}
/******************************************************************************
 * @brief DRV_DCMotorHWInit
 * @param None
 * @return None
 ******************************************************************************/
static void DRV_DCMotorHWInit(void)
{
    ///////////////////////////////
    // GPIO Init
    ///////////////////////////////
    PWM_PIN_CLK();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin   = SLEEP_PIN;
    HAL_GPIO_Init(SLEEP_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pin       = PWM_1_PIN;
    GPIO_InitStruct.Alternate = PWM_1_AF;
    HAL_GPIO_Init(PWM_1_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = PWM_2_PIN;
    GPIO_InitStruct.Alternate = PWM_2_AF;
    HAL_GPIO_Init(PWM_2_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = PWM_3_PIN;
    GPIO_InitStruct.Alternate = PWM_3_AF;
    HAL_GPIO_Init(PWM_3_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = PWM_4_PIN;
    GPIO_InitStruct.Alternate = PWM_4_AF;
    HAL_GPIO_Init(PWM_4_PORT, &GPIO_InitStruct);

    ///////////////////////////////
    // Timer PWM Init
    ///////////////////////////////
    LL_TIM_InitTypeDef TimerInit;
    LL_TIM_OC_InitTypeDef TimerConfigOC;
    LL_TIM_StructInit(&TimerInit);
    LL_TIM_OC_StructInit(&TimerConfigOC);

    // initialize clock
    PWM_TIMER_CLK();

    TimerInit.Autoreload        = PWM_PERIOD;
    TimerInit.ClockDivision     = LL_TIM_CLOCKDIVISION_DIV1;
    TimerInit.CounterMode       = LL_TIM_COUNTERMODE_UP;
    TimerInit.Prescaler         = 0;
    TimerInit.RepetitionCounter = 0;

    while (LL_TIM_Init(PWM_1_TIMER, &TimerInit) != SUCCESS)
        ;
    while (LL_TIM_Init(PWM_2_TIMER, &TimerInit) != SUCCESS)
        ;
    while (LL_TIM_Init(PWM_3_TIMER, &TimerInit) != SUCCESS)
        ;
    while (LL_TIM_Init(PWM_4_TIMER, &TimerInit) != SUCCESS)
        ;

    TimerConfigOC.OCMode       = LL_TIM_OCMODE_PWM1;
    TimerConfigOC.CompareValue = 0;
    while (LL_TIM_OC_Init(PWM_1_TIMER, PWM_1_CHANNEL, &TimerConfigOC) != SUCCESS)
        ;
    while (LL_TIM_OC_Init(PWM_2_TIMER, PWM_2_CHANNEL, &TimerConfigOC) != SUCCESS)
        ;
    while (LL_TIM_OC_Init(PWM_3_TIMER, PWM_3_CHANNEL, &TimerConfigOC) != SUCCESS)
        ;
    while (LL_TIM_OC_Init(PWM_4_TIMER, PWM_4_CHANNEL, &TimerConfigOC) != SUCCESS)
        ;

    LL_TIM_EnableCounter(PWM_1_TIMER);
    LL_TIM_CC_EnableChannel(PWM_1_TIMER, PWM_1_CHANNEL);

    LL_TIM_EnableCounter(PWM_2_TIMER);
    LL_TIM_CC_EnableChannel(PWM_2_TIMER, PWM_2_CHANNEL);

    LL_TIM_EnableCounter(PWM_3_TIMER);
    LL_TIM_CC_EnableChannel(PWM_3_TIMER, PWM_3_CHANNEL);

    LL_TIM_EnableCounter(PWM_4_TIMER);
    LL_TIM_CC_EnableChannel(PWM_4_TIMER, PWM_4_CHANNEL);
}
