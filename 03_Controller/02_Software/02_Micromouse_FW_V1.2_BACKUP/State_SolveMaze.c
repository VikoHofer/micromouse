/*************************************************************
Project		: Micromouse V1.1
File		: State_SolveMaze.c
Description	: State Solvemaze -> Solve the Maze (find shortest path)
Date		: 05.11.2024
*************************************************************/

#include "StateMachine.h"
#include "Floodfill.h"

#ifndef STATE_SOLVE_H
#define STATE_SOLVE_H


State_t const State_SolveMaze = {State_SolveMaze_Enter, State_SolveMaze_Process, State_SolveMaze_Exit};

// initialize everthing
void State_SolveMaze_Enter(void)
{
	//TODO: SetUp 
	//PY_Printf("SOLVE MAZE ENTER");
}

// Process
State_t const* State_SolveMaze_Process(void)
{
    while(1){
        if(BUTTON1_Pressed()){
            SolveMaze(maze,mouseMaze);
        }
}
//	while(1)
//	{
//		if(BUTTON_USER_Pressed()){
//			while(BUTTON_USER_Pressed()){}
//			PY_Printf("LEAVE SOLVE MAZE");
//			return &State_FastRun;
//		}
//	
//	}
	//TODO: Driving

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