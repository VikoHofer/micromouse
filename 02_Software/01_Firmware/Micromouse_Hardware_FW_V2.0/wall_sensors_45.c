#include "wall_sensors_45.h"
#include "config.h"
#include "TimeOfLight.h"
#include "BLEDebugger.h"

#if WALL_45_BACKEND == WALL_45_BACKEND_OPT3101
#include "opt3101.h"
#endif

static uint16_t s_left45_mm  = WALL45_INVALID_MM;
static uint16_t s_right45_mm = WALL45_INVALID_MM;

void WallSensors45_Init(void)
{
    s_left45_mm  = WALL45_INVALID_MM;
    s_right45_mm = WALL45_INVALID_MM;
	
	printf("WallSensor Init\n");
#if WALL_45_BACKEND == WALL_45_BACKEND_OPT3101

    if(OPT3101_InitAll() != 0){
		printf("OPT3101 Init Failed!");
	}

#else

    (void)VL6180X_StartContinuousAt(tof_sensor_addr[TOF_IDX_LEFT_45], 20);
    (void)VL6180X_StartContinuousAt(tof_sensor_addr[TOF_IDX_RIGHT_45], 20);

#endif
}

void WallSensors45_Update(void)
{
    uint16_t mm;

#if WALL_45_BACKEND == WALL_45_BACKEND_OPT3101

    OPT3101_UpdateAll();

    mm = OPT3101_ReadDistanceMm(OPT3101_LEFT_45);
    if (mm != OPT3101_INVALID_MM) {
        s_left45_mm = mm;
    }

    mm = OPT3101_ReadDistanceMm(OPT3101_RIGHT_45);
    if (mm != OPT3101_INVALID_MM) {
        s_right45_mm = mm;
    }

#else

    if (VL6180X_ReadContinuousIfReadyAt(tof_sensor_addr[TOF_IDX_LEFT_45], &mm) == TOF_OK) {
        s_left45_mm = mm;
    }

    if (VL6180X_ReadContinuousIfReadyAt(tof_sensor_addr[TOF_IDX_RIGHT_45], &mm) == TOF_OK) {
        s_right45_mm = mm;
    }

#endif
}

uint16_t WallSensors45_ReadLeftMm(void)
{
    return s_left45_mm;
}

uint16_t WallSensors45_ReadRightMm(void)
{
    return s_right45_mm;
}