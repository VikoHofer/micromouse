/*************************************************************
Project		: Micromouse V2.0
File		: State_SolveMaze.c
Description	: State Solvemaze -> Solve the Maze (find shortest path)
Date		: 11.03.2026
*************************************************************/

#include "StateMachine.h"

#ifndef STATE_SOLVE_H
#define STATE_SOLVE_H

#include "opt3101.h"

void Test_IR45Sensors(void)
{
    while (1)
    {
        float steering_dps;

        IR_Update();

        MotionController_UpdateToFCache();
        steering_dps = MotionController_CalcWallSteeringDps();

        printf("IR: L raw=%.1f filt=%.1f scaled=%.1f | "
               "R raw=%.1f filt=%.1f scaled=%.1f | "
               "steer=%.2f dps\r\n",
               IR_ReadRaw(IR_SENSOR_LEFT_45),
               IR_ReadFiltered(IR_SENSOR_LEFT_45),
               IR_ReadScaled(IR_SENSOR_LEFT_45),
               IR_ReadRaw(IR_SENSOR_RIGHT_45),
               IR_ReadFiltered(IR_SENSOR_RIGHT_45),
               IR_ReadScaled(IR_SENSOR_RIGHT_45),
               steering_dps);

        HAL_Delay(100);
    }
}

static void cb_update_test_sensor(int id)
{
    encoder_update();
	//MotionController_Update(0);
    //WallSensors_Update();

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
	
	//Test_IR45Sensors();
	
	//move(1, 200, 2500, 0);
	MotionController_Test_Standing();


	
	float speed = 1100;
	
	while(1){
//		printf("Sensor FRONT1: %f\n", readSensor_Front());
//		printf("Sensor FRONT2: %f\n", readSensor_Front2());
//		printf("Sensor LEFT: %f\n", readSensor_Left());
//		printf("Sensor RIGHT: %f\n", readSensor_Right());
		//HAL_Delay(1000);
		if(BUTTON1_Pressed()){
			HAL_Delay(5000);
			
			Motion_Start();
			
			//Motion_CommandForward(710.0f, 100.0f, 1000.0f, 0.0f, true);
			
//Funktionierende Teststrecke mehr Speed auf geraden
			Motion_CommandForward(710.0f, 2000.0f, 1000.0f, 300.0f, true);
			while(1){
				Motion_CommandCurveAngle(-180.0f, 90.0f, 300.0f, 99999.0f, 300.0f);
				Motion_CommandCurveAngle(95.0f, 90.0f, 500.0f, 99999.0f, 500.0f);
				Motion_CommandCurveAngle(-95.0f, 90.0f, 500.0f, 99999.0f, 500.0f);
				Motion_CommandForward(355.0f, 1000.0f, 1000.0f, 300.0f, true);
				Motion_CommandCurveAngle(180.0f, 90.0f, 300.0f, 99999.0f, 300.0f);
				Motion_CommandCurveAngle(-180.0f, 90.0f, 300.0f, 99999.0f, 300.0f);
				Motion_CommandForward(175.0f, 1000.0f, 1000.0f, 500.0f, true);
				Motion_CommandCurveAngle(-95.0f, 90.0f, 500.0f, 99999.0f, 500.0f);
				Motion_CommandForward(530.0f, 1000.0f, 1000.0f, 500.0f, true);
				Motion_CommandCurveAngle(-95.0f, 90.0f, 500.0f, 99999.0f, 500.0f);
				Motion_CommandForward(710.0f, 2000.0f, 2000.0f, 300.0f, true);
			}
			
//RHA Distanzen bei höheren Geschwindigkeiten	
			//Motion_CommandForward(690.0f, 3000.0f, 3000.0f, 0.0f, true);
			//Motion_CommandForward(170.0f, 3000.0f, 3000.0f, 0.0f, true);
			//Motion_CommandForward(345.0f, 3000.0f, 3000.0f, 0.0f, true);
			
//Funktionierende Teststrecke schneller			
//			while(1){
//				Motion_CommandForward(715.0f, 1000.0f, 1000.0f, 300.0f, true);
//				Motion_CommandCurveAngle(-180.0f, 90.0f, 300.0f, 99999.0f, 500.0f);
//				Motion_CommandCurveAngle(95.0f, 90.0f, 500.0f, 99999.0f, 500.0f);
//				Motion_CommandCurveAngle(-95.0f, 90.0f, 500.0f, 99999.0f, 1000.0f);
//				Motion_CommandForward(355.0f, 1000.0f, 1000.0f, 300.0f, true);
//				Motion_CommandCurveAngle(180.0f, 90.0f, 300.0f, 99999.0f, 300.0f);
//				Motion_CommandCurveAngle(-180.0f, 90.0f, 300.0f, 99999.0f, 1000.0f);
//				Motion_CommandForward(175.0f, 1000.0f, 1000.0f, 500.0f, true);
//				Motion_CommandCurveAngle(-95.0f, 90.0f, 500.0f, 99999.0f, 1000.0f);
//				Motion_CommandForward(535.0f, 1000.0f, 1000.0f, 500.0f, true);
//				Motion_CommandCurveAngle(-95.0f, 90.0f, 500.0f, 99999.0f, 1000.0f);
//			}
			
			
			//Funktionierende Teststrecke
//			while(1){
//				Motion_CommandForward(720.0f, 300.0f, 2500.0f, 0.0f, true);
//				Motion_CommandCurveAngle(-180.0f, 90.0f, 300.0f, 2500.0f, 0.0f);
//				Motion_CommandCurveAngle(90.0f, 90.0f, 300.0f, 2500.0f, 0.0f);
//				Motion_CommandCurveAngle(-90.0f, 90.0f, 300.0f, 2500.0f, 300.0f);
//				Motion_CommandForward(360.0f, 300.0f, 2500.0f, 0.0f, true);
//				Motion_CommandCurveAngle(180.0f, 90.0f, 300.0f, 2500.0f, 0.0f);
//				Motion_CommandCurveAngle(-180.0f, 90.0f, 300.0f, 2500.0f, 300.0f);
//				Motion_CommandForward(180.0f, 300.0f, 2500.0f, 300.0f, true);
//				Motion_CommandCurveAngle(-90.0f, 90.0f, 300.0f, 2500.0f, 300.0f);
//				Motion_CommandForward(540.0f, 300.0f, 2500.0f, 300.0f, true);
//				Motion_CommandCurveAngle(-90.0f, 90.0f, 300.0f, 2500.0f, 0.0f);
//			}

			
			
			//Test für 180° Kurve abwechselnd links rechts
//				Motion_CommandCurveAngle(180.0f, 90.0f, 300.0f, 3000.0f, 0.0f);
//				Motion_CommandCurveAngle(-180.0f, 90.0f, 300.0f, 3000.0f, 0.0f);
//				Motion_CommandCurveAngle(180.0f, 90.0f, 300.0f, 3000.0f, 0.0f);
//				Motion_CommandCurveAngle(-180.0f, 90.0f, 300.0f, 3000.0f, 0.0f);
//				Motion_CommandCurveAngle(180.0f, 90.0f, 300.0f, 3000.0f, 0.0f);
			
			//Test für Diagonale
//			Motion_CommandForward(90.0f, 300.0f, 2500.0f, 0.0f,false);
//			Motion_CommandTurn(-45.0f, 300.0f, 2500.0f, 0.0f);
//			Motion_CommandForward(383.0f, 300.0f, 2500.0f, 0.0f,false);
//			Motion_CommandTurn(-45.0f, 300.0f, 2500.0f, 0.0f);
			
			//Test Diagonale mit Kurven
//				Motion_CommandCurveAngle(-90.0f, 90.0f, 500.0f, 3000.0f, 0.0f);
//				Motion_CommandCurveAngle(90.0f, 90.0f, 500.0f, 3000.0f, 0.0f);
//				Motion_CommandCurveAngle(-90.0f, 90.0f, 500.0f, 3000.0f, 0.0f);
			
			//Test für max speed (max settings)
//				Motion_CommandCurveAngle(-180.0f, 90.0f, 500.0f, 3000.0f, 2000.0f);
//				Motion_CommandForward(720.0f, 2000.0f, 2500.0f, 500.0f,true);




			


			
			/* 1) Geradeaus und am Ende nicht auf 0 abbremsen */
			//Motion_CommandCurveAngle(89.0f, 90.0f, 200.0f, 2000.0f, 0.0f);
//			while(1){
//				Motion_CommandForward(180.0f, 400.0f, 2500.0f, 400.0f);
//				Motion_CommandCurveAngle(89.0f, 90.0f, 400.0f, 2000.0f, 400.0f);
//				Motion_CommandForward(180.0f, 400.0f, 2500.0f, 400.0f);
//				Motion_CommandForward(180.0f, 400.0f, 2500.0f, 400.0f);
//				Motion_CommandCurveAngle(89.0f, 90.0f, 400.0f, 2000.0f, 400.0f);
//			
//			}
//			
//			

//			Motion_CommandForward(180.0f, 300.0f, 2500.0f, 300.0f);
////			

//			Motion_CommandForward(90.0f, 300.0f, 2500.0f, 300.f);

//			/* optional später wieder ausblenden */
//			
//			Motion_CommandCurveAngle(90.0f, 90.0f, 300.0f, 2000.0f, 300.0f);
//			//Motion_CommandTurn(90, 400, 0, 3500);
//			
//			Motion_CommandForward(360.0f, 300.0f, 2500.0f, 300.f);

			Motion_Stop();
		}
			
//			move(4, speed, 2500, 0);
//			turn(-1);
//			move(1, speed, 2500, 0);
//			turn(-1);
//			move(4, speed, 2500, 0);
//			turn(1);
//			move(1, speed, 2500, 0);
//			turn(1);
//			move(3, speed, 2500, 0);
//			turn(-1);
//			move(4, speed, 2500, 0);
//			turn(1);
//			move(1, speed, 2500, 0);
//			turn(1);
//			move(6, speed, 2500, 0);
//			turn(1);
//			move(4, speed, 2500, 0);

//			HAL_Delay(1000);
//			turn(1);
//			//HAL_Delay(500);
//			turn(1);
//			//HAL_Delay(500);
//			turn(1);
//			//HAL_Delay(500);
//			turn(1);
			

			
//		if(BUTTON1_Pressed()){
//				Delay_ms(1000);
//				Board_LED_On(1);
//				//chain_profiles(500,1500,angleArray,currentAngleArrayIndex,totalDistance);
//				//chain_profiles(500,1500,angleArray,currentAngleArrayIndex,totalDistance);
//				printf("SolveMaze/n");
//				SolveMaze(maze,mouseMaze);

//			}

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