/*************************************************************
Project		: Micromouse V1.0
File		: Timer.h
Description	: Timer for the Mouse
Date		: 09.09.2024
*************************************************************/

#include "Timer.h"
#include "BOARD_LED.h"

#define TIMER_COUNT 4  // Jetzt gibt es zwei Timer

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim12;


// stores TIMER infos
typedef struct Timer_Context_s {
    TIM_TypeDef *const Timer;
    IRQn_Type const InterruptNumber;
    bool Initialized;
    uint32_t TickCount;
    Timer_Callback_t Callback;
} Timer_Context_t;
static Timer_Context_t Timer_Context[TIMER_COUNT] = {
    {TIM2, TIM2_IRQn, false, 0, NULL},
    {TIM5, TIM5_IRQn, false, 0, NULL},
	{TIM9, TIM1_BRK_TIM9_IRQn, false, 0, NULL},
	{TIM12, TIM8_BRK_TIM12_IRQn, false, 0, NULL}
};

// Get TIMER infos
static inline Timer_Context_t *Timer_GetContext(int id) {
    if (id < 0 || id >= TIMER_COUNT) {
        return NULL;
    }
    return &Timer_Context[id];
}

// Enable TIMER CLK
void EnableClock(int id) {
    if (id == TIMER_ID_TIM2) {
        __TIM2_CLK_ENABLE();
    } else if (id == TIMER_ID_TIM5) {
        __TIM5_CLK_ENABLE();
    } else if (id == TIMER_ID_TIM9){
		__TIM9_CLK_ENABLE();
	}else if (id == TIMER_ID_TIM12){
		__TIM12_CLK_ENABLE();
	}
}

// Universal TIMER ISR
static void Timer_ISR(int id) {
    volatile Timer_Context_t *context = Timer_GetContext(id);
    if (context == NULL) {
        return;
    }

    if (context->Callback != NULL) {
        context->Callback(id);
    } else {
        return;
    }
}


// Interrupt Handler (ISR)
void TIM2_IRQHandler(void) {
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET) {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
        Timer_ISR(TIMER_ID_TIM2);
    }
}
void TIM5_IRQHandler(void) {
    if (__HAL_TIM_GET_FLAG(&htim5, TIM_FLAG_UPDATE) != RESET) {
        __HAL_TIM_CLEAR_FLAG(&htim5, TIM_FLAG_UPDATE);
        Timer_ISR(TIMER_ID_TIM5);
    }
}

void TIM1_BRK_TIM9_IRQHandler(void) {
    if (__HAL_TIM_GET_FLAG(&htim9, TIM_FLAG_UPDATE) != RESET) {
        __HAL_TIM_CLEAR_FLAG(&htim9, TIM_FLAG_UPDATE);
        Timer_ISR(TIMER_ID_TIM9);
    }
}

void TIM8_BRK_TIM12_IRQHandler(void) {
    if (__HAL_TIM_GET_FLAG(&htim12, TIM_FLAG_UPDATE) != RESET) {
        __HAL_TIM_CLEAR_FLAG(&htim12, TIM_FLAG_UPDATE);
        Timer_ISR(TIMER_ID_TIM12);
    }
}


// Initialize Timer
int Timer_Initialize(int id, int milliSeconds, Timer_Callback_t callback) {
    EnableClock(id);
	
	if (callback == NULL) {
        return -1;
    }

    int periodValue = 10 * milliSeconds;

    volatile Timer_Context_t *context = Timer_GetContext(id);
    if (context == NULL) {
        return -1;
    }

    TIM_HandleTypeDef *htim;
    if (id == TIMER_ID_TIM2) {
        htim = &htim2;
    } else if (id == TIMER_ID_TIM5) {
        htim = &htim5;
    } else if (id == TIMER_ID_TIM9) {
        htim = &htim9;
    } else if (id == TIMER_ID_TIM12) {
        htim = &htim12;
    } 

    htim->Instance = context->Timer;
    htim->Init.Prescaler = 18000 - 1;  // Timer-Takt von 180 MHz / 18000 = 10 kHz
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = periodValue;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(htim);

    HAL_TIM_Base_Start_IT(htim);

    HAL_NVIC_SetPriority(context->InterruptNumber, 5, 0);
    HAL_NVIC_EnableIRQ(context->InterruptNumber);

    

    context->Callback = callback;
    context->Initialized = true;

    return 0;
}


// Start // Stop
void Timer_Start(int id) {
    volatile Timer_Context_t *context = Timer_GetContext(id);
    if (context == NULL) {
        return;
    }

    TIM_HandleTypeDef *htim;
    if (id == TIMER_ID_TIM2) {
        htim = &htim2;
    } else if (id == TIMER_ID_TIM5) {
        htim = &htim5;
    } else if (id == TIMER_ID_TIM9) {
        htim = &htim9;
    } else if (id == TIMER_ID_TIM12) {
        htim = &htim12;
    }

    HAL_TIM_Base_Start_IT(htim);
}

void Timer_Stop(int id) {
    volatile Timer_Context_t *context = Timer_GetContext(id);
    if (context == NULL) {
        return;
    }

    TIM_HandleTypeDef *htim;
    if (id == TIMER_ID_TIM2) {
        htim = &htim2;
    } else if (id == TIMER_ID_TIM5) {
        htim = &htim5;
    } else if (id == TIMER_ID_TIM9) {
        htim = &htim9;
    } else if (id == TIMER_ID_TIM12) {
        htim = &htim12;
    }

    HAL_TIM_Base_Stop_IT(htim);
}
