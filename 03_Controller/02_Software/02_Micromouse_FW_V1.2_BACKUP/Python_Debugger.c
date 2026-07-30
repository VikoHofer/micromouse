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
#include "drive_control.h"


// -------------- globals and defines --------------------------------------------------

#define INTERBYTE_TIMEOUT_MS 5
#define MAX_DATA 253

// Array to store the state of each sensor
static uint8_t active_sensors[NUM_SENSORS] = {0}; 

// -------------- test functions --------------------------------------------------

void Send_Active_Sensors(int id) {
    for (uint8_t sensorID = 0; sensorID < NUM_SENSORS; sensorID++) {
        if (active_sensors[sensorID]) {
            uint32_t sensorValue = 0;
            if (Sensor_Read(sensorID, &hadc1, &sensorValue) == SENSOR_OK) {
                uint8_t data[3];  // 1 byte for sensor ID, 2 bytes for sensor value
                data[0] = sensorID;
                data[1] = (((uint16_t)sensorValue) >> 8) & 0xFF;  // MSB
                data[2] = (uint16_t)sensorValue & 0xFF;         // LSB
                Create_Frame(CMD_SENSOR_VALUE, data, sizeof(data));
				//PY_Printf("ID: %d, Value: %d, Len: %d", sensorID, sensorValue, sizeof(data));
            }
        }
    }
	//PY_Printf("Active sensor TEST\n ");
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

	
	Create_Frame(CMD_STRING, (uint8_t*)buffer, strlen(buffer)+1);
}

/******************************************************
* Initialize used peripheral modules
******************************************************/
void PY_Debugger_Init(void){
	Timer_Initialize(TIMER_ID_TIM5, 60, Send_Active_Sensors);
	Timer_Stop(TIMER_ID_TIM5);
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








// -------------- private functions --------------------------------------------------



/******************************************************
* get called when parsing is done
* cmds will be executed!
******************************************************/
static void Analyse_Data(uint8_t tag, uint8_t *data, uint8_t data_len){
	
	uint8_t cmd_PWM = 0;
	uint16_t cmd_SPEED = 0;
	
	//PY_Printf("Tag: %d, Len: %d, Data: %d", tag, data_len, data[0]);
	
    // np check
    if (data != 0) {
		
        switch (tag)
        {
			// loopback: send string back
            case CMD_STRING: 
                Create_Frame(tag, data, data_len);
                break;
			
			// activate sensor to send data
            case CMD_SENSOR_ACTIVATE: 
				Timer_Start(TIMER_ID_TIM5);
				if (data_len == 1) {
					uint8_t sensorID = data[0];
					if (sensorID < NUM_SENSORS) {
						active_sensors[sensorID] = 1;  // Activate the sensor
					}
				}
				break;
			// activate sensor to send data
            case CMD_SENSOR_DEACTIVATE: 
				if (data_len == 1) {
					uint8_t sensorID = data[0];
					if (sensorID < NUM_SENSORS) {
						active_sensors[sensorID] = 0;  // Deactivate the sensor
					}
				}
				break;
			
			// Motor Test
			case CMD_GET_ID:
				cmd_PWM = data[0];
				
				PY_Printf("Motor Identification!!");
				PY_Printf("Volt: %d, PWM: %d", ((cmd_PWM+1) / 51), cmd_PWM);
			
				Get_ID_Motor(cmd_PWM);
				break;
			
			// Sprungantwort
			case CMD_STEP_PD: 
				cmd_SPEED = (data[0] << 8) | data[1];		
				PD_Step_Response(cmd_SPEED);
				break;
			
			// Weg fahren
			case CMD_MOVE_PD: 
				PD_Move();
				break;

            default:
                break;
        }
        
    }
}


/******************************************************
* get called every time a byte has received
* parses the single bytes to a frame
******************************************************/
static void PY_Protocol_Parser(uint8_t rxByte) {
	
    // state machine
    static enum {IDLE, TAG, LENGTH, DATA, DONE} state = IDLE;
    static uint8_t len, tag, cnt = 0; 
    static uint8_t data[MAX_DATA];
    static uint32_t ticksForLastByte = 0;
	
	//PY_Printf("rxByte: %d", rxByte);
    
    // timeout detection
    if (HAL_GetTick() - ticksForLastByte > INTERBYTE_TIMEOUT_MS){
		state = IDLE;
	}
    ticksForLastByte = HAL_GetTick();
    
    switch (state) {
        case IDLE:
            // Expecting a tag
            tag = rxByte;
            state = TAG;
            break;
            
        case TAG:
            // Expecting length
            len = rxByte;
            if (len <= MAX_DATA) {
                cnt = 0;
                state = LENGTH;
            } else {
                // Invalid length, reset state
                state = IDLE;
            }
            break;
            
        case LENGTH:
            // Expecting data
            data[cnt++] = rxByte;
		
            if (cnt >= len) {
                // Parse done
				Analyse_Data(tag, data, len);
				state = IDLE;
            }
            break;

        default:
            // Set IDLE when error occurs
            state = IDLE;
            break;
    }
}




// -------------- public functions --------------------------------------------------

/******************************************************
* create a data frame an sends it
******************************************************/
void Create_Frame(uint8_t tag, uint8_t* data, uint8_t data_len) {
    // np check
    if (data == 0) {
        return;
    }
    
    // send tag
    Uart6_PutChar(tag);
    
    // send length
    Uart6_PutChar(data_len);
    
    // send data
    for (int i = 0; i < data_len; i++) {
        Uart6_PutChar(data[i]);
    }
}

/******************************************************
* checks if data was received and parses data
******************************************************/
void PY_Process(void) {
    int8_t rxData = 0;
    rxData = Uart6_GetChar();
    
    if (rxData != NO_DATA_IN_RX_BUFFER) {
        PY_Protocol_Parser(rxData);
    }
}




/************************************
* Data Send Functions
************************************/
void PY_SEND_PWM(uint8_t const PWM) {
    uint8_t data[1];  
    data[0] = PWM;    
	
    Create_Frame(CMD_SEND_PWM, data, 1);
}
void PY_SEND_SPEED(int16_t const SPEED) {
    uint8_t data[2];
    uint16_t unsigned_speed = (uint16_t)SPEED;  // Cast to unsigned to handle bit shifts correctly

    data[0] = (unsigned_speed >> 8) & 0xFF;  // MSB
    data[1] = unsigned_speed & 0xFF;         // LSB
    
    Create_Frame(CMD_SEND_SPEED, data, 2);
}
