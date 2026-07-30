#ifndef WALL_SENSORS_45_H
#define WALL_SENSORS_45_H

#include <stdint.h>

#define WALL45_INVALID_MM   0xFFFFU

void WallSensors45_Init(void);
void WallSensors45_Update(void);

uint16_t WallSensors45_ReadLeftMm(void);
uint16_t WallSensors45_ReadRightMm(void);

#endif