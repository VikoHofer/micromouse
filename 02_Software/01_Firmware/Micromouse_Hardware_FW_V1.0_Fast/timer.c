/*************************************************************
Project     : Micromouse
File        : Timer.c
Description : Timer abstraction
Date        : 15.04.2026
*************************************************************/

#include "Timer.h"
#include "config.h"
#include "board_config.h"
#include "BOARD_LED.h"
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stddef.h>

#define TIMER_COUNT 4

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim12;

/*
 * Timer clocks come from board_config.h via config.h:
 *
 * V1:
 *   TIM_APB1_CLOCK_HZ = 84 MHz
 *   TIM_APB2_CLOCK_HZ = 84 MHz
 *
 * V2:
 *   TIM_APB1_CLOCK_HZ = 90 MHz
 *   TIM_APB2_CLOCK_HZ = 180 MHz
 */

/* stores TIMER infos */
typedef struct Timer_Context_s {
    TIM_TypeDef *const Timer;
    IRQn_Type const InterruptNumber;
    bool Initialized;
    uint32_t TickCount;
    Timer_Callback_t Callback;
} Timer_Context_t;

static Timer_Context_t Timer_Context[TIMER_COUNT] = {
    {TIM2,  TIM2_IRQn,             false, 0, NULL},
    {TIM5,  TIM5_IRQn,             false, 0, NULL},
    {TIM9,  TIM1_BRK_TIM9_IRQn,    false, 0, NULL},
    {TIM12, TIM8_BRK_TIM12_IRQn,   false, 0, NULL}
};

static inline Timer_Context_t *Timer_GetContext(int id)
{
    if (id < 0 || id >= TIMER_COUNT) {
        return NULL;
    }
    return &Timer_Context[id];
}

static TIM_HandleTypeDef *Timer_GetHandle(int id)
{
    switch (id) {
        case TIMER_ID_TIM2:  return &htim2;
        case TIMER_ID_TIM5:  return &htim5;
        case TIMER_ID_TIM9:  return &htim9;
        case TIMER_ID_TIM12: return &htim12;
        default:             return NULL;
    }
}

static uint32_t Timer_GetInputClockHz(int id)
{
    switch (id) {
        case TIMER_ID_TIM9:
            return TIM_APB2_CLOCK_HZ;

        case TIMER_ID_TIM2:
        case TIMER_ID_TIM5:
        case TIMER_ID_TIM12:
            return TIM_APB1_CLOCK_HZ;

        default:
            return 0U;
    }
}

static void Timer_EnableClock(int id)
{
    switch (id) {
        case TIMER_ID_TIM2:
            __HAL_RCC_TIM2_CLK_ENABLE();
            break;

        case TIMER_ID_TIM5:
            __HAL_RCC_TIM5_CLK_ENABLE();
            break;

        case TIMER_ID_TIM9:
            __HAL_RCC_TIM9_CLK_ENABLE();
            break;

        case TIMER_ID_TIM12:
            __HAL_RCC_TIM12_CLK_ENABLE();
            break;

        default:
            break;
    }
}

static void Timer_ISR(int id)
{
    Timer_Context_t *context = Timer_GetContext(id);
    if (context == NULL) {
        return;
    }

    context->TickCount++;

    if (context->Callback != NULL) {
        context->Callback(id);
    }
}

void TIM2_IRQHandler(void)
{
    if ((__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET) &&
        (__HAL_TIM_GET_IT_SOURCE(&htim2, TIM_IT_UPDATE) != RESET)) {
        __HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
        Timer_ISR(TIMER_ID_TIM2);
    }
}

void TIM5_IRQHandler(void)
{
    if ((__HAL_TIM_GET_FLAG(&htim5, TIM_FLAG_UPDATE) != RESET) &&
        (__HAL_TIM_GET_IT_SOURCE(&htim5, TIM_IT_UPDATE) != RESET)) {
        __HAL_TIM_CLEAR_IT(&htim5, TIM_IT_UPDATE);
        Timer_ISR(TIMER_ID_TIM5);
    }
}

void TIM1_BRK_TIM9_IRQHandler(void)
{
    if ((__HAL_TIM_GET_FLAG(&htim9, TIM_FLAG_UPDATE) != RESET) &&
        (__HAL_TIM_GET_IT_SOURCE(&htim9, TIM_IT_UPDATE) != RESET)) {
        __HAL_TIM_CLEAR_IT(&htim9, TIM_IT_UPDATE);
        Timer_ISR(TIMER_ID_TIM9);
    }
}

void TIM8_BRK_TIM12_IRQHandler(void)
{
    if ((__HAL_TIM_GET_FLAG(&htim12, TIM_FLAG_UPDATE) != RESET) &&
        (__HAL_TIM_GET_IT_SOURCE(&htim12, TIM_IT_UPDATE) != RESET)) {
        __HAL_TIM_CLEAR_IT(&htim12, TIM_IT_UPDATE);
        Timer_ISR(TIMER_ID_TIM12);
    }
}

int Timer_Initialize(int id, int milliSeconds, Timer_Callback_t callback)
{
    Timer_Context_t *context = Timer_GetContext(id);
    TIM_HandleTypeDef *htim = Timer_GetHandle(id);
    uint32_t timer_clk_hz;
    uint32_t prescaler;
    uint32_t period;

    if ((context == NULL) || (htim == NULL) || (callback == NULL) || (milliSeconds <= 0)) {
        return -1;
    }

    timer_clk_hz = Timer_GetInputClockHz(id);
    if (timer_clk_hz == 0U) {
        return -1;
    }

    /*
     * Timer tick frequency:
     *   f_tick = timer_clk / (PSC + 1)
     *
     * We choose:
     *   f_tick = TIMER_BASE_TICK_HZ
     */
    prescaler = (timer_clk_hz / TIMER_BASE_TICK_HZ) - 1U;

    /*
     * For N milliseconds:
     *   required ticks = N * TIMER_TICKS_PER_MS
     */
    period = ((uint32_t)milliSeconds * TIMER_TICKS_PER_MS) - 1U;

    Timer_EnableClock(id);

    htim->Instance = context->Timer;
    htim->Init.Prescaler = prescaler;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = period;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(htim) != HAL_OK) {
        return -1;
    }

    __HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);

    HAL_NVIC_SetPriority(context->InterruptNumber, 5, 0);
    HAL_NVIC_EnableIRQ(context->InterruptNumber);

    context->TickCount = 0;
    context->Callback = callback;
    context->Initialized = true;

    return 0;
}

void Timer_Start(int id)
{
    Timer_Context_t *context = Timer_GetContext(id);
    TIM_HandleTypeDef *htim = Timer_GetHandle(id);

    if ((context == NULL) || (htim == NULL) || (!context->Initialized)) {
        return;
    }

    __HAL_TIM_SET_COUNTER(htim, 0);
    __HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);
    HAL_TIM_Base_Start_IT(htim);
}

void Timer_Stop(int id)
{
    Timer_Context_t *context = Timer_GetContext(id);
    TIM_HandleTypeDef *htim = Timer_GetHandle(id);

    if ((context == NULL) || (htim == NULL) || (!context->Initialized)) {
        return;
    }

    HAL_TIM_Base_Stop_IT(htim);
}