/*************************************************************
Project     : Micromouse V1.0
File        : sensor.c
Description : Read Sensors using ADC / Implementation
Date        : 09.09.2024
*************************************************************/

#include "sensor.h"
#include "config.h"
#include "adc.h"
#include "Delay.h"

typedef struct {
    GPIO_TypeDef* GPIO_Port;
    uint16_t GPIO_Pin;
} GPIO_Pin_t;

/*
 * SENSOR_ID_1 = RIGHT_45
 *   ADC    : PB0  -> ADC_CHANNEL_8
 *   Switch : PC13
 *
 * SENSOR_ID_2 = LEFT_45
 *   ADC    : PA4  -> ADC_CHANNEL_4
 *   Switch : PC2
 */
static const GPIO_Pin_t SENSOR_PIN[NUM_SENSORS] =
{
    { GPIOB, GPIO_PIN_0 },   /* SENSOR_ID_1 = RIGHT ADC */
    { GPIOA, GPIO_PIN_4 }    /* SENSOR_ID_2 = LEFT  ADC */
};

static const GPIO_Pin_t VCC_SENSOR_PIN[NUM_SENSORS] =
{
    { GPIOC, GPIO_PIN_13 },  /* SENSOR_ID_1 = RIGHT switch */
    { GPIOC, GPIO_PIN_2  }   /* SENSOR_ID_2 = LEFT  switch */
};

static void Sensor_AllEmittersOff(void)
{
    unsigned int i;

    for (i = 0U; i < NUM_SENSORS; i++)
    {
        HAL_GPIO_WritePin(VCC_SENSOR_PIN[i].GPIO_Port,
                          VCC_SENSOR_PIN[i].GPIO_Pin,
                          GPIO_PIN_RESET);
    }
}

static uint32_t Sensor_GetAdcChannel(SensorID sensor)
{
    switch (sensor)
    {
        case SENSOR_ID_1:
            return ADC_CHANNEL_8;   /* PB0 = RIGHT */

        case SENSOR_ID_2:
            return ADC_CHANNEL_4;   /* PA4 = LEFT */

        default:
            return UINT32_MAX;
    }
}

Sensor_RetType Sensor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    unsigned int i;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    for (i = 0U; i < NUM_SENSORS; i++)
    {
        GPIO_InitStruct.Pin  = SENSOR_PIN[i].GPIO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(SENSOR_PIN[i].GPIO_Port, &GPIO_InitStruct);
    }

    for (i = 0U; i < NUM_SENSORS; i++)
    {
        GPIO_InitStruct.Pin   = VCC_SENSOR_PIN[i].GPIO_Pin;
        GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

        HAL_GPIO_Init(VCC_SENSOR_PIN[i].GPIO_Port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(VCC_SENSOR_PIN[i].GPIO_Port,
                          VCC_SENSOR_PIN[i].GPIO_Pin,
                          GPIO_PIN_RESET);
    }


    if (ADC_Init(&hadc1) != ADC_OK) {
        return SENSOR_ERR;
    }


    if (ADC_Enable(&hadc1) != ADC_OK) {
        return SENSOR_ERR;
    }

    return SENSOR_OK;
}

Sensor_RetType Sensor_Read(SensorID sensor, ADC_HandleTypeDef* adcID, uint32_t* dist)
{
    GPIO_Pin_t pin;
    uint32_t adc_channel;
    uint32_t adc_dark;
    uint32_t adc_lit;

    if ((uint32_t)sensor >= NUM_SENSORS) {
        return SENSOR_ERR_INVALID_PARAM;
    }

    if ((adcID == NULL) || (dist == NULL)) {
        return SENSOR_ERR_INVALID_PARAM;
    }

    pin = VCC_SENSOR_PIN[sensor];
    adc_channel = Sensor_GetAdcChannel(sensor);

    if (adc_channel == UINT32_MAX) {
        return SENSOR_ERR_INVALID_PARAM;
    }

    Sensor_AllEmittersOff();
    HAL_Delay(1);

    /* Dark-Messung: alle IR-Emitter aus */
    adc_dark = ADC_Convert(adcID, adc_channel);

    HAL_GPIO_WritePin(pin.GPIO_Port, pin.GPIO_Pin, GPIO_PIN_SET);
    HAL_Delay(IR_DELAY);

    /* Lit-Messung: nur eigener Emitter an */
    adc_lit = ADC_Convert(adcID, adc_channel);

    Sensor_AllEmittersOff();

    if ((adc_dark == UINT32_MAX) || (adc_lit == UINT32_MAX)) {
        *dist = 0U;
        return SENSOR_ERR_READ;
    }

    if ((adc_dark > 4095U) || (adc_lit > 4095U)) {
        *dist = 0U;
        return SENSOR_ERR_READ;
    }

    if (adc_lit > adc_dark) {
        *dist = adc_lit - adc_dark;
    } else {
        *dist = 0U;
    }

    return SENSOR_OK;
}

Sensor_RetType Sensor_Activate_Emitter(SensorID sensor)
{
    if ((uint32_t)sensor >= NUM_SENSORS) {
        return SENSOR_ERR_INVALID_PARAM;
    }

    HAL_GPIO_WritePin(VCC_SENSOR_PIN[sensor].GPIO_Port,
                      VCC_SENSOR_PIN[sensor].GPIO_Pin,
                      GPIO_PIN_SET);

    return SENSOR_OK;
}

Sensor_RetType Sensor_Deactivate_Emitter(SensorID sensor)
{
    if ((uint32_t)sensor >= NUM_SENSORS) {
        return SENSOR_ERR_INVALID_PARAM;
    }

    HAL_GPIO_WritePin(VCC_SENSOR_PIN[sensor].GPIO_Port,
                      VCC_SENSOR_PIN[sensor].GPIO_Pin,
                      GPIO_PIN_RESET);

    return SENSOR_OK;
}

Sensor_RetType Sensor_TestAll(uint32_t* dist)
{
    unsigned int i;

    if (dist == NULL) {
        return SENSOR_ERR_INVALID_PARAM;
    }

    for (i = 0U; i < NUM_SENSORS; i++)
    {
        if (Sensor_Read((SensorID)i, &hadc1, &dist[i]) != SENSOR_OK) {
            dist[i] = UINT32_MAX;
        }
    }

    return SENSOR_OK;
}

Sensor_RetType Sensor_DeInit(void)
{
    unsigned int i;

    for (i = 0U; i < NUM_SENSORS; i++)
    {
        HAL_GPIO_DeInit(SENSOR_PIN[i].GPIO_Port,
                        SENSOR_PIN[i].GPIO_Pin);
    }

    for (i = 0U; i < NUM_SENSORS; i++)
    {
        HAL_GPIO_WritePin(VCC_SENSOR_PIN[i].GPIO_Port,
                          VCC_SENSOR_PIN[i].GPIO_Pin,
                          GPIO_PIN_RESET);

        HAL_GPIO_DeInit(VCC_SENSOR_PIN[i].GPIO_Port,
                        VCC_SENSOR_PIN[i].GPIO_Pin);
    }

    ADC_Disable(&hadc1);

    return SENSOR_OK;
}

/*
 * Legacy-Funktion.
 * Bei nur 2 Sensoren gibt es keinen middleValue mehr.
 *
 * Header:
 * Sensor_RetType GetValuesForMouse(uint32_t *leftValue,
 *                                  uint32_t *middleValue,
 *                                  uint32_t *rightValue);
 */
Sensor_RetType GetValuesForMouse(uint32_t *leftValue,
                                 uint32_t *middleValue,
                                 uint32_t *rightValue)
{
    uint32_t right = 0U;
    uint32_t left  = 0U;
    Sensor_RetType ret = SENSOR_OK;

    if ((leftValue == NULL) || (rightValue == NULL)) {
        return SENSOR_ERR_INVALID_PARAM;
    }

    if (Sensor_Read(SENSOR_ID_1, &hadc1, &right) != SENSOR_OK) {
        right = UINT32_MAX;
        ret = SENSOR_ERR_READ;
    }

    if (Sensor_Read(SENSOR_ID_2, &hadc1, &left) != SENSOR_OK) {
        left = UINT32_MAX;
        ret = SENSOR_ERR_READ;
    }

    *leftValue  = left;
    *rightValue = right;

    if (middleValue != NULL) {
        *middleValue = UINT32_MAX;
    }

    return ret;
}