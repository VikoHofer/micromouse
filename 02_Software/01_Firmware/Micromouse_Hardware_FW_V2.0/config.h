/*************************************************************
Project		: Micromouse V2.0
File		: config.h
Description	: Collection of all configuration defines and constants
Date		: 09.09.2024
*************************************************************/

#include "stdbool.h"
#include "stdint.h"

#ifndef CONFIG_H
#define CONFIG_H

// ***** CONFIG **************************************

//#define CLK_16_MHZ
#define CLK_180_MHZ



// configurates all used control loop timer
#define LOOP_INTERVAL 0.001

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
#define PWM_PERIOD 1023

// f_PWM = f_TIM / ((PSC + 1) · (ARR + 1))

// System CLK = 180 MHz -> PWM Freq = 22KHz
#define PWM_PRESCALER 3
#define SYSTEM_CLK 180

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


#define ENCODER_COUNTS_PER_REV      271.0f
#define COUNTS_PER_ROUND			271
#define WHEEL_DIAMETER_MM           32.0f
#define WHEEL_SIZE 100.53 // Umfang
#define COUNTER_MAX 65535
#define WHEEL_BASE_MM               75.0f
#define CONTROL_PERIOD_S            LOOP_INTERVAL


#define ENCODER_LEFT_SIGN           (+1)
#define ENCODER_RIGHT_SIGN          (-1)


// ***** Motion Control **************************************

#define M_PI 3.14159265359

// Step Response test
#define STEP_INPUT_PWM 1000
#define ANGLE_INPUT_PWM 400
#define SAMPLE_TIME_MS 20
#define SAMPLE_POINTS 50

/* PWM limit */
#define MAX_PWM               2000.0f

/* ----------------------------------------------------------
 * Incremental controller gains
 *
 * IMPORTANT:
 * For UKMARSBOT-style implementation:
 *   output = KP * error + KD * (error - error_old)
 *
 * If you have continuous Kd from MATLAB:
 *   KD_discrete = Kd_continuous / CONTROL_PERIOD_S
 * ---------------------------------------------------------- */

/* Example from your identified values */
#define FWD_KP                74.78f
#define FWD_KD                3533.58f   /* 3.53358 / 0.001 */

#define ROT_KP                121.51f
#define ROT_KD                2722.31f   /* 2.72231 / 0.001 */

#define DISTANCE_FIELD 175
#define ROTATION_DEG 90.5


/* Feedforward */
#define FEEDFORWARD            true

#define FF_KS                  126.10f
#define FF_KV                  0.390264f
#define FF_KA                  0.080109f

/* Erstmal optional Acc-FF abschalten */
#define FF_USE_ACCELERATION    1


// ***** Sensors **************************************
#define WALL_45_BACKEND_TOF       0
#define WALL_45_BACKEND_OPT3101   1

#define WALL_45_BACKEND           WALL_45_BACKEND_OPT3101

#if WALL_45_BACKEND == WALL_45_BACKEND_OPT3101

#define WALL45_UPDATE_DIVIDER   1U

#else

#define WALL45_UPDATE_DIVIDER   5U

#endif


#endif