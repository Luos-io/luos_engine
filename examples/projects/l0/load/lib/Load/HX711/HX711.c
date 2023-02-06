/******************************************************************************
 * @file HX711 library
 * @brief Inspired by HX711 library for Arduino
 * https://github.com/bogde/HX711
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <HX711.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PIN_CLK()                     \
    do                                \
    {                                 \
        __HAL_RCC_GPIOA_CLK_ENABLE(); \
    } while (0U)

#define DAT_Pin       GPIO_PIN_0
#define DAT_GPIO_Port GPIOA
#define CLK_Pin       GPIO_PIN_1
#define CLK_GPIO_Port GPIOA

#define MSBFIRST     1
#define LSBFIRST     0
#define DEFAULT_GAIN 128
/*******************************************************************************
 * Variables
 ******************************************************************************/
hx711_t hx711 = {DEFAULT_GAIN, 0, 1};
/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief delayMicroseconds
 * @param timeus
 * @return None
 ******************************************************************************/
void delayMicroseconds(int timeus)
{
    volatile int useless = 0;
    for (int x = 0; x < timeus; x++)
    {
        for (int i = 0; i < 6; i++)
        {
            useless++;
        }
    }
}
/******************************************************************************
 * @brief Make shiftIn() be aware of clockspeed
 * @param bitOrder
 * @return None
 ******************************************************************************/
uint8_t hx711_shiftIn(uint8_t bitOrder)
{
    uint8_t plop = 0;
    uint8_t i;

    for (i = 0; i < 8; ++i)
    {
        HAL_GPIO_WritePin(CLK_GPIO_Port, CLK_Pin, 1);
        delayMicroseconds(1);
        if (bitOrder == LSBFIRST)
            plop |= HAL_GPIO_ReadPin(DAT_GPIO_Port, DAT_Pin) << i;
        else
            plop |= HAL_GPIO_ReadPin(DAT_GPIO_Port, DAT_Pin) << (7 - i);
        HAL_GPIO_WritePin(CLK_GPIO_Port, CLK_Pin, 0);
        delayMicroseconds(1);
    }
    return plop;
}
/******************************************************************************
 * @brief hx711_is_ready
 * @param
 * @return None
 ******************************************************************************/
uint8_t hx711_is_ready(void)
{
    return HAL_GPIO_ReadPin(DAT_GPIO_Port, DAT_Pin) == 0;
}
/******************************************************************************
 * @brief hx711_set_gain
 * @param gain
 * @return None
 ******************************************************************************/
void hx711_set_gain(uint8_t gain)
{
    switch (gain)
    {
        case 128: // channel A, gain factor 128
            hx711.GAIN = 1;
            break;
        case 64: // channel A, gain factor 64
            hx711.GAIN = 3;
            break;
        case 32: // channel B, gain factor 32
            hx711.GAIN = 2;
            break;
    }

    HAL_GPIO_WritePin(CLK_GPIO_Port, CLK_Pin, 0);
    hx711_read();
}
/******************************************************************************
 * @brief hx711_read
 * @param gain
 * @return None
 ******************************************************************************/
long hx711_read()
{

    // Wait for the chip to become ready.
    hx711_wait_ready(0);

    // Define structures for reading data into.
    unsigned long value = 0;
    uint8_t data[3]     = {0};
    uint8_t filler      = 0x00;

    // Protect the read sequence from system interrupts.  If an interrupt occurs during
    // the time the PD_SCK signal is high it will stretch the length of the clock pulse.
    // If the total pulse time exceeds 60 uSec this will cause the HX711 to enter
    // power down mode during the middle of the read sequence.  While the device will
    // wake up when PD_SCK goes low again, the reset starts a new conversion cycle which
    // forces DOUT high until that cycle is completed.
    //
    // The result is that all subsequent bits read by shiftIn() will read back as 1,
    // corrupting the value returned by read().

    // Pulse the clock pin 24 times to read the data.
    data[2] = hx711_shiftIn(MSBFIRST);
    data[1] = hx711_shiftIn(MSBFIRST);
    data[0] = hx711_shiftIn(MSBFIRST);

    // Set the channel and the gain factor for the next reading using the clock pin.
    for (unsigned int i = 0; i < hx711.GAIN; i++)
    {
        __disable_irq();
        HAL_GPIO_WritePin(CLK_GPIO_Port, CLK_Pin, 1);
        delayMicroseconds(1);
        HAL_GPIO_WritePin(CLK_GPIO_Port, CLK_Pin, 0);
        __enable_irq();
        delayMicroseconds(1);
    }

    // End of critical section.

    // Replicate the most significant bit to pad out a 32-bit signed integer
    if (data[2] & 0x80)
    {
        filler = 0xFF;
    }
    else
    {
        filler = 0x00;
    }

    // Construct a 32-bit signed integer
    value = ((unsigned long)filler << 24 | (unsigned long)data[2] << 16 | (unsigned long)data[1] << 8 | (unsigned long)data[0]);

    return (long)value;
}
/******************************************************************************
 * @brief hx711_wait_ready
 * @param delay_ms
 * @return None
 ******************************************************************************/
void hx711_wait_ready(unsigned long delay_ms)
{
    // Wait for the chip to become ready.
    // This is a blocking implementation and will
    // halt the program until a load cell is connected.
    while (!hx711_is_ready())
    {
        HAL_Delay(delay_ms);
    }
}
/******************************************************************************
 * @brief hx711_wait_ready_retry
 * @param delay_ms, retries
 * @return None
 ******************************************************************************/
uint8_t hx711_wait_ready_retry(int retries, unsigned long delay_ms)
{
    // Wait for the chip to become ready by
    // retrying for a specified amount of attempts.
    // https://github.com/bogde/HX711/issues/76
    int count = 0;
    while (count < retries)
    {
        if (hx711_is_ready())
        {
            return 1;
        }
        HAL_Delay(delay_ms);
        count++;
    }
    return 0;
}
/******************************************************************************
 * @brief hx711_wait_ready_timeout
 * @param delay_ms, retries
 * @return None
 ******************************************************************************/
uint8_t hx711_wait_ready_timeout(unsigned long timeout, unsigned long delay_ms)
{
    // Wait for the chip to become ready until timeout.
    unsigned long millisStarted = HAL_GetTick();
    while (HAL_GetTick() - millisStarted < timeout)
    {
        if (hx711_is_ready())
        {
            return 1;
        }
        HAL_Delay(delay_ms);
    }
    return 0;
}
/******************************************************************************
 * @brief hx711_read_average
 * @param times
 * @return None
 ******************************************************************************/
long hx711_read_average(uint8_t times)
{
    long sum = 0;
    for (uint8_t i = 0; i < times; i++)
    {
        sum += hx711_read();
    }
    return sum / times;
}
/******************************************************************************
 * @brief hx711_get_value
 * @param times
 * @return None
 ******************************************************************************/
double hx711_get_value(uint8_t times)
{
    return hx711_read_average(times) - hx711.OFFSET;
}
/******************************************************************************
 * @brief hx711_get_units
 * @param times
 * @return None
 ******************************************************************************/
float hx711_get_units(uint8_t times)
{
    return hx711_get_value(times) / hx711.SCALE;
}
/******************************************************************************
 * @brief hx711_tare
 * @param times
 * @return None
 ******************************************************************************/
void hx711_tare(uint8_t times)
{
    volatile double sum = hx711_read_average(times);
    hx711_set_offset(sum);
}
/******************************************************************************
 * @brief hx711_set_scale
 * @param scale
 * @return None
 ******************************************************************************/
void hx711_set_scale(float scale)
{
    hx711.SCALE = scale;
}
/******************************************************************************
 * @brief hx711_get_scale
 * @param  None
 * @return None
 ******************************************************************************/
float hx711_get_scale()
{
    return hx711.SCALE;
}
/******************************************************************************
 * @brief hx711_get_scale
 * @param  None
 * @return None
 ******************************************************************************/
void hx711_set_offset(long offset)
{
    hx711.OFFSET = offset;
}
/******************************************************************************
 * @brief hx711_get_offset
 * @param  None
 * @return None
 ******************************************************************************/
long hx711_get_offset()
{
    return hx711.OFFSET;
}
/******************************************************************************
 * @brief hx711_power_down
 * @param  None
 * @return None
 ******************************************************************************/
void hx711_power_down()
{
    HAL_GPIO_WritePin(CLK_GPIO_Port, CLK_Pin, 0);
    HAL_GPIO_WritePin(CLK_GPIO_Port, CLK_Pin, 1);
}
/******************************************************************************
 * @brief hx711_power_up
 * @param  None
 * @return None
 ******************************************************************************/
void hx711_power_up()
{
    HAL_GPIO_WritePin(CLK_GPIO_Port, CLK_Pin, 0);
}
/******************************************************************************
 * @brief hx711_hw_init
 * @param gain
 * @return None
 ******************************************************************************/
void hx711_HWInit(void)
{
    // peripherial init
    PIN_CLK();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin              = DAT_Pin;
    GPIO_InitStruct.Mode             = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull             = GPIO_NOPULL;
    HAL_GPIO_Init(DAT_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = CLK_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(CLK_GPIO_Port, &GPIO_InitStruct);
}
/******************************************************************************
 * @brief hx711_init
 * @param gain
 * @return None
 ******************************************************************************/
void hx711_Init(void)
{
    hx711_HWInit();

    hx711_set_gain(hx711.GAIN);
    hx711_set_offset(hx711.OFFSET);
    hx711_set_scale(hx711.SCALE);
}
/******************************************************************************
 * @brief hx711_ReadValue
 * @param gain
 * @return None
 ******************************************************************************/
uint8_t hx711_ReadValue(force_t *load)
{
    uint8_t result = FAILED;
    if (hx711_is_ready())
    {
        *load  = ForceOD_ForceFrom_N(hx711_get_units(1));
        result = SUCCEED;
    }
    return result;
}
