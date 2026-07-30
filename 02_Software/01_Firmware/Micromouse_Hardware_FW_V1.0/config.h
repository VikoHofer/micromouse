/*************************************************************
Project		: Micromouse V1.2
File		: config.h
Description	: Collection of all configuration defines and constants
Date		: 07.01.2026
*************************************************************/

#include "stdbool.h"
#include "stdint.h"

#ifndef CONFIG_H
#define CONFIG_H

// ***** CONFIG **************************************

#define CLK_16_MHZ
// #define CLK_180_MHZ

/***********************************
* uncomment this define to activate 
* debugging feature via uart
***********************************/
#define PY_DEBUG_ACTIVATED 1


// choose which mouse is in use
//#define MOUSE_1_1
#define MOUSE_1_2


// configurates all used control loop timer
#define LOOP_INTERVAL 0.005

// ***** MOTOR **************************************

/************************************
* using TIM 2 to create PWM Signals
* TIM4 (AF2)
* CH3 -> PB8 (Motor Right)
* CH4 -> PB9 (Motor Left)
************************************
* Control PINS (Forward / Backward)
* Motor Left: AIN1 -> PA10
* Motor Left: AIN2 -> PC9
* Motor Right: BIN1 -> PB5
* Motor Right: BIN2 -> PC8
************************************
* CW: 	IN1 = HIGH, IN2 = LOW 
* CCW:	IN1 = LOW,  IN2 = HIGH
* Stop: IN1 = LOW,  IN2 = LOW
*************************************/


// PWM configuration

// Duty cycle
#define PWM_PERIOD 255

// f_PWM = f_TIM / ((PSC + 1) · (ARR + 1))

# ifdef CLK_16_MHZ
	// System CLK = 16 MHz -> PWM Freq = 16 MHz
	#define PWM_PRESCALER 0
	#define SYSTEM_CLK 16
# elif CLK_180_MHZ
	// System CLK = 180 MHz -> PWM Freq = 16 MHz
	#define PWM_PRESCALER 16 
	#define SYSTEM_CLK 180
#endif

// ***** ENCODER **************************************

/************************************
* TIM3: (AF2) // LEFT
* - CH1 -> PA6
* - CH2 -> PA7
*---------------------
* TIM1: (AF1) // RIGHT
* - CH1 -> PA8
* - CH2 -> PA9
************************************/


#define WHEEL_SIZE 100.53 // Umfang
#define COUNTER_MAX 65535
#define IIR_ORDER 4


#ifdef MOUSE_1_1
	#define COUNTS_PER_ROUND 120
#endif 
#ifdef MOUSE_1_2
	#define COUNTS_PER_ROUND 60
#endif

// ***** IR Sensor **************************************

// Time delay for measuring one IR-Sensor [ms]
#define IR_DELAY 2

// ***** IR Sensor Normalize **************************************

// Steering adjustment limit
#define ADJUSTMENT_LIMIT 120


// Sensor thresholds
#ifdef MOUSE_1_1
	// Offset between Left and Right 45° Sensor
	#define IR_OFFSET 140

	#define FRONT_LEFT_NO_WALL 3200
	#define FRONT_RIGHT_NO_WALL 3200
	
	#define FRONT_LEFT_MIDDLE 2950
	#define FRONT_RIGHT_MIDDLE 2650
	
#endif 

#ifdef MOUSE_1_2
	// Offset between Left and Right 45° Sensor
	#define IR_OFFSET 140
	
	#define FRONT_LEFT_NO_WALL 3200
	#define FRONT_RIGHT_NO_WALL 3200
	
	#define FRONT_LEFT_MIDDLE 2800
	#define FRONT_RIGHT_MIDDLE 2450
#endif




// ***** Motion Control **************************************

#define M_PI 3.14159265359

// Step Response test
#define STEP_INPUT_PWM 100
#define ANGLE_INPUT_PWM 100
#define SAMPLE_TIME_MS 10
#define SAMPLE_POINTS 100


static bool SENSOR_ENABLE = true;
static bool STEERING_ENABLE = true;


#ifdef MOUSE_1_1

	#define KP_pos 8.348
	#define KD_pos 0.555 //With D filtering

	#define KP_ang 2.076
	#define KD_ang 0.183//0.212 //0.07 OK With D filtering
	//#define KD_ang 9.7

	// ACTUAL DISTANCES
	#define DISTANCE_FIELD 181
	#define DISTANCE_90DEG 138.5f

	// SENSOR_SENSIBILITY
	#define KP_ir 1.8 // 2.5
	#define KD_ir 0//0.7

#endif

#ifdef MOUSE_1_2

	//#define KP_pos 6
	//#define KD_pos 0.555 //With D filtering
	
	#define KP_pos 6
	#define KD_pos 0.8 //With D filtering

	//#define KP_ang 2.076//2.076
	////#define KD_ang 0.183//0.212 //0.07 OK With D filtering
	//#define KD_ang 0.212//15

	#define KP_ang 2
	#define KD_ang 0.5 //0.07 OK With D filtering

	// ACTUAL DISTANCES
	#define DISTANCE_FIELD 181
	#define DISTANCE_90DEG 79

	// SENSOR_SENSIBILITY
	#define KP_ir 1.5 // 2.5
	#define KD_ir 0//0.7
	
#endif




#endif