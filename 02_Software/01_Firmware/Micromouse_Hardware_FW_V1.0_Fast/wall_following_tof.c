#include "wall_following_tof.h"
#include "wall_sensors.h"
#include "config.h"
#include <math.h>
#include <stdbool.h>

static float s_error_filt = 0.0f;
static float s_error_prev = 0.0f;

void WallFollowing_ToF_Init(void)
{
    s_error_filt = 0.0f;
    s_error_prev = 0.0f;
}

void WallFollowing_ToF_Update(void)
{
    /* keine eigene Cachelogik hier;
       Sensorcache wird in WallSensors_ToF_Update() gepflegt */
}

float WallFollowing_ToF_GetCorrectionDps(void)
{
    float left_val;
    float right_val;
    bool left_valid;
    bool right_valid;

    float left_error = 0.0f;
    float right_error = 0.0f;
    float error_mm = 0.0f;
    float derror;
    float adjust_dps;

    left_val  = WallSensors_Read(WALL_SENSOR_LEFT_45);
    right_val = WallSensors_Read(WALL_SENSOR_RIGHT_45);

    left_valid  = (left_val  >= (float)TOF_WALL_VALID_MIN_MM) &&
                  (left_val  <= (float)TOF_WALL_VALID_MAX_MM);

    right_valid = (right_val >= (float)TOF_WALL_VALID_MIN_MM) &&
                  (right_val <= (float)TOF_WALL_VALID_MAX_MM);

    if (!left_valid && !right_valid)
    {
        s_error_filt = 0.0f;
        s_error_prev = 0.0f;
        return 0.0f;
    }

    if (left_valid) {
        left_error = left_val - TOF_LEFT_MIDDLE_MM;
    }

    if (right_valid) {
        right_error = TOF_RIGHT_MIDDLE_MM - right_val;
    }

    if (left_valid && right_valid) {
        error_mm = 0.5f * (left_error + right_error);
    } else if (left_valid) {
        error_mm = left_error;
    } else {
        error_mm = right_error;
    }

    if (fabsf(error_mm) < TOF_WALL_DEADBAND_MM) {
        error_mm = 0.0f;
    }

    s_error_filt = TOF_WALL_FILTER_ALPHA * s_error_filt +
                   (1.0f - TOF_WALL_FILTER_ALPHA) * error_mm;

    derror = (s_error_filt - s_error_prev) / CONTROL_PERIOD_S;
    s_error_prev = s_error_filt;

    adjust_dps = TOF_WALL_KP * s_error_filt + TOF_WALL_KD * derror;

    if (adjust_dps > TOF_WALL_MAX_ADJUST_DPS) {
        adjust_dps = TOF_WALL_MAX_ADJUST_DPS;
    }
    if (adjust_dps < -TOF_WALL_MAX_ADJUST_DPS) {
        adjust_dps = -TOF_WALL_MAX_ADJUST_DPS;
    }

    return adjust_dps;
}