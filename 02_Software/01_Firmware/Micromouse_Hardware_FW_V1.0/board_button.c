/*************************************************************
Project		: Micromouse V1.0
File		: board_button.c
Description	: Read Button states on Micromouse / implementation
Date		: 09.09.2024
*************************************************************/

#include "board_button.h"


// struct to identifier GPIO
typedef struct {
    GPIO_TypeDef* GPIO_Port;
    uint16_t GPIO_Pin;
} GPIO_Pin_t;


// BUTTON GPIO Pins 
static const GPIO_Pin_t BUTTON_PIN[] = 
{
    { GPIOB, GPIO_PIN_1 }, // Button 1
    { GPIOB, GPIO_PIN_2 } // Button 2
};

// initialize all GPIOs for button usage
void BUTTON_Init(void)
{
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	for (int i = 0; i < sizeof(BUTTON_PIN) / sizeof(BUTTON_PIN[0]); ++i) {
        GPIO_Pin_t pin = BUTTON_PIN[i];
        
        // initialize GPIO-Pins as Input
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.Pin = pin.GPIO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL; 
        HAL_GPIO_Init(pin.GPIO_Port, &GPIO_InitStruct);
    }

}



// Button 1
bool BUTTON1_Pressed(void)
{
	uint8_t pinState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
	
	if(pinState == GPIO_PIN_SET)
	{
		return false;
	} 
	else 
	{
		return true;
	}
}

// Button 2
bool BUTTON2_Pressed(void)
{
	uint8_t pinState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
	
	if(pinState == GPIO_PIN_SET)
	{
		return false;
	} 
	else 
	{
		return true;
	}
}


