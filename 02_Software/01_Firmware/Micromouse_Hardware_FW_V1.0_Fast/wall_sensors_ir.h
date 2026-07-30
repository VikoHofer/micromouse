#ifndef WALL_SENSORS_IR_H
#define WALL_SENSORS_IR_H

#include <stdbool.h>

typedef enum
{
    WALL_SENSOR_LEFT = 5,
    WALL_SENSOR_LEFT_45 = 2,
    WALL_SENSOR_FRONT_1 = 0,
    WALL_SENSOR_FRONT_2 = 1,
    WALL_SENSOR_RIGHT_45 = 3,
    WALL_SENSOR_RIGHT = 4
} wall_sensor_id_t;


// Initialize ADC and Sensors
void WallSensors_IR_Init(void);

// Update sensor cache 
void WallSensors_IR_Update(void);

// Read Cached Sensors -> non blocking read
float WallSensors_IR_Read(wall_sensor_id_t id);

// Check if there is a wall 
bool WallSensors_IR_CheckFrontWall(void);
bool WallSensors_IR_CheckFrontMiddle(void);

#endif