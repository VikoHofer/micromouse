/*************************************************************
Project		: Micromouse V1.1
File		: State_Initialze.c
Description	: State Initialize -> set everything up
Date		: 05.11.2024
*************************************************************/

#include "StateMachine.h"
#include "current_sensor.h"
#include "FloodFill.h"
#include "board_button.h"
#include "sensor_normalize.h"
#include "board_led.h"
#include "TimeOfLight.h"

#ifndef STATE_INIT_H
#define STATE_INIT_H




State_t const State_Initialize = {State_Initialize_Enter, State_Initialize_Process, State_Initialize_Exit};

// initialize everthing
void State_Initialize_Enter(void)
{
	// initialize HAL-Library
	HAL_Init();
    //RHA TEST Inits
    BUTTON_Init();
	Board_LED_Init();
    Current_Sensor_Init();
    ADC_Init(&hadc1);
    Sensor_Init();
	
    #undef HSE_VALUE
		#define HSE_VALUE 8000000
    // Clock config
		//SystemClock_Config();
		volatile uint32_t sysclk_freq; 
		sysclk_freq = HAL_RCC_GetSysClockFreq();
		SystemCoreClockConfigure_HSE(); // -> 16 Mhz
    //SystemCoreClockConfigure_HSE_120Mhz();// -> 120 Mhz Test !!!!!!!!!VERURSACHT möglicherweise random Hardfaults
		//SystemCoreClockConfigure_HSI_120Mhz(); // Test, ob 120Mhz am internen quarz funktionieren
	  sysclk_freq = HAL_RCC_GetSysClockFreq();
    SysTick_Init(sysclk_freq / 1000);
    
	//Test Time Of Flight Init
//	HAL_I2C_MspInit(&hi2c1);
//	MX_I2C1_Init();
//  
//	VL53L0X_TestLoop();
	

	// Hardware Init
	Motion_Init();
	Motor_Init();
	Encoder_Init();
	Uart6_Init();
	BUTTON_Init();
	//Board_LED_Init();
	
	// Start Encoder
	Encoder_Start();
	
	// Init Debugger 
	//PY_Debugger_Init();
	
	// TODO: Maze Init
	// TODO: Set LED => signals mouse is intialized
	
	
	
	PY_Printf("Initialize Done!!");
	PY_Printf("Micromouse Running on: %d Hz", SystemCoreClock);
	
}

// initialization process
State_t const* State_Initialize_Process(void)
{
	Encoder_Reset();
	ADC_Init(&hadc1);
	Sensor_Init();
    InitMouseMaze(mouseMaze);
    InitMaze(maze);
	
	while(1){
		if(BUTTON2_Pressed()){
			Board_LED_On(0);
			// Press when mouse is aligned in middle 
			IR_Calibrate_Reference();
			PY_Printf("IR Ref Left: %f", S_ref_left_45);
			PY_Printf("IR Ref Right: %f", S_ref_right_45);
			return &State_SolveMaze;
		}
	}


	
	// switch to SolzeMaze state
	return &State_SolveMaze;
	
}

// ---------------------
void State_Initialize_Exit(void)
{
	// Turn all Leds off
}


#endif 