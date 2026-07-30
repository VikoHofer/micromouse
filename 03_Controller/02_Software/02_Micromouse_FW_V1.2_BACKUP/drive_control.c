/*************************************************************
Project		: Micromouse V1.0
File		: drive_control.c
Description	: PID-Control for micromouse / Implementation
Date		: 09.09.2024
*************************************************************/


#include "drive_control.h"
#include <stdio.h>
#include "encoder.h"
#include "MotorDriver.h"
#include "Print.h"
#include "Delay.h"
#include "Python_Debugger.h"

/*******************************************************************************
* Experimental Constants
*******************************************************************************/
#define DAMPING_RATIO 0.707 // Beta
#define SETTLING_TIME 0.80	// Td

#define Km 16.2 // mm/s / Volt
#define Tm 0.164 // 0.63 * Km => Time constant

static double KP_calculated = 0;
static double KD_calculated = 0;

/*******************************************************************************
* PID Anti-Windup Constants
*******************************************************************************/
#define MAX_INTEGRAL 2000.0
#define MIN_PWM_THRESHOLD 15
#define MAX_PWM 255


/*******************************************************************************
* calculate kp and kd depending on meassured values
*******************************************************************************/
void Calculate_Constants(void){

	KP_calculated = (Tm / Km) * (16/((DAMPING_RATIO*DAMPING_RATIO)*(SETTLING_TIME*SETTLING_TIME)));
	
	KD_calculated = ((8*Tm) - SETTLING_TIME) / (SETTLING_TIME*Km);
}



void Speed_Ctrl(PIDController* pid_L, PIDController* pid_R) {

    // Calculate time
    static uint32_t prev_time_ms = 0;
    uint32_t current_time_ms = HAL_GetTick();
    uint32_t elapsedTime = current_time_ms - prev_time_ms;
    double dt_sec = elapsedTime / 1000.0;

    // Get current speed from encoders
    double current_encoder_L = Encoder_GetSpeed_Left();
    double current_encoder_R = Encoder_GetSpeed_Right();

    // Adjust the PWM values based on the angle correction
    double setPWM_R = (PID_Compute(pid_R, current_encoder_R, 0.001));
    double setPWM_L = (PID_Compute(pid_L, current_encoder_L, 0.001));
	

	Motor_Right_SetPWM(setPWM_R);
	Motor_Left_SetPWM(setPWM_L);
	
	
    // Update previous time
    prev_time_ms = current_time_ms;
}

/*******************************************************************************
* Set PWM Value and sends Encoder Data to PY Debugger
*******************************************************************************/
void Get_ID_Motor(uint8_t const PWM){
	int16_t speedR = 0;
	
	Motor_Right_Forwards();
	Motor_Right_SetSpeed(PWM);
	
	Motor_Left_Forwards();
	Motor_Left_SetSpeed(PWM);
	
	Encoder_Reset();
	
	// send PWM [Voltage]
	//PY_SEND_PWM(PWM);
	
	for(uint16_t i = 0; i < 300 ; i++) // 3 Sec Messen
	{
		speedR = Encoder_GetSpeed_Right();
		// send speed [mm/s]
		PY_SEND_SPEED(speedR);
		//PY_Printf("Speed: %d", speedR);
		Delay_ms(5);
	}
	
	// Stoppen
	Motor_Right_Brake();
	Motor_Right_SetSpeed(0);
	
	Motor_Left_Brake();
	Motor_Left_SetSpeed(0);

}



/*******************************************************************************
* Make a step with PD-Controler and send data to PY Debugger
*******************************************************************************/
void PD_Step_Response(int16_t _speedCmd){
	
	Encoder_Reset();

	Calculate_Constants();
	PY_Printf("Step Response PD!");
	PY_Printf("SET SPEED: %d", _speedCmd);
	PY_Printf("KP: %f", KP_calculated);
	PY_Printf("KD: %f", KD_calculated);
	
	// PD Controller
	static PIDController testPID_R = {0, 0, 0, 0, 0, 0};
	testPID_R.kp = 0.129474;
	testPID_R.ki = 0.30343;
//	testPID_R.kp = 0.07;
//	testPID_R.ki = 2.8;
//	testPID_R.kd = 0.00018;
	testPID_R.setpoint = _speedCmd;
	
	// activate Motor
	Motor_Right_Forwards();
	
	int16_t current_encoder_R = 0;
	
	for(uint16_t i = 0; i < 1000 ; i++) // 1 Sec Messen
	{
		
		if(i > 300) // Little Delay
		{
			current_encoder_R = Encoder_GetSpeed_Right();
			PY_SEND_SPEED(current_encoder_R);		
			
			double setPWM_R = (int16_t)(PID_Compute(&testPID_R, (double)current_encoder_R, 0.001));
			PY_SEND_PWM(setPWM_R);
			
			
			Motor_Right_SetPWM(setPWM_R);
			
			PY_Printf("PWM: %f", setPWM_R);
			PY_Printf("SPEED: %d", current_encoder_R);
			
		} 
		else
		{
			PY_SEND_SPEED(0);
			PY_SEND_PWM(0);
		}
		
		
		
		Delay_ms(1);
	}
	// Stoppen
	Motor_Right_Brake();
	Motor_Right_SetSpeed(0);
	
	// FINSIH

}


/*******************************************************************************
* Make a move test with PD-Controller and send data to PY Debugger
*******************************************************************************/
void PD_Move(void){
	Encoder_Reset();
	Calculate_Constants();
	PY_Printf("Move Test PD!");
	PY_Printf("KP: %f", KP_calculated);
	PY_Printf("KD: %f", KD_calculated);
	
	// PD Controller
	static PIDController testPID_R = {0, 0, 0, 0, 0, 0};
	testPID_R.kp = 0.07;
	testPID_R.ki = 2.8;
	testPID_R.kd = 0.00018;
	
	// Activate motor once at the start
	Motor_Right_Forwards();
	
	double current_encoder_R = 0;
	double setpoint = 0;
	double max_speed = 3000;  // Max speed in mm/s
	int ramp_up_end = 750;   // End of acceleration phase (750 ms scaled to 1 ms steps)
	int constant_end = 1250; // End of constant speed phase (1250 ms scaled to 1 ms steps)

	for(uint16_t i = 0; i < 2000; i++) // 2 Sec measurement loop
	{
		// Setpoint adjustments for trapezoidal velocity profile
		if (i <= ramp_up_end) {
			// Acceleration phase (ramp up)
			setpoint = (double)i / ramp_up_end * max_speed;
		} 
		else if (i <= constant_end) {
			// Constant speed phase
			setpoint = max_speed;
		} 
		else {
			// Deceleration phase (ramp down)
			setpoint = max_speed * (1.0 - (double)(i - constant_end) / (2000 - constant_end));
		}
		
		//PY_Printf("i: %d, setpoint: %f", i, setpoint);
		
		testPID_R.setpoint = setpoint;
		
		// Get the current speed from the encoder
		current_encoder_R = Encoder_GetSpeed_Right();
		PY_SEND_SPEED(current_encoder_R);		
		
		// Calculate PWM using the PD controller
		double setPWM_R = (PID_Compute(&testPID_R, (double)current_encoder_R, 0.001));
		
		Motor_Right_SetPWM(setPWM_R);
		
		// Send PWM to the debugger and set the motor speed
		PY_SEND_PWM(setPWM_R);

		// Delay to control timing of each loop iteration
		Delay_ms(1);
	}

	// Stop motor after loop finishes
	Motor_Right_Brake();
	Motor_Right_SetSpeed(0);
}