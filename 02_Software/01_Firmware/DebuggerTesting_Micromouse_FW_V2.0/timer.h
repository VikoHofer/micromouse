/*************************************************************
Project		: Micromouse V1.0
File		: Timer.h
Description	: Timer for the Mouse
Date		: 09.09.2024
*************************************************************/

#ifndef TIMER_H
#define TIMER_H


#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"

#include <stdint.h>
#include <stdbool.h>

typedef enum Timer_Id_e {
  TIMER_ID_TIM2 = 0,
  TIMER_ID_TIM5 = 1,
  TIMER_ID_TIM9 = 2,
  TIMER_ID_TIM12 = 3
   

} Timer_Id_t;


/*******************************************************************************
Function pointer type for timer callback functions.
*******************************************************************************/
typedef void (*Timer_Callback_t)(int id);

/*******************************************************************************
Initializes the specified timer hardware.

id: The id of the timer (see Timer_Id_t).

returns: Zero on success, an error code otherwise.
*******************************************************************************/
int Timer_Initialize(int id,int milliSeconds,Timer_Callback_t callback);


/*******************************************************************************
Start Timer 

id: The id of the timer (see Timer_Id_t).
*******************************************************************************/
void Timer_Start(int id);

/*******************************************************************************
Stop Timer 

id: The id of the timer (see Timer_Id_t).
*******************************************************************************/
void Timer_Stop(int id);


#endif











