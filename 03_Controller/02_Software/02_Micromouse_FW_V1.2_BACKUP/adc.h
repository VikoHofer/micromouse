/*************************************************************
Project		: Micromouse V1.0
File		: adc.h
Description	: Configure ADC-Peripheral
Date		: 09.09.2024
*************************************************************/

#ifndef ADC_Included_H
#define ADC_Included_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h" 
#include "stm32f4xx_hal_adc.h"

/* Defines  ------------------------------------------------------------------*/


/* Exported types ------------------------------------------------------------*/
typedef enum 
{
    ADC_OK = 0,
    ADC_ERR = 1,
    ADC_ERR_INVALID_PARAM = 2
} ADC_RetType;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

/* Exported functions ------------------------------------------------------- */

/**
  * @brief  Initialize ADC including GPIO ports.
  * @param  ADC_Handle for ADC_Instance
  * @retval The result of the operation.
*/
ADC_RetType ADC_Init(ADC_HandleTypeDef* id);


/**
  * @brief  Converts sensor input.
  * @param  ADC_Handle for ADC_Instance, channel for selecting sensor pin
  * @retval The result of the operation.
*/
uint32_t ADC_Convert(ADC_HandleTypeDef* id, uint32_t channel);

/**
  * @brief  Disables ADC
  * @param  ADC_Handle for ADC_Instance
  * @retval The result of the operation.
*/
ADC_RetType ADC_Disable(ADC_HandleTypeDef* id);

/**
  * @brief  Enables ADC
  * @param  ADC_Handle for ADC_Instance
  * @retval The result of the operation.
*/
ADC_RetType ADC_Enable(ADC_HandleTypeDef* id);

#endif
