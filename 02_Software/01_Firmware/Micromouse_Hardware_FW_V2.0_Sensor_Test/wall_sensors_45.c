#include "wall_sensors_45.h"
#include "config.h"
#include "BLEDebugger.h"

#if WALL_45_BACKEND == WALL_45_BACKEND_OPT3101
#include "opt3101.h"

#elif WALL_45_BACKEND == WALL_45_BACKEND_TOF
#include "TimeOfLight.h"

#elif WALL_45_BACKEND == WALL_45_BACKEND_ADC_IR
#include "sensor_normalize.h"

#else
#error "Invalid WALL_45_BACKEND selected"
#endif


static uint16_t s_left45_mm  = WALL45_INVALID_MM;
static uint16_t s_right45_mm = WALL45_INVALID_MM;


/*
 * Achtung:
 * Beim ADC-IR-Backend ist der Wert aktuell kein echter mm-Wert,
 * sondern ein skalierter ADC-Wert.
 *
 * Falls du später eine Distanzkalibrierung hast, muss hier von ADC -> mm
 * umgerechnet werden.
 */
static uint16_t WallSensors45_FloatToUint16(float value)
{
    if (value < 0.0f) {
        return WALL45_INVALID_MM;
    }

    if (value >= 65535.0f) {
        return 65534U;
    }

    return (uint16_t)(value + 0.5f);
}


void WallSensors45_Init(void)
{
    s_left45_mm  = WALL45_INVALID_MM;
    s_right45_mm = WALL45_INVALID_MM;

    printf("WallSensor Init\n");

#if WALL_45_BACKEND == WALL_45_BACKEND_OPT3101

    if (OPT3101_InitAll() != 0) {
        printf("OPT3101 Init Failed!\n");
    }

#elif WALL_45_BACKEND == WALL_45_BACKEND_TOF

    (void)VL6180X_StartContinuousAt(tof_sensor_addr[TOF_IDX_LEFT_45], 20);
    (void)VL6180X_StartContinuousAt(tof_sensor_addr[TOF_IDX_RIGHT_45], 20);

#elif WALL_45_BACKEND == WALL_45_BACKEND_ADC_IR

    IR_Init();

#endif
}


void WallSensors45_Update(void)
{
#if WALL_45_BACKEND == WALL_45_BACKEND_OPT3101

    uint16_t mm;

    OPT3101_UpdateAll();

    mm = OPT3101_ReadDistanceMm(OPT3101_LEFT_45);
    if (mm != OPT3101_INVALID_MM) {
        s_left45_mm = mm;
    }

    mm = OPT3101_ReadDistanceMm(OPT3101_RIGHT_45);
    if (mm != OPT3101_INVALID_MM) {
        s_right45_mm = mm;
    }

#elif WALL_45_BACKEND == WALL_45_BACKEND_TOF

    uint16_t mm;

    if (VL6180X_ReadContinuousIfReadyAt(tof_sensor_addr[TOF_IDX_LEFT_45], &mm) == TOF_OK) {
        s_left45_mm = mm;
    }

    if (VL6180X_ReadContinuousIfReadyAt(tof_sensor_addr[TOF_IDX_RIGHT_45], &mm) == TOF_OK) {
        s_right45_mm = mm;
    }

#elif WALL_45_BACKEND == WALL_45_BACKEND_ADC_IR

    float left_value;
    float right_value;

    IR_Update();

    left_value  = IR_ReadScaled(IR_SENSOR_LEFT_45);
    right_value = IR_ReadScaled(IR_SENSOR_RIGHT_45);

    s_left45_mm  = left_value;
    s_right45_mm = right_value;

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