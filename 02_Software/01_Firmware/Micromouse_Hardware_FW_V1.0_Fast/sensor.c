/*************************************************************
Project		: Micromouse V1.0
File		: sensor.c
Description	: Read Sensors using ADC / Implementation
Date		: 09.09.2024
*************************************************************/

#include "sensor.h"
#include "config.h"
#include "adc.h"
#include "Delay.h"


// struct to identify GPIO
typedef struct {
    GPIO_TypeDef* GPIO_Port;
    uint16_t GPIO_Pin;
} GPIO_Pin_t;

// array for every Sensor_GPIO pinout 
static const GPIO_Pin_t SENSOR_PIN[] = 
{
    { GPIOA, GPIO_PIN_0 },
    { GPIOA, GPIO_PIN_1 },
    { GPIOA, GPIO_PIN_4 },
    { GPIOB, GPIO_PIN_0 },
    { GPIOC, GPIO_PIN_1 },
    { GPIOC, GPIO_PIN_0 }
};

// array for every Sensor_GPIO_VCC
static const GPIO_Pin_t VCC_SENSOR_PIN[] = 
{
    { GPIOB, GPIO_PIN_7 }, 
    { GPIOC, GPIO_PIN_13 },
    { GPIOC, GPIO_PIN_10 },
    { GPIOC, GPIO_PIN_11 },
    { GPIOC, GPIO_PIN_2 },
    { GPIOC, GPIO_PIN_3 }
};

/**
  * @brief  Initialize Sensor including GPIO ports.
  * @param  None
  * @retval The result of the operation.
*/
Sensor_RetType Sensor_Init(void){
    
    // enable clock
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // init sensor input GPIOs
    for (unsigned int i = 0; i < sizeof(SENSOR_PIN) / sizeof(SENSOR_PIN[0]); ++i) {
        GPIO_Pin_t pin = SENSOR_PIN[i];
        
        // initialize GPIO-Pins as Input
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.Pin = pin.GPIO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL; 
        HAL_GPIO_Init(pin.GPIO_Port, &GPIO_InitStruct);
    }
    
    // init sensor vcc GPIOs
    for (unsigned int i = 0; i < sizeof(VCC_SENSOR_PIN) / sizeof(VCC_SENSOR_PIN[0]); ++i) {
        GPIO_Pin_t pin = VCC_SENSOR_PIN[i];
        
        // initialize GPIO-Pins as Input
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.Pin = pin.GPIO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN; 
        HAL_GPIO_Init(pin.GPIO_Port, &GPIO_InitStruct);
        
        // Set the VCC pin to low initially
        HAL_GPIO_WritePin(pin.GPIO_Port, pin.GPIO_Pin, GPIO_PIN_RESET);
    }
	
	if (ADC_Enable(&hadc1) != ADC_OK){
        return SENSOR_ERR;
    }

    return SENSOR_OK;  
}

/**
  * @brief  read from specified Sensor-ID.
  * @param  SensorID, distance, ADC-Channel
  * @retval The result of the operation.
*/
Sensor_RetType Sensor_Read(SensorID sensor, ADC_HandleTypeDef* adcID, uint32_t* dist){
    GPIO_Pin_t pin = VCC_SENSOR_PIN[sensor];
	
	// Turn off the VCC pin after the reading is done
    HAL_GPIO_WritePin(pin.GPIO_Port, pin.GPIO_Pin, GPIO_PIN_RESET);
	
    uint32_t adc_channel = 0;
	
	uint32_t adc_dark = 0;
	uint32_t adc_lit = 0;

	
	// change channels according to our pin config
    switch(sensor) {
        case SENSOR_ID_1:
            adc_channel = ADC_CHANNEL_0;
            break;
        case SENSOR_ID_2:
            adc_channel = ADC_CHANNEL_1;
            break;
        case SENSOR_ID_3:
            adc_channel = ADC_CHANNEL_4;
            break;
        case SENSOR_ID_4:
            adc_channel = ADC_CHANNEL_8;
            break;
        case SENSOR_ID_5:
            adc_channel = ADC_CHANNEL_11;
            break;
        case SENSOR_ID_6:
            adc_channel = ADC_CHANNEL_10;
            break;
        default:
            return SENSOR_ERR_INVALID_PARAM;
    }

	// measure light without emitter
	//adc_dark = ADC_Convert(adcID, adc_channel);
    
    // turn on VCC_PIN 
    HAL_GPIO_WritePin(pin.GPIO_Port, pin.GPIO_Pin, GPIO_PIN_SET);
	
	// Wait until diode is stable before read it
	// MINIMANLE ZEIT HERAUSFINDEN !!!!!!!!!!!!!!!!!
	//Delay_ms(IR_DELAY);
	HAL_Delay(IR_DELAY);
	
	// measure light with emitter
	adc_lit = ADC_Convert(adcID, adc_channel);
 
	// subtract ambiet light distubance
    *dist = adc_lit;
	
	HAL_GPIO_WritePin(pin.GPIO_Port, pin.GPIO_Pin, GPIO_PIN_RESET);
    
    return SENSOR_OK;
}

/**
  * @brief  activate emitter diode
  * @param  SensorID
  * @retval The result of the operation.
*/
Sensor_RetType Sensor_Activate_Emitter(SensorID sensor){
	GPIO_Pin_t pin = VCC_SENSOR_PIN[sensor];
	// turn on VCC_PIN 
    HAL_GPIO_WritePin(pin.GPIO_Port, pin.GPIO_Pin, GPIO_PIN_SET);
	
	return SENSOR_OK;
}

/**
  * @brief  deactivate emitter diode
  * @param  SensorID
  * @retval The result of the operation.
*/
Sensor_RetType Sensor_Deactivate_Emitter(SensorID sensor){
	GPIO_Pin_t pin = VCC_SENSOR_PIN[sensor];

	HAL_GPIO_WritePin(VCC_SENSOR_PIN[sensor].GPIO_Port, VCC_SENSOR_PIN[sensor].GPIO_Pin, GPIO_PIN_RESET);

	return SENSOR_OK;
}

/**
  * @brief  reads every sensor.
  * @param  distance variable
  * @retval The result of the operation.
*/
Sensor_RetType Sensor_TestAll(uint32_t* dist){
    
    for(int i = 0; i < NUM_SENSORS; i++){
        Sensor_Read(i, &hadc1, dist); // Read each sensor using ADC1
        dist++;
    }   
       
    return SENSOR_OK;
}

/**
  * @brief  disables every referenced GPIO.
  * @param  none
  * @retval The result of the operation.
*/
Sensor_RetType Sensor_DeInit(void){
    
    // deinit sensor out GPIOs
    for (unsigned int i = 0; i < sizeof(SENSOR_PIN) / sizeof(SENSOR_PIN[0]); i++){            
        HAL_GPIO_DeInit(SENSOR_PIN[i].GPIO_Port, SENSOR_PIN[i].GPIO_Pin);
    }
    
    // deinit sensor vcc GPIOs
    for (unsigned int i = 0; i < sizeof(VCC_SENSOR_PIN) / sizeof(VCC_SENSOR_PIN[0]); ++i) {
        HAL_GPIO_DeInit(VCC_SENSOR_PIN[i].GPIO_Port, VCC_SENSOR_PIN[i].GPIO_Pin);
    }
    
    return SENSOR_OK;    
}

// TODO: process every sensor value
//RHA wird nicht mehr verwendet? L?SCHEN?
/**
  * @brief  Reads sensor values and assigns them to the provided pointers for right, middle, and left sensors.
  * @param  rightValue: Pointer to the variable where the value for the right sensor will be stored.
  * @param  middleValue: Pointer to the variable where the value for the middle sensor will be stored.
  * @param  leftValue: Pointer to the variable where the value for the left sensor will be stored.
  * @retval The result of the operation.
*/
Sensor_RetType GetValuesForMouse(uint32_t *rightValue, uint32_t* middleValue, uint32_t *leftValue){
    
    uint32_t dist0 = 0;
    uint32_t dist1 = 0;
    uint32_t dist2 = 0;
    uint32_t dist3 = 0;
    uint32_t dist4 = 0;
    uint32_t dist5 = 0;
    
    if(Sensor_Read(0, &hadc1, &dist0) != SENSOR_OK){
        // TODO: Error handling reset corresponding distx value 
    }
    
    if(Sensor_Read(1, &hadc1, &dist1) != SENSOR_OK){
        // TODO: Error handling reset corresponding distx value 
    }
    
    if(Sensor_Read(2, &hadc1, &dist2) != SENSOR_OK){
        // TODO: Error handling reset corresponding distx value 
    }
       
    if(Sensor_Read(3, &hadc1, &dist3) != SENSOR_OK){
        // TODO: Error handling reset corresponding distx value
    }        
       
    if(Sensor_Read(4, &hadc1, &dist4) != SENSOR_OK){
        // TODO: Error handling reset corresponding distx value 
    }
        
    if(Sensor_Read(5, &hadc1, &dist5) != SENSOR_OK){
        // TODO: Error handling reset corresponding distx value
    }
    

    *rightValue = dist4;
    *middleValue = dist3;
    *leftValue = dist5;
    
    return SENSOR_OK;
}