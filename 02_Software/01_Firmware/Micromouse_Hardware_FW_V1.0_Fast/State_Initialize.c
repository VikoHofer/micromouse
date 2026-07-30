/*************************************************************
Project		: Micromouse V2.0
File		: State_Initialze.c
Description	: State Initialize -> set everything up
Date		: 11.03.2026
*************************************************************/

#include "StateMachine.h"


State_t const State_Initialize = {State_Initialize_Enter, State_Initialize_Process, State_Initialize_Exit};



// initialize everthing
void State_Initialize_Enter(void)
{
	// initialize HAL-Library
	HAL_Init();
	
	// config clock
	if (Clock_Config_BoardDefault() != HAL_OK) {
		while(1);
	}

	
	
	// peripheral init
	BUTTON_Init();
	Board_LED_Init();
	
	Uart6_Init();
	
	Motor_Init();
	
	encoder_init();
	encoder_start();

	
	MotionController_Init();
    MotionController_Reset();
	
	
	WallSensors_IR_Init();
	WallFollowing_Init();
	
	Motion_Init();
	
	InitMouseMaze(mouseMaze);
    InitMaze(maze);

	printf("Initialize Done!!");
	printf("Micromouse Running on: %d Hz", SystemCoreClock);

}

// initialization process
State_t const* State_Initialize_Process(void)
{
    if (BUTTON2_Pressed()) {
		
		IR_CalibrateReference();
		return &State_SolveMaze;
		
	}

    return &State_Initialize;
}

// ---------------------
void State_Initialize_Exit(void)
{
	// Turn all Leds off
}

