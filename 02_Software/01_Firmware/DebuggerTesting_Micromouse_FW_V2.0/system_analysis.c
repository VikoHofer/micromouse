/*************************************************************
Project		: Micromouse V2.0
File		: system_analysis.c
Description	: Functionality to make controlled motion on the mouse
Date		: 11.03.2026
*************************************************************/

#include "motion_control.h"
#include <stdio.h>
#include "encoder.h"
#include "motor_driver.h"
#include "Delay.h"
#include "board_button.h"
#include "timer.h"
#include "config.h"
#include "board_led.h"
#include <stdlib.h>
#include <string.h>
#include "BLEDebugger.h"

static const uint8_t TIME_HEADER[5] = "TIME\n"; 
static const uint8_t VELOCITY_HEADER[9] = "VELOCITY\n"; 
/*******************************************************************************
* SYSTEM ANALYSE
*******************************************************************************
* Create step response of the mouse
* 1. Button Press -> Drive and sample data
* 2. Button Press -> Send data via PY Debugger
*******************************************************************************/

//void create_step_respose(void){
//	encoder_reset();
//	
//	float time_array[SAMPLE_POINTS];
//	float speed_array[SAMPLE_POINTS];
//	
//	float forward_speed = 0;
//		
//		Delay_ms(1000);
//		Timer_Start(TIMER_ID_TIM2);
//	
//		Motor_Left_SetPWM(STEP_INPUT_PWM);
//		Motor_Right_SetPWM(STEP_INPUT_PWM);
//		
//		// measure
//		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
//        {
//			// Get current speed from encoders
//			float right_speed = encoder_get_right_speed_mms();
//			float left_speed = encoder_get_left_speed_mms();
//			forward_speed = (right_speed + left_speed) / 2;
//			
//			time_array[i] = i * SAMPLE_TIME_MS;
//			speed_array[i] = forward_speed;
//			
//			
//			Delay_ms(SAMPLE_TIME_MS);
//        }
//		
//		Motor_Left_SetPWM(0);
//		Motor_Right_SetPWM(0);
//		
//
//		Timer_Stop(TIMER_ID_TIM2);
//		
//		uint8_t sample_buf[8];
//		//printf("TIME\n");
//		// print time
//		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
//		{
//			if(i == 0){
//				WriteProtocol(cmd_STEP,TIME_HEADER,sizeof(TIME_HEADER));
//			}
//			memcpy(sample_buf,&time_array[i],4);
//			WriteProtocol(cmd_STEP,sample_buf,sizeof(sample_buf));
//			Delay_ms(5);
//		}
//		
//		//printf("VELOCITY\n");
//		// print velocity
//		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
//		{
//			if(i == 0){
//				WriteProtocol(cmd_STEP,VELOCITY_HEADER,sizeof(VELOCITY_HEADER));
//			}
//			memcpy(sample_buf,&speed_array[i],4);
//			WriteProtocol(cmd_STEP,sample_buf,sizeof(sample_buf));
//			Delay_ms(5);
//		}
//		
//		
void create_step_respose(void){
	encoder_reset();
	
	float time_array[SAMPLE_POINTS];
	float speed_array[SAMPLE_POINTS];
	
	float forward_speed = 0;
		
    Delay_ms(1000);
    Timer_Start(TIMER_ID_TIM2);

    Motor_Left_SetPWM(STEP_INPUT_PWM);
    Motor_Right_SetPWM(STEP_INPUT_PWM);
    
    // measure
    for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
    {
        // Get current speed from encoders
        float right_speed = encoder_get_right_speed_mms();
        float left_speed = encoder_get_left_speed_mms();
        forward_speed = (right_speed + left_speed) / 2;
        
        time_array[i] = i * SAMPLE_TIME_MS;
        speed_array[i] = forward_speed;
        
        Delay_ms(SAMPLE_TIME_MS);
    }
    
    Motor_Left_SetPWM(0);
    Motor_Right_SetPWM(0);
    
    Timer_Stop(TIMER_ID_TIM2);
    
    // FIX 1: Change array size to exactly 4 bytes (the size of a float)
    uint8_t sample_buf[4];
    
    // print time
    for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
    {
        if(i == 0){
            WriteProtocol(cmd_STEP,TIME_HEADER,sizeof(TIME_HEADER));
            // FIX 2: Wait 20ms to force the BLE module to send the header 
            // in its own packet so C# doesn't swallow the first float!
            Delay_ms(20); 
        }
        memcpy(sample_buf,&time_array[i],4);
        // sizeof(sample_buf) is now safely 4 bytes
        WriteProtocol(cmd_STEP,sample_buf,sizeof(sample_buf));
        Delay_ms(5);
    }
    
    // print velocity
    for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
    {
        if(i == 0){
            WriteProtocol(cmd_STEP,VELOCITY_HEADER,sizeof(VELOCITY_HEADER));
            // FIX 2: Wait 20ms to prevent packet merging
            Delay_ms(20);
        }
        memcpy(sample_buf,&speed_array[i],4);
        WriteProtocol(cmd_STEP,sample_buf,sizeof(sample_buf));
        Delay_ms(5);
    }
    
    // FIX 3: The Ultimate Hardware Flush. 
    // Send 64 bytes of padding. This is larger than the maximum MTU of 
    // almost all BLE modules, guaranteeing the final float is pushed out.
    uint8_t dummy_padding[64] = {0};
    WriteProtocol((TxCMD_t)0xF8, dummy_padding, 64);
}
void create_step_angle_vel(void)
{
    encoder_reset();

    float time_array[SAMPLE_POINTS];
    float yaw_rate_array[SAMPLE_POINTS];

    if (BUTTON1_Pressed()) {

        Delay_ms(1000);

        Motor_Left_SetPWM(ANGLE_INPUT_PWM);
        Motor_Right_SetPWM(-ANGLE_INPUT_PWM);

        for (uint16_t i = 0; i < SAMPLE_POINTS; i++)
        {
            time_array[i] = i * SAMPLE_TIME_MS;
            yaw_rate_array[i] = encoder_get_yaw_rate_degs();   // oder _degs()

            Delay_ms(SAMPLE_TIME_MS);
        }

        Motor_Left_SetPWM(0);
        Motor_Right_SetPWM(0);

        while(!BUTTON1_Pressed()) {}

        printf("TIME\n");
        for (uint16_t i = 0; i < SAMPLE_POINTS; i++)
        {
            printf("%f\n", time_array[i]);
            Delay_ms(5);
        }

        printf("YAW_RATE_DEG_S\n");
        for (uint16_t i = 0; i < SAMPLE_POINTS; i++)
        {
            printf("%f\n", yaw_rate_array[i]);
            Delay_ms(5);
        }
    }
}

