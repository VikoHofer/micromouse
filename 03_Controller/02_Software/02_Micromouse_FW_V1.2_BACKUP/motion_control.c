/*************************************************************
Project		: Micromouse V1.2
File		: motion_control.c
Description	: Functionality to make controlled motion on the mouse
Date		: 23.12.2024
*************************************************************/

#include "motion_control.h"
#include <stdio.h>
#include "encoder.h"
#include "MotorDriver.h"
#include "Delay.h"
#include "Python_Debugger.h"
#include "board_button.h"
#include "adc.h"
#include "sensor.h"
#include "profile.h"
#include <math.h>

/*******************************************************************************
* DEFINES
*******************************************************************************/
#define STEP_INPUT_PWM 110
#define ANGLE_INPUT_PWM 110
#define SAMPLE_TIME_MS 10
#define SAMPLE_POINTS 300

#define DISTANCE_FIELD 200
#define DISTANCE_90DEG 151

#define KP_pos 17.28
#define KD_pos 2.45
#define KP_ang 25.04
#define KD_ang 1.78
#define KP_ir 0.02
#define KD_ir 0.1
#define LOOP_INTERVAL 1 // testen

#define leftMiddleValue 1600
#define rightMiddleValue 1600
#define hasLeftWall 2200
#define hasRightWall 2200

#define ADJUSTMENT_LIMIT 50


/*******************************************************************************
* SYSTEM ANALYSE
*******************************************************************************
* Create step response of the mouse
* 1. Button Press -> Drive and sample data
* 2. Button Press -> Send data via PY Debugger
*******************************************************************************/
void create_step_respose(void){
	Encoder_Reset();
	
	float time_array[SAMPLE_POINTS];
	float speed_array[SAMPLE_POINTS];
	
	float forward_speed = 0;
	
	if(BUTTON1_Pressed()){
		
		Delay_ms(1000);
		
		Motor_Left_SetPWM(STEP_INPUT_PWM);
		Motor_Right_SetPWM(STEP_INPUT_PWM);
		
		// measure
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
        {
			// Get current speed from encoders
			double speed_L = Encoder_GetSpeed_Left();
			double speed_R = Encoder_GetSpeed_Right();
			forward_speed = (speed_L + speed_R)/2;
			
			time_array[i] = i * SAMPLE_TIME_MS;
			speed_array[i] = apply_fir_filter_velocity(forward_speed);
			
			
			Delay_ms(SAMPLE_TIME_MS);
        }
		
		Motor_Left_SetPWM(0);
		Motor_Right_SetPWM(0);
		
		while(!BUTTON1_Pressed()){} // wait for print data
		
		PY_Printf("TIME");
		// print time
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
		{
			PY_Printf("%f", time_array[i]);
			Delay_ms(5);
		}
		PY_Printf("VELOCITY");
		// print velocity
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
		{
			// speed
			PY_Printf("%f", speed_array[i]);
			Delay_ms(5);
		}
	}

}
void create_step_angle_vel(void){
	Encoder_Reset();
	
	float time_array[SAMPLE_POINTS];
	float speed_array[SAMPLE_POINTS];
	
	float angle_velocity = 0;
	
	if(BUTTON1_Pressed()){
		
		Delay_ms(1000);
		
		Motor_Left_SetPWM(ANGLE_INPUT_PWM);
		Motor_Right_SetPWM(-ANGLE_INPUT_PWM);
		
		// measure
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
        {
			// Get current speed from encoders
			double speed_L = Encoder_GetSpeed_Left();
			double speed_R = Encoder_GetSpeed_Right();
			angle_velocity = (speed_L - speed_R);
			
			time_array[i] = i * SAMPLE_TIME_MS;
			speed_array[i] = apply_fir_filter_velocity(angle_velocity);
			
			Delay_ms(SAMPLE_TIME_MS);
        }
		
		Motor_Left_SetPWM(0);
		Motor_Right_SetPWM(0);
		
		while(!BUTTON1_Pressed()){} // wait for print data
		
		PY_Printf("TIME");
		// print time
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
		{
			PY_Printf("%f", time_array[i]);
			Delay_ms(5);
		}
		PY_Printf("ANGLE VELOCITY");
		// print velocity
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
		{
			// speed
			PY_Printf("%f", speed_array[i]);
			Delay_ms(5);
		}
	}

}

/*******************************************************************************
* IR-Sensors
*******************************************************************************/
// PA0
// duration 2ms!!
double readSensor_Left_45(void){
	double Distance = 0;
	uint32_t sensorValue = 0;

	if (Sensor_Read(SENSOR_ID_1, &hadc1, &sensorValue) == SENSOR_OK) {
		Distance = (double)sensorValue;
	}

	return Distance;
}

// PA4
// duration 2ms!!
double readSensor_Right_45(void){
	double Distance = 0;
	uint32_t sensorValue = 0;

	if (Sensor_Read(SENSOR_ID_3, &hadc1, &sensorValue) == SENSOR_OK) {
		Distance = (double)sensorValue;
	}

	return Distance;
}

// PB0
double readSensor_Front(void){
	double Distance = 0;
	uint32_t sensorValue = 0;

	if (Sensor_Read(SENSOR_ID_4, &hadc1, &sensorValue) == SENSOR_OK) {
		Distance = (double)sensorValue;
	}

	return Distance;
}


/*******************************************************************************
* position control
*******************************************************************************
* output -> desired PWM value
*******************************************************************************/
static double position_controller(double set_position){
	static double old_error = 0;
	
    // Get current position from encoders
    double position_L = Get_Encoder_Position_mm_Left();
    double position_R = Get_Encoder_Position_mm_Right();
	double position = (position_L + position_R) / 2;
	
	double error = set_position - position;
	double diff = error - old_error;
	

    // Adjust the PWM values based on the position correction
    double pwm = KP_pos * error + KD_pos * diff;

	// update old error
	old_error = error;
	
	return pwm;
}

/*******************************************************************************
* limit_correction_ir
*******************************************************************************/
static double limit_correction_ir(double correction_ir, double min_limit, double max_limit) {
    if (correction_ir > max_limit) {
        return max_limit;
    } else if (correction_ir < min_limit) {
        return min_limit;
    }
    return correction_ir;
}

/*******************************************************************************
* IR-Sensor error
*******************************************************************************
* calculate angle error by IR-Sensors
*******************************************************************************/
double ir_angle_error(void){
	double error = 0;
	double sensor_L = readSensor_Left_45();
	double sensor_R = readSensor_Right_45();
	
	double leftError = leftMiddleValue - sensor_L;
	double rightError = rightMiddleValue - sensor_R;
	
	if (sensor_L < hasLeftWall && sensor_R < hasRightWall) // both walls
	{
		error = (leftError - rightError);;
	}
	else if (sensor_L < hasLeftWall) // only left wall
	{
		error = (2 * leftError);
		
	}
	else if (sensor_R < hasRightWall) // only right wall
	{
		error = (-2 * rightError);
	}
	else 
	{
		error = 0;
	}
	
	return error;
}

/*******************************************************************************
* caluclate angle correction by IR-Sensors
*******************************************************************************
* calculate angle error by IR-Sensors
*******************************************************************************/
double calculate_steering_adjustment(double error){
	double angle_adjustment = 0;
	static double old_error = 0;
	
	double pTerm = KP_ir * error;
	double dTerm = KD_ir * (error - old_error);
	angle_adjustment = (pTerm + dTerm) * LOOP_INTERVAL;
	
	// limit the output!! -> maybe need a correction
	angle_adjustment = limit_correction_ir(angle_adjustment, -ADJUSTMENT_LIMIT, ADJUSTMENT_LIMIT);
	old_error = error;
	
	return angle_adjustment;
}

/*******************************************************************************
* angle control
*******************************************************************************
* output -> desired PWM value
*******************************************************************************/
static double angle_controller(double set_angle, bool sensor_on){
	static double old_error_encoder = 0;
	double error_encoder = 0;
	double diff = 0;
	double pwm = 0;
	
	// get angle error by IR-Sensors
	double error_ir = ir_angle_error();
	// calculate angle adjustment
	double adjustment = calculate_steering_adjustment(error_ir);

    // Get current position from encoders
    double position_L = Get_Encoder_Position_mm_Left();
    double position_R = Get_Encoder_Position_mm_Right();
	double angle = (position_L - position_R);
	
	// calculate angle error by encoders
	error_encoder = set_angle - angle;
	
	// add ir-adjustment when activated
	if (sensor_on){
		error_encoder += adjustment;
	}
	
	// D-Term
	diff = (error_encoder - old_error_encoder) / LOOP_INTERVAL;
	
	// Adjust the PWM values based on the angle correction
    pwm = (KP_ang * error_encoder + KD_ang * diff);

	// update old error
	old_error_encoder = error_encoder;
	
	return pwm;
}



/*******************************************************************************
* update motor controller
*******************************************************************************
* input: Angle & Velocity
*******************************************************************************/
void update_motor_controllers(double setAngle, double setPosition, bool sensor_on){
	double leftMotor = 0;
	double rightMotor = 0;
	
	// position control
	double forwardSpeed = position_controller(setPosition);
	
	// angle  control
	double angleCorrection = angle_controller(setAngle, sensor_on);
	
	// angle control....
	leftMotor = forwardSpeed+angleCorrection;
	rightMotor = forwardSpeed-angleCorrection;
	
	
	Motor_Left_SetPWM(leftMotor);
	Motor_Right_SetPWM(rightMotor);
}



/*******************************************************************************
* move
*******************************************************************************
* input: distance (fields)
* return: false if wall is in front
*******************************************************************************/
bool move(uint8_t n){

	Profile forward_profile;
	float top_speed = 300.0f;    
    float final_speed = 0.0f;      
    float acceleration = 600.0f;
	float goalDistance = n * DISTANCE_FIELD;
	int i = 0;
	
	
	Encoder_Reset();
	Profile_Reset(&forward_profile);
	Profile_Init(&forward_profile);
	
	Profile_Start(&forward_profile, goalDistance, top_speed, final_speed, acceleration);
	
	while (!Profile_IsFinished(&forward_profile)) {
		// update profile
		Profile_Update(&forward_profile);

		// get target position
		float target_position = Profile_GetPosition(&forward_profile);
		

		update_motor_controllers(0, target_position ,true);
			
		Delay_ms(1);
	}
	
	Motor_Left_SetPWM(0);
	Motor_Right_SetPWM(0);
	return true;
}

/*******************************************************************************
* turn slow (left => -n, right => n: 90�)
*******************************************************************************/
void turn(uint8_t n)
{
	Profile rotation_profile;
	float top_speed = 500.0f;       
    float final_speed = 0.0f;       
    float acceleration = 1500.0f;
	float goalDistance = (1 * n * DISTANCE_90DEG);

	Encoder_Reset();
	Profile_Reset(&rotation_profile);
	Profile_Init(&rotation_profile);
	
	Profile_Start(&rotation_profile, goalDistance, top_speed, final_speed, acceleration);
	
	while (!Profile_IsFinished(&rotation_profile)) {
		// update profile
		Profile_Update(&rotation_profile);

		// get target position
		float target_angle = Profile_GetPosition(&rotation_profile);

		update_motor_controllers(target_angle, 0 ,false);
		

		Delay_ms(1);
	}
	
	Motor_Left_SetPWM(0);
	Motor_Right_SetPWM(0);
}

