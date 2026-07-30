#ifndef WALL_SENSORS_H
#define WALL_SENSORS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    WALL_SENSOR_LEFT = 5,
    WALL_SENSOR_LEFT_45 = 2,
    WALL_SENSOR_FRONT_1 = 0,
    WALL_SENSOR_FRONT_2 = 1,
    WALL_SENSOR_RIGHT_45 = 3,
    WALL_SENSOR_RIGHT = 4
} wall_sensor_id_t;

/* Gemeinsame Hardware-unabhängige API */
void WallSensors_Init(void);
void WallSensors_Update(void);

float WallSensors_Read(wall_sensor_id_t id);
bool WallSensors_CheckFrontWall(void);
bool WallSensors_CheckFrontMiddle(void);

/* ----------------------------------------------------------
 * Legacy / top-level API
 * Diese Namen dürfen im restlichen Projekt weiter benutzt werden.
 * ---------------------------------------------------------- */

static inline double readSensor_Left(void)
{
    return (double)WallSensors_Read(WALL_SENSOR_LEFT);
}

static inline double readSensor_Left_45(void)
{
    return (double)WallSensors_Read(WALL_SENSOR_LEFT_45);
}

static inline double readSensor_Right(void)
{
    return (double)WallSensors_Read(WALL_SENSOR_RIGHT);
}

static inline double readSensor_Right_45(void)
{
    return (double)WallSensors_Read(WALL_SENSOR_RIGHT_45);
}

static inline double readSensor_Front(void)
{
    return (double)WallSensors_Read(WALL_SENSOR_FRONT_1);
}

static inline double readSensor_Front2(void)
{
    return (double)WallSensors_Read(WALL_SENSOR_FRONT_2);
}

static inline bool check_front_wall(void)
{
    return WallSensors_CheckFrontWall();
}

static inline bool check_front_middle(void)
{
    return WallSensors_CheckFrontMiddle();
}

#endif