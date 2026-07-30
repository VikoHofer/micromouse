/*************************************************************
Project		: Micromouse V2.0
File		: encoder.h
Description	: Read Encoders using TIMER
Date		: 11.03.2026
*************************************************************/

#include <stdint.h>

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"

/************************************
* TIM3: (AF2) // LEFT
* - CH1 -> PA6
* - CH2 -> PA7
*---------------------
* TIM1: (AF1) // RIGHT
* - CH1 -> PA8
* - CH2 -> PA9
************************************/


#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>

void encoder_init(void);
void encoder_start(void);
void encoder_stop(void);
void encoder_reset(void);

/* cyclic updates for control loop */
void encoder_update(void);

/* Rohdaten */
int32_t encoder_get_left_counts(void);
int32_t encoder_get_right_counts(void);

int16_t encoder_get_left_delta_counts(void);
int16_t encoder_get_right_delta_counts(void);

/* one wheel */
float encoder_get_left_mm(void);
float encoder_get_right_mm(void);

float encoder_get_left_speed_mms(void);
float encoder_get_right_speed_mms(void);

/* forward change per cycle */
float encoder_get_forward_change_mm(void);
float encoder_get_rotation_change_rad(void);
float encoder_get_rotation_change_deg(void);

float encoder_get_distance_mm(void);
float encoder_get_angle_rad(void);
float encoder_get_segment_angle_deg(void);


/* Segment-Reset */
void encoder_reset_segment(void);

/* Segment */
float encoder_get_segment_distance_mm(void);
float encoder_get_segment_angle_rad(void);

int16_t encoder_raw_left(void);
int16_t encoder_raw_right(void);

// angle velocity
float encoder_get_yaw_rate_rads(void);
float encoder_get_yaw_rate_degs(void);

#endif

