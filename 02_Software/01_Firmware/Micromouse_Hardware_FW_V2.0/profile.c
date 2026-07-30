/*************************************************************
Project		: Micromouse V1.2
File		: profile.h
Description	: Create velocity profiles instead of control position
Date		: 23.12.2024
*************************************************************/

#include "profile.h"
#include <math.h>
#include <stddef.h>
#include "Lock.h"



void Profile_Init(Profile* profile) {
    if (profile == NULL) return;
    profile->state = CS_IDLE;
    profile->speed = 0.0f;
    profile->position = 0.0f;
    profile->sign = 1;
    profile->acceleration = 0.0f;
    profile->one_over_acc = 1.0f;
    profile->target_speed = 0.0f;
    profile->final_speed = 0.0f;
    profile->final_position = 0.0f;
}

void Profile_Reset(Profile* profile) {
    if (profile == NULL) return;
    profile->position = 0.0f;
    profile->speed = 0.0f;
    profile->target_speed = 0.0f;
    profile->state = CS_IDLE;
}


void Profile_Start(Profile* profile, float distance, float top_speed, float final_speed, float acceleration)
{
    if (profile == NULL) return;

    profile->sign = (distance < 0.0f) ? -1 : 1;
    distance = fabsf(distance);

    top_speed = fabsf(top_speed);
    final_speed = fabsf(final_speed);
    acceleration = fabsf(acceleration);

    if (final_speed > top_speed) {
        final_speed = top_speed;
    }

    profile->position = 0.0f;
    profile->speed = 0.0f;
    profile->final_position = distance;
    profile->target_speed = profile->sign * top_speed;
    profile->final_speed = profile->sign * final_speed;
    profile->acceleration = acceleration;
    profile->one_over_acc = (acceleration >= 1.0f) ? (1.0f / acceleration) : 1.0f;
    profile->state = CS_ACCELERATING;
}

void Profile_Start_manual(Profile* profile, float distance, float top_speed, float final_speed, float acceleration, float position) {
    if (profile == NULL) return;
	
	if (final_speed > top_speed) {
			final_speed = top_speed;
	}

	lock_key_t k1 = lock_acquire();
	
	profile->position = position;
	profile->final_position = distance;
	profile->target_speed = top_speed;
	profile->final_speed = final_speed;
	profile->acceleration = acceleration;
	profile->one_over_acc = (acceleration >= 1.0f) ? 1.0f / acceleration : 1.0f;
	profile->state = CS_ACCELERATING;
	
	lock_release(k1);

}

void Profile_Stop(Profile* profile) {
    if (profile == NULL) return;
    profile->target_speed = 0.0f;
    Profile_Finish(profile);
}

void Profile_Finish(Profile* profile) {
    if (profile == NULL) return;
    profile->speed = profile->target_speed;
    profile->state = CS_FINISHED;
}

void Profile_Update(Profile* profile) {
	
	// Calculate time
//    static uint32_t prev_time_ms = 0;
//	const uint32_t MAX_LOOP_INTERVAL_MS = 12;
//	
//    uint32_t current_time_ms = HAL_GetTick();
//    uint32_t elapsedTime = current_time_ms - prev_time_ms;
//	
//	if (elapsedTime > MAX_LOOP_INTERVAL_MS) {
//        elapsedTime = MAX_LOOP_INTERVAL_MS;
//    }
	
//    float LOOP_INTERVAL = elapsedTime / 1000.0;
	
	
    if (profile == NULL || profile->state == CS_IDLE) return;

    float delta_v = profile->acceleration * LOOP_INTERVAL;
    float remaining = fabsf(profile->final_position) - fabsf(profile->position);

    if (profile->state == CS_ACCELERATING) {
        if (remaining < Profile_GetBrakingDistance(profile)) {
            profile->state = CS_BRAKING;
            profile->target_speed = (profile->final_speed == 0.0f) ? profile->sign * 15.0f : profile->final_speed;
        }
    }

    if (profile->speed < profile->target_speed) {
        profile->speed += delta_v;
        if (profile->speed > profile->target_speed) {
            profile->speed = profile->target_speed;
        }
    } else if (profile->speed > profile->target_speed) {
        profile->speed -= delta_v;
        if (profile->speed < profile->target_speed) {
            profile->speed = profile->target_speed;
        }
    }
		
		profile->position += profile->speed * LOOP_INTERVAL;

    if (profile->state != CS_FINISHED && remaining < 0.125f) {
        profile->state = CS_FINISHED;
        profile->target_speed = profile->final_speed;
    }
	
	// Debugging
//	PY_Printf("Intervall: %f", LOOP_INTERVAL);
//	PY_Printf("Position: %f", profile->position);
//	PY_Printf("Speed: %f", profile->speed);
	
	
	// Update previous time
//    prev_time_ms = current_time_ms;
}

bool Profile_IsFinished(Profile* profile) {
    return profile != NULL && profile->state == CS_FINISHED;
}

float Profile_GetBrakingDistance(Profile* profile) {
    if (profile == NULL) return 0.0f;
    return fabsf(profile->speed * profile->speed - profile->final_speed * profile->final_speed) * 0.5f * profile->one_over_acc;
}

float Profile_GetPosition(Profile* profile) {
    if (profile == NULL) return 0.0f;
    return profile->position;
}

float Profile_GetSpeed(Profile* profile) {
    if (profile == NULL) return 0.0f;
    return profile->speed;
}

//float Profile_GetIncrement(Profile* profile) {
//    if (profile == NULL) return 0.0f;
//    return profile->speed * LOOP_INTERVAL;
//}

void Profile_SetSpeed(Profile* profile, float speed) {
    if (profile == NULL) return;
    profile->speed = speed;
}

void Profile_SetTargetSpeed(Profile* profile, float speed) {
    if (profile == NULL) return;
    profile->target_speed = speed;
}

void Profile_AdjustPosition(Profile* profile, float adjustment) {
    if (profile == NULL) return;
    profile->position += adjustment;
}

void Profile_SetPosition(Profile* profile, float position) {
    if (profile == NULL) return;
    profile->position = position;
}

void Profile_SetFinalPosition(Profile* profile, float position) {
    if (profile == NULL) return;
    profile->final_position = position;
}

float Profile_GetRemainingDistance(Profile* profile){
	if (profile == NULL) return 0.0f;
	return fabsf(profile->final_position) - fabsf(profile->position);
	
}

void Profile_SetAcceleration(Profile* profile, float acceleration){
	if (profile == NULL) return;
	profile->acceleration = acceleration;
}
