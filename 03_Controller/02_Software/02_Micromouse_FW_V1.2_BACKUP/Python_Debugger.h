/*************************************************************
Project		: Micromouse V1.0
File		: Python_Debugger.h
Description	: Communication and Testing using UART with Python Debugger
Date		: 09.09.2024
*************************************************************/

#ifndef PY_DEBUGGER_H
#define PY_DEBUGGER_H

#include "Uart.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include "string.h"

/***********************************
* uncomment this define to activate 
* debugging feature via uart
***********************************/
#define PY_DEBUG_ACTIVATED 1



// tags (COMMANDS)
typedef enum{
	CMD_STRING 	= 0x01,
	CMD_SENSOR_ACTIVATE = 0x02, 	// (Data: SensorID (1 Byte))
	CMD_SENSOR_DEACTIVATE = 0x03, 	// (Data: SensorID (1 Byte))
	CMD_SENSOR_VALUE = 0x04,		// (Data: 1. SensorID, 2. Value (2 Byte))
	CMD_GET_ID = 0x05,				// Data: 1 Byte	 = PWM Value
	CMD_SEND_PWM = 0x06,			// Data: 1 Byte PWM Value
	CMD_SEND_SPEED = 0x07,			// Data: 2 Byte (MSB, LSB)
	CMD_STEP_PD = 0x08,				// Data: 2 Byte (MSB, LSB)
	CMD_MOVE_PD = 0x09				// Data: 1 Byte	 = 0x00
} eCommand;

// Initialize all used peripherals
void PY_Debugger_Init(void);

// Create a frame and sends it
void Create_Frame(uint8_t tag, uint8_t* data, uint8_t data_len);

// checks if data was received and parses data
void PY_Process(void);


// printf using PY protocol
void PY_Printf(const char* format, ...);


/************************************
* Data Send Functions
************************************/
void PY_SEND_PWM(uint8_t const PWM);
void PY_SEND_SPEED(int16_t const SPEED);

#endif