/******************************************************************************
 * @file biometric_security
 * @brief This is an app exemple for a Biometric Security System. It won't work
 * as is so if you want to see it inside of a project, I made multiple version
 * that you can go check on my github :
 * https://github.com/mariebidouille
 * @author MarieBidouille
 * @version 0.0.0
 ******************************************************************************/
#ifndef BIOMETRIC_SECURITY_H
#define BIOMETRIC_SECURITY_H

#include "luos_engine.h"
#include "robus_network.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/
void BiometricSecurity_Init(void);
void BiometricSecurity_Loop(void);

#endif /* BIOMETRIC_SECURITY_H */
