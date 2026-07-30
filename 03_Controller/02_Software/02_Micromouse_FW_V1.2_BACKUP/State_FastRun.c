/*************************************************************
Project		: Micromouse V1.1
File		: State_FastRun.c
Description	: State FastRun -> Drive the shortest path
Date		: 05.11.2024
*************************************************************/

#include "StateMachine.h"

#ifndef STATE_RUN_H
#define STATE_RUN_HW


State_t const State_FastRun = {State_FastRun_Enter, State_FastRun_Process, State_FastRun_Exit};

// set up
void State_FastRun_Enter(void)
{
	//TODO: SetUp 
}

// Process
State_t const* State_FastRun_Process(void)
{
//	while(1)
//	{
//		if(BUTTON_USER_Pressed()){
//			while(BUTTON_USER_Pressed()){}
//			PY_Printf("LEAVE FAST RUN");
//			return &State_Initialize;
//		}
//	
//	}
//	
	//TODO: Driving

	// switch to first state
	return &State_Initialize;
}

// ---------------------
void State_FastRun_Exit(void)
{
	while(1){
		// FINSISH!!!!!!!!!!
	}
}


#endif 