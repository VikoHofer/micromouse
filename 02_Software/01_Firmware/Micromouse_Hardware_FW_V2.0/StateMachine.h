/*************************************************************
Project		: Micromouse V2.0
File		: StateMachine.h
Description	: Statemachine for the Micromouse
Date		: 11.03.2026
*************************************************************/

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H
  
/*********************************************************
					INCLUDES
*********************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// includes STM32-Librarys
#include "stm32f4xx.h"
#include "stm32f4xx_hal_gpio.h"

// includes costum builds
#include "clock_config.h"
#include "BLEDebugger.h"

#include "board_button.h"
#include "board_led.h"

#include "Uart.h"

#include "motor_driver.h"

#include "encoder.h"

#include "timer.h"

#include "motion_control.h"

#include "imu_test.h"

#include "TimeOfLight.h"

#include "opt3101.h"

#include "bno055_hal.h"

#include "system_analysis.h"

#include "profile.h"

#include "FloodFill.h"

#include "wall_sensors_45.h"
/*********************************************************
					STATEMACHINE
*********************************************************/
typedef void (*StateTransitionFunction_t)(void); 
typedef struct State_s const * (*StateFunction_t)(void); 

// defines one state
typedef struct State_s { 
    StateTransitionFunction_t Enter; 
    StateFunction_t Process; 
    StateTransitionFunction_t Exit; 
} State_t;

// definition of different States
extern State_t const State_Initialize;
extern State_t const State_SolveMaze;
extern State_t const State_FastRun;

// Initialize State
void State_Initialize_Enter(void);
State_t const* State_Initialize_Process(void);
void State_Initialize_Exit(void);

// SolveMaze State
void State_SolveMaze_Enter(void);
State_t const* State_SolveMaze_Process(void);
void State_SolveMaze_Exit(void);


// execute in the main loop
void StateMachine_Run(State_t const * state);

#endif