/******************************************************************************
 * @file distance driver
 * @brief driver vl53l0x
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "vl53l0x_drv.h"
#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
VL53L0X_RangingMeasurementData_t ranging_data;
I2C_HandleTypeDef VL53L0_I2CHandle;
VL53L0X_Dev_t dev;
/*******************************************************************************
 * Function
 ******************************************************************************/
void vl53l0x_HW_Init(void);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void vl53l0x_DRVInit(void)
{
    vl53l0x_HW_Init();

    //reset sensor
    HAL_GPIO_WritePin(SHUTDOWN_PORT, SHUTDOWN_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(SHUTDOWN_PORT, SHUTDOWN_PIN, GPIO_PIN_SET);
    HAL_Delay(5);

    dev.addr = 0x52;

    VL53L0X_DataInit(&dev);
    VL53L0X_StaticInit(&dev);

    uint32_t ref_spad_count;
    uint8_t is_aperture_spads;
    VL53L0X_PerformRefSpadManagement(&dev, &ref_spad_count, &is_aperture_spads);

    uint8_t vhv_settings;
    uint8_t phase_cal;
    VL53L0X_PerformRefCalibration(&dev, &vhv_settings, &phase_cal);

    //VL53L0X_PerformOffsetCalibration(&dev,distmm, offset_um);

    VL53L0X_SetDeviceMode(&dev, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);
    VL53L0X_StartMeasurement(&dev);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
uint8_t vl53l0x_DrvRead(linear_position_t *distance)
{
    uint8_t result             = FAILED;
    uint8_t data_ready         = 0;
    static bool mesure_pending = false;

    if (mesure_pending == false)
    {
        VL53L0X_GetMeasurementDataReady(&dev, &data_ready);
        if (data_ready)
        {
            mesure_pending = true;
            VL53L0X_GetRangingMeasurementData(&dev, &ranging_data);
            VL53L0X_ClearInterruptMask(&dev, VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY);
        }
    }
    else
    {
        if (ranging_data.RangeStatus == 0)
        {
            *distance = LinearOD_PositionFrom_mm((float)ranging_data.RangeMilliMeter);
        }
        else
        {
            *distance = -0.001;
        }
        mesure_pending = false;
        result         = SUCCEED;
    }

    return result;
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void vl53l0x_HW_Init(void)
{
    //peripherial init
    PIN_CLK();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin  = INT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(INT_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = SHUTDOWN_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SHUTDOWN_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin       = SCL_PIN;
    GPIO_InitStruct.Alternate = SCL_AF;
    HAL_GPIO_Init(SCL_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = SDA_PIN;
    GPIO_InitStruct.Alternate = SDA_AF;
    HAL_GPIO_Init(SDA_PORT, &GPIO_InitStruct);

    I2C_CLK();

    VL53L0_I2CHandle.Instance              = VL53L0_I2C;
    VL53L0_I2CHandle.Init.Timing           = 0x2010091A;
    VL53L0_I2CHandle.Init.OwnAddress1      = 0;
    VL53L0_I2CHandle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    VL53L0_I2CHandle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    VL53L0_I2CHandle.Init.OwnAddress2      = 0;
    VL53L0_I2CHandle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    VL53L0_I2CHandle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    VL53L0_I2CHandle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
    while (HAL_I2C_Init(&VL53L0_I2CHandle) != HAL_OK)
        ;
    while (HAL_I2CEx_ConfigAnalogFilter(&VL53L0_I2CHandle, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
        ;
    while (HAL_I2CEx_ConfigDigitalFilter(&VL53L0_I2CHandle, 0) != HAL_OK)
        ;
}
