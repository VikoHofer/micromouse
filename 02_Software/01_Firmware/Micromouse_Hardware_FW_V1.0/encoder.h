/*************************************************************
Project		: Micromouse V1.0
File		: encoder.h
Description	: Read Encoders using TIMER
Date		: 09.09.2024
*************************************************************/

#include <stdint.h>

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"

/************************************
* TIM3: (AF2) // LEFT
* - CH1 -> PA6
* - CH2 -> PA7
*---------------------
* TIM1: (AF1) // RIGHT
* - CH1 -> PA8
* - CH2 -> PA9
************************************/



/*******************************************************************************
* Initialize and configure Timer for encoder mode
*******************************************************************************/
void Encoder_Init(void);

/*******************************************************************************
* Start/Stop Timer
*******************************************************************************/
void Encoder_Start(void);
void Encoder_Stop(void);


/*******************************************************************************
* Get counter value from TIMER (x4 Mode -> 4x Count with one cycle)
* Raw Counter Value (60 counts for one round)
*******************************************************************************/
double Get_Encoder_Position_Raw_Right(void);
double Get_Encoder_Position_Raw_Left(void);

/*******************************************************************************
* Get Position in Rounds
*******************************************************************************/
double Get_Encoder_Position_Rounds_Right(void);
double Get_Encoder_Position_Rounds_Left(void);


/*******************************************************************************
* Filter Velocity signals
*******************************************************************************/
float apply_fir_filter_velocity(float input);
float apply_fir_filter_angle_vel(float input);

/*******************************************************************************
* Get Position in mm
*******************************************************************************/
double Get_Encoder_Position_mm_Right(void);
double Get_Encoder_Position_mm_Left(void);

double Get_Encoder_Position_mm_Right_reset_Turn(void);
double Get_Encoder_Position_mm_Left_reset_Turn(void);


/*******************************************************************************
* Get Speed in mm/s
*******************************************************************************/
double Encoder_GetSpeed_Right();
double Encoder_GetSpeed_Left();

/*******************************************************************************
* Reset Encoders
*******************************************************************************/
void Encoder_Reset(void);
void Encoder_Reset_reset_Turn(void);


/*******************************************************************************
* Update Encoder to calculate forward increment
*******************************************************************************/
void Encoder_Update(void);

/*******************************************************************************
* Get incremental forward change since last tick
*******************************************************************************/#
double Encoder_Get_fwd_change();

/*******************************************************************************
* Get incremental rotational change
*******************************************************************************/
double Encoder_Get_rot_change();
