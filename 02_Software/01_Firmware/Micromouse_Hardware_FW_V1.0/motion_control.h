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

typedef struct {
    int16_t turns;
		float startPosition;
		float endPosition;
		float correctionValue;
} angleInput;

/*******************************************************************************
* Initialize timer for update mechanism
*******************************************************************************/
void Motion_Init();

/*******************************************************************************
* steering control
*******************************************************************************
* 1. calculate error between diagonal IR sensors
* 2. caclulate actual angle which need to be compensated by angle controller
*******************************************************************************/
double ir_angle_error(void);
double calculate_steering_adjustment(double error);


/*******************************************************************************
* update motor controller (Position)
*******************************************************************************
* input: Angle & Position
*******************************************************************************/
void update_motor_controllers(double setAngle, double setPosition, bool sensor_on);



/*******************************************************************************
* front_alignment
*******************************************************************************
* correct position of the mouse infront of a wall
*******************************************************************************/
void front_alignment(void);

/*******************************************************************************
* move
*******************************************************************************
* input: distance (fields)
* return: false if wall is in front
*******************************************************************************/
bool move(float n, float top_speed, float acceleration, float final_speed);
bool move_without_Sensor(float n, float top_speed, float acceleration, float final_speed);


/*******************************************************************************
* move with dynamic curves
*******************************************************************************
* input: n * distance
* curve_at -> distance when curve starts
* curve_type -> 0..right, 1..left 
* return: false if wall is in front
*******************************************************************************/
bool move_with_curves(uint8_t n, float top_speed, float acceleration, uint8_t curve_at, uint8_t curve_type);


/*******************************************************************************
* smooth turn, 90 degrees
*******************************************************************************/
void smooth_turn(float top_speed, float acceleration, float direction);

/*******************************************************************************
* turn slow (left => -n, right => n: 90°)
*******************************************************************************/
void turn(float n);

/*******************************************************************************
* smooth turn, turning with forward speed () smooth_turn_arc(90.0f, 60.0f, 400.0f, 2000.0f);
*******************************************************************************/
bool smooth_turn_arc(float angle_deg,
                     float radius_mm,
                     float arc_speed_mm_s,
                     float arc_acc_mm_s2);


bool straight_then_smooth_turn(float straight_mm,
                               float cruise_speed,      // mm/s (final_speed der Geraden)
                               float straight_acc,      // mm/s^2
                               float turn_angle_deg,    // +90 / -90 / etc.
                               float turn_radius_mm,    // Bogenradius (Roboter-Mittelpunkt)
                               float turn_acc);

//ist geschwindigkeitsabhängig															 
bool drive_half_circle(float cruise_speed, float acc);
bool drive_curve(float cruise_speed, float acc, float target_angle_deg, float drive_before_curve);
bool chain_profiles(float cruise_speed, float acc, angleInput* angleInputArray, uint8_t n_AngleInputArray, float drive_total);
															 

/*******************************************************************************
* SYSTEM ANALYSE
*******************************************************************************
* Create step response of the mouse
* 1. Button Press -> Drive and sample data
* 2. Button Press -> Send data via PY Debugger
*******************************************************************************/
void create_step_respose(void);
void create_step_angle_vel(void);

void SetCurveFactorLeft(double val);
void SetCurveFactorRight(double val);


#endif