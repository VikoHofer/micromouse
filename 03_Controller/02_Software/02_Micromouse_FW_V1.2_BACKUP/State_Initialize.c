/*************************************************************
Project		: Micromouse V1.1
File		: State_Initialze.c
Description	: State Initialize -> set everything up
Date		: 05.11.2024
*************************************************************/

#include "StateMachine.h"

#ifndef STATE_INIT_H
#define STATE_INIT_H




State_t const State_Initialize = {State_Initialize_Enter, State_Initialize_Process, State_Initialize_Exit};

// initialize everthing
void State_Initialize_Enter(void)
{
	// initialize HAL-Library
	HAL_Init();
	
	
	
	// Clock config
	SystemCoreClockConfigure();// -> 180 Mhz
	SysTick_Init(HAL_RCC_GetHCLKFreq() / 1000);
	

	// Hardware Init
	Motor_Init();
	Encoder_Init();
	Uart6_Init();
	BUTTON_Init();
	//Board_LED_Init();
	
	// Start Encoder
	Encoder_Start();
	
	// Init Debugger 
	PY_Debugger_Init();
	
	// TODO: Maze Init
	// TODO: Set LED => signals mouse is intialized
	
	
	
	PY_Printf("Initialize Done!!");
	PY_Printf("Micromouse Running on: %d MHz", SystemCoreClock);
	
}

// initialization process
State_t const* State_Initialize_Process(void)
{
	Encoder_Reset();
	ADC_Init(&hadc1);
	Sensor_Init();
	
	double error = 0;

	while(1){
	//PY_Process();
	
	PY_Printf("Sensor L %f", readSensor_Left_45());
	PY_Printf("Sensor R %f", readSensor_Right_45());
	PY_Printf("Sensor FRONT %f", readSensor_Front());
	Delay_ms(500);
	
////		//create_step_angle_vel();
//		if(BUTTON1_Pressed()){
//			Delay_ms(1000);
//			
//			turn(1);
//			Delay_ms(1000);
			//turn(-1);
//			move(2);
//			turn(1);
//			move(5);
//			turn(1);
//			move(1);
//			turn(-1);
//			move(1);
//			turn(1);
//			move(2);
			
//			while(1){
//				turn_left(1);
//				Delay_ms(500);
//			}
			
//			move(1);
//			turn_right(1);
//			move(1);
//			turn_left(1);
//			move(1);
//			turn_left(4);
			
//			move(1);
//			move(1);
//			turn_right(1);
//			move(1);
//			turn_right(1);
//			move(3);
//			turn_right(1);
//			move(1);
			//move(1);
//			while(1){
//				update_motor_controllers(0,0, true);
////				error = ir_angle_error();
////				PY_Printf("IR ERROR: %f", error);
////				PY_Printf("ADJUSTMENT: %F", calculate_steering_adjustment(error));
////				PY_Printf("----------------");
//				Delay_ms(1);
//				
//			}
		//}

		//}
	}
	
		

	/*
	* In loop until Btn1 is pressed
	while(1){
		* Choose direction with  btn2
		* LEDs will show which direction is set (Startposition)
	}
	*/

	
	// switch to SolzeMaze state
	return &State_SolveMaze;
	
}

// ---------------------
void State_Initialize_Exit(void)
{
	// Turn all Leds off
}


#endif 