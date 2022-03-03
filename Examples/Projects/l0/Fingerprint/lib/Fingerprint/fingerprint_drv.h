/******************************************************************************
 * @file fingerprint_drv
 * @brief This is a driver example for a fingerprint sensor. It will work as is
 * but if you want to see it inside of a project, I am using it to create a 
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/STM32F0-Luos-Biometric-Security-System
 * @author mariebidouille
 * @version 0.0.0
 ******************************************************************************/
#ifndef FINGERPRINT_DRV_H
#define FINGERPRINT_DRV_H

#include "fingerprint_com.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

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

#endif /* FINGERPRINT_DRV_H */