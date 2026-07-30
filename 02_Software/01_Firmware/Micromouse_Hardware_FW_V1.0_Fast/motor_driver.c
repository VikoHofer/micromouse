/*************************************************************
Project     : Micromouse
File        : motor_driver.c
Description : Motor control and PWM generation
Date        : 15.04.2026
*************************************************************/

#include "motor_driver.h"
#include "config.h"
#include "board_config.h"
#include "BLEDebugger.h"

/************************************
 * TIM4 PWM:
 * CH3 -> PB8
 * CH4 -> PB9
 *
 * PWM frequency is derived from:
 *   timer clock (board dependent)
 *   PWM_PRESCALER
 *   PWM_PERIOD
 ************************************/

typedef struct {
    GPIO_TypeDef* GPIO_Port;
    uint16_t GPIO_Pin;
} GPIO_Pin_t;

static const GPIO_Pin_t MOTOR_CTRL_PIN[] =
{
    { GPIOA, GPIO_PIN_10 },
    { GPIOC, GPIO_PIN_9  },
    { GPIOB, GPIO_PIN_5  },
    { GPIOC, GPIO_PIN_8  }
};

void Motor_Ctrl_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    for (int i = 0; i < (int)(sizeof(MOTOR_CTRL_PIN) / sizeof(MOTOR_CTRL_PIN[0])); ++i) {
        GPIO_Pin_t pin = MOTOR_CTRL_PIN[i];

        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = pin.GPIO_Pin;
        GPIO_InitStruct.Mode = MODE_OUTPUT;
        HAL_GPIO_Init(pin.GPIO_Port, &GPIO_InitStruct);
    }
}

#if TARGET_MOTOR == MOTOR_HW_V1

void Motor_Left_Forwards(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,  GPIO_PIN_RESET);
}

void Motor_Left_Backwards(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,  GPIO_PIN_SET);
}

void Motor_Left_Brake(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,  GPIO_PIN_RESET);
}

void Motor_Right_Forwards(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
}

void Motor_Right_Backwards(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
}

void Motor_Right_Brake(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
}

#endif

TIM_HandleTypeDef tim4 = {0};

static void GPIO_Motor_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_PWM_Init = {0};
    GPIO_PWM_Init.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_PWM_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_PWM_Init.Pull = GPIO_PULLUP;
    GPIO_PWM_Init.Speed = GPIO_SPEED_HIGH;
    GPIO_PWM_Init.Alternate = GPIO_AF2_TIM4;

    HAL_GPIO_Init(GPIOB, &GPIO_PWM_Init);
}

void Motor_Init(void)
{
    __HAL_RCC_TIM4_CLK_ENABLE();

    GPIO_Motor_Init();
    Motor_Ctrl_Init();

    Motor_Left_Brake();
    Motor_Right_Brake();

    tim4.Instance = TIM4;
    tim4.Init.Prescaler = PWM_PRESCALER;
    tim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim4.Init.Period = PWM_PERIOD;
    tim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    tim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    tim4.Init.RepetitionCounter = 0;

    HAL_TIM_PWM_Init(&tim4);

    TIM_OC_InitTypeDef sConfigOC1 = {0};
    sConfigOC1.OCMode = TIM_OCMODE_PWM1;
    sConfigOC1.Pulse = 0;
    sConfigOC1.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC1.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&tim4, &sConfigOC1, TIM_CHANNEL_3);

    TIM_OC_InitTypeDef sConfigOC2 = {0};
    sConfigOC2.OCMode = TIM_OCMODE_PWM1;
    sConfigOC2.Pulse = 0;
    sConfigOC2.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC2.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&tim4, &sConfigOC2, TIM_CHANNEL_4);

    HAL_TIM_PWM_Start(&tim4, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&tim4, TIM_CHANNEL_4);

    __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_4, 0);
}

#define PWM_MAX PWM_PERIOD

static int16_t limitPWM(int16_t pwm)
{
    if (pwm > PWM_MAX) {
        return PWM_MAX;
    } else if (pwm < -PWM_MAX) {
        return -PWM_MAX;
    }
    return pwm;
}

void Motor_Left_SetPWM(int16_t pwm)
{
    int16_t limited = limitPWM(pwm);

    if (pwm == 0) {
        Motor_Left_Brake();
        __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_3, 0);
        return;
    }

    if (pwm > 0) {
        Motor_Left_Forwards();
        __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_3, (uint32_t)limited);
    } else {
        Motor_Left_Backwards();
        __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_3, (uint32_t)(-limited));
    }

}

void Motor_Right_SetPWM(int16_t pwm)
{
    if (pwm == 0) {
        Motor_Right_Brake();
        __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_4, 0);
        return;
    }

    if (pwm > 0) {
        Motor_Right_Forwards();
        __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_4, limitPWM(pwm));
    } else {
        Motor_Right_Backwards();
        __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_4, (uint16_t)(-limitPWM(pwm)));
    }
}