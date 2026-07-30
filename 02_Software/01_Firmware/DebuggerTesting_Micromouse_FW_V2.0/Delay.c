/*************************************************************
Project		: Micromouse V1.0
File		: Delay.c
Description	: Handling SysTicks to use Delay-Function / Implementation
Date		: 09.09.2024
*************************************************************/

#include <stdint.h>
#include "Delay.h"


static volatile uint32_t msTicks;                            /* counts 1ms timeTicks */
/*----------------------------------------------------------------------------
 * SysTick_Handler:
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
	msTicks++;
	HAL_IncTick();
}

void SysTick_Init(uint32_t ticks) {
    SysTick_Config(ticks);
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(SysTick_IRQn);          
}
/*----------------------------------------------------------------------------
 * Delay: delays a number of Systicks
 *----------------------------------------------------------------------------*/
void Delay_ms (uint32_t dlyTicks) {
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) { __NOP(); }
}

uint32_t SysTick_Get(void)
{
	return msTicks;
}
