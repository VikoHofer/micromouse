/*************************************************************
Project		: Micromouse V1.0
File		: board_led.h
Description	: Controll Board LED
Date		: 09.09.2024
*************************************************************/

// !!!!!! PINS ANPASSEN !!!!!!!!!

#ifndef BOARD_LED_H
#define BOARD_LED_H


void Board_LED_Init(void);

// Board test functions
void Toggle_Board_LED(void);
void Board_LED_On(void);
void Board_LED_Off(void);


#endif
