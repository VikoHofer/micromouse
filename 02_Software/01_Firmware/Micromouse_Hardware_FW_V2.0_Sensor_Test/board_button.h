/*************************************************************
Project		: Micromouse V2.0
File		: board_button.h
Description	: Read Button states on Micromouse
Date		: 11.03.2026
*************************************************************/


#include "stm32f4xx.h"
#include "stm32f4xx_hal_gpio.h"
#include <stdbool.h>


// initialize selected GPIOs for button usage
void BUTTON_Init(void);


// Button 1
bool BUTTON1_Pressed(void);

// Button 2
bool BUTTON2_Pressed(void);



