/*************************************************************
Project		: Micromouse V1.1
File		: State_SolveMaze.c
Description	: State Solvemaze -> Solve the Maze (find shortest path)
Date		: 05.11.2024IR_Left45: 2950.000000
*************************************************************/

#include "StateMachine.h"
#include "Floodfill.h"
#include "Gyro.h"
#include "imu_test.h"
#include "TimeOfLight.h"


#ifndef STATE_SOLVE_H
#define STATE_SOLVE_H
#define M_PI 3.14159265359

State_t const State_SolveMaze = {State_SolveMaze_Enter, State_SolveMaze_Process, State_SolveMaze_Exit};

static void cb_controller_test(int id){
	update_motor_controllers(0, 0, true);
}

// initialize everthing
void State_SolveMaze_Enter(void)
{
	//TODO: SetUp 
	//PY_Printf("SOLVE MAZE ENTER");
}

uint16_t const angleArraySize = 256;
angleInput angleArray[256];

static uint16_t currentAngleArrayIndex = 0;
volatile float totalDistance = 0.0f;

void AddAngleInput(float lengthBeforCurve, int8_t turn, float correction)
{
	float const radius_mm = 90.0f;
	angleArray[currentAngleArrayIndex].correctionValue = correction;
	angleArray[currentAngleArrayIndex].startPosition = totalDistance + lengthBeforCurve;
	
	angleArray[currentAngleArrayIndex].turns = turn;

	currentAngleArrayIndex++;
	volatile float arc_len    = (float)M_PI * radius_mm * 0.5f * correction; // Bogenl?nge
	totalDistance = totalDistance + arc_len + lengthBeforCurve;
	angleArray[currentAngleArrayIndex].endPosition = totalDistance;
	
}


// Process
State_t const* State_SolveMaze_Process(void)
{
	float const rightCorrection = 0.95f;
	float const leftCorrection = 0.9f;
	
	//RHA Test Gyro
//	MX_Gyro_GPIO_Init();
//	MX_Gyro_I2C1_Init();
//	BNO055_Init_Gyro(&hi2c2);
//	
//	volatile vec3 testVar;
//	
//	volatile uint8_t chip_id = 0;
//	HAL_I2C_Mem_Read(&hi2c2, BNO055_ADDR, 0x00, 1, &chip_id, 1, 100);
//	
//	while(1){
//		testVar = BNO055_ReadLinearAccel(&hi2c2);
//		testVar.x = 0;
//		testVar.y = 0;
//		testVar.z = 0;
//	}
		
	//RHA Test Imu
//	BNO055_Handle_t bno;
//	MX_I2C2_Init();
//	HAL_I2C_MspInit(&hi2c2);
//	BNO055_Init(&bno,&hi2c2,BNO055_ADDRESS_B);
//	volatile bno055_powermode_t powerMode = POWER_MODE_SUSPEND;
//	volatile bno055_opmode_t opMode = OPERATION_MODE_CONFIG;
//	BNO055_GetPowerMode(&bno,&powerMode);
//	BNO055_GetOperationMode(&bno, &opMode);
//	
//	BNO055_RunTest(&bno);
//	
//	BNO055_GetPowerMode(&bno,&powerMode);
//	BNO055_GetOperationMode(&bno, &opMode);
//	
//	while(1){
//		Acceleration(&bno);
//		//IMUDirection(&bno);
//		//AngularVelocity(&bno);
//	}
	
	//RHA Test dynamic driving
	AddAngleInput(380.0f, 1, rightCorrection);
	AddAngleInput(380.0f, -1, rightCorrection);
	AddAngleInput(380.0f, 1, rightCorrection);
	AddAngleInput(380.0f, -1, rightCorrection);
	
//	AddAngleInput(190.0f, -1, leftCorrection);
//	AddAngleInput(190.0f, -1, leftCorrection);
//	
//	AddAngleInput(185.0f, 1, rightCorrection);
//	AddAngleInput(185.0f, 1, rightCorrection);
//	AddAngleInput(185.0f, 1, rightCorrection);
//	AddAngleInput(185.0f, 1, rightCorrection);
//	
//	AddAngleInput(190.0f, -1, leftCorrection);
//	AddAngleInput(190.0f, -1, leftCorrection);
//	AddAngleInput(190.0f, -1, leftCorrection);
//	AddAngleInput(190.0f, -1, leftCorrection);
//	
//	AddAngleInput(190.0f, 1, rightCorrection);
//	AddAngleInput(190.0f, 1, rightCorrection);
//	AddAngleInput(190.0f, 1, rightCorrection);
//	AddAngleInput(190.0f, 1, rightCorrection);
//	
//	AddAngleInput(190.0f, -1, leftCorrection);
//	AddAngleInput(190.0f, -1, leftCorrection);
//	AddAngleInput(190.0f, -1, leftCorrection);
//	AddAngleInput(190.0f, -1, leftCorrection);
//	
//	AddAngleInput(190.0f, 1, rightCorrection);
//	AddAngleInput(190.0f, 1, rightCorrection);
//	AddAngleInput(190.0f, 1, rightCorrection);
//	AddAngleInput(190.0f, 1, rightCorrection);
//	
//	AddAngleInput(190.0f, -1, leftCorrection);
//	AddAngleInput(190.0f, -1, leftCorrection);
//	AddAngleInput(190.0f, -1, leftCorrection);
//	AddAngleInput(190.0f, -1, leftCorrection);

		
	//SOLVER CODE
		while(1){
			
			double IR_Left = readSensor_Left();
			//double IR_Right = readSensor_Right();
			double IR_Front = readSensor_Front();
			double IR_Front2 = readSensor_Front2();


			PY_Printf("IR_Left: %f \n", IR_Left);
			//PY_Printf("IR_Right: %f \n", IR_Right);
			PY_Printf("IR_Front: %f \n", IR_Front);
			PY_Printf("IR_Front2: %f \n", IR_Front2);
			
			Delay_ms(800);
			
			if(BUTTON1_Pressed()){
				Delay_ms(1000);
				Board_LED_On(1);
				chain_profiles(500,1500,angleArray,currentAngleArrayIndex,totalDistance);
				//chain_profiles(500,1500,angleArray,currentAngleArrayIndex,totalDistance);
				//SolveMaze(maze,mouseMaze);

			}
			//else if (BUTTON2_Pressed()){
				//Delay_ms(1000);
				//drive_half_circle(1000,1500);
					//drive_curve(500,1500,90,120);
//				Delay_ms(1000);
//				move(0.5f,1000,1500,0);
//				Delay_ms(1000);
//				move(1,1000,1500,0);
			//}
		}
			
	
	
//	double IR_Left = 0;
//	double IR_Right = 0;
//	double ir_error = 0;
//	while(1){
//		if(BUTTON1_Pressed()){
//			Delay_ms(1000);
//			SolveMaze(maze,mouseMaze);
//		}
//	}
    
	
	//IR Testing

//	volatile double IR_Left = 0;
//	volatile double IR_Right = 0;
//	volatile double IR_Front = 0;
//	volatile double IR_Front2 = 0;
//	volatile double IR_Left45 = 0;
//	volatile double IR_Right45 = 0;
//	
//while(1) {
//	

////	
////	
////	Motor_Left_SetPWM(50);
////	Motor_Right_SetPWM(50);
////	
//	
//		if(BUTTON1_Pressed()){
//			
////			Motor_Left_SetPWM(50);
////			Motor_Right_SetPWM(50);
//			
//			while(1){
//				Delay_ms(1000);
////				IR_Left = readSensor_Left();
////				IR_Right = readSensor_Right();
////				IR_Front = readSensor_Front();
////				IR_Front2 = readSensor_Front2();
//				IR_Left45 = readSensor_Left_45();
//				IR_Right45 = readSensor_Right_45();
////				PY_Printf("IR_Right45: %f", IR_Right45);
////				PY_Printf("IR_Right: %f", IR_Right);
////				PY_Printf("IR_Front: %f", IR_Front);
////				PY_Printf("IR_Front2: %f", IR_Front2);
////				PY_Printf("IR_Left45: %f", IR_Left45);
////				PY_Printf("IR_Right45: %f", IR_Right45);
//				PY_Printf("Encoder Left: %f", Get_Encoder_Position_Rounds_Left());
//				PY_Printf("Encoder Right: %f", Get_Encoder_Position_Rounds_Right());
//			}
//			//SolveMaze(maze,mouseMaze);

//		}
//		
//	}
//	//PY_Printf("IR_Left: %f", IR_Left);
//	PY_Printf("IR_Right: %f", IR_Right);
//	PY_Printf("IR_Left45: %f", IR_Left45);
//	PY_Printf("IR_Right45: %f", IR_Right45);

		
		
		


//uint32_t period_ms = (uint32_t)(LOOP_INTERVAL*1000);
//	while(1){
//		
////		PY_Printf("Encoder Left: %f\n", Get_Encoder_Position_Rounds_Left());
////		PY_Printf("Encoder Right: %f\n", Get_Encoder_Position_Rounds_Right());
////		Delay_ms(500);
//		// Solving show case
//		if(BUTTON1_Pressed()){
////			Delay_ms(1000);
////			SolveMaze(maze,mouseMaze);
//			Timer_Initialize(TIMER_ID_TIM12, 5, cb_controller_test);
//			//move(6, 4000, 5000, 0);

//		}
//		
//		
//		// Fast Run show case
//		if(BUTTON2_Pressed()){
//			Delay_ms(1000);
////			turn(1);
////			turn(1);
////			turn(1);
////			turn(1);
//			move(1, 300, 1500, 0);
////			move(4, 500, 1500, 0);
////			move(1, 300, 1500, 0);
////			move(1, 500, 1500, 0);
////			
//			//turn(1);
////			
////			move(1, 500, 1500, 0);
////			move(1, 500, 1500, 0);
////			
////			turn(1);
////			
////			move(1, 500, 1500, 0);
////			move(1, 500, 1500, 0);
////			
////			turn(1);
////			
////			move(1, 500, 1500, 0);
////			move(1, 500, 1500, 0);
////			
////			turn(-1);
////			
////			move(1, 500, 1500, 0);
////			move(1, 500, 1500, 0);
////			
//		}
//		
////		// Dynamic curves
////		if(BUTTON1_Pressed()){
////			Delay_ms(1000);
////			smooth_turn(500, 1500, 1);

////		}
//	
//	}
		

	



	// switch to FastRun state
	//Fast run is in solve Maze included
	return &State_FastRun;
}

// ---------------------
void State_SolveMaze_Exit(void)
{
	// TODO: 
	// LED => Maze Solved!!!
}


#endif 