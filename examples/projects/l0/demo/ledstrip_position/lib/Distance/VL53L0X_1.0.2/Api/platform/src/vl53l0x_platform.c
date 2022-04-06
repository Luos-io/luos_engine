#include "vl53l0x_platform.h"
#include "vl53l0x_api.h"
#include "vl53l0x_drv.h"

#include "stm32f0xx_hal.h"

int VL53L0X_i2c_init(void)
{
    return 0;
}

int32_t VL53L0X_i2c_close(void)
{
    return VL53L0X_ERROR_NOT_IMPLEMENTED;
}

static int i2c_write(int addr, uint8_t cmd, uint8_t *data, uint8_t len)
{
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(&VL53L0_I2CHandle, (uint16_t)addr, cmd, 1, data, len, 10);
    if (ret == HAL_OK)
        return VL53L0X_ERROR_NONE;
    else
        return VL53L0X_ERROR_CONTROL_INTERFACE;
}

static int i2c_read(int addr, uint8_t cmd, uint8_t *data, uint8_t len)
{
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(&VL53L0_I2CHandle, (uint16_t)addr, cmd, 1, data, len, 10);
    if (ret == HAL_OK)
        return VL53L0X_ERROR_NONE;
    else
        return VL53L0X_ERROR_CONTROL_INTERFACE;
}

VL53L0X_Error VL53L0X_LockSequenceAccess(VL53L0X_DEV Dev)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    return Status;
}

VL53L0X_Error VL53L0X_UnlockSequenceAccess(VL53L0X_DEV Dev)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    return Status;
}

VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count)
{
    return i2c_write(Dev->addr, index, pdata, count);
}

VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count)
{
    return i2c_read(Dev->addr, index, pdata, count);
}

VL53L0X_Error VL53L0X_WrByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data)
{
    return i2c_write(Dev->addr, index, &data, 1);
}

VL53L0X_Error VL53L0X_WrWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data)
{
    uint8_t buf[4];
    buf[1] = data >> 0 & 0xFF;
    buf[0] = data >> 8 & 0xFF;
    return i2c_write(Dev->addr, index, buf, 2);
}

VL53L0X_Error VL53L0X_WrDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t data)
{
    uint8_t buf[4];
    buf[3] = data >> 0 & 0xFF;
    buf[2] = data >> 8 & 0xFF;
    buf[1] = data >> 16 & 0xFF;
    buf[0] = data >> 24 & 0xFF;
    return i2c_write(Dev->addr, index, buf, 4);
}

VL53L0X_Error VL53L0X_UpdateByte(VL53L0X_DEV Dev, uint8_t index, uint8_t AndData, uint8_t OrData)
{

    int32_t status_int;
    uint8_t data;

    status_int = i2c_read(Dev->addr, index, &data, 1);

    if (status_int != 0)
    {
        return status_int;
    }

    data = (data & AndData) | OrData;
    return i2c_write(Dev->addr, index, &data, 1);
}

VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *data)
{
    uint8_t tmp = 0;
    int ret     = i2c_read(Dev->addr, index, &tmp, 1);
    *data       = tmp;
    // printf("%u\n", tmp);
    return ret;
}

VL53L0X_Error VL53L0X_RdWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *data)
{
    uint8_t buf[2];
    int ret      = i2c_read(Dev->addr, index, buf, 2);
    uint16_t tmp = 0;
    tmp |= buf[1] << 0;
    tmp |= buf[0] << 8;
    // printf("%u\n", tmp);
    *data = tmp;
    return ret;
}

VL53L0X_Error VL53L0X_RdDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *data)
{
    uint8_t buf[4];
    int ret      = i2c_read(Dev->addr, index, buf, 4);
    uint32_t tmp = 0;
    tmp |= buf[3] << 0;
    tmp |= buf[2] << 8;
    tmp |= buf[1] << 16;
    tmp |= buf[0] << 24;
    *data = tmp;
    // printf("%zu\n", tmp);
    return ret;
}

VL53L0X_Error VL53L0X_PollingDelay(VL53L0X_DEV Dev)
{
    return VL53L0X_ERROR_NONE;
}
