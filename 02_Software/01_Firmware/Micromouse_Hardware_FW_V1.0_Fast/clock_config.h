/*************************************************************
Project		: Micromouse V2.0
File		: clock_config.h
Description	: Configurate systemClock (HSI) -> 180 Mhz
Date		: 11.03.2026
*************************************************************/


#ifndef CLOCK_CONFIG_H
#define CLOCK_CONFIG_H

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef Clock_Config_BoardDefault(void);

#endif