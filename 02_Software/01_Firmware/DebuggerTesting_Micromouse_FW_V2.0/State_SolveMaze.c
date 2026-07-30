/*************************************************************
Project		: Micromouse V2.0
File		: State_SolveMaze.c
Description	: State Solvemaze -> Solve the Maze (find shortest path)
Date		: 11.03.2026
*************************************************************/

#include "StateMachine.h"
#include "timer.h"
#include "encoder.h"
#ifndef STATE_SOLVE_H
#define STATE_SOLVE_H


static void cb_update_test_sensor(int id)
{
    encoder_update();

}


State_t const State_SolveMaze = {State_SolveMaze_Enter, State_SolveMaze_Process, State_SolveMaze_Exit};

// initialize everthing
void State_SolveMaze_Enter(void)
{
	//TODO: SetUp 
}

// Process
State_t const* State_SolveMaze_Process(void)
{
	
	Board_LED_On(1);
	
	//MotionController_Test_Standing();
	
	float speed = 1100;
	// step function
	// Comment this in to register Step function 
	Timer_Initialize(TIMER_ID_TIM2,  1, cb_update_test_sensor);
	
	//move(1,300,1000,0);
	while(1){
 
		BLEDebugger_Process();
		HAL_Delay(100);

	}

	// switch to FastRun state
	return &State_FastRun;
}

// ---------------------
void State_SolveMaze_Exit(void)
{
	// TODO: 
	// LED => Maze Solved!!!
}


#endif 