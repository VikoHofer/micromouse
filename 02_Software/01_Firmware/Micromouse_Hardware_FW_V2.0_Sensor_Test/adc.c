/*************************************************************
Project		: Micromouse V1.0
File		: adc.c
Description	: Configure ADC-Peripheral / Implementation
Date		: 09.09.2024
*************************************************************/

#include "adc.h"

ADC_HandleTypeDef hadc1;

/**
  * @brief  Initialize ADC including GPIO ports.
  * @param  None
  * @retval The result of the operation.
*/
ADC_RetType ADC_Init(ADC_HandleTypeDef* id){
    
     __HAL_RCC_ADC1_CLK_ENABLE();
    
    
    ADC_ChannelConfTypeDef sConfig;
    
    // Initialize ADC handle
    id->Instance = ADC1;
    id->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    id->Init.Resolution = ADC_RESOLUTION_12B;
    id->Init.ScanConvMode = DISABLE;
    id->Init.ContinuousConvMode = DISABLE;
    id->Init.DiscontinuousConvMode = DISABLE;
    id->Init.DataAlign = ADC_DATAALIGN_RIGHT;
    id->Init.NbrOfConversion = 1;
    id->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    id->Init.DMAContinuousRequests = DISABLE;
    id->Init.ExternalTrigConv = ADC_SOFTWARE_START;
    id->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;

    // Initialize ADC peripheral
    if (HAL_ADC_Init(id) != HAL_OK)
    {
        // Initialization error
        return ADC_ERR;
    }
    
    return ADC_OK;
}

/**
  * @brief  Converts sensor input.
  * @param  ADC_Handle for ADC_Instance, channel for selecting sensor pin
  * @retval The result of the operation.
*/
uint32_t ADC_Convert(ADC_HandleTypeDef* id, uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig;

    // Configure the ADC channel
    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;

    if (HAL_ADC_ConfigChannel(id, &sConfig) != HAL_OK) {
        // Channel configuration error
        return UINT32_MAX;  // oder einen anderen speziellen Wert
    }

    // Start ADC conversion
    if (HAL_ADC_Start(id) != HAL_OK) {
        // ADC start error
        return UINT32_MAX;  // oder einen anderen speziellen Wert
    }

    // Wait for the conversion to complete
    if (HAL_ADC_PollForConversion(id, 100) != HAL_OK) {  // 100ms Timeout als Beispiel
        // ADC conversion error
        return UINT32_MAX;  // oder einen anderen speziellen Wert
    }

    // Get the converted value
    uint32_t convertedValue = HAL_ADC_GetValue(id);

    return convertedValue;  
}

/**
  * @brief  Disables ADC
  * @param  ADC_Handle for ADC_Instance
  * @retval The result of the operation.
*/
ADC_RetType ADC_Disable(ADC_HandleTypeDef* id){

    HAL_ADC_Stop(id);
    
    return ADC_OK;
}

/**
  * @brief  Enables ADC
  * @param  ADC_Handle for ADC_Instance
  * @retval The result of the operation.
*/
ADC_RetType ADC_Enable(ADC_HandleTypeDef* id){

    HAL_ADC_Start(id);

    return ADC_OK;
}

