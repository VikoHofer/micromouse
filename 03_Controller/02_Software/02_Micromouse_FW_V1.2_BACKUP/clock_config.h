/*************************************************************
Project		: Micromouse V1.0
File		: clock_config.h
Description	: Configurate systemClock (HSI) -> 180 Mhz
Date		: 09.09.2024
*************************************************************/


#ifndef CLOCK_CONF_INCLUDE
#define CLOCK_CONF_INCLUDE
/*----------------------------------------------------------------------------
 * SystemCoreClockConfigure: configure SystemCoreClock using HSI
                             (HSE is not populated on Nucleo board)
 *----------------------------------------------------------------------------*/
void SystemCoreClockConfigure(void);

#endif