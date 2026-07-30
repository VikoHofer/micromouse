/*************************************************************
Project		: Micromouse V1.0
File		: current_sensor.h
Description	: configure current sensor module
Date		: 19.12.2024
*************************************************************/

#ifndef CURRENT_SENSOR_Included_H
#define CURRENT_SENSOR_Included_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h" 

/* Defines  ------------------------------------------------------------------*/
#define CNUM_SENSORS 2

/* Exported types ------------------------------------------------------------*/
typedef enum 
{
    CSENSOR_OK = 0,
    CSENSOR_ERR = -1,
    CSENSOR_ERR_INVALID_PARAM = -2,
    CSENSOR_ERR_READ = -3,
    CSENSOR_ERR_INVALID_DATA = -4
} Sensor_Current_RetType;

typedef enum
{
    SENSOR_LEFT = 0,     // x_SENSOR_PIN[0]
    SENSOR_RIGHT = 1,    // x_SENSOR_PIN[1]
} curr_SensorID;

/* Exported functions ------------------------------------------------------- */

/**
  * @brief  Initialize current sensors.
  * @param  None
  * @retval Sensor_RetType: Return value indicating the status.
*/
Sensor_Current_RetType Current_Sensor_Init(void);


/**
  * @brief  read from specified Sensor-ID.
  * @param  SensorID, ADC-Channel, distance variable
  * @retval Sensor_RetType: Return value indicating the status.
*/
Sensor_Current_RetType Current_Sensor_Read(curr_SensorID sensor, ADC_HandleTypeDef* adcID, uint32_t* dist);


/**
  * @brief  reads every sensor.
  * @param  distance variable (array perhaps)
  * @retval Sensor_RetType: Return value indicating the status.
*/
Sensor_Current_RetType Current_Sensor_TestAll(uint32_t* dist);


/**
  * @brief  disables every referenced GPIO.
  * @param  none
  * @retval Sensor_RetType: Return value indicating the status.
*/
Sensor_Current_RetType Current_Sensor_DeInit(void);


#endif