/*************************************************************
Project		: Micromouse V1.2
File		: sensor_normalize.c
Description	: normalize IR sensors non linear behaviour
Date		: 8.10.2025
*************************************************************/


#include "sensor_normalize.h"
#include "adc.h"
#include "config.h"
#include "sensor.h"
#include "Delay.h"

#define ADC_MAX_VAL       4000u
#define ADC_MIN_EFF       2u  
#define IR_MIN            0.0f
#define IR_MAX            1000.0f  // Sicherheits-Clamp

/*
0.05	sehr stark gegl?ttet, sehr tr?ge	fc ? 0.3 Hz
0.10	stark gegl?ttet	fc ? 0.6 Hz
0.20	mittlere Gl?ttung	fc ? 1.3 Hz
0.40	leicht gegl?ttet	fc ? 3 Hz
0.70	kaum gegl?ttet, sehr reaktiv	fc ? 8 Hz
*/
const double alpha = 0.5;

double S_ref_left_45  = 0;
double S_ref_right_45 = 0;
double LEFT45_SCALE = 0;
double RIGHT45_SCALE = 0;

// values when mouse is in middle
const float S_ref_left = 0;
const float S_ref_right = 0;
const float S_ref_left_front1 = 0;
const float S_ref_left_front2 = 0;

const int NOMINAL_VALUE = 100;

// Anzahl Messungen f?r Mittelwert
#define CALIB_SAMPLES 20
#define CALIB_DELAY_MS 10

// Hilfsfunktion: Mittelwert aus n ADC-Lesungen
static double average_sensor(SensorID id)
{
    uint32_t sum = 0;
    uint32_t val = 0;
    for (int i = 0; i < CALIB_SAMPLES; i++) {
        if(id == SENSOR_ID_4){
			val = readSensor_Right_45();
		} else if(id == SENSOR_ID_3) {
			val = readSensor_Left_45();
		}
            sum += val;
        Delay_ms(CALIB_DELAY_MS);
    }
    return (double)(sum / CALIB_SAMPLES);
}

// evaluate reference values in the middle of the mouse
//void IR_Calibrate_Reference(void)
//{
//    S_ref_left_45  = average_sensor(SENSOR_ID_3);
//    S_ref_right_45 = average_sensor(SENSOR_ID_4);
//}
void IR_Calibrate_Reference(void)
{
    S_ref_left_45  = average_sensor(SENSOR_ID_3);
    S_ref_right_45 = average_sensor(SENSOR_ID_4);
	
	LEFT45_SCALE = (float)NOMINAL_VALUE / S_ref_left_45; 
	RIGHT45_SCALE = (float)NOMINAL_VALUE / S_ref_right_45; 
}

// clamp ADC values
uint16_t clamp_u16(uint32_t v){
    if (v < ADC_MIN_EFF) return ADC_MIN_EFF;
    if (v > ADC_MAX_VAL) return ADC_MAX_VAL;
    return (uint16_t)v;
}


// normalize exp ADC values into linear
double IR_from_ADC(double adc, double S_ref){
    uint16_t S     = clamp_u16((uint32_t)ADC_MAX_VAL - adc);
    uint16_t Sref  = clamp_u16((uint32_t)ADC_MAX_VAL - S_ref);
    float num = logf((float)Sref);
    float den = logf((float)S);
    if (den <= 0.0f) den = 1.0f;
    float y = 100.0f * (num / den);              // =100 bei Referenz
    if (y < IR_MIN)  y = IR_MIN;
    if (y > IR_MAX)  y = IR_MAX;
    return y;
}

/*******************************************************************************
* IR-Sensors
*******************************************************************************/


double readSensor_Left_45(void){
	double Distance = 0;
	static double Distance_Old = 0;
	double out = 0;
	uint32_t sensorValue = 0;

	if (Sensor_Read(SENSOR_ID_3, &hadc1, &sensorValue) == SENSOR_OK) {
		Distance = (double)sensorValue;
	}
	
	out = Distance_Old + alpha * (Distance-Distance_Old);
	Distance_Old = out;
	
	return out;
}

// PA4
// duration 2ms!!
double readSensor_Right_45(void){
	double Distance = 0;
	static double Distance_Old = 0;
	double out = 0;
	uint32_t sensorValue = 0;

	if (Sensor_Read(SENSOR_ID_4, &hadc1, &sensorValue) == SENSOR_OK) {
		Distance = (double)sensorValue - IR_OFFSET;
	}
	
	out = Distance_Old + alpha * (Distance-Distance_Old);
	Distance_Old = out;

	return out;
}

double readSensor_Left(void){
	double Distance = 0;
	uint32_t sensorValue = 0;

	if (Sensor_Read(SENSOR_ID_6, &hadc1, &sensorValue) == SENSOR_OK) {
		Distance = (double)sensorValue;
	}

	return Distance;
}




double readSensor_Right(void){
	double Distance = 0;
	uint32_t sensorValue = 0;

	if (Sensor_Read(SENSOR_ID_5, &hadc1, &sensorValue) == SENSOR_OK) {
		Distance = (double)sensorValue;
	}

	return Distance;
}

// PB0
double readSensor_Front(void){
	double Distance = 0;
	uint32_t sensorValue = 0;

	if (Sensor_Read(SENSOR_ID_1, &hadc1, &sensorValue) == SENSOR_OK) {
		Distance = (double)sensorValue;
	}

	return Distance;
}

double readSensor_Front2(void){
	double Distance = 0;
	uint32_t sensorValue = 0;

	if (Sensor_Read(SENSOR_ID_2, &hadc1, &sensorValue) == SENSOR_OK) {
		Distance = (double)sensorValue;
	}

	return Distance;
}

/*******************************************************************************
* check front wall
* return: 	true -> WALL
			false -> NO WALL
*******************************************************************************/
bool check_front_wall(){
	double sensor_FL = readSensor_Front();
	double sensor_FR = readSensor_Front2();
	
	if(sensor_FL < FRONT_LEFT_MIDDLE && sensor_FR < FRONT_RIGHT_MIDDLE){
		return true;
	} else {
		return false;
	}
	
}

