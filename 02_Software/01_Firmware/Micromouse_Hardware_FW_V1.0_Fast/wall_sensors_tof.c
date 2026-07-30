#include "wall_sensors_tof.h"
#include "TimeOfLight.h"
#include "config.h"

#ifndef TOF_IDX_LEFT
#define TOF_IDX_LEFT       3U
#endif

#ifndef TOF_IDX_LEFT_45
#define TOF_IDX_LEFT_45    4U
#endif

#ifndef TOF_IDX_FRONT_1
#define TOF_IDX_FRONT_1    0U
#endif

#ifndef TOF_IDX_FRONT_2
#define TOF_IDX_FRONT_2    5U
#endif

#ifndef TOF_IDX_RIGHT_45
#define TOF_IDX_RIGHT_45   1U
#endif

#ifndef TOF_IDX_RIGHT
#define TOF_IDX_RIGHT      2U
#endif

#ifndef TOF_FRONT_WALL_THRESHOLD_MM
#define TOF_FRONT_WALL_THRESHOLD_MM    90.0f
#endif

#ifndef TOF_FRONT_MIDDLE_THRESHOLD_MM
#define TOF_FRONT_MIDDLE_THRESHOLD_MM  65.0f
#endif

static float tof_mm_to_float(uint16_t mm)
{
    if (mm == VL6180X_INVALID_RANGE) {
        return -1.0f;
    }
    return (float)mm;
}

static float tof_read_direct(uint8_t idx)
{
    return tof_mm_to_float(VL6180X_ReadSensorByIndex(idx));
}

void WallSensors_ToF_Init(void)
{
    MX_I2C2_Init();
    (void)VL6180X_InitAll();
    TOF_Continuous45_Init();
}

void WallSensors_ToF_Update(void)
{
    TOF_Continuous45_Update();
}

float WallSensors_ToF_Read(wall_sensor_id_t id)
{
    switch (id)
    {
        case WALL_SENSOR_LEFT:
            return tof_read_direct(TOF_IDX_LEFT);

        case WALL_SENSOR_LEFT_45:
            if (TOF_Left45HasValidData()) {
                return tof_mm_to_float(TOF_GetLeft45CachedMm());
            }
            return tof_read_direct(TOF_IDX_LEFT_45);

        case WALL_SENSOR_FRONT_1:
            return tof_read_direct(TOF_IDX_FRONT_1);

        case WALL_SENSOR_FRONT_2:
            return tof_read_direct(TOF_IDX_FRONT_2);

        case WALL_SENSOR_RIGHT_45:
            if (TOF_Right45HasValidData()) {
                return tof_mm_to_float(TOF_GetRight45CachedMm());
            }
            return tof_read_direct(TOF_IDX_RIGHT_45);

        case WALL_SENSOR_RIGHT:
            return tof_read_direct(TOF_IDX_RIGHT);

        default:
            return -1.0f;
    }
}

bool WallSensors_ToF_CheckFrontWall(void)
{
    float d1 = WallSensors_ToF_Read(WALL_SENSOR_FRONT_1);
    float d2 = WallSensors_ToF_Read(WALL_SENSOR_FRONT_2);

    if (d1 < 0.0f && d2 < 0.0f) {
        return false;
    }

    if (d1 >= 0.0f && d1 < TOF_FRONT_WALL_THRESHOLD_MM) {
        return true;
    }

    if (d2 >= 0.0f && d2 < TOF_FRONT_WALL_THRESHOLD_MM) {
        return true;
    }

    return false;
}

bool WallSensors_ToF_CheckFrontMiddle(void)
{
    float d1 = WallSensors_ToF_Read(WALL_SENSOR_FRONT_1);
    float d2 = WallSensors_ToF_Read(WALL_SENSOR_FRONT_2);

    if (d1 < 0.0f && d2 < 0.0f) {
        return false;
    }

    if (d1 >= 0.0f && d2 >= 0.0f) {
        float middle = 0.5f * (d1 + d2);
        return (middle < TOF_FRONT_MIDDLE_THRESHOLD_MM);
    }

    if (d1 >= 0.0f) {
        return (d1 < TOF_FRONT_MIDDLE_THRESHOLD_MM);
    }

    return (d2 < TOF_FRONT_MIDDLE_THRESHOLD_MM);
}