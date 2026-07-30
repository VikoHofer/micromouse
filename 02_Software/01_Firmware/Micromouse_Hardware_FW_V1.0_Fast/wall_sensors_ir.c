#include "wall_sensors_ir.h"
#include "sensor_normalize.h"
#include "config.h"
#include "adc.h"

void WallSensors_IR_Init(void)
{
	ADC_Init(&hadc1);
    IR_Init();
    IR_Update();
    
}

void WallSensors_IR_Update(void)
{
    IR_Update();
}

float WallSensors_IR_Read(wall_sensor_id_t id)
{
    switch (id)
    {
        case WALL_SENSOR_LEFT_45:
            return IR_ReadScaled(IR_SENSOR_LEFT_45);

        case WALL_SENSOR_RIGHT_45:
            return IR_ReadScaled(IR_SENSOR_RIGHT_45);

        default:
            return IR_INVALID_VALUE;
    }
}

bool WallSensors_IR_CheckFrontWall(void)
{
    float fl = IR_ReadHardwareScaled(IR_SENSOR_FRONT_LEFT);
    float fr = IR_ReadHardwareScaled(IR_SENSOR_FRONT_RIGHT);

    if (fl < 0.0f || fr < 0.0f) {
        return false;
    }

    return ((fl + fr) > FRONT_THRESHOLD);
}

bool WallSensors_IR_CheckFrontMiddle(void)
{
    float fl = IR_ReadHardwareScaled(IR_SENSOR_FRONT_LEFT);
    float fr = IR_ReadHardwareScaled(IR_SENSOR_FRONT_RIGHT);

    if (fl < 0.0f || fr < 0.0f) {
        return false;
    }

    return ((fl + fr) > FRONT_MIDDLE_THRESHOLD);
}