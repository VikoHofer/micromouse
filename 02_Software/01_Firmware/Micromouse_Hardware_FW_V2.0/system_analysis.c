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



/*******************************************************************************
* SYSTEM ANALYSE
*******************************************************************************
* Create step response of the mouse
* 1. Button Press -> Drive and sample data
* 2. Button Press -> Send data via PY Debugger
*******************************************************************************/
void create_step_respose(void){
	encoder_reset();
	
	float time_array[SAMPLE_POINTS];
	float speed_array[SAMPLE_POINTS];
	
	float forward_speed = 0;
	
	if(BUTTON1_Pressed()){
		
		Delay_ms(1000);
		
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
		
		while(!BUTTON1_Pressed()){} // wait for print data
		
		printf("TIME\n");
		// print time
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
		{
			printf("%f\n", time_array[i]);
			Delay_ms(5);
		}
		printf("VELOCITY");
		// print velocity
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
		{
			// speed
			printf("%f\n", speed_array[i]);
			Delay_ms(5);
		}
	}

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

