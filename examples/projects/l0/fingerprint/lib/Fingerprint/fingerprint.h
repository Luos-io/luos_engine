/******************************************************************************
 * @file fingerprint
 * @brief This is a driver example for a fingerprint sensor. It will work as is
 * but if you want to see it inside of a project, I am using it to create a
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/STM32F0-Luos-Biometric-Security-System
 * @author mariebidouille
 * @version 0.0.0
 ******************************************************************************/
#ifndef FINGERPRINT_H_
#define FINGERPRINT_H_

#include "luos_engine.h"
#include "robus.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/
void Fingerprint_Init(void);
void Fingerprint_Loop(void);

#endif /* FINGERPRINT_H_ */
