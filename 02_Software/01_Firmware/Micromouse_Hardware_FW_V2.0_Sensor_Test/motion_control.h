/*************************************************************
Project     : Micromouse V2.0
File        : motion_controller.h
Description : Incremental forward / rotation controller
Date        : 18.03.2026
*************************************************************/

#ifndef MOTION_CONTROLLER_H
#define MOTION_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    bool controller_output_enabled;

    float previous_fwd_error;
    float previous_rot_error;

    float fwd_error;
    float rot_error;

    float velocity_ref_mms;   /* desired forward velocity [mm/s] */
    float omega_ref_dps;      /* desired yaw rate [deg/s] */

    float left_output_pwm;
    float right_output_pwm;

} MotionController_t;

extern MotionController_t motion_ctrl;

void Motion_Init(void);
void MotionController_Init(void);
void MotionController_Reset(void);
void MotionController_Enable(void);
void MotionController_Disable(void);
void MotionController_Stop(void);

void MotionController_Test_Standing(void);

void MotionController_SetCommand(float velocity_mms, float omega_rads);
void MotionController_Update(float steering_adjustment_rads);

float MotionController_GetLeftPWM(void);
float MotionController_GetRightPWM(void);
float MotionController_GetForwardError(void);
float MotionController_GetRotationError(void);

void MotionController_UpdateToFCache(void);
float MotionController_CalcWallSteeringDps(void);

// move functions
bool move(float n_fields, float top_speed, float acceleration, float final_speed);
void turn(float n_quarter_turns);


// experimental move functions for continious driving

// start moving section
void Motion_Start(void);

// stop moving section
void Motion_Stop(void);


// COMMANDS
void Motion_CommandForward(float distance_mm, float top_speed, float accel, float final_speed, bool sensorEnable);
void Motion_CommandTurn(float angle_deg, float top_omega, float alpha, float final_omega);
bool Motion_CommandCurve(float arc_length_mm, float radius_mm, float top_speed_mms, float accel_mms2, float final_speed_mms);
bool Motion_CommandCurveAngle(float arc_angle_deg, float radius_mm, float top_speed_mms, float accel_mms2, float final_speed_mms);


#endif