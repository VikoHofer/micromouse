/*************************************************************
Project		: Micromouse V1.0
File		: Python_Debugger.h
Description	: Communication and Testing using UART with Python Debugger
Date		: 09.09.2024
*************************************************************/

#ifndef PY_DEBUGGER_H
#define PY_DEBUGGER_H

#include "Uart.h"
#include "config.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include "string.h"


// Initialize all used peripherals
void PY_Debugger_Init(void);


// printf using PY protocol
void PY_Printf(const char* format, ...);



#endif