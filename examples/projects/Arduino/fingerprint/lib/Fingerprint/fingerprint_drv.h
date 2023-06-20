/******************************************************************************
 * @file fingerprint_drv
 * @brief This is a driver example for a fingerprint sensor. It will work as is
 * but if you want to see it inside of a project, I am using it to create a
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/Arduino-Luos-Biometric-Security-System
 * @author MarieBidouille
 * @version 0.0.0
 ******************************************************************************/
#ifndef FINGERPRINT_DRV_H_
#define FINGERPRINT_DRV_H_

#include "luos_engine.h"
#include "robus.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
// Serial2 pin and pad definitions (in Arduino files Variant.h & Variant.cpp)
#define PIN_FINGERPRINT_COM_RX 3 // Pin description number for PIO_SERCOM on D12
#define PIN_FINGERPRINT_COM_TX 4 // Pin description number for PIO_SERCOM on D10

#define PAD_FINGERPRINT_COM_TX (UART_TX_PAD_0)   // SERCOM pad 0
#define PAD_FINGERPRINT_COM_RX (SERCOM_RX_PAD_1) // SERCOM pad 1

#define FINGERPRINT_BAUDRATE 57600

#define FINGERPRINT_SERCOM sercom2
#define FINGERPRINT_COM    Serial2

#define MAX_TEMPLATE 127
#define MAX_TRY      500

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/
void FingerprintDrv_Init(void);

uint8_t FingerprintDrv_Enroll(void);
uint8_t FingerprintDrv_CheckAuth(void);
uint8_t FingerprintDrv_DeleteAll(void);

#endif /* FINGERPRINT_DRV_H_ */
