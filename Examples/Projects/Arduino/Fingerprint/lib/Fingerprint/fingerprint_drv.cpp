/******************************************************************************
 * @file fingerprint_drv
 * @brief This is a driver example for a fingerprint sensor. It will work as is
 * but if you want to see it inside of a project, I am using it to create a 
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/Arduino-Luos-Biometric-Security-System
 * @author MarieBidouille
 * @version 0.0.0
 ******************************************************************************/
#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include "wiring_private.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "fingerprint_drv.h"

#ifdef __cplusplus
}
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
int n_try = 0;

// Instantiate the Serial2 class
Uart FINGERPRINT_COM(&FINGERPRINT_SERCOM, PIN_FINGERPRINT_COM_RX, PIN_FINGERPRINT_COM_TX, PAD_FINGERPRINT_COM_RX, PAD_FINGERPRINT_COM_TX);

Adafruit_Fingerprint sensor = Adafruit_Fingerprint(&FINGERPRINT_COM);

/*******************************************************************************
 * Functions
 ******************************************************************************/
uint16_t FingerprintDrv_RefreshID(void);

/******************************************************************************
 * @brief hardware init must be call in service init
 * @param None
 * @return None
 ******************************************************************************/
void FingerprintDrv_Init(void)
{
    sensor.begin(FINGERPRINT_BAUDRATE);

    pinPeripheral(PIN_FINGERPRINT_COM_RX, PIO_SERCOM_ALT);
    pinPeripheral(PIN_FINGERPRINT_COM_TX, PIO_SERCOM_ALT);
}

void SERCOM2_Handler() // Interrupt handler for SERCOM1
{
    Serial2.IrqHandler();
}

uint16_t FingerprintDrv_RefreshID(void)
{
    sensor.getTemplateCount();
    return (sensor.templateCount < MAX_TEMPLATE) ? sensor.templateCount++ : MAX_TEMPLATE;
}

/******************************************************************************
 * @brief you must present a finger that is already saved to add a finger if the database isn't empty
 * @param None
 * @return Template id if success
 ******************************************************************************/
uint8_t FingerprintDrv_Enroll(void)
{
    n_try = 0;

    while (sensor.getImage() != FINGERPRINT_OK)
    {
        delay(500);
        if (n_try++ >= MAX_TRY)
            return false;
    }

    if (sensor.image2Tz(1) != FINGERPRINT_OK)
        return false;

    delay(1000);

    while (sensor.getImage() != FINGERPRINT_NOFINGER)
        ;

    n_try = 0;

    while (sensor.getImage() != FINGERPRINT_OK)
    {
        delay(500);
        if (n_try++ >= MAX_TRY)
            return false;
    }

    if (sensor.image2Tz(2) != FINGERPRINT_OK)
        return false;

    if (sensor.createModel() != FINGERPRINT_OK)
        return false;

    if (sensor.storeModel(FingerprintDrv_RefreshID() + 1) != FINGERPRINT_OK)
        return false;

    return true;
}

/******************************************************************************
 * @brief delete the database
 * @param None
 * @return True if 
 ******************************************************************************/
uint8_t FingerprintDrv_DeleteAll(void)
{
    if (FingerprintDrv_CheckAuth())
    {
        if (sensor.emptyDatabase() == FINGERPRINT_OK)
            return true;
    }

    return false;
}
/******************************************************************************
 * @brief check if there's a matching template in the database
 * @param None
 * @return True if there's a match
 ******************************************************************************/
uint8_t FingerprintDrv_CheckAuth(void)
{
    sensor.getTemplateCount();
    if (sensor.templateCount == 0)
        return true;

    n_try = 0;

    while (sensor.getImage() != FINGERPRINT_OK)
    {
        delay(500);
        if (n_try++ >= MAX_TRY)
            return false;
    }

    if (sensor.image2Tz() != FINGERPRINT_OK)
        return false;

    if (sensor.fingerSearch() != FINGERPRINT_OK)
        return false;

    return true;
}