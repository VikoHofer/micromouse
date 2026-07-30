/*************************************************************
Project		: Micromouse V1.2
File		: pid.h
Description	: PID-Controller
Date		: 23.12.2024
*************************************************************/

#ifndef PID_H
#define PID_H


// PID Controller parameters
typedef struct {
    double kp;   // Proportional gain
    double ki;   // Integral gain
    double kd;   // Derivative gain
    double setpoint;  // Desired value
    double integral;  // Integral term
    double previous_error;  // Previous error
} PIDController;

void PID_Init(PIDController *pid, double kp, double ki, double kd, double setpoint, double integral_start);

double PID_Compute(PIDController *pid, double current_value, double dt);

#endif