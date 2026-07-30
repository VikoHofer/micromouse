/*************************************************************
Project		: Micromouse V1.0
File		: pid_test.h
Description	: PID Test, distance and angle control
Date		: 07.10.2024
*************************************************************/

#ifndef PID_TEST
#define PID_TEST

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>


void resetPID();

void updatePID();

void setPIDGoalD(int16_t distance);

void setPIDGoalA(int16_t angle);

bool PIDdone(void);


#endif