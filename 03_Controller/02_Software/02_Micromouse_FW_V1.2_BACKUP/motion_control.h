/*************************************************************
Project		: Micromouse V1.2
File		: motion_control.h
Description	: Functionality to make controlled motion on the mouse
Date		: 23.12.2024
*************************************************************/

#ifndef MOTION_H
#define MOTION_H

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
* SYSTEM ANALYSE
*******************************************************************************
* Create step response of the mouse
* 1. Button Press -> Drive and sample data
* 2. Button Press -> Send data via PY Debugger
*******************************************************************************/
void create_step_respose(void);
void create_step_angle_vel(void);



/*******************************************************************************
* IR-Sensors
*******************************************************************************/
// PA0
// duration 5ms!!
double readSensor_Left_45(void);

// PA1
// duration 5ms!!
double readSensor_Right_45(void);

// PB0
double readSensor_Front(void);

double ir_angle_error(void);
double calculate_steering_adjustment(double error);


/*******************************************************************************
* update motor controller (Position)
*******************************************************************************
* input: Angle & Position
*******************************************************************************/
void update_motor_controllers(double setAngle, double setPosition, bool sensor_on);



/*******************************************************************************
* move
*******************************************************************************
* input: distance (fields)
* return: false if wall is in front
*******************************************************************************/
bool move(uint8_t n);

/*******************************************************************************
* turn slow (left => -n, right => n: 90°)
*******************************************************************************/
void turn(uint8_t n);





#endif