/*************************************************************
Project		: Micromouse V1.0
File		: Delay.h
Description	: Handling SysTicks to use Delay-Function
Date		: 09.09.2024
*************************************************************/


#ifndef DELAY_INCLUDED_H
#define DELAY_INCLUDED_H

#include "stm32f4xx.h"

/*----------------------------------------------------------------------------
 * SysTick_Handler:
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void);

void SysTick_Init(uint32_t ticks);
/*----------------------------------------------------------------------------
 * Delay: delays a number of Systicks
 *----------------------------------------------------------------------------*/
void Delay_ms (uint32_t dlyTicks);

//Returns current ticks
uint32_t SysTick_Get(void);



#endif 