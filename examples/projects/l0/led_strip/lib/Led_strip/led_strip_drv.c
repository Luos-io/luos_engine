/******************************************************************************
 * @file led strip driver
 * @brief driver example a simple led strip
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "led_strip_drv.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define OVERHEAD         (9 * 24) // Number of data to add to create a reset between frames
#define DECOMP_BUFF_SIZE (MAX_LED_NUMBER * 24 + OVERHEAD)
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile char buf[DECOMP_BUFF_SIZE] = {0};
/*******************************************************************************
 * Function
 ******************************************************************************/
static void convert_color(color_t color, int led_nb);

/******************************************************************************
 * @brief driver init must be call in service init
 * @param None
 * @return None
 ******************************************************************************/
void LedStripDrv_Init(void)
{
    TIM2->CCR1 = 0;
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_1);
    // initialize buffer
    memset((void *)buf, 0, DECOMP_BUFF_SIZE);
    // start DMA
    HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t *)buf, DECOMP_BUFF_SIZE);
}

/******************************************************************************
 * @brief write in buffer tranmitted through dma
 * @param matrix of colors
 * @return None
 ******************************************************************************/
void LedStripDrv_Write(color_t *matrix)
{
    // Convert matrix into stream data
    for (int i = 0; i < MAX_LED_NUMBER; i++)
    {
        convert_color(matrix[i], i);
    }
}
/******************************************************************************
 * @brief convert each rgb value to pixel values
 * @param color_t rgb value, number of led
 * @return None
 ******************************************************************************/
static void convert_color(color_t color, int led_nb)
{ // It could be GRB
    char remap[3] = {color.g, color.r, color.b};
    for (int y = 0; y < 3; y++)
    {
        for (int i = 0; i < 8; i++)
        {
            if (remap[y] & (1 << (7 - i)))
            {
                buf[(led_nb * 24) + ((y * 8) + i)] = 38;
            }
            else
            {
                buf[(led_nb * 24) + ((y * 8) + i)] = 19;
            }
        }
    }
}