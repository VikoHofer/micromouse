/*************************************************************
Project		: Micromouse V1.0
File		: board_led.c
Description	: Control Board LEDs using an array (Implementation)
Date		: 09.09.2024
*************************************************************/

#include "stm32f4xx.h"
#include "stm32f4xx_hal_gpio.h"
#include "board_led.h"

// LED Pins and Ports
static GPIO_TypeDef* LED_PORTS[NUM_LEDS] = {GPIOA, GPIOA, GPIOD}; 
static const uint16_t LED_PINS[NUM_LEDS] = {GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_2};

//// Init Board LEDs
void Board_LED_Init(void) {
    GPIO_InitTypeDef initStruct;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

    // Initialisiere jeden LED-Port und Pin
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
		

        // Configure GPIO
        initStruct.Pin = LED_PINS[i];
        initStruct.Mode = GPIO_MODE_OUTPUT_PP;
        initStruct.Pull = GPIO_NOPULL;
        initStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(LED_PORTS[i], &initStruct);

        // Turn off all LEDs initially
        HAL_GPIO_WritePin(LED_PORTS[i], LED_PINS[i], GPIO_PIN_RESET);
    }
}

// Toggle a specific LED
void Toggle_Board_LED(uint8_t led_num) {
    if (led_num < NUM_LEDS) {
        HAL_GPIO_TogglePin(LED_PORTS[led_num], LED_PINS[led_num]);
    }
}

// Turn on a specific LED
void Board_LED_On(uint8_t led_num) {
    if (led_num < NUM_LEDS) {
        HAL_GPIO_WritePin(LED_PORTS[led_num], LED_PINS[led_num], GPIO_PIN_SET);
    }
}

// Turn off a specific LED
void Board_LED_Off(uint8_t led_num) {
    if (led_num < NUM_LEDS) {
        HAL_GPIO_WritePin(LED_PORTS[led_num], LED_PINS[led_num], GPIO_PIN_RESET);
    }
}
