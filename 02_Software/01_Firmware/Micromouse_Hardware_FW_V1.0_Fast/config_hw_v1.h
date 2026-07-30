/*************************************************************
Project		: Micromouse V2.0
File		: config_hw_v1.h
Description	: Hardware specific configuration V1
Date		: 15.04.2026
*************************************************************/

#ifndef CONFIG_HW_V1_H
#define CONFIG_HW_V1_H


#define MOTOR_HW_V1 1

#define TARGET_MOTOR MOTOR_HW_V1

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
#define WHEEL_BASE_MM               80
#define  LOOP_INTERVAL				0.001f
#define CONTROL_PERIOD_S            LOOP_INTERVAL


#define ENCODER_LEFT_SIGN           (+1)
#define ENCODER_RIGHT_SIGN          (-1)

// Encoder Layout
#define ENCODER_HW_V1 1

#define TARGET_ENCODER ENCODER_HW_V1


// ***** Motion Control **************************************

#define M_PI 3.14159265359

// Step Response test
#define STEP_INPUT_PWM 1000
#define ANGLE_INPUT_PWM 400
#define SAMPLE_TIME_MS 20
#define SAMPLE_POINTS 80

/* PWM limit */
#define MAX_PWM               1023.0f

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
#define FWD_KP                98.355425f
#define FWD_KD                4958.947f

#define ROT_KP                124.132427f
#define ROT_KD                3264.995f

#define DISTANCE_FIELD 175
#define ROTATION_DEG 90


/* Feedforward */
#define FEEDFORWARD            true

#define FF_KS                  84.761725f
#define FF_KV                  0.458874
#define FF_KA                  0.107783

/* Activate FF-acc */
#define FF_USE_ACCELERATION    1






#endif
