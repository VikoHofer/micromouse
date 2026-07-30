/*************************************************************
Project     : Micromouse V1.0
File        : Timer.c
Description : Timer for the Mouse
Date        : 09.09.2024
*************************************************************/

#include "Timer.h"
#include "config.h"
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
 * Assumption for 180 MHz clock configuration:
 *
 * SYSCLK = 180 MHz
 * APB1   = 45 MHz  -> APB1 timer clocks = 90 MHz
 * APB2   = 90 MHz  -> APB2 timer clocks = 180 MHz
 *
 * TIM2, TIM5, TIM12 are on APB1
 * TIM9 is on APB2
 *
 * If your clock tree is different, adjust the values below.
 */
#define TIM_APB1_CLOCK_HZ   90000000U
#define TIM_APB2_CLOCK_HZ  180000000U

/* We use a timer base of 10 kHz => 0.1 ms per timer tick */
#define TIMER_BASE_TICK_HZ      10000U
#define TIMER_TICKS_PER_MS      10U

// stores TIMER infos
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

// Get TIMER infos
static inline Timer_Context_t *Timer_GetContext(int id)
{
    if (id < 0 || id >= TIMER_COUNT) {
        return NULL;
    }
    return &Timer_Context[id];
}

// Get HAL handle from timer id
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

// Get timer input clock for selected timer
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

// Enable TIMER CLK
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

// Universal TIMER ISR
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

// Interrupt Handler (ISR)
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

// Initialize Timer
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
     *   f_tick = 10 kHz  => 1 tick = 0.1 ms
     */
    prescaler = (timer_clk_hz / TIMER_BASE_TICK_HZ) - 1U;

    /*
     * For N milliseconds at 10 ticks/ms:
     *   required ticks = N * 10
     * Timer counts from 0 .. ARR  => ARR = ticks - 1
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

// Start
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

// Stop
void Timer_Stop(int id)
{
    Timer_Context_t *context = Timer_GetContext(id);
    TIM_HandleTypeDef *htim = Timer_GetHandle(id);

    if ((context == NULL) || (htim == NULL) || (!context->Initialized)) {
        return;
    }

    HAL_TIM_Base_Stop_IT(htim);
}