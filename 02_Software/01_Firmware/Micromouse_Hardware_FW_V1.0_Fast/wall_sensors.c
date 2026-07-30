#include "wall_sensors.h"
#include "config.h"
#include "adc.h"

#if TARGET_SENSOR_HW == SENSOR_HW_TOF
#include "wall_sensors_tof.h"
#elif TARGET_SENSOR_HW == SENSOR_HW_IR
#include "wall_sensors_ir.h"
#else
#error "Unknown TARGET_SENSOR_HW"
#endif

void WallSensors_Init(void)
{
#if TARGET_SENSOR_HW == SENSOR_HW_TOF
    WallSensors_ToF_Init();
#elif TARGET_SENSOR_HW == SENSOR_HW_IR
	ADC_Init(&hadc1);
    WallSensors_IR_Init();
#endif
}

void WallSensors_Update(void)
{
#if TARGET_SENSOR_HW == SENSOR_HW_TOF
    WallSensors_ToF_Update();
#elif TARGET_SENSOR_HW == SENSOR_HW_IR
    WallSensors_IR_Update();
#endif
}

float WallSensors_Read(wall_sensor_id_t id)
{
#if TARGET_SENSOR_HW == SENSOR_HW_TOF
    return WallSensors_ToF_Read(id);
#elif TARGET_SENSOR_HW == SENSOR_HW_IR
    return WallSensors_IR_Read(id);
#else
    (void)id;
    return -1.0f;
#endif
}

bool WallSensors_CheckFrontWall(void)
{
#if TARGET_SENSOR_HW == SENSOR_HW_TOF
    return WallSensors_ToF_CheckFrontWall();
#elif TARGET_SENSOR_HW == SENSOR_HW_IR
    return WallSensors_IR_CheckFrontWall();
#else
    return false;
#endif
}

bool WallSensors_CheckFrontMiddle(void)
{
#if TARGET_SENSOR_HW == SENSOR_HW_TOF
    return WallSensors_ToF_CheckFrontMiddle();
#elif TARGET_SENSOR_HW == SENSOR_HW_IR
    return WallSensors_IR_CheckFrontMiddle();
#else
    return false;
#endif
}