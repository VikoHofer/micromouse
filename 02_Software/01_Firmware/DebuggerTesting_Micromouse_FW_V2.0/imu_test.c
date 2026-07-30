#include "imu_test.h"
#include <math.h>
#include <stdio.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// -----------------------------
// BNO055 Skalierungen
// -----------------------------
// Linear Acceleration: 100 LSB = 1 m/s²
#define BNO055_LIA_SCALE_MPS2      100.0f

// Gyroscope: 16 LSB = 1 deg/s
#define BNO055_GYRO_SCALE_DPS      16.0f

// Euler: 16 LSB = 1 deg
#define BNO055_EULER_SCALE_DEG     16.0f

// -----------------------------
// Filter / Schwellenwerte
// -----------------------------
#define IMU_DT_MIN_S               0.001f
#define IMU_DT_MAX_S               0.050f

// Exponential Low Pass für Beschleunigung
#define IMU_ACCEL_LPF_ALPHA        0.2f

// Kleine Beschleunigungen unterdrücken
#define IMU_ACCEL_DEADBAND_MPS2    0.03f

// Sehr kleine Geschwindigkeiten auf 0 setzen
#define IMU_VEL_DEADBAND_MPS       0.01f

// Stillstandserkennung
#define IMU_STATIONARY_ACCEL_MPS2  0.05f
#define IMU_STATIONARY_GYRO_DPS    1.5f

// Leckfaktor gegen Integrationsdrift
#define IMU_VELOCITY_LEAK          0.999f

// -----------------------------
// Falls Vorwärtsachse nicht X ist:
// 0 => X, 1 => Y, 2 => Z
// -----------------------------
#define IMU_FORWARD_AXIS           0

static float get_forward_axis_raw(const bno055_vector_t *v)
{
#if IMU_FORWARD_AXIS == 0
    return (float)v->x;
#elif IMU_FORWARD_AXIS == 1
    return (float)v->y;
#else
    return (float)v->z;
#endif
}

static float apply_deadband(float x, float db)
{
    if (x > db)  return x;
    if (x < -db) return x;
    return 0.0f;
}

HAL_StatusTypeDef IMU_Motion_Init(BNO055_Handle_t *bno, IMU_MotionState_t *imu)
{
    if ((bno == NULL) || (imu == NULL)) {
        return HAL_ERROR;
    }

    imu->forward_velocity_mps   = 0.0f;
    imu->yaw_rate_dps           = 0.0f;
    imu->yaw_rate_rps           = 0.0f;
    imu->heading_deg            = 0.0f;
    imu->accel_forward_mps2     = 0.0f;

    imu->gyro_bias_z_raw        = 0.0f;
    imu->accel_bias_forward_raw = 0.0f;

    imu->last_tick_ms           = HAL_GetTick();
    imu->initialized            = true;
    imu->calibrated             = false;

    return HAL_OK;
}

HAL_StatusTypeDef IMU_Motion_Calibrate(BNO055_Handle_t *bno, IMU_MotionState_t *imu, uint16_t samples)
{
    if ((bno == NULL) || (imu == NULL) || (samples == 0)) {
        printf("Calib: invalid args\n");
        return HAL_ERROR;
    }

    bno055_vector_t accel;
    bno055_vector_t gyro;

    float accel_sum = 0.0f;
    float gyro_sum  = 0.0f;
    uint16_t ok_cnt = 0;
    uint16_t accel_fail = 0;
    uint16_t gyro_fail = 0;

    for (uint16_t i = 0; i < samples; i++) {
        HAL_StatusTypeDef sa = BNO055_ReadLinearAcceleration(bno, &accel);
        HAL_StatusTypeDef sg = BNO055_ReadGyroscope(bno, &gyro);

        if (sa != HAL_OK) {
            accel_fail++;
            continue;
        }
        if (sg != HAL_OK) {
            gyro_fail++;
            continue;
        }

        accel_sum += get_forward_axis_raw(&accel);
        gyro_sum  += (float)gyro.z;
        ok_cnt++;

        if (i < 5) {
            printf("sample %u: acc=(%d,%d,%d) gyro=(%d,%d,%d)\n",
                   i, accel.x, accel.y, accel.z, gyro.x, gyro.y, gyro.z);
        }

        HAL_Delay(5);
    }

    printf("Calib result: ok=%u accel_fail=%u gyro_fail=%u\n",
           ok_cnt, accel_fail, gyro_fail);

    if (ok_cnt == 0) {
        return HAL_ERROR;
    }

    imu->accel_bias_forward_raw = accel_sum / (float)ok_cnt;
    imu->gyro_bias_z_raw        = gyro_sum  / (float)ok_cnt;

    imu->forward_velocity_mps   = 0.0f;
    imu->accel_forward_mps2     = 0.0f;
    imu->yaw_rate_dps           = 0.0f;
    imu->yaw_rate_rps           = 0.0f;
    imu->last_tick_ms           = HAL_GetTick();
    imu->calibrated             = true;

    printf("Calib OK: accel_bias=%f gyro_bias=%f\n",
           imu->accel_bias_forward_raw,
           imu->gyro_bias_z_raw);

    return HAL_OK;
}

HAL_StatusTypeDef IMU_Motion_Update(BNO055_Handle_t *bno, IMU_MotionState_t *imu)
{
    if ((bno == NULL) || (imu == NULL) || (!imu->initialized) || (!imu->calibrated)) {
        return HAL_ERROR;
    }

    bno055_vector_t accel_raw;
    bno055_vector_t gyro_raw;
    bno055_vector_t euler_raw;

    if (BNO055_ReadLinearAcceleration(bno, &accel_raw) != HAL_OK) {
        return HAL_ERROR;
    }

    if (BNO055_ReadGyroscope(bno, &gyro_raw) != HAL_OK) {
        return HAL_ERROR;
    }

    if (BNO055_ReadEuler(bno, &euler_raw) != HAL_OK) {
        return HAL_ERROR;
    }

    uint32_t now_ms = HAL_GetTick();
    float dt = (now_ms - imu->last_tick_ms) / 1000.0f;
    imu->last_tick_ms = now_ms;

    if (dt < IMU_DT_MIN_S) dt = IMU_DT_MIN_S;
    if (dt > IMU_DT_MAX_S) dt = IMU_DT_MAX_S;

    // -----------------------------
    // Gyro Z -> Winkelgeschwindigkeit
    // -----------------------------
    float gyro_z_raw = (float)gyro_raw.z - imu->gyro_bias_z_raw;
    imu->yaw_rate_dps = gyro_z_raw / BNO055_GYRO_SCALE_DPS;
    imu->yaw_rate_rps = imu->yaw_rate_dps * (M_PI / 180.0f);

    // -----------------------------
    // Euler Heading
    // -----------------------------
    imu->heading_deg = ((float)euler_raw.x) / BNO055_EULER_SCALE_DEG;

    // -----------------------------
    // Vorwärtsbeschleunigung
    // -----------------------------
    float accel_forward_raw = get_forward_axis_raw(&accel_raw) - imu->accel_bias_forward_raw;
    float accel_forward_mps2 = accel_forward_raw / BNO055_LIA_SCALE_MPS2;

    // Tiefpass
    imu->accel_forward_mps2 =
        IMU_ACCEL_LPF_ALPHA * accel_forward_mps2 +
        (1.0f - IMU_ACCEL_LPF_ALPHA) * imu->accel_forward_mps2;

    // Deadband
    imu->accel_forward_mps2 =
        apply_deadband(imu->accel_forward_mps2, IMU_ACCEL_DEADBAND_MPS2);

    // -----------------------------
    // Geschwindigkeit integrieren
    // -----------------------------
    imu->forward_velocity_mps += imu->accel_forward_mps2 * dt;

    // leichter Leckfaktor gegen Drift
    imu->forward_velocity_mps *= IMU_VELOCITY_LEAK;

    // -----------------------------
    // Stillstandserkennung
    // -----------------------------
    if ((fabsf(imu->accel_forward_mps2) < IMU_STATIONARY_ACCEL_MPS2) &&
        (fabsf(imu->yaw_rate_dps)       < IMU_STATIONARY_GYRO_DPS)) {
        imu->forward_velocity_mps = 0.0f;
    }

    // sehr kleine Werte auf 0
    if (fabsf(imu->forward_velocity_mps) < IMU_VEL_DEADBAND_MPS) {
        imu->forward_velocity_mps = 0.0f;
    }

    return HAL_OK;
}

float IMU_GetForwardVelocity(const IMU_MotionState_t *imu)
{
    if (imu == NULL) return 0.0f;
    return imu->forward_velocity_mps;
}

float IMU_GetYawRateDeg(const IMU_MotionState_t *imu)
{
    if (imu == NULL) return 0.0f;
    return imu->yaw_rate_dps;
}

float IMU_GetYawRateRad(const IMU_MotionState_t *imu)
{
    if (imu == NULL) return 0.0f;
    return imu->yaw_rate_rps;
}

float IMU_GetHeadingDeg(const IMU_MotionState_t *imu)
{
    if (imu == NULL) return 0.0f;
    return imu->heading_deg;
}