/*************************************************************
Project		: Micromouse V1.2
File		: profile.h
Description	: Create velocity profiles instead of control position
Date		: 23.12.2024
*************************************************************/

#ifndef PROFILE_H
#define PROFILE_H

#include <stdint.h>
#include <stdbool.h>
#include "Delay.h"
#include "config.h"


// Enumerationen für den Zustand des Profils
typedef enum {
    CS_IDLE = 0,
    CS_ACCELERATING,
    CS_BRAKING,
    CS_FINISHED
} ProfileState;

// Strukturdefinition für ein Geschwindigkeitsprofil
typedef struct {
    volatile uint8_t state;
    volatile float speed;
    volatile float position;
    int8_t sign;
    float acceleration;
    float one_over_acc;
    float target_speed;
    float final_speed;
    float final_position;
} Profile;

// Funktionsprototypen
void Profile_Init(Profile* profile);
void Profile_Reset(Profile* profile);
void Profile_Start(Profile* profile, float distance, float top_speed, float final_speed, float acceleration);
void Profile_Start_manual(Profile* profile, float distance, float top_speed, float final_speed, float acceleration, float position);
void Profile_Stop(Profile* profile);
void Profile_Finish(Profile* profile);
void Profile_Update(Profile* profile);
float Profile_GetBrakingDistance(Profile* profile);
float Profile_GetPosition(Profile* profile);
float Profile_GetSpeed(Profile* profile);
//float Profile_GetIncrement(Profile* profile);
float Profile_GetAcceleration(Profile* profile);
void Profile_SetSpeed(Profile* profile, float speed);
void Profile_SetTargetSpeed(Profile* profile, float speed);
void Profile_AdjustPosition(Profile* profile, float adjustment);
void Profile_SetPosition(Profile* profile, float position);
bool Profile_IsFinished(Profile* profile);


#endif