#ifndef TIME_OF_FLIGHT_H
#define TIME_OF_FLIGHT_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* --- Configuration --- */
#define NUM_SENSORS             6
#define VL6180X_DEFAULT_ADDR    0x29
#define VL6180X_INVALID_RANGE   0xFFFF

/* --- Essential Register Map (Selected) --- */
#define VL6180X_REG_IDENTIFICATION_MODEL_ID          0x000
#define VL6180X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO     0x014
#define VL6180X_REG_SYSTEM_INTERRUPT_CLEAR           0x015
#define VL6180X_REG_SYSTEM_FRESH_OUT_OF_RESET        0x016
#define VL6180X_REG_SYSRANGE_START                   0x018
#define VL6180X_REG_SYSALS_START                     0x038
#define VL6180X_REG_RESULT_RANGE_VAL                 0x062
#define VL6180X_REG_RESULT_RANGE_STATUS              0x04D
#define VL6180X_REG_I2C_SLAVE_DEVICE_ADDRESS         0x212
#define VL6180X_REG_RESULT_INTERRUPT_STATUS_GPIO     0x04F
#define VL6180X_REG_SYSRANGE_MAX_CONVERGENCE_TIME    0x01C
#define VL6180X_REG_SYSRANGE_INTERMEASUREMENT_PERIOD 0x01B

typedef enum {
    TOF_OK = 0,
    TOF_ERR_I2C,
    TOF_ERR_TIMEOUT,
    TOF_ERR_PARAM,
    TOF_ERR_NOT_READY
} tof_err_t;

typedef struct {
    I2C_HandleTypeDef* hi2c;
    uint16_t dev_address;
    uint32_t timeout;
    uint8_t id;
} VL6180X_Handle_t;

extern I2C_HandleTypeDef hi2c2;
extern const uint8_t tof_sensor_addr[NUM_SENSORS];

void MX_I2C2_Init(void);
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c);

void Sensor_SHDN_Init(void);
int VL6180X_InitAll(void);
uint16_t VL6180X_ReadRangeSingleAt(uint8_t device_addr_7bit);
uint16_t VL6180X_ReadSensorByIndex(uint8_t idx);
int VL6180X_ReadAll(uint16_t out_mm[NUM_SENSORS]);
tof_err_t VL6180X_ReadRangeSingleAtEx(uint8_t addr, uint16_t *out_mm);
tof_err_t VL6180X_StartContinuousAt(uint8_t addr, uint8_t intermeasurement_ms);
tof_err_t VL6180X_StopContinuousAt(uint8_t addr);
tof_err_t VL6180X_ReadContinuousAt(uint8_t addr, uint16_t *out_mm);
tof_err_t VL6180X_ReadContinuousIfReadyAt(uint8_t addr, uint16_t *out_mm);

/* 45° continuous helpers */
void TOF_Continuous45_Init(void);
void TOF_Continuous45_Update(void);
uint16_t TOF_GetLeft45CachedMm(void);
uint16_t TOF_GetRight45CachedMm(void);
uint8_t TOF_Left45HasValidData(void);
uint8_t TOF_Right45HasValidData(void);

#endif