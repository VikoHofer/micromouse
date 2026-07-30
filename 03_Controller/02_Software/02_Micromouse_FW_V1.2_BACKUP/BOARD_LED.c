/*************************************************************
Project		: Micromouse V1.0
File		: board_led.c
Description	: Controll Board LED // Implementation
Date		: 09.09.2024
*************************************************************/

// !!!!!! PINS ANPASSEN !!!!!!!!!

#include "stm32f4xx.h"
#include "stm32f4xx_hal_gpio.h"
#include "BOARD_LED.h"

//// Init Board LED
void Board_LED_Init(void){
    
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
    GPIO_InitTypeDef initStruct;
    
	initStruct.Pin = GPIO_PIN_5;
    initStruct.Mode = MODE_OUTPUT;
    
    
   HAL_GPIO_Init(GPIOA, &initStruct);
    
}



void Toggle_Board_LED(void){
    
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}

void Board_LED_On(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

void Board_LED_Off(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

