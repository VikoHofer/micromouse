/*************************************************************
Project		: Micromouse V1.0
File		: board_led.h
Description	: Control Board LEDs using an array
Date		: 09.09.2024
*************************************************************/

#ifndef BOARD_LED_H
#define BOARD_LED_H

#include "stm32f4xx_hal.h"

#define NUM_LEDS 3  // Anzahl der LEDs

void Board_LED_Init(void);

// Board LED control functions
void Toggle_Board_LED(uint8_t led_num);
void Board_LED_On(uint8_t led_num);
void Board_LED_Off(uint8_t led_num);

#endif