#ifndef IMU_MOTION_H
#define IMU_MOTION_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include "bno055_hal.h"
#include "BLEDebugger.h"

typedef struct
{
    float forward_velocity_mps;     // geschätzte Vorwärtsgeschwindigkeit [m/s]
    float yaw_rate_dps;             // Winkelgeschwindigkeit um Z [deg/s]
    float yaw_rate_rps;             // Winkelgeschwindigkeit um Z [rad/s]
    float heading_deg;              // Heading [deg]

    float accel_forward_mps2;       // gefilterte Vorwärtsbeschleunigung [m/s²]

    float gyro_bias_z_raw;          // Bias des Gyro-Z Rohwerts
    float accel_bias_forward_raw;   // Bias der Vorwärtsbeschleunigung Rohwert

    uint32_t last_tick_ms;
    bool initialized;
    bool calibrated;
} IMU_MotionState_t;



extern IMU_MotionState_t imu;


HAL_StatusTypeDef IMU_Motion_Init(BNO055_Handle_t *bno, IMU_MotionState_t *imu);
HAL_StatusTypeDef IMU_Motion_Calibrate(BNO055_Handle_t *bno, IMU_MotionState_t *imu, uint16_t samples);
HAL_StatusTypeDef IMU_Motion_Update(BNO055_Handle_t *bno, IMU_MotionState_t *imu);

float IMU_GetForwardVelocity(const IMU_MotionState_t *imu);
float IMU_GetYawRateDeg(const IMU_MotionState_t *imu);
float IMU_GetYawRateRad(const IMU_MotionState_t *imu);
float IMU_GetHeadingDeg(const IMU_MotionState_t *imu);

#endif