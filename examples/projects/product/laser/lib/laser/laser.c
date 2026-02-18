/******************************************************************************
 * @file laser
 * @brief driver example a laser
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "laser.h"
#include "product_config.h"
#include "main.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

ratio_t stream_buf[4096];
streaming_channel_t stream;
time_luos_t period;
control_t laser_control;
buffer_mode_t laser_buffer_mode = SINGLE;
TIM_TypeDef *pwmtimer           = LASER_PWM_TIMER;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Laser_MsgHandler(service_t *service, const msg_t *msg);

/******************************************************************************
 * @brief DRV_DCMotorHWInit
 * @param None
 * @return None
 ******************************************************************************/
static void laser_pwmInit(void)
// {
//     ///////////////////////////////
//     // GPIO Init
//     ///////////////////////////////
//     PWM_PIN_CLK();

//     GPIO_InitTypeDef GPIO_InitStruct = {0};

//     GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pin       = LASER_PWM_PIN;
//     GPIO_InitStruct.Alternate = LASER_PWM_AF;
//     HAL_GPIO_Init(LASER_PWM_PORT, &GPIO_InitStruct);

//     ///////////////////////////////
//     // Timer PWM Init
//     ///////////////////////////////
//     LL_TIM_InitTypeDef TimerInit;
//     LL_TIM_OC_InitTypeDef TimerConfigOC;
//     LL_TIM_StructInit(&TimerInit);
//     LL_TIM_OC_StructInit(&TimerConfigOC);

//     // initialize clock
//     PWM_TIMER_CLK();

//     TimerInit.Autoreload        = PWM_PERIOD;
//     TimerInit.ClockDivision     = LL_TIM_CLOCKDIVISION_DIV1;
//     TimerInit.CounterMode       = LL_TIM_COUNTERMODE_UP;
//     TimerInit.Prescaler         = 0;
//     TimerInit.RepetitionCounter = 0;

//     while (LL_TIM_Init(LASER_PWM_TIMER, &TimerInit) != SUCCESS)
//         ;

//     TimerConfigOC.OCMode       = LL_TIM_OCMODE_PWM1;
//     TimerConfigOC.CompareValue = 0;
//     while (LL_TIM_OC_Init(LASER_PWM_TIMER, LASER_PWM_CHANNEL, &TimerConfigOC) != SUCCESS)
//         ;

//     LL_TIM_EnableCounter(LASER_PWM_TIMER);
//     LL_TIM_CC_EnableChannel(LASER_PWM_TIMER, LASER_PWM_CHANNEL);
// }

{
    ///////////////////////////////
    // Timer PWM Init
    ///////////////////////////////
    // Initialize clock
    PWM_TIMER_CLK();
    TIM_HandleTypeDef htim;
    TIM_ClockConfigTypeDef sClockSourceConfig           = {0};
    TIM_MasterConfigTypeDef sMasterConfig               = {0};
    TIM_OC_InitTypeDef sConfigOC                        = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim.Instance               = LASER_PWM_TIMER;
    htim.Init.Prescaler         = 0;
    htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim.Init.Period            = PWM_PERIOD;
    htim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim.Init.RepetitionCounter = 0;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigOC.OCMode       = TIM_OCMODE_PWM1;
    sConfigOC.Pulse        = 0;
    sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim, &sConfigOC, LASER_PWM_CHANNEL) != HAL_OK)
    {
        Error_Handler();
    }
    sBreakDeadTimeConfig.OffStateRunMode  = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel        = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime         = 0;
    sBreakDeadTimeConfig.BreakState       = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity    = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim, &sBreakDeadTimeConfig) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_TIM_Base_Init(&htim);
    HAL_TIM_Base_Start(&htim);
    HAL_TIM_PWM_Init(&htim);
    HAL_TIM_PWM_Start(&htim, LASER_PWM_CHANNEL);
    ///////////////////////////////
    // GPIO Init
    ///////////////////////////////
    PWM_PIN_CLK();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pin       = LASER_PWM_PIN;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = LASER_PWM_AF;
    HAL_GPIO_Init(LASER_PWM_PORT, &GPIO_InitStruct);
}

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
static void laser_pwmSetPower(ratio_t power)
{
    LASER_PWM_TIMER->CCR2 = (uint32_t)((RatioOD_RatioTo_Percent(power) * (PWM_PERIOD)) / 100.0);
}

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Laser_Init(void)
{
    // Init the timer generating the PWM
    laser_pwmInit();

    // Init the Luos service
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    Luos_CreateService(Laser_MsgHandler, POWER_TYPE, "laser", revision);
    stream             = Streaming_CreateChannel(stream_buf, sizeof(stream_buf), sizeof(ratio_t));
    period             = TimeOD_TimeFrom_s(1.0 / DEFAULT_SAMPLE_FREQUENCY); // Configure the trajectory samplerate at 100Hz
    laser_control.flux = STOP;
}

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Laser_Loop(void)
{
    static time_luos_t last_time = {.raw = 0.0};
    if (laser_control.flux == PLAY)
    {
        // Check if it's time to update the trajectory
        if ((Luos_Timestamp().raw - last_time.raw) > period.raw)
        {
            if ((Streaming_GetAvailableSampleNB(&stream) > 0))
            {
                // We have available samples
                // update the last time
                last_time = Luos_Timestamp();
                // Get the next point
                ratio_t point;
                Streaming_GetSample(&stream, (void *)&point, 1);
                // Send the point to the laser
                laser_pwmSetPower(point);
            }
            else
            {
                // We don't have any available samples
                switch (laser_buffer_mode)
                {
                    case SINGLE:
                        // We are in single mode, we have to loop on the ring buffer
                        // Put the read pointer at the begining of the buffer
                        stream.sample_ptr = stream.ring_buffer;
                        // stop the trjectory
                        laser_control.flux = STOP;
                        break;
                    case CONTINUOUS:
                        // We are in continuous mode, we have to loop on the ring buffer
                        // Put the read pointer at the begining of the buffer
                        stream.sample_ptr = stream.ring_buffer;
                        // Get the first sample
                        if (Streaming_GetAvailableSampleNB(&stream) == 0)
                        {
                            // We don't have any new sample to compute
                            // stop the trajectory
                            laser_control.flux = STOP;
                            return;
                        }
                        // update the last time
                        last_time = Luos_Timestamp();
                        // Get the next point
                        ratio_t point;
                        Streaming_GetSample(&stream, (void *)&point, 1);
                        // Send the point to the laser
                        laser_pwmSetPower(point);
                        break;
                    case STREAM:
                        // We are in stream mode, we don't have any new data so we stop the trajectory
                        laser_control.flux = STOP;
                        break;
                    default:
                        LUOS_ASSERT(0);
                        break;
                }
            }
        }
    }
    else
    {
        // Stop the laser
        ratio_t power;
        power.raw = 0;
        laser_pwmSetPower(power);
    }
}

/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Laser_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == GET_CMD)
    {
        // The laser don't send anything back
        return;
    }
    if (msg->header.cmd == CONTROL)
    {
        // Get the laser_control value
        ControlOD_ControlFromMsg(&laser_control, msg);
    }
    if (msg->header.cmd == RATIO)
    {
        if (laser_buffer_mode == STREAM)
        {
            // The laser is in single mode, we can consider it as a streaming of point that will be consumed
            Luos_ReceiveStreaming(service, msg, &stream);
        }
        else
        {
            // The laser is in SINGLE or CONTINUOUS mode, The buffer need to be loaded with the trajectory and the laser will play it from the begining of the buffer to the end.
            int size = Luos_ReceiveData(service, msg, (uint8_t *)&stream_buf);
            if (size > 0)
            {
                LUOS_ASSERT(size <= sizeof(stream_buf));
                Streaming_ResetChannel(&stream);
                Streaming_AddAvailableSampleNB(&stream, size / stream.data_size);
            }
        }
    }
    if (msg->header.cmd == BUFFER_MODE)
    {
        laser_buffer_mode = msg->data[0];
    }
    if (msg->header.cmd == TIME)
    {
        // Get the time
        TimeOD_TimeFromMsg(&period, msg);
    }
}
