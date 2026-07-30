/*************************************************************
Project		: Micromouse V1.0
File		: sensor.h
Description	: Configurate sensor module
Date		: 09.09.2024
*************************************************************/

#ifndef SENSOR_Included_H
#define SENSOR_Included_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h" 

/* Defines  ------------------------------------------------------------------*/
#define NUM_SENSORS 6

/* Exported types ------------------------------------------------------------*/
typedef enum 
{
    SENSOR_OK = 0,
    SENSOR_ERR = -1,
    SENSOR_ERR_INVALID_PARAM = -2,
    SENSOR_ERR_READ = -3
} Sensor_RetType;

typedef enum
{
    SENSOR_ID_1 = 0,    // x_SENSOR_PIN[0]
    SENSOR_ID_2 = 1,    // x_SENSOR_PIN[1]
    SENSOR_ID_3 = 2,    // x_SENSOR_PIN[2]
    SENSOR_ID_4 = 3,    // x_SENSOR_PIN[3]
    SENSOR_ID_5 = 4,    // x_SENSOR_PIN[4]
    SENSOR_ID_6 = 5     // x_SENSOR_PIN[5]
} SensorID;

/* Exported functions ------------------------------------------------------- */

/**
  * @brief  Initialize Sensor including GPIO ports.
  * @param  None
  * @retval The result of the operation.
*/
Sensor_RetType Sensor_Init(void);


/**
  * @brief  read from specified Sensor-ID.
  * @param  SensorID, ADC-Channel, distance variable
  * @retval The result of the operation.
*/
Sensor_RetType Sensor_Read(SensorID sensor, ADC_HandleTypeDef* adcID, uint32_t* dist);



/**
  * @brief  Activate Transmitter Diode for Anglecontrol
  * @param  None
  * @retval void
*/
void activateVCC_AngleSensors(void);

/**
  * @brief  Deactivate Transmitter Diode for Anglecontrol
  * @param  None
  * @retval void
*/
void deactivateVCC_AngleSensors(void);

///**
//  * @brief  Read left angle sensor
//  * @param  None
//  * @retval void
//*/
//double readSensor_Left_45(void);

///**
//  * @brief  read right angle sensor
//  * @param  None
//  * @retval void
//*/
//double readSensor_Right_45(void);




/**
  * @brief  reads every sensor.
  * @param  distance variable (array perhaps)
  * @retval The result of the operation.
*/
Sensor_RetType Sensor_TestAll(uint32_t* dist);


/**
  * @brief  disables every referenced GPIO.
  * @param  none
  * @retval The result of the operation.
*/
Sensor_RetType Sensor_DeInit(void);


/**
  * @brief  Reads sensor values and assigns them to the provided pointers for right, middle, and left sensors.
  * @param  rightValue: Pointer to the variable where the value for the right sensor will be stored.
  * @param  middleValue: Pointer to the variable where the value for the middle sensor will be stored.
  * @param  leftValue: Pointer to the variable where the value for the left sensor will be stored.
  * @retval The result of the operation.
*/
Sensor_RetType GetValuesForMouse(uint32_t *leftValue,uint32_t* middleValue,uint32_t *rightValue);




#endif