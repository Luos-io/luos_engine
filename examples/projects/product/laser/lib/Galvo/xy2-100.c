/******************************************************************************
 * @file xy2-100
 * @brief driver for the xy2-100 protocol
 * @author Luos
 * @version 0.0.0
 *
 *
 * This driver allow to drive a xy2-100 protocol (meaning 2bytes X and Y data at 100KHz transmission)
 * This driver use a combination of timer + DMA + GPIO port to generate the signal in hardware without MCU intervention.
 * This way you can make it run on almost any MCU with these peripherals and still have a very precise signal and plenty of CPU time for other tasks.
 *
 * The timer is used to generate the clock signal at the right frequency. Each timer overflow generate a trigger to the DMA. At each trigger the DMA send the next byte to the GPIO port.
 * DMA generate an interrupt at half transfer and at transfer complete. This allow to manage the older half of buffer while the DMA is sending the new one. The DMA is configured to ring on the buffer to have a continuous flux on information.
 * You can increase the buffer size to reduce the number of IRQ and increase the CPU time available for other tasks.
 *
 * The input of this driver is a sample ring buffer you can feed with data providing a given sampling frequency. This driver will consume the data at the right frequency and linearize the trajectory between the samples.
 * You can start pause or stop the trajectory execution at any time.
 ******************************************************************************/

#include "xy2-100.h"
#include "galvo_config.h"

#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_system.h"
#include "gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    uint32_t x;
    int32_t incr_x;
    uint32_t target_x;
    uint32_t y;
    int32_t incr_y;
    uint32_t target_y;
    int32_t point_nbr_to_target;
} xy2_interpolation_t;

void Xy_TransferCompleteCallback(DMA_HandleTypeDef *hdma);
void Xy_HalfTransferCallback(DMA_HandleTypeDef *hdma);
inline bool Xy_ComputeNextPoints(uint8_t *data);
inline bool Xy_GetNextTrajSample(void);
inline uint8_t Xy_Parity(uint16_t v);
inline uint32_t Xy_BuildMsg(uint16_t data);

/*******************************************************************************
 * Variables
 ******************************************************************************/

DMA_HandleTypeDef hdma_tim_up;
TIM_HandleTypeDef htim;

control_t traj_control = {.unmap = 0};
streaming_channel_t *streaming_channel;
xy2_interpolation_t xy2_interpolation;
uint8_t dma_buff[GALVO_BUFFER_SIZE] = {0};
volatile time_luos_t sample_period;
buffer_mode_t xy_buffer_mode = SINGLE;
/*******************************************************************************
 * Function
 ******************************************************************************/

// Init the pins timer and DMA peripherals to drive the xy2-100 protocol
void Xy_Init(void)
{
    // Init pins as output GPIO
    GALVO_PIN_CLOCK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin              = GALVO_CLOCK_PIN | GALVO_SYNC_PIN | GALVO_X_PIN | GALVO_Y_PIN | GALVO_ENABLE_PIN;
    GPIO_InitStruct.Mode             = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull             = GPIO_NOPULL;
    GPIO_InitStruct.Speed            = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GALVO_PORT, &GPIO_InitStruct);

    // Init timer
    GALVO_TIMER_CLOCK_ENABLE();
    uint32_t timer_count                  = (MCUFREQ / (GALVO_BAUDRATE * 2));
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim.Instance               = GALVO_TIMER;
    htim.Init.Prescaler         = 0;
    htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim.Init.Period            = timer_count - 1;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim);
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig);

    // Init DMA
    GALVO_DMA_CLOCK_ENABLE();
    hdma_tim_up.Instance                 = GALVO_DMA_CHANNEL;
    hdma_tim_up.Init.Request             = GALVO_DMA_REQUEST;
    hdma_tim_up.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tim_up.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tim_up.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tim_up.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tim_up.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tim_up.Init.Mode                = DMA_CIRCULAR;
    hdma_tim_up.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_tim_up);

    // Enable timer DMA request enable
    __HAL_TIM_ENABLE_DMA(&htim, TIM_DMA_UPDATE);

    // Set DMA half transfert callback
    HAL_DMA_RegisterCallback(&hdma_tim_up, HAL_DMA_XFER_HALFCPLT_CB_ID, Xy_HalfTransferCallback);
    // Set DMA transfert complete callback
    HAL_DMA_RegisterCallback(&hdma_tim_up, HAL_DMA_XFER_CPLT_CB_ID, Xy_TransferCompleteCallback);

    // Init DMA interrupt
    NVIC_SetPriority(GALVO_DMA_IRQ, 2);
    NVIC_EnableIRQ(GALVO_DMA_IRQ);

    // Start timer
    HAL_TIM_Base_Start(&htim);

    // Consider the trajectory as stopped
    traj_control.flux                     = STOP;
    xy2_interpolation.point_nbr_to_target = 0;
}

// This callback is raised when the DMA transfert is complete, the last half of the buffer have to be updated with the next points
void Xy_TransferCompleteCallback(DMA_HandleTypeDef *hdma)
{
    static bool traj_end_complete = false;
    // Check if the trajectory is finished
    if (traj_end_complete == true)
    {
        // We just played the end of the trajectory, stop the DMA
        // Stop DMA
        HAL_DMA_Abort_IT(&hdma_tim_up);
        // Consider the trajectory as stopped
        traj_control.flux = STOP;
        traj_end_complete = false;
        return;
    }
    else
    {
        // Compute the next points and fill the second half of the buffer
        traj_end_complete = Xy_ComputeNextPoints(dma_buff + (GALVO_BUFFER_SIZE / 2));
    }
}

// This callback is raised when the DMA transfert is half complete, the first half of the buffer have to be updated with the next points
void Xy_HalfTransferCallback(DMA_HandleTypeDef *hdma)
{
    static bool traj_end_half = false;
    // Check if the trajectory is finished
    if (traj_end_half == true)
    {
        // We just played the end of the trajectory, stop the DMA
        // Stop DMA
        HAL_DMA_Abort_IT(&hdma_tim_up);
        // Consider the trajectory as stopped
        traj_control.flux = STOP;
        traj_end_half     = false;
        return;
    }
    else
    {
        // Compute the next points and fill the second half of the buffer
        traj_end_half = Xy_ComputeNextPoints(dma_buff);
    }
}

// This is the replacement of the DMA IRQ handler provided by ST. If you have your ST code doing this job you can remove this function.
void DMA1_Channel3_IRQHandler()
{
    // You can unable the led and check the pin with an oscilloscope to see the occupation ratio of the galvo on the MCU
    // turn the led on
    // HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    // Manage the IRQ
    HAL_DMA_IRQHandler(&hdma_tim_up);
    // turn the led off
    // HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

// Start the trajectory with a given buffer at a given sampling frequency
void Xy_Start(streaming_channel_t *stream, time_luos_t period)
{
    // First check if we are already running
    if (traj_control.flux == PLAY)
    {
        // We are already running, just continue
        return;
    }
    streaming_channel = stream;
    sample_period     = period;
    // Compute the first points and fill half of the buffer
    Xy_ComputeNextPoints(dma_buff);
    // Compute the second points and fill the other half of the buffer
    Xy_ComputeNextPoints(dma_buff + (GALVO_BUFFER_SIZE / 2));
    // Start DMA
    HAL_DMA_Start_IT(&hdma_tim_up, (uint32_t)dma_buff, (uint32_t)&GALVO_PORT->ODR, GALVO_BUFFER_SIZE);

    // Consider the trajectory as running
    traj_control.flux = PLAY;
}

// Stop the trajectory
void Xy_Stop(void)
{
    // Stop DMA
    HAL_DMA_Abort_IT(&hdma_tim_up);
    // Disable the galvo
    HAL_GPIO_WritePin(GALVO_PORT, GALVO_ENABLE_PIN, GPIO_PIN_RESET);
    // Consider the trajectory as stopped
    traj_control.flux = STOP;
}

// Compute the next points and fill half of the buffer
// this function return true if the buffer is filled with the last point.
bool Xy_ComputeNextPoints(uint8_t *data)
{
    // Compute interpolation of points using the last point and the target point
    uint32_t clk_tick_nbr    = 0;
    const uint32_t msg_sync  = 0x0FFFFE;
    const uint32_t tick_size = (GALVO_BUFFER_SIZE / 2);
    volatile uint32_t msg_X;
    volatile uint32_t msg_Y;
    if (xy2_interpolation.point_nbr_to_target == -1)
    {
        // We already finished the trajectory fill the buffer with zeros
        memset((void *)data, 0, tick_size);
        xy2_interpolation.point_nbr_to_target = 0;
        // Those are not the last point, this is just void data, return false.
        return false;
    }

    while (clk_tick_nbr < tick_size)
    {
        if (xy2_interpolation.point_nbr_to_target == 0)
        {
            // We need to get a new target point
            if (Xy_GetNextTrajSample() == false)
            {
                // We don't have any new sample to compute, fill the buffer with the last point first time will be enabled, the other ones will not.
                // Feel the 40 bytes of the buffer containing the 20 bits of the message. This will allo to stop the DMA with a good value.
                // We have to divide the value by 2^16 to get the real value because we use integer instead of float in the xy2_interpolation_t struct to optimize the computation.
                msg_X = Xy_BuildMsg((uint16_t)(xy2_interpolation.x >> 16));
                msg_Y = Xy_BuildMsg((uint16_t)(xy2_interpolation.y >> 16));
                for (int i = 0; i < 20; i++)
                {
                    uint8_t sync_bit             = (msg_sync >> (19 - i)) & 1;
                    uint8_t x_bit                = (msg_X >> (19 - i)) & 1;
                    uint8_t y_bit                = (msg_Y >> (19 - i)) & 1;
                    data[clk_tick_nbr + (i * 2)] = GALVO_CLOCK_PIN | (sync_bit * GALVO_SYNC_PIN) | (x_bit * GALVO_X_PIN) | (y_bit * GALVO_Y_PIN) | GALVO_ENABLE_PIN;
                    // Reverse the GALVO_CLOCK_PIN
                    data[clk_tick_nbr + (i * 2) + 1] = data[clk_tick_nbr + (i * 2)] & ~GALVO_CLOCK_PIN;
                }
                clk_tick_nbr += 40;
                memset((void *)&data[clk_tick_nbr], 0, tick_size - clk_tick_nbr);
                xy2_interpolation.point_nbr_to_target--;
                // This part of the buffer contain the last point, return true.
                return true;
            }
        }
        // Compute the next point
        xy2_interpolation.x = xy2_interpolation.x + xy2_interpolation.incr_x;
        xy2_interpolation.y = xy2_interpolation.y + xy2_interpolation.incr_y;
        // Decrease the number of point to reach the target
        xy2_interpolation.point_nbr_to_target--;
        // Create the messages for x and y
        // We have to divide the value by 2^16 to get the real value because we use integer instead of float in the xy2_interpolation_t struct to optimize the computation.
        msg_X = Xy_BuildMsg((uint16_t)(xy2_interpolation.x >> 16));
        msg_Y = Xy_BuildMsg((uint16_t)(xy2_interpolation.y >> 16));
        // Feel the 40 bytes of the buffer containing the 20 bits of the message
        for (int i = 0; i < 20; i++)
        {
            uint8_t sync_bit             = (msg_sync >> (19 - i)) & 1;
            uint8_t x_bit                = (msg_X >> (19 - i)) & 1;
            uint8_t y_bit                = (msg_Y >> (19 - i)) & 1;
            data[clk_tick_nbr + (i * 2)] = GALVO_CLOCK_PIN | (sync_bit * GALVO_SYNC_PIN) | (x_bit * GALVO_X_PIN) | (y_bit * GALVO_Y_PIN) | GALVO_ENABLE_PIN;
            // Reverse the GALVO_CLOCK_PIN
            data[clk_tick_nbr + (i * 2) + 1] = data[clk_tick_nbr + (i * 2)] & ~GALVO_CLOCK_PIN;
        }
        clk_tick_nbr += 40;
    }
    // This part of the buffer contain normal points, return false.
    return false;
}

// Compute the parity of a 16bits value
uint8_t Xy_Parity(uint16_t v)
{
    uint8_t t = (uint8_t)v ^ v >> 8;
    t ^= t >> 4;
    t ^= t >> 2;
    t ^= t >> 1;
    return (t & 1);
}

// Build a xy2-100 message for one axis
uint32_t Xy_BuildMsg(uint16_t data)
{
    return (1 << 17) | (data << 1) | Xy_Parity(data);
}

// Get the next sample from the ring buffer and compute dome values to optimize the trajectory interpolation
bool Xy_GetNextTrajSample(void)
{
    // Move the target point as current position
    xy2_interpolation.x = xy2_interpolation.target_x;
    xy2_interpolation.y = xy2_interpolation.target_y;
    // Get the next sample from the ring buffer
    uint16_t sample[2];
    if (Streaming_GetAvailableSampleNB(streaming_channel) == 0)
    {
        // We don't have any new sample to compute
        switch (xy_buffer_mode)
        {
            case SINGLE:
                // We are in single mode, we have to loop on the ring buffer
                // Put the read pointer at the begining of the buffer
                streaming_channel->sample_ptr = streaming_channel->ring_buffer;
                return false;
                break;
            case CONTINUOUS:
                // We are in continuous mode, we have to loop on the ring buffer
                // Put the read pointer at the begining of the buffer
                streaming_channel->sample_ptr = streaming_channel->ring_buffer;
                // Get the first sample
                if (Streaming_GetAvailableSampleNB(streaming_channel) == 0)
                {
                    // We don't have any new sample to compute
                    return false;
                }
                break;
            case STREAM:
                // We are in stream mode, we have to wait for new data
                return false;
                break;
            default:
                LUOS_ASSERT(0);
                break;
        }
    }
    Streaming_GetSample(streaming_channel, (void *)sample, 1);
    // Convert the sample into a position
    // To optimize the computation we use integer instead of float. We have to multiply the values by 2^16 to keep as much precision as possible.
    xy2_interpolation.target_x = sample[0] << 16;
    xy2_interpolation.target_y = sample[1] << 16;

    // Compute the number of point to reach the target
    xy2_interpolation.point_nbr_to_target = ((GALVO_BAUDRATE / 20) / (1 / TimeOD_TimeTo_s(sample_period)));

    // Compute the increment to interpolate
    xy2_interpolation.incr_x = ((int32_t)(xy2_interpolation.target_x - xy2_interpolation.x) / xy2_interpolation.point_nbr_to_target);
    xy2_interpolation.incr_y = ((int32_t)(xy2_interpolation.target_y - xy2_interpolation.y) / xy2_interpolation.point_nbr_to_target);
    return true;
}

// Set the galvo in single mode or loop mode
void Xy_BufferMode(buffer_mode_t mode)
{
    xy_buffer_mode = mode;
}

void Xy_SetPeriod(time_luos_t period)
{
    sample_period = period;
}
