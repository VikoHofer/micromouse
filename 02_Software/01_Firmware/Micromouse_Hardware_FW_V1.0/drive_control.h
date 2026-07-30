/*************************************************************
Project		: Micromouse V1.0
File		: drive_control.h
Description	: PID-Control for micromouse
Date		: 09.09.2024
*************************************************************/

#ifndef DRIVE_CTRL_INCLUDED
#define DRIVE_CTRL_INCLUDED

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include "pid.h"


// calculate kp and kd 
void Calculate_Constants(void);



/*******************************************************************************
* Set PWM Value and sends Encoder Data to PY Debugger
*******************************************************************************/
void Get_ID_Motor(uint8_t const PWM);

/*******************************************************************************
* Make a step with PD-Controler and send data to PY Debugger
*******************************************************************************/
void PD_Step_Response(int16_t _speedCmd);

/*******************************************************************************
* Make a move test with PD-Controller and send data to PY Debugger
*******************************************************************************/
void PD_Move(void);

// Control Speed of mouse
void Speed_Ctrl(PIDController* pid_L, PIDController* pid_R);


#endif