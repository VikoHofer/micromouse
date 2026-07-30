
#include <stdio.h>
#include <stdint.h>
#include "Gyro.h"

void BNO055_Init_Gyro(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance != I2C2) return;


	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	__HAL_RCC_I2C2_CLK_ENABLE();

	GPIO_InitTypeDef g = {0};
	g.Mode = GPIO_MODE_AF_OD;
	g.Pull = GPIO_PULLUP;                 // zum Debug!
	g.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	g.Alternate = GPIO_AF4_I2C2;

	g.Pin = GPIO_PIN_10;                  // SCL
	HAL_GPIO_Init(GPIOB, &g);

	g.Pin = GPIO_PIN_12;                  // SDA (siehe Punkt 2)
	HAL_GPIO_Init(GPIOC, &g);
}

vec3 BNO055_ReadLinearAccel(I2C_HandleTypeDef *hi2c)
{
    uint8_t buffer[6];
    vec3 accel;

    if(HAL_I2C_Mem_Read(hi2c, BNO055_ADDR, 0x28, 1, buffer, 6, 100) != HAL_OK) {
			// Fehler behandeln oder Debug-Ausgabe
			accel.x = accel.y = accel.z = 0;
			return accel;
		}

    volatile int16_t ax = (int16_t)(buffer[1] << 8 | buffer[0]);
    volatile int16_t ay = (int16_t)(buffer[3] << 8 | buffer[2]);
    volatile int16_t az = (int16_t)(buffer[5] << 8 | buffer[4]);

    accel.x = ax / 100.0f;
    accel.y = ay / 100.0f;
    accel.z = az / 100.0f;

    return accel;
}

void MX_Gyro_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Clock aktivieren */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Default Output Level setzen */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14, GPIO_PIN_RESET);

    /* RESET, BOOT und ADR Pins konfigurieren */
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* optional: SDA Pin vorbereiten (falls nicht über I2C init) */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;

    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* optional: SCL Pin */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void MX_Gyro_I2C1_Init(void)
{
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;       // erstmal 100k zum Debug
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(&hi2c2) != HAL_OK) while(1);
}


