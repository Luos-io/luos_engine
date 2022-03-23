/******************************************************************************
 * @file servo
 * @brief This is a simple driver for a servo motor. It will work as is
 * but if you want to see it inside of a project, I am using it to create a 
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/L432KC-Luos-Biometric-Security-System
 * @author MarieBidouille
 * @version 0.0.0
 ******************************************************************************/
#ifndef SERVO_H
#define SERVO_H

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/
void Servo_Init(void);
void Servo_Loop(void);

#endif /* SERVO_H */
