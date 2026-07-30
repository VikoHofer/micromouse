/*************************************************************
Project		: Micromouse V1.0
File		: Print.h
Description	: Print Functions for Debugging using Uart6
Date		: 09.09.2024
*************************************************************/

#ifndef PRINT_H
#define PRINT_H

#include <stdarg.h>
#include <stdio.h>
#include "string.h"
#include "Uart.h"


/***********************************
* uncomment this define to activate 
* debugging feature via uart
***********************************/
#define DEBUG_ACTIVATED 1

// Init Uart
void Print_Initialize(void);

// like the normal PrintF
void uart_printf(const char* format, ...);

// send normal strings
void uart_print(const char* str);

#endif