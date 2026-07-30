/*************************************************************
Project		: Micromouse V1.0
File		: Python_Debugger.c
Description	: Communication and Testing using UART with Python Debugger / Implementation
Date		: 09.09.2024
*************************************************************/

#include "Python_Debugger.h"
#include "sensor.h"
#include "adc.h"
#include "Timer.h"
#include "MotorDriver.h"
#include "encoder.h"


// -------------- globals and defines --------------------------------------------------

#define INTERBYTE_TIMEOUT_MS 5
#define MAX_DATA 253

// Array to store the state of each sensor
static uint8_t active_sensors[NUM_SENSORS] = {0}; 

/******************************************************
* create a data frame an sends it
******************************************************/
static void Create_Frame(uint8_t* data, uint8_t data_len) {
    // np check
    if (data == 0) {
        return;
    }
    
    // send length
    Uart6_PutChar(data_len);
    
    // send data
    for (int i = 0; i < data_len; i++) {
        Uart6_PutChar(data[i]);
    }
}


#ifdef PY_DEBUG_ACTIVATED

/******************************************************
* printf using PY protocol
******************************************************/
void PY_Printf(const char* format, ...){
	char buffer[256]; // Assuming a maximum string length of 256 characters
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

	
	Create_Frame((uint8_t*)buffer, strlen(buffer)+1);
}



#else 



/******************************************************
* printf using PY protocol
******************************************************/
void PY_Printf(const char* format, ...){
}

/******************************************************
* Initialize used peripheral modules
******************************************************/
void PY_Debugger_Init(void){

}

#endif



