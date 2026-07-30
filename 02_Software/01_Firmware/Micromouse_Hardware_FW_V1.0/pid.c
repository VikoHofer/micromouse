/*************************************************************
Project		: Micromouse V1.2
File		: pid.c
Description	: PID-Controller
Date		: 23.12.2024
*************************************************************/


#include "pid.h"

#define MAX_INTEGRAL 10000

// Initialize PID controller
void PID_Init(PIDController *pid, double kp, double ki, double kd, double setpoint, double integral_start) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->setpoint = setpoint;
    pid->integral = integral_start;
    pid->previous_error = 0.0;
}

// Calculate PID output (with Anti-Windup)
double PID_Compute(PIDController *pid, double current_value, double dt) {
    double error = pid->setpoint - current_value;
	
    pid->integral += error * dt;
	
    // Anti-Windup: limit integral term
    if (pid->integral > MAX_INTEGRAL) {
        pid->integral = MAX_INTEGRAL;
    } else if (pid->integral < -MAX_INTEGRAL) {
        pid->integral = -MAX_INTEGRAL;
    }
	
	
	double derivative = (error - pid->previous_error);// / dt;
  
    double output = (pid->kp * error) + (pid->ki * pid->integral) + (pid->kd * derivative);
	
    pid->previous_error = error;

	
    return output;
}