#ifndef WALL_SENSORS_TOF_H
#define WALL_SENSORS_TOF_H

#include "wall_sensors.h"
#include <stdbool.h>

void WallSensors_ToF_Init(void);
void WallSensors_ToF_Update(void);
float WallSensors_ToF_Read(wall_sensor_id_t id);
bool WallSensors_ToF_CheckFrontWall(void);
bool WallSensors_ToF_CheckFrontMiddle(void);

#endif