/*************************************************************
Project		: Micromouse V1.2
File		: sensor_normalize.h
Description	: normalize IR sensors non linear behaviour
Date		: 8.10.2025
*************************************************************/

#ifndef SENS_NORM_H
#define SENS_NORM_H


#include <math.h>
#include <stdint.h>
#include <stdbool.h>

extern double S_ref_left_45;
extern double S_ref_right_45;

// get middle values when mouse is in middle
void IR_Calibrate_Reference(void);

// normalize exp ADC values into linear
double IR_from_ADC(double adc, double S_ref);

/*******************************************************************************
* IR-Sensors
*******************************************************************************/
// PA0
// duration 5ms!!
double readSensor_Left(void);
double readSensor_Left_45(void);

// PA1
// duration 5ms!!
double readSensor_Right(void);
double readSensor_Right_45(void);

// PB0
double readSensor_Front(void);
double readSensor_Front2(void);

// return falls when wall is in front
bool check_front_wall();
bool check_front_middle();



#endif