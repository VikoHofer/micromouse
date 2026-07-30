/*************************************************************
Project     : Micromouse V2.0
File        : encoder.c
Description : Encoder module using timer encoder mode
Date        : 11.03.2026
*************************************************************/

#include "encoder.h"
#include "config.h"
#include "stm32f4xx_hal.h"

/************************************
* TIM3: RIGHT
* - CH1 -> PA6
* - CH2 -> PA7
*
* TIM1: LEFT
* - CH1 -> PA8
* - CH2 -> PA9
************************************/

TIM_HandleTypeDef htim3 = {0};
TIM_HandleTypeDef htim1 = {0};


#define PI_F 3.14159265358979f
#define MM_PER_COUNT ((PI_F * WHEEL_DIAMETER_MM) / ENCODER_COUNTS_PER_REV)

/*
 * Speed estimation:
 * encoder_update() runs every CONTROL_PERIOD_S (e.g. 1 ms)
 * Speed is estimated over a moving window of N samples.
 *
 * Example:
 * CONTROL_PERIOD_S = 0.001f
 * SPEED_FILTER_SAMPLES = 10
 * => speed window = 10 ms
 */
#define SPEED_FILTER_SAMPLES 20

/* -------------------------------------------------------------------------- */
/* Internal state                                                             */
/* -------------------------------------------------------------------------- */

static volatile int32_t s_total_counts_left = 0;
static volatile int32_t s_total_counts_right = 0;

static volatile int16_t s_delta_counts_left = 0;
static volatile int16_t s_delta_counts_right = 0;

static volatile float s_left_mm = 0.0f;
static volatile float s_right_mm = 0.0f;

static volatile float s_left_speed_mms = 0.0f;
static volatile float s_right_speed_mms = 0.0f;
static volatile float s_yaw_rate_rads = 0.0f;

static volatile float s_forward_change_mm = 0.0f;
static volatile float s_rotation_change_rad = 0.0f;

static volatile float s_distance_mm = 0.0f;
static volatile float s_angle_rad = 0.0f;

static volatile float s_segment_distance_mm = 0.0f;
static volatile float s_segment_angle_rad = 0.0f;

static uint16_t s_last_hw_count_left = 0;
static uint16_t s_last_hw_count_right = 0;

/* Moving-window speed estimator */
static volatile int16_t s_hist_left[SPEED_FILTER_SAMPLES] = {0};
static volatile int16_t s_hist_right[SPEED_FILTER_SAMPLES] = {0};
static volatile int32_t s_sum_left = 0;
static volatile int32_t s_sum_right = 0;
static volatile uint8_t s_hist_index = 0;

/* -------------------------------------------------------------------------- */
/* GPIO init                                                                  */
/* -------------------------------------------------------------------------- */

static void encoder_gpio_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    /* TIM3: PA6 / PA7 */
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* TIM1: PA8 / PA9 */
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOA, &gpio);
}

/* -------------------------------------------------------------------------- */
/* Timer init                                                                 */
/* -------------------------------------------------------------------------- */

void encoder_init(void)
{
    encoder_gpio_init();

    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_TIM1_CLK_ENABLE();

    TIM_Encoder_InitTypeDef enc = {0};

    enc.EncoderMode = TIM_ENCODERMODE_TI12;

    enc.IC1Polarity = TIM_ICPOLARITY_RISING;
    enc.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    enc.IC1Prescaler = TIM_ICPSC_DIV1;
    enc.IC1Filter = 2;   /* ggf. 0..4 testen */

    enc.IC2Polarity = TIM_ICPOLARITY_RISING;
    enc.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    enc.IC2Prescaler = TIM_ICPSC_DIV1;
    enc.IC2Filter = 2;

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 0;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 0xFFFF;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Encoder_Init(&htim3, &enc);

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 0xFFFF;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Encoder_Init(&htim1, &enc);

    encoder_reset();
}

void encoder_start(void)
{
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    __HAL_TIM_SET_COUNTER(&htim1, 0);

    s_last_hw_count_left = 0;
    s_last_hw_count_right = 0;

    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
}

void encoder_stop(void)
{
    HAL_TIM_Encoder_Stop(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Stop(&htim1, TIM_CHANNEL_ALL);
}

void encoder_reset(void)
{
    uint8_t i;

    __HAL_TIM_SET_COUNTER(&htim3, 0);
    __HAL_TIM_SET_COUNTER(&htim1, 0);

    s_total_counts_left = 0;
    s_total_counts_right = 0;

    s_delta_counts_left = 0;
    s_delta_counts_right = 0;

    s_left_mm = 0.0f;
    s_right_mm = 0.0f;

    s_left_speed_mms = 0.0f;
    s_right_speed_mms = 0.0f;

    s_forward_change_mm = 0.0f;
    s_rotation_change_rad = 0.0f;

    s_distance_mm = 0.0f;
    s_angle_rad = 0.0f;

    s_segment_distance_mm = 0.0f;
    s_segment_angle_rad = 0.0f;

    s_last_hw_count_left = 0;
    s_last_hw_count_right = 0;

    s_sum_left = 0;
    s_sum_right = 0;
    s_hist_index = 0;
	
	s_yaw_rate_rads = 0.0f;

    for (i = 0; i < SPEED_FILTER_SAMPLES; i++) {
        s_hist_left[i] = 0;
        s_hist_right[i] = 0;
    }
}


void encoder_reset_segment(void)
{
    s_segment_distance_mm = 0.0f;
    s_segment_angle_rad = 0.0f;
}

/* -------------------------------------------------------------------------- */
/* Core update                                                                */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* Core update                                                                */
/* -------------------------------------------------------------------------- */

void encoder_update(void)
{
	uint16_t hw_right = 0;
    uint16_t hw_left  = 0;

#if TARGET_ENCODER == ENCODER_HW_V2
		hw_right = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);
		hw_left  = (uint16_t)__HAL_TIM_GET_COUNTER(&htim1);
#elif TARGET_ENCODER == ENCODER_HW_V1
    hw_right = (uint16_t)__HAL_TIM_GET_COUNTER(&htim1);
    hw_left  = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);
#endif

    int16_t dleft  = (int16_t)(hw_left - s_last_hw_count_left);
    int16_t dright = (int16_t)(hw_right - s_last_hw_count_right);

    s_last_hw_count_left = hw_left;
    s_last_hw_count_right = hw_right;

    dleft  = (int16_t)(dleft  * ENCODER_LEFT_SIGN);
    dright = (int16_t)(dright * ENCODER_RIGHT_SIGN);

    s_delta_counts_left = dleft;
    s_delta_counts_right = dright;

    /* Absolute counts / path */
    s_total_counts_left += dleft;
    s_total_counts_right += dright;

    s_left_mm = (float)s_total_counts_left * MM_PER_COUNT;
    s_right_mm = (float)s_total_counts_right * MM_PER_COUNT;

    /* Incremental motion per update step */
    {
        float left_change_mm = (float)dleft * MM_PER_COUNT;
        float right_change_mm = (float)dright * MM_PER_COUNT;

        s_forward_change_mm = 0.5f * (left_change_mm + right_change_mm);
        s_rotation_change_rad = (right_change_mm - left_change_mm) / WHEEL_BASE_MM;

        s_distance_mm += s_forward_change_mm;
        s_angle_rad += s_rotation_change_rad;
        s_segment_distance_mm += s_forward_change_mm;
        s_segment_angle_rad += s_rotation_change_rad;
    }

    /* Moving-window speed estimation */
    s_sum_left  -= s_hist_left[s_hist_index];
    s_sum_right -= s_hist_right[s_hist_index];

    s_hist_left[s_hist_index]  = dleft;
    s_hist_right[s_hist_index] = dright;

    s_sum_left  += dleft;
    s_sum_right += dright;

    s_hist_index++;
    if (s_hist_index >= SPEED_FILTER_SAMPLES) {
        s_hist_index = 0;
    }

    {
        const float speed_window_s = CONTROL_PERIOD_S * (float)SPEED_FILTER_SAMPLES;
        s_left_speed_mms  = ((float)s_sum_left  * MM_PER_COUNT) / speed_window_s;
        s_right_speed_mms = ((float)s_sum_right * MM_PER_COUNT) / speed_window_s;
    }
	{
		const float speed_window_s = CONTROL_PERIOD_S * (float)SPEED_FILTER_SAMPLES;
		s_left_speed_mms  = ((float)s_sum_left  * MM_PER_COUNT) / speed_window_s;
		s_right_speed_mms = ((float)s_sum_right * MM_PER_COUNT) / speed_window_s;

		s_yaw_rate_rads = (s_right_speed_mms - s_left_speed_mms) / WHEEL_BASE_MM;
	}
}

int16_t encoder_raw_left(void){
	return (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
}
int16_t encoder_raw_right(void){
	return (int16_t)__HAL_TIM_GET_COUNTER(&htim1);
}

/* -------------------------------------------------------------------------- */
/* Getters                                                                    */
/* -------------------------------------------------------------------------- */

int32_t encoder_get_left_counts(void)
{
    return s_total_counts_left;
}

int32_t encoder_get_right_counts(void)
{
    return s_total_counts_right;
}

int16_t encoder_get_left_delta_counts(void)
{
    return s_delta_counts_left;
}

int16_t encoder_get_right_delta_counts(void)
{
    return s_delta_counts_right;
}

float encoder_get_left_mm(void)
{
    return s_left_mm;
}

float encoder_get_right_mm(void)
{
    return s_right_mm;
}

float encoder_get_left_speed_mms(void)
{
    return s_left_speed_mms;
}

float encoder_get_right_speed_mms(void)
{
    return s_right_speed_mms;
}

float encoder_get_forward_change_mm(void)
{
    return s_forward_change_mm;
}

float encoder_get_rotation_change_rad(void)
{
    return s_rotation_change_rad;
}
float encoder_get_rotation_change_deg(void)
{
    return s_rotation_change_rad * 57.29578f;
}

float encoder_get_distance_mm(void)
{
    return s_distance_mm;
}

float encoder_get_angle_rad(void)
{
    return s_angle_rad;
}

float encoder_get_segment_angle_deg(void)
{
    return s_segment_angle_rad * 57.29578f;
}
float encoder_get_yaw_rate_rads(void)
{
    return s_yaw_rate_rads;
}

float encoder_get_yaw_rate_degs(void)
{
    return s_yaw_rate_rads * 57.29578f;
}