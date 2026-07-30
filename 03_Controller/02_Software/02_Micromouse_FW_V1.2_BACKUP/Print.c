/*************************************************************
Project		: Micromouse V1.0
File		: Print.c
Description	: Print Functions for Debugging using Uart6 / Implementation
Date		: 09.09.2024
*************************************************************/

#include "Print.h"

void Print_Initialize(void){
	Uart6_Init();
}

#ifdef DEBUG_ACTIVATED

// Print format function with UART transmission
void uart_printf(const char *format, ...)
{
    char buffer[256]; // Assuming a maximum string length of 256 characters
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Uart6_SendData((uint8_t *)buffer, strlen(buffer));
}

// Print function without formatting with UART transmission
void uart_print(const char *string)
{
    Uart6_SendData((uint8_t *)string, strlen(string));
}

#else

// Print format function with UART transmission
void uart_printf(const char *format, ...)
{
	// no debugging
}

// Print function without formatting with UART transmission
void uart_print(const char *string)
{
    // no debugging
}

#endif