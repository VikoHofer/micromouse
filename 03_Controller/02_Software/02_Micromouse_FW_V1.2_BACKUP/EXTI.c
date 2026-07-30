/*************************************************************
Project		: Micromouse V1.0
File		: EXTI.c
Description	: Configure PINs for External IR usage
Date		: 09.09.2024
*************************************************************/

// !!!!!!!!!!!! WURDE NOCHT NICHT GETESTET !!!!!!!!!!!!!!!!

#include "EXTI.h"


/*************************************
EXTI-Callback functions
*************************************/
// Define the callback functions for each pin
void EXTI_Callback_Pin1(void) {
    // Callback code for Pin 1
}

void EXTI_Callback_Pin2(void) {
    // Callback code for Pin 2
}

// Create an array of callback function pointers
EXTI_CallbackFunc EXTI_Callbacks[] = {
    EXTI_Callback_Pin1,
    EXTI_Callback_Pin2
};

// Structure to hold GPIO pins
typedef struct {
    GPIO_TypeDef* GPIO_Port;
    uint16_t GPIO_Pin;
} GPIO_Pin_t;

// Define GPIO pins
static const GPIO_Pin_t BUTTON_PIN[] = {
    { GPIOC, GPIO_PIN_13 }, // Example pin 1
    { GPIOA, GPIO_PIN_0 }   // Example pin 2
};


/*************************************
EXTI-Configuration
*************************************/
void EXTI_Initialize(void)
{
	
	// configuration loop
	for (int i = 0; i < sizeof(BUTTON_PIN) / sizeof(BUTTON_PIN[0]); ++i) {
		GPIO_Pin_t pin = BUTTON_PIN[i];
		
		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_InitStruct.Pin = pin.GPIO_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING; 
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(pin.GPIO_Port, &GPIO_InitStruct);
		
		EXTI_HandleTypeDef hexti;
		hexti.Line = pin.GPIO_Pin;
		
		// Konfiguriere EXTI-Leitung
		EXTI_ConfigTypeDef extiConfig;
		extiConfig.Line = pin.GPIO_Pin;
		extiConfig.Mode = EXTI_MODE_INTERRUPT;
		extiConfig.Trigger = EXTI_TRIGGER_RISING; // rising edge or falling edge
		HAL_EXTI_SetConfigLine(&hexti, &extiConfig);

		// set callback functions
		HAL_EXTI_RegisterCallback(&hexti, HAL_EXTI_COMMON_CB_ID, EXTI_Callbacks[i]);
        
    }
	

}