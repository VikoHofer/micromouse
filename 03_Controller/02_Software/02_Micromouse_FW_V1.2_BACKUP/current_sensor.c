/*************************************************************
Project		: Micromouse V1.0
File		: current_sensor.c
Description	: Read Sensors using ADC / Implementation
Date		: 19.12.2024
*************************************************************/

#include "current_sensor.h"
#include "adc.h"

// struct to identify GPIO
typedef struct {
    GPIO_TypeDef* GPIO_Port;
    uint16_t GPIO_Pin;
} GPIO_Pin_t;

// array for every Sensor_GPIO pinout 
static const GPIO_Pin_t CURRENT_SENSOR_PIN[] = 
{
    { GPIOC, GPIO_PIN_4 },
    { GPIOC, GPIO_PIN_5 }
};

/**
  * @brief  Initialize current sensors.
  * @param  None
  * @retval Sensor_RetType: Return value indicating the status.
*/
Sensor_Current_RetType Current_Sensor_Init(void){
    
    // enable clock
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // init sensor input GPIOs
    for (unsigned int i = 0; i < sizeof(CURRENT_SENSOR_PIN) / sizeof(CURRENT_SENSOR_PIN[0]); ++i) {
        GPIO_Pin_t pin = CURRENT_SENSOR_PIN[i];
        
        // initialize GPIO-Pins as Input
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.Pin = pin.GPIO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL; 
        HAL_GPIO_Init(pin.GPIO_Port, &GPIO_InitStruct);
    }
    
    return CSENSOR_OK;  
}

/**
  * @brief  read from specified Sensor-ID.
  * @param  SensorID, distance, ADC-Channel
  * @retval Sensor_RetType: Return value indicating the status.
*/
Sensor_Current_RetType Current_Sensor_Read(curr_SensorID sensor, ADC_HandleTypeDef* adcID, uint32_t* dist){
    
    uint32_t adc_channel = 0;

    if (ADC_Disable(adcID)!= ADC_OK){
        return CSENSOR_ERR;
    }
    
    if (ADC_Enable(adcID) != ADC_OK){
        return CSENSOR_ERR;
    }
    
    // change channels according to our pin config
    switch(sensor) {
        case SENSOR_LEFT:
            adc_channel = ADC_CHANNEL_14;
            break;
        case SENSOR_RIGHT:
            adc_channel = ADC_CHANNEL_15;
            break;
        default:
            return CSENSOR_ERR_INVALID_PARAM;
    }

    // Start ADC conversion using the specified channel
    if(ADC_Convert(adcID,adc_channel) == ADC_ERR){
        *dist = 0;
        return CSENSOR_ERR_READ;
    }
    
    *dist = ADC_Convert(adcID, adc_channel);
    
    /*if (*dist < 0 || *dist > 5000) {
        return SENSOR_ERR_INVALID_DATA;
    }*/
 
    return CSENSOR_OK;
}

/**
  * @brief  reads every sensor.
  * @param  distance variable
  * @retval Sensor_RetType: Return value indicating the status.
*/
Sensor_Current_RetType Current_Sensor_TestAll(uint32_t* dist){
    
    for(int i = 0; i < CNUM_SENSORS; i++){
        Current_Sensor_Read(i, &hadc1, dist); // Read each sensor using ADC2
        dist++;
    }   
       
    return CSENSOR_OK;
}

/**
  * @brief  disables every referenced GPIO.
  * @param  none
  * @retval Sensor_RetType: Return value indicating the status.
*/
Sensor_Current_RetType Current_Sensor_DeInit(void){
    
    // deinit current sensor out GPIOs
    for (unsigned int i = 0; i < sizeof(CURRENT_SENSOR_PIN) / sizeof(CURRENT_SENSOR_PIN[0]); i++){            
        HAL_GPIO_DeInit(CURRENT_SENSOR_PIN[i].GPIO_Port, CURRENT_SENSOR_PIN[i].GPIO_Pin);
    }
    
    return CSENSOR_OK;    
}
