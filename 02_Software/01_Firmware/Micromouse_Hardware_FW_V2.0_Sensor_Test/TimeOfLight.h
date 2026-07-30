#ifndef TIME_OF_FLIGHT_H
#define TIME_OF_FLIGHT_H

#include "stm32f4xx_hal.h"
#include "wall_sensors_45.h"
#include <stdint.h>
#include <stdbool.h>

/* --- Configuration --- */
#define NUM_SENSORS             6
#define VL6180X_DEFAULT_ADDR    0x29
#define VL6180X_INVALID_RANGE   0xFFFF

/* --- Essential Register Map (Selected) --- */
#define VL6180X_REG_IDENTIFICATION_MODEL_ID         0x000
#define VL6180X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO    0x014
#define VL6180X_REG_SYSTEM_INTERRUPT_CLEAR          0x015
#define VL6180X_REG_SYSTEM_FRESH_OUT_OF_RESET       0x016
#define VL6180X_REG_SYSRANGE_START                  0x018
#define VL6180X_REG_SYSALS_START                    0x038
#define VL6180X_REG_RESULT_RANGE_VAL                0x062
#define VL6180X_REG_RESULT_RANGE_STATUS             0x04D
#define VL6180X_REG_I2C_SLAVE_DEVICE_ADDRESS        0x212
#define VL6180X_REG_RESULT_INTERRUPT_STATUS_GPIO    0x04F
#define VL6180X_REG_SYSRANGE_MAX_CONVERGENCE_TIME   0x01C
#define VL6180X_REG_SYSRANGE_INTERMEASUREMENT_PERIOD 0x01B

/**
 * @brief Error codes for TOF operations
 */
typedef enum {
    TOF_OK = 0,
    TOF_ERR_I2C,
    TOF_ERR_TIMEOUT,
    TOF_ERR_PARAM,
    TOF_ERR_NOT_READY
} tof_err_t;

/**
 * @brief Handle for a single VL6180X sensor instance
 */
typedef struct {
    I2C_HandleTypeDef* hi2c;
    uint16_t dev_address;   // 7-bit address left-shifted once
    uint32_t timeout;
    uint8_t id;             // User-assigned sensor ID (0 to NUM_SENSORS-1)
} VL6180X_Handle_t;

/* Global I2C handle */
extern I2C_HandleTypeDef hi2c2;

/* Sensor addresses (7-bit) */
extern const uint8_t tof_sensor_addr[NUM_SENSORS];

/* I2C init */
void MX_I2C2_Init(void);
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c);

/* --- Driver API --- */
void Sensor_SHDN_Init(void);
int VL6180X_InitAll(void);
int VL6180X_InitMask(uint32_t active_mask);
uint16_t VL6180X_ReadRangeSingleAt(uint8_t device_addr_7bit);
uint16_t VL6180X_ReadSensorByIndex(uint8_t idx);
int VL6180X_ReadAll(uint16_t out_mm[NUM_SENSORS]);
tof_err_t VL6180X_ReadRangeSingleAtEx(uint8_t addr, uint16_t *out_mm);
tof_err_t VL6180X_StartContinuousAt(uint8_t addr, uint8_t intermeasurement_ms);
tof_err_t VL6180X_StopContinuousAt(uint8_t addr);
tof_err_t VL6180X_ReadContinuousAt(uint8_t addr, uint16_t *out_mm);
tof_err_t VL6180X_ReadContinuousIfReadyAt(uint8_t addr, uint16_t *out_mm);

#endif /* TIME_OF_FLIGHT_H */


/* ============================================================
 *  Legacy Floodfill Wrapper
 *  Alte API-Namen -> neue VL6180X-API
 * ============================================================
 *
 * Passe diese Indizes an deine reale Sensorreihenfolge an.
 * Beispielbelegung:
 *   0 = Left
 *   1 = Left45
 *   2 = Front
 *   3 = Front2
 *   4 = Right45
 *   5 = Right
 */
#ifndef TOF_IDX_LEFT
#define TOF_IDX_LEFT       3
#endif

#ifndef TOF_IDX_LEFT_45
#define TOF_IDX_LEFT_45    4
#endif

#ifndef TOF_IDX_FRONT
#define TOF_IDX_FRONT      0
#endif

#ifndef TOF_IDX_FRONT2
#define TOF_IDX_FRONT2     5
#endif

#ifndef TOF_IDX_RIGHT_45
#define TOF_IDX_RIGHT_45   1
#endif

#ifndef TOF_IDX_RIGHT
#define TOF_IDX_RIGHT      2
#endif

#define TOF_SENSOR_MASK(idx) (1UL << (idx))
// Diagonale sensoren deaktivieren bei Verwendung von OPT3101
#if (WALL_45_BACKEND == WALL_45_BACKEND_OPT3101) || \
    (WALL_45_BACKEND == WALL_45_BACKEND_ADC_IR)

#define TOF_ACTIVE_MASK \
    (TOF_SENSOR_MASK(TOF_IDX_LEFT)   | \
     TOF_SENSOR_MASK(TOF_IDX_FRONT)  | \
     TOF_SENSOR_MASK(TOF_IDX_FRONT2) | \
     TOF_SENSOR_MASK(TOF_IDX_RIGHT))

#else

#define TOF_ACTIVE_MASK \
    (TOF_SENSOR_MASK(TOF_IDX_LEFT)     | \
     TOF_SENSOR_MASK(TOF_IDX_LEFT_45)  | \
     TOF_SENSOR_MASK(TOF_IDX_FRONT)    | \
     TOF_SENSOR_MASK(TOF_IDX_FRONT2)   | \
     TOF_SENSOR_MASK(TOF_IDX_RIGHT_45) | \
     TOF_SENSOR_MASK(TOF_IDX_RIGHT))

#endif


/* Schwellwerte für Wallerkennung */
#ifndef TOF_FRONT_WALL_THRESHOLD_MM
#define TOF_FRONT_WALL_THRESHOLD_MM    90.0
#endif

#ifndef TOF_FRONT_MIDDLE_THRESHOLD_MM
#define TOF_FRONT_MIDDLE_THRESHOLD_MM  65.0
#endif


/* Optionaler Rückgabewert bei Sensorfehler */
#ifndef TOF_LEGACY_INVALID_RETURN
#define TOF_LEGACY_INVALID_RETURN      (-1.0)
#endif


static inline double TOF_ReadSensorLegacy(uint8_t idx)
{
    uint16_t mm = VL6180X_ReadSensorByIndex(idx);

    if (mm == VL6180X_INVALID_RANGE) {
        return TOF_LEGACY_INVALID_RETURN;
    }

    return (double)mm;
}

static inline double readSensor_Left_45(void)
{
    uint16_t mm = WallSensors45_ReadLeftMm();

    if (mm == WALL45_INVALID_MM) {
        return TOF_LEGACY_INVALID_RETURN;
    }

    return (double)mm;
}

static inline double readSensor_Right_45(void)
{
    uint16_t mm = WallSensors45_ReadRightMm();

    if (mm == WALL45_INVALID_MM) {
        return TOF_LEGACY_INVALID_RETURN;
    }

    return (double)mm;
}


/* PA0 */
// duration 5ms!!
static inline double readSensor_Left(void)
{
    return TOF_ReadSensorLegacy(TOF_IDX_LEFT);
}


/* PA1 */
// duration 5ms!!
static inline double readSensor_Right(void)
{
    return TOF_ReadSensorLegacy(TOF_IDX_RIGHT);
}


/* PB0 */
static inline double readSensor_Front(void)
{
    return TOF_ReadSensorLegacy(TOF_IDX_FRONT);
}

static inline double readSensor_Front2(void)
{
    return TOF_ReadSensorLegacy(TOF_IDX_FRONT2);
}


/*
 * Wandprüfung vorne
 *
 * Aktuell:
 *   true  = Wand erkannt
 *   false = keine Wand / ungültige Messung
 *
 * Falls dein alter Floodfill die Logik invertiert erwartet,
 * einfach '!check_front_wall()' verwenden oder die Funktion anpassen.
 */
static inline bool check_front_wall(void)
{
    double d1 = readSensor_Front();
    double d2 = readSensor_Front2();

    if (d1 < 0.0 && d2 < 0.0) {
        return false;
    }

    if (d1 >= 0.0 && d1 < TOF_FRONT_WALL_THRESHOLD_MM) {
        return true;
    }

    if (d2 >= 0.0 && d2 < TOF_FRONT_WALL_THRESHOLD_MM) {
        return true;
    }

    return false;
}

static inline bool check_front_middle(void)
{
    double d1 = readSensor_Front();
    double d2 = readSensor_Front2();

    if (d1 < 0.0 && d2 < 0.0) {
        return false;
    }

    if (d1 >= 0.0 && d2 >= 0.0) {
        double middle = 0.5 * (d1 + d2);
        return (middle < TOF_FRONT_MIDDLE_THRESHOLD_MM);
    }

    if (d1 >= 0.0) {
        return (d1 < TOF_FRONT_MIDDLE_THRESHOLD_MM);
    }

    return (d2 < TOF_FRONT_MIDDLE_THRESHOLD_MM);
}