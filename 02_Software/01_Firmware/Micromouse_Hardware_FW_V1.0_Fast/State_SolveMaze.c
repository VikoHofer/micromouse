/*************************************************************
Project		: Micromouse V2.0
File		: State_SolveMaze.c
Description	: State Solvemaze -> Solve the Maze (find shortest path)
Date		: 11.03.2026
*************************************************************/

#include "StateMachine.h"

#ifndef STATE_SOLVE_H
#define STATE_SOLVE_H


State_t const State_SolveMaze = {State_SolveMaze_Enter, State_SolveMaze_Process, State_SolveMaze_Exit};

static void cb_update_test_sensor(int id)
{
    encoder_update();
	//MotionController_Update(0);
    //WallSensors_Update();

}



// initialize everthing
void State_SolveMaze_Enter(void)
{
	//TODO: SetUp 
}

// Process
State_t const* State_SolveMaze_Process(void)
{
	
	//Board_LED_On(1);
	
	//MotionController_Test_Standing();
	
	float speed = 1100;
//	
//	Timer_Initialize(TIMER_ID_TIM2,  1, cb_update_test_sensor);

//	HAL_Delay(2000);
//	
//    Timer_Start(TIMER_ID_TIM2);
////	
//	MotionController_SetCommand(
//        30,   /* mm/s */
//        0                                  /* deg/s */
//    );
//	
//	
//	while(1){
//		//create_step_respose();
//		create_step_angle_vel();
//	}
	
//	MotionController_Test_Standing();

	
	while(1){
		
//		
//		WallSensors_IR_Update();
//////		
//		printf("Sensor LEFT45: %f\n", WallSensors_IR_Read(WALL_SENSOR_LEFT_45));
//printf("Sensor Right45: %f\n", WallSensors_IR_Read(WALL_SENSOR_RIGHT_45));
//		printf("Sensor LEFT45: %f\n", readSensor_Left_45());
//		printf("Sensor Right45: %f\n", readSensor_Right_45());
//		printf("Sensor Right: %f\n", readSensor_Right());
//		printf("Sensor Left: %f\n", readSensor_Left());
//		printf("Sensor Front: %f\n", readSensor_Front());
//		printf("Sensor Front2: %f\n", readSensor_Front2());

//		printf("Correction: %f\n", WallFollowing_GetCorrectionDps());

//		printf("Sensor LEFT: %f\n", readSensor_Left());
//		printf("Sensor RIGHT: %f\n", readSensor_Right());
//		printf("ENC_RIGHT: %d\n", encoder_get_right_counts());
//		printf("ENC_LEFT: %d\n", encoder_get_left_counts());
		
//		printf("ENC_RIGHT: %f\n", encoder_get_right_mm());
//		printf("ENC_LEFT: %f\n", encoder_get_left_mm());
//		printf("ANgle: %f\n", encoder_get_angle_rad());
		//HAL_Delay(500);
		
		float vel_forward = 1500;
		float vel_angle = 700;

		float acc_forward = 4000.0f;
		float acc_curve = 999999.0f;
	
	
////Test gerade soviel speed wie m�glich
//		while(1){
//			if(BUTTON1_Pressed()){
//				HAL_Delay(1000);
//				//Funktionierende Teststrecke mehr Speed auf geraden
//				Motion_Start();
//				
//				Motion_CommandForward(70.0f, vel_forward, acc_forward, vel_angle, true);
//				Motion_CommandCurveAngle(-185.0f, 85.0f, vel_angle, 999999.0f, vel_angle);
//				Motion_CommandCurveAngle(90.0f, 85.0f, vel_angle, 999999.0f, vel_angle);
//				Motion_CommandForward(900.0f, vel_forward, acc_forward, 0, true);

//				Motion_Stop();
//			}
//		}
	
	
//Teststrecke bestes Ergebnis
//		if(BUTTON1_Pressed()){
//			HAL_Delay(1000);
//			//Funktionierende Teststrecke mehr Speed auf geraden
//			Motion_Start();
//			
//			Motion_CommandForward(680.0f, vel_forward, acc_forward, vel_angle);
//			while(1){
//				Motion_CommandCurveAngle(-185.0f, 85.0f, vel_angle, 999999.0f, vel_angle);
//				Motion_CommandCurveAngle(65.0f, 65.0f, vel_angle, 999999.0f, vel_angle);
//				Motion_CommandForward(105.0f, vel_angle, acc_forward, vel_angle);
//				Motion_CommandCurveAngle(-60.0f, 65.0f, vel_angle, 999999.0f, vel_forward);
//				Motion_CommandForward(360.0f, vel_forward, acc_forward, vel_angle);
//				Motion_CommandCurveAngle(155.0f, 85.0f, vel_angle, 999999.0f, vel_angle);
//				Motion_CommandForward(105.0f, vel_angle, acc_forward, vel_angle);
//				Motion_CommandCurveAngle(-150.0f, 85.0f, vel_angle, 999999.0f, vel_forward);
//				Motion_CommandForward(205.0f, vel_forward, acc_forward, vel_angle);
//				Motion_CommandCurveAngle(-90.0f, 85.0f, vel_angle, 999999.0f, vel_forward);
//				Motion_CommandForward(530.0f, vel_forward, acc_forward, vel_angle);
//				Motion_CommandCurveAngle(-90.0f, 85.0f, vel_angle, 999999.0f, vel_forward);
//				Motion_CommandForward(690.0f, vel_forward, acc_forward, vel_angle);
//			}
//			//Motion_CommandForward(170.0f, vel_forward, acc_forward, 0);

////			while(1){
////					Motion_CommandCurveAngle(-180.0f, 85.0f, vel_angle, 99999.0f, vel_angle);
////					Motion_CommandCurveAngle(95.0f, 85.0f, vel_angle, 99999.0f, vel_angle);
////					Motion_CommandCurveAngle(-95.0f, 90.0f, vel_angle, 99999.0f, vel_angle);
////					Motion_CommandForward(350.0f, vel_forward, acc_forward, vel_angle);
////					Motion_CommandCurveAngle(180.0f, 85.0f, vel_angle, 99999.0f, vel_angle);
////					Motion_CommandCurveAngle(-180.0f, 85.0f, vel_angle, 99999.0f, vel_angle);
////					Motion_CommandForward(175.0f, vel_forward, acc_forward, vel_angle);
////					Motion_CommandCurveAngle(-95.0f, 90.0f, vel_angle, 99999.0f, vel_angle);
////					Motion_CommandForward(520.0f, vel_forward, acc_forward, vel_angle);
////					Motion_CommandCurveAngle(-95.0f, 90.0f, vel_angle, 99999.0f, vel_angle);
////					Motion_CommandForward(680.0f, vel_forward, acc_forward, vel_angle);
////			}
//			
//			

////            Motion_CommandForward(710.0f, vel_forward, acc_forward, 0.0f);
////            while(1){
////                Motion_CommandCurveAngle(-180.0f, 85.0f, vel_angle, 99999.0f, vel_angle);
////                Motion_CommandCurveAngle(95.0f, 85.0f, vel_angle, 99999.0f, vel_angle);
////                Motion_CommandCurveAngle(-95.0f, 90.0f, vel_angle, 99999.0f, vel_angle);
////                Motion_CommandForward(355.0f, vel_forward, acc_forward, vel_angle);
////                Motion_CommandCurveAngle(180.0f, 85.0f, vel_angle, 99999.0f, vel_angle);
////                Motion_CommandCurveAngle(-180.0f, 85.0f, vel_angle, 99999.0f, vel_angle);
////                Motion_CommandForward(210.0f, vel_forward, acc_forward, vel_angle);
////                Motion_CommandCurveAngle(-95.0f, 90.0f, vel_angle, 99999.0f, vel_angle);
////                Motion_CommandForward(530.0f, vel_forward, acc_forward, vel_angle);
////                Motion_CommandCurveAngle(-95.0f, 90.0f, vel_angle, 99999.0f, vel_angle);
////                Motion_CommandForward(710.0f, vel_forward, acc_forward, vel_angle);
////            }
//			Motion_Stop();
//			//move(6, 800, 1000, 0);
//			//move(6, 800, 1000, 0);
////			turn(1);
////			turn(1);
////			turn(1);
////			turn(1);
//			//turn(1);
//			//Motion_Start();
////			
//			//Motion_CommandCurveAngle(89.0f, 90.0f, 400.0f, 2000.0f, 400.0f);
////			Motion_CommandCurveAngle(-89.0f, 90.0f, 400.0f, 2000.0f, 400.0f);
////			Motion_CommandCurveAngle(89.0f, 90.0f, 400.0f, 2000.0f, 400.0f);
////			Motion_CommandCurveAngle(-89.0f, 90.0f, 400.0f, 2000.0f, 400.0f);
////			Motion_CommandCurveAngle(89.0f, 90.0f, 400.0f, 2000.0f, 400.0f);

//			/* 1) Geradeaus und am Ende nicht auf 0 abbremsen */
//			//Motion_CommandCurveAngle(89.0f, 90.0f, 200.0f, 2000.0f, 0.0f);
////			while(1){
////				Motion_CommandForward(180.0f, 400.0f, 2500.0f, 400.0f);
////				Motion_CommandCurveAngle(89.0f, 90.0f, 400.0f, 2000.0f, 400.0f);
////				Motion_CommandForward(180.0f, 400.0f, 2500.0f, 400.0f);
////				Motion_CommandForward(180.0f, 400.0f, 2500.0f, 400.0f);
////				Motion_CommandCurveAngle(89.0f, 90.0f, 400.0f, 2000.0f, 400.0f);
////			
////			}
////			
////			

//			//Motion_CommandForward(360.0f, 400.0f, 2500.0f, 300.0f);
//////			

////			Motion_CommandForward(90.0f, 300.0f, 2500.0f, 300.f);

////			/* optional sp�ter wieder ausblenden */
////			
//			//Motion_CommandCurveAngle(90.0f, 90.0f, 300.0f, 2000.0f, 300.0f);
////			//Motion_CommandTurn(90, 400, 0, 3500);
////			
////			Motion_CommandForward(360.0f, 300.0f, 2500.0f, 300.f);

//			//Motion_Stop();
//			
//			//move(6, 100, 2500, 0);
////			turn(-1);
////			move(1, speed, 2500, 0);
////			turn(-1);
////			move(4, speed, 2500, 0);
////			turn(1);
////			move(1, speed, 2500, 0);
////			turn(1);
////			move(3, speed, 2500, 0);
////			turn(-1);
////			move(4, speed, 2500, 0);
////			turn(1);
////			move(1, speed, 2500, 0);
////			turn(1);
////			move(6, speed, 2500, 0);
////			turn(1);
////			move(4, speed, 2500, 0);

////			HAL_Delay(1000);
////			turn(1);
////			//HAL_Delay(500);
////			turn(1);
////			//HAL_Delay(500);
////			turn(1);
////			//HAL_Delay(500);
////			turn(1);
//		}
			

			
		if(BUTTON1_Pressed()){
				Delay_ms(1000);
				Board_LED_On(1);
				//chain_profiles(500,1500,angleArray,currentAngleArrayIndex,totalDistance);
				//chain_profiles(500,1500,angleArray,currentAngleArrayIndex,totalDistance);
				SolveMaze(maze,mouseMaze);

			}

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