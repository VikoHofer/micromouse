/*************************************************************
Project		: Micromouse V1.0
File		: MotorDriver.h
Description	: Create PWM Signals to controll Motor
Date		: 09.09.2024
*************************************************************/

#ifndef MOTORDRIVER_H
#define MOTORDRIVER_H

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"

#include <stdint.h>

/************************************
* using TIM 2 to create PWM Signals
* TIM4 (AF2)
* CH3 -> PB8 (Motor Right)
* CH4 -> PB9 (Motor Left)
************************************
* Control PINS (Forward / Backward)
* Motor Left: AIN1 -> PA10
* Motor Left: AIN2 -> PB2
* Motor Right: BIN1 -> PB5
* Motor Right: BIN2 -> PB4
************************************
* CW: 	IN1 = HIGH, IN2 = LOW 
* CCW:	IN1 = LOW,  IN2 = HIGH
* Stop: IN1 = LOW,  IN2 = LOW
*************************************/




// initialize TIMER3 as PWM generator
void Motor_Init(void);

// direction controll
void Motor_Left_Forwards(void);
void Motor_Left_Backwards (void);
void Motor_Left_Brake (void);

void Motor_Right_Forwards(void);
void Motor_Right_Backwards (void);
void Motor_Right_Brake (void);

// set dutycycle of PWM signal (0 - 255)
void Motor_Left_SetSpeed(uint16_t speed);
void Motor_Right_SetSpeed(uint16_t speed);

// set pwm (forward and backwards ctrl (-255 - 255))
void Motor_Left_SetPWM(double pwm);
void Motor_Right_SetPWM(double pwm);


#endif