#include "wall_following_ir.h"
#include "wall_sensors_ir.h"
#include "config.h"
#include "BLEDebugger.h"
#include <math.h>
#include <stdbool.h>

static float s_error_filt = 0.0f;
static float s_error_prev = 0.0f;

typedef struct
{
    float v_mms;
    float kp;
    float kd;
} WallGain_t;

static const WallGain_t s_wall_gains[] =
{
    {  200.0f, 3.5f, 0.5f },
    {  600.0f, 4.0f, 1.0f },
    { 1000.0f, 5.0f, 1.0f },
    { 1400.0f, 5.0f, 1.0f },
    { 1500.0f, 6.0f, 1.5f },
    { 1700.0f, 6.0f, 3.0f },
    { 2000.0f, 6.5f, 4.0f },
};

#define WALL_GAIN_COUNT \
    ((uint32_t)(sizeof(s_wall_gains) / sizeof(s_wall_gains[0])))
		

void WallFollowing_IR_Init(void)
{
    s_error_filt = 0.0f;
    s_error_prev = 0.0f;
}

static void WallFollowing_GetGains(float velocity_mms, float *kp, float *kd)
{
    float v = fabsf(velocity_mms);

    if (v <= s_wall_gains[0].v_mms)
    {
        *kp = s_wall_gains[0].kp;
        *kd = s_wall_gains[0].kd;
        return;
    }

    for (uint32_t i = 0U; i < WALL_GAIN_COUNT - 1U; i++)
    {
        const WallGain_t *g0 = &s_wall_gains[i];
        const WallGain_t *g1 = &s_wall_gains[i + 1U];

        if (v <= g1->v_mms)
        {
            float t = (v - g0->v_mms) / (g1->v_mms - g0->v_mms);

            *kp = g0->kp + t * (g1->kp - g0->kp);
            *kd = g0->kd + t * (g1->kd - g0->kd);
            return;
        }
    }

    *kp = s_wall_gains[WALL_GAIN_COUNT - 1U].kp;
    *kd = s_wall_gains[WALL_GAIN_COUNT - 1U].kd;
}




float WallFollowing_IR_GetCorrectionDps(float velocity_mms)
{
    float left_val;
    float right_val;
    bool left_valid;
    bool right_valid;

    float left_error = 0.0f;
    float right_error = 0.0f;
    float error = 0.0f;
    float derror;
    float adjust_dps;

	
		
	// adaptiv KP and KD
	float kp;
	float kd;

	
	
    left_val  = WallSensors_IR_Read(WALL_SENSOR_LEFT_45);
    right_val = WallSensors_IR_Read(WALL_SENSOR_RIGHT_45);
	

    left_valid  = (left_val  >= IR_WALL_VALID_MIN) &&
                  (left_val  <= IR_WALL_VALID_MAX);

    right_valid = (right_val >= IR_WALL_VALID_MIN) &&
                  (right_val <= IR_WALL_VALID_MAX);

    if (!left_valid && !right_valid)
    {
        s_error_filt = 0.0f;
        s_error_prev = 0.0f;
        return 0.0f;
    }

    if (left_valid) {
        left_error = left_val - IR_WALL_LEFT_REF;
    }

    if (right_valid) {
        right_error = IR_WALL_RIGHT_REF - right_val;
    }

    if (left_valid && right_valid) {
        error = 0.5f * (left_error + right_error);
    } else if (left_valid) {
        error = left_error;
    } else {
        error = right_error;
    }

    if (fabsf(error) < IR_WALL_DEADBAND) {
        error = 0.0f;
    }

    s_error_filt = IR_WALL_FILTER_ALPHA * s_error_filt +
                   (1.0f - IR_WALL_FILTER_ALPHA) * error;

    derror = (s_error_filt - s_error_prev) / CONTROL_PERIOD_S;
    s_error_prev = s_error_filt;

    adjust_dps = IR_WALL_KP * s_error_filt + IR_WALL_KD * derror;
	
	// adaptiv KP and KD
//	WallFollowing_GetGains(velocity_mms, &kp, &kd);
//	adjust_dps = kp * s_error_filt + kd * derror;
	
    if (adjust_dps > IR_WALL_MAX_ADJUST_DPS) {
        adjust_dps = IR_WALL_MAX_ADJUST_DPS;
    }
    if (adjust_dps < -IR_WALL_MAX_ADJUST_DPS) {
        adjust_dps = -IR_WALL_MAX_ADJUST_DPS;
    }
//	printf("L=%.1f R=%.1f validL=%d validR=%d err=%.2f steer=%.2f\n",
//       left_val, right_val, left_valid, right_valid,
//       error, adjust_dps);

    return adjust_dps;
}