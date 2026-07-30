/*************************************************************
Project		: Micromouse V1.0
File		: EXTI.h
Description	: Configure PINs for External IR usage
Date		: 09.09.2024
*************************************************************/

// !!!!!!!!!!!! WURDE NOCHT NICHT GETESTET !!!!!!!!!!!!!!!!

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_exti.h"

// initialize selected GPIOs for External IR usage
void EXTI_Initialize(void);


/*************************************
EXTI-Callback functions
*************************************/

// Define the callback function type
typedef void (*EXTI_CallbackFunc)(void);

// Define the callback functions for each pin
void EXTI_Callback_Pin1(void);
void EXTI_Callback_Pin2(void);
