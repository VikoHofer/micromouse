/*************************************************************
Project		: Micromouse V1.0
File		: pid_test.c
Description	: PID Test, distance and angle control
Date		: 07.10.2024
*************************************************************/


#include "pid_test.h"
#include <stdio.h>
#include "encoder.h"
#include "MotorDriver.h"
#include "Print.h"
#include "Delay.h"
#include "Python_Debugger.h"

static double const Motor_right_offset = 0.921;


/* Declaration of global variables used in this file */

/* distance constants, goal, threshold & errors: */
float kPx = /* your proportional constant here, experimentally determined*/0.3; // Beschleunigung 0.5 - 0.2
float kDx = /* your derivative constant here, experimentally determined*/ 1.15; // 0.2 - 0.1
float goalDistance = 0;
int distanceThreshold = 10; /* This variable determines the range (in encoder counts) at which the move is finished. For example if this variable if 5, the mouse will consider the goal reached if the distance error is in the range -5 <= distanceError <= 5 */
float previousDistanceErrors[6] = {0, 0, 0, 0, 0, 0};		/* stores previous error terms to average them for more accurate derivative correction */

/* angle constants, goal, threshold & errors: */
float kPw = /* your proportional constant here, experimentally determined*/0.3;
float kDw = /* your derivative constant here, experimentally determined*/ 0;
float goalAngle = 0;
int angleThreshold = 10; /* This variable determines the range at which the turn is finished. For example if this variable if 5, the mouse will consider the goal reached if the angle error is in the range -5 <= angleError <= 5 */
float previousAngleErrors[6] = {0, 0, 0, 0, 0, 0};			/* stores previous error terms to average them for more accurate derivative correction */

/* This variable stores the minimum pwm at which your mouse moves (experimentally determined). It's needed at times when the distance/angle correction set the motor pwms to a value lower than one at which the mouse can move at. This also depends on battery voltage, but a rough estimate should be fine */
float minimumSpeed = /* your constant here, experimentally determined*/ 110;

static int32_t systickCalls = 0;


void resetPID() {
	/* resetting global variables */
	for (int i = 0; i < 6; ++i) {
		previousDistanceErrors[i] = 0;
		previousAngleErrors[i] = 0;
	}
	goalDistance = 0;
	goalAngle = 0;
	systickCalls = 0;
	
	/* resetting the motors & encoders */
	Encoder_Reset();
	Motor_Right_SetPWM(0);
	Motor_Left_SetPWM(0);
}

void updatePID() {
	PY_Printf("------updatePID begin---------");
		
	// Calculate time
    static uint32_t prev_time_ms = 0;
    uint32_t current_time_ms = HAL_GetTick();
    uint32_t elapsedTime = current_time_ms - prev_time_ms;
    double dt_sec = (double)elapsedTime; // 1000.0;
	
	/* Updating the distance errors & calculating the distance correction */
	
	int distanceError = goalDistance - (Get_Encoder_Position_mm_Right() + Get_Encoder_Position_mm_Left()) / 2;
	PY_Printf("distanceError: %d", distanceError);
	/* finds the average of the past 5 error values & updates errors array: */
	int avgDistanceErrors = 0;
	for (int i = 0; i < 5; ++i) {
		previousDistanceErrors[i] = previousDistanceErrors[i + 1];
		avgDistanceErrors += previousDistanceErrors[i];
	}
	previousDistanceErrors[5] = distanceError;
	avgDistanceErrors = (avgDistanceErrors + distanceError) / 5;
	int distanceCorrection = kPx * distanceError + (kDx * (distanceError - avgDistanceErrors)/dt_sec);
	
	
	/* Updating the angle errors & calculating the angle correction */
	
	int angleError = goalAngle - (Get_Encoder_Position_mm_Left() - Get_Encoder_Position_mm_Right());
	/* finds the average of the past 5 error values & updates errors array: */
	int avgAngleErrors = 0;
	for (int i = 0; i < 5; ++i) {
		previousAngleErrors[i] = previousAngleErrors[i + 1];
		avgAngleErrors += previousAngleErrors[i];
	}
	previousAngleErrors[5] = angleError;
	avgAngleErrors = (avgAngleErrors + angleError) / 5;
	int angleCorrection = kPw * angleError + (kDw * (angleError - avgAngleErrors)/dt_sec);
	
	
	/* Update motor pwm values while accounting for minimum speed */
	
	int rightPWM = distanceCorrection - angleCorrection;
	int leftPWM = distanceCorrection + angleCorrection;
	
	if (rightPWM > -minimumSpeed && rightPWM < minimumSpeed) {
		if (rightPWM > 0)
			rightPWM = minimumSpeed;
		else
			rightPWM = -minimumSpeed;
	}
	
	if (leftPWM > -minimumSpeed && leftPWM < minimumSpeed) {
		if (leftPWM > 0)
			leftPWM = minimumSpeed;
		else
			leftPWM = -minimumSpeed;
	}
	
	Motor_Right_SetPWM(rightPWM*Motor_right_offset);
	Motor_Left_SetPWM(leftPWM);
	
	
	/* checks if it the current encoder counts are within the goal threshold, if so, increments the systickCalls variable */
	if ((distanceError >= -distanceThreshold && distanceError <= distanceThreshold) && (angleError >= -angleThreshold && angleError <= angleThreshold))
	{
		PY_Printf("sysTickCall: %d", systickCalls);
		systickCalls++;
	}
	else 
		systickCalls = 0;
	
	// Update previous time
    prev_time_ms = current_time_ms;
	
	PY_Printf("------updatePID end---------\n");
}

void setPIDGoalD(int16_t distance) {
	goalDistance = distance;
}

void setPIDGoalA(int16_t angle) {
	goalAngle = angle;
}

bool PIDdone(void) { 
	PY_Printf("sysTickCall: %d", systickCalls);
	if (systickCalls >= 50){
		PY_Printf("PID DONE!!!");
		return true;
	}
	else 
		return false;
}
