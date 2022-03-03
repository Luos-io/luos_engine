/******************************************************************************
 * @file fingerprint_drv
 * @brief This is a driver example for a fingerprint sensor. It will work as is
 * but if you want to see it inside of a project, I am using it to create a 
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/STM32F0-Luos-Biometric-Security-System
 * @author mariebidouille
 * @version 0.0.0
 ******************************************************************************/
#include "fingerprint_drv.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
int n_try = 0;

/*******************************************************************************
 * Functions
 ******************************************************************************/
void FingerprintDrv_GPIOInit(void);

/******************************************************************************
 * @brief Init must be call in the service init
 * @param None
 * @return None
 ******************************************************************************/
void FingerprintDrv_Init(void)
{
    HAL_Delay(1000);
    FingerprintDrv_GPIOInit();
    FingerprintCom_Init(FINGERPRINT_BAUDRATE);
}

/******************************************************************************
 * @brief Init must be call in drv init
 * @param None
 * @return None
 ******************************************************************************/
void FingerprintDrv_GPIOInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    //Activate Clock for PIN choosen in luosHAL
    FINGERPRINT_CLOCK_ENABLE();

    //Configure GPIO pin : TxPin
    GPIO_InitStruct.Pin       = FINGERPRINT_COM_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = FINGERPRINT_COM_TX_AF;
    HAL_GPIO_Init(FINGERPRINT_COM_TX_PORT, &GPIO_InitStruct);

    //Configure GPIO pin : RxPin
    GPIO_InitStruct.Pin       = FINGERPRINT_COM_RX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = FINGERPRINT_COM_RX_AF;
    HAL_GPIO_Init(FINGERPRINT_COM_RX_PORT, &GPIO_InitStruct);
}

/******************************************************************************
 * @brief Enroll a new fingerprint in the sensor
 * @param None
 * @return true if successful false if not 
 ******************************************************************************/
uint8_t FingerprintDrv_Enroll(void)
{
    n_try = 0;

    while (Fingerprint_GetImage() != FINGERPRINT_OK)
    {
        HAL_Delay(50);
        if (n_try++ >= MAX_TRY)
            return false;
    }

    if (Fingerprint_Image2Tz(1) != FINGERPRINT_OK)
        return false;

    HAL_Delay(1000);

    while (Fingerprint_GetImage() != FINGERPRINT_NOFINGER)
        ;

    n_try = 0;

    while (Fingerprint_GetImage() != FINGERPRINT_OK)
    {
        HAL_Delay(50);
        if (n_try++ >= MAX_TRY)
            return false;
    }

    if (Fingerprint_Image2Tz(2) != FINGERPRINT_OK)
        return false;

    if (Fingerprint_CreateModel() != FINGERPRINT_OK)
        return false;

    if (Fingerprint_StoreModel(Fingerprint_GetTemplateCount() + 1) != FINGERPRINT_OK)
        return false;

    return true;
}

/******************************************************************************
 * @brief Delete all the fingerprints in the sensor
 * @param None
 * @return true if successful false if not 
 ******************************************************************************/
uint8_t FingerprintDrv_DeleteAll(void)
{
    if (FingerprintDrv_CheckAuth())
    {
        if (Fingerprint_EmptyDatabase() == FINGERPRINT_OK)
            return true;
    }

    return false;
}

/******************************************************************************
 * @brief Check if a fingerprint is in the database 
 * @param None
 * @return true if successful false if not 
 ******************************************************************************/
uint8_t FingerprintDrv_CheckAuth(void)
{
    if (Fingerprint_GetTemplateCount() == 0)
        return true;

    n_try = 0;

    while (Fingerprint_GetImage() != FINGERPRINT_OK)
    {
        HAL_Delay(50);
        if (n_try++ >= MAX_TRY)
            return false;
    }

    if (Fingerprint_Image2Tz(1) != FINGERPRINT_OK)
        return false;

    if (Fingerprint_SearchFinger() != FINGERPRINT_OK)
        return false;

    return true;
}
