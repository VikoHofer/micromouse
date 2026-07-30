#ifndef GYRO_H
#define GYRO_H

#include "stm32f4xx_hal.h"

#define BNO055_ADDR (0x28 << 1)

extern I2C_HandleTypeDef hi2c2;

typedef struct
{
    float x;
    float y;
    float z;
} vec3;

void MX_Gyro_GPIO_Init(void);
void MX_Gyro_I2C1_Init(void);
void BNO055_Init_Gyro(I2C_HandleTypeDef *hi2c);
vec3 BNO055_ReadLinearAccel(I2C_HandleTypeDef *hi2c);


#endif