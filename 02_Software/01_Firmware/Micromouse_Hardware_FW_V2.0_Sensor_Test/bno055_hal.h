#ifndef BNO055_HAL_H
#define BNO055_HAL_H
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h> 

#define BNO055_ADDRESS_A 0x28
#define BNO055_ADDRESS_B 0x29
#define BNO055_ID        0xA0 

#define BNO055_CHIP_ID_ADDR               0x00
#define BNO055_PAGE_ID_ADDR               0x07
#define BNO055_ACCEL_DATA_X_LSB_ADDR        0x08
#define BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR 0x28
#define BNO055_MAG_DATA_X_LSB_ADDR        0x0E
#define BNO055_GYRO_DATA_X_LSB_ADDR       0x14
#define BNO055_EULER_H_LSB_ADDR           0x1A
#define BNO055_QUATERNION_DATA_W_LSB_ADDR 0x20
#define BNO055_CALIB_STAT_ADDR            0x35
#define BNO055_SYS_STAT_ADDR              0x39
#define BNO055_SYS_ERR_ADDR               0x3A
#define BNO055_OPR_MODE_ADDR              0x3D
#define BNO055_PWR_MODE_ADDR              0x3E
#define BNO055_SYS_TRIGGER_ADDR           0x3F 


typedef enum {
    OPERATION_MODE_CONFIG        = 0x00,
    OPERATION_MODE_ACCONLY       = 0x01,
    OPERATION_MODE_MAGONLY       = 0x02,
    OPERATION_MODE_GYRONLY       = 0x03,
    OPERATION_MODE_ACCMAG        = 0x04,
    OPERATION_MODE_ACCGYRO       = 0x05,
    OPERATION_MODE_MAGGYRO       = 0x06,
    OPERATION_MODE_AMG           = 0x07,
    OPERATION_MODE_IMUPLUS       = 0x08,
    OPERATION_MODE_COMPASS       = 0x09,
    OPERATION_MODE_M4G           = 0x0A,
    OPERATION_MODE_NDOF_FMC_OFF  = 0x0B,
    OPERATION_MODE_NDOF          = 0x0C
} bno055_opmode_t; 

typedef enum {
    POWER_MODE_NORMAL   = 0x00,
    POWER_MODE_LOWPOWER = 0x01,
    POWER_MODE_SUSPEND  = 0x02
} bno055_powermode_t; 

typedef struct { int16_t x, y, z; } bno055_vector_t;
typedef struct { int16_t w, x, y, z; } bno055_quaternion_t;
typedef struct { uint8_t system, gyro, accel, mag; } bno055_calibration_t; 

typedef struct {
    I2C_HandleTypeDef* hi2c;
    uint16_t dev_address;   // 7-bit address left-shifted once (HAL expects this)
    bno055_opmode_t mode;
    uint32_t timeout;
} BNO055_Handle_t; 


extern BNO055_Handle_t bno;

HAL_StatusTypeDef BNO055_Init(BNO055_Handle_t* bno, I2C_HandleTypeDef* hi2c, uint8_t address_7bit);
HAL_StatusTypeDef BNO055_SetOperationMode(BNO055_Handle_t* bno, bno055_opmode_t mode);
HAL_StatusTypeDef BNO055_SetPowerMode(BNO055_Handle_t* bno, bno055_powermode_t mode);
HAL_StatusTypeDef BNO055_GetPowerMode(BNO055_Handle_t *bno, bno055_powermode_t *mode);
HAL_StatusTypeDef BNO055_GetOperationMode(BNO055_Handle_t *bno, bno055_opmode_t *mode);
HAL_StatusTypeDef BNO055_DiagnoseI2C(BNO055_Handle_t* bno); 

HAL_StatusTypeDef BNO055_ReadAccelerometer(BNO055_Handle_t* bno, bno055_vector_t* accel);
HAL_StatusTypeDef BNO055_ReadLinearAcceleration(BNO055_Handle_t* bno, bno055_vector_t* accel);
HAL_StatusTypeDef BNO055_ReadMagnetometer(BNO055_Handle_t* bno, bno055_vector_t* mag);
HAL_StatusTypeDef BNO055_ReadGyroscope(BNO055_Handle_t* bno, bno055_vector_t* gyro);
HAL_StatusTypeDef BNO055_ReadEuler(BNO055_Handle_t* bno, bno055_vector_t* euler);
HAL_StatusTypeDef BNO055_ReadQuaternion(BNO055_Handle_t* bno, bno055_quaternion_t* quat); 

HAL_StatusTypeDef BNO055_GetCalibrationStatus(BNO055_Handle_t* bno, bno055_calibration_t* calib);
HAL_StatusTypeDef BNO055_GetSystemStatus(BNO055_Handle_t* bno, uint8_t* sys_stat, uint8_t* sys_err);
HAL_StatusTypeDef BNO055_GetChipID(BNO055_Handle_t* bno, uint8_t* chip_id); 

HAL_StatusTypeDef BNO055_IsDeviceReady(BNO055_Handle_t* bno);
HAL_StatusTypeDef BNO055_RunTest(BNO055_Handle_t* bno); 

//Acceleration wird zum Einstellen der Regler benötigt
void Acceleration(BNO055_Handle_t *bno);
//IMUDirection soll angeben, ob eine 90 Grad Drehung durchgeführt wurde
void IMUDirection(BNO055_Handle_t *bno);
void AngularVelocity(BNO055_Handle_t *bno);

#endif 