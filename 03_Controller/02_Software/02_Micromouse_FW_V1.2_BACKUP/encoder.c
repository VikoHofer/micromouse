/*************************************************************
Project		: Micromouse V1.0
File		: encoder.c
Description	: Read Encoders using TIMER / Implementation
Date		: 09.09.2024
*************************************************************/

#include "encoder.h"
#include "Print.h"
#include "Python_Debugger.h"
#include <stm32f4xx_hal.h>

#define WHEEL_SIZE 78.53 // Umfang
#define COUNTER_MAX 65535
#define IIR_ORDER 4

/************************************
* TIM3: (AF2) // LEFT
* - CH1 -> PA6
* - CH2 -> PA7
*---------------------
* TIM1: (AF1) // RIGHT
* - CH1 -> PA8
* - CH2 -> PA9
************************************/
TIM_HandleTypeDef htim3 = {0};
TIM_HandleTypeDef htim1 = {0};


/*******************************************************************************
* Initialize GPIO for Encoder use
*******************************************************************************/
static void GPIO_Encoder_Init(void){
	
	// enable clock
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	// init gpio PA6 and PA7 -> TIM3
	GPIO_InitTypeDef GPIO_Encoder_Init = {0};
	
	GPIO_Encoder_Init.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_Encoder_Init.Mode = GPIO_MODE_AF_PP;
	GPIO_Encoder_Init.Pull = GPIO_PULLUP;
	GPIO_Encoder_Init.Speed = GPIO_SPEED_HIGH;
	GPIO_Encoder_Init.Alternate = GPIO_AF2_TIM3; // AlternateFunction TIM3
	
	HAL_GPIO_Init(GPIOA, &GPIO_Encoder_Init);
	
	
	// init gpio PA8 and PA9 -> TIM1	
	GPIO_Encoder_Init.Pin = GPIO_PIN_8 |GPIO_PIN_9;
	GPIO_Encoder_Init.Mode = GPIO_MODE_AF_PP;
	GPIO_Encoder_Init.Pull = GPIO_PULLUP;
	GPIO_Encoder_Init.Speed = GPIO_SPEED_HIGH;
	GPIO_Encoder_Init.Alternate = GPIO_AF1_TIM1; // AlternateFunction TIM1
	
	HAL_GPIO_Init(GPIOA, &GPIO_Encoder_Init);
	
}


/*******************************************************************************
* Initialize and configure Timer for encoder mode
*******************************************************************************/
void Encoder_Init(void)
{
	
	// Initialize GPIO-Pins
	GPIO_Encoder_Init();
	
	// Enable Clock
	__HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_TIM1_CLK_ENABLE();
	
	// Timer handle
	TIM_Encoder_InitTypeDef config_encoder = {0};
	 TIM_MasterConfigTypeDef sMasterConfig = {0};
	
	// Encoder config
	config_encoder.EncoderMode = TIM_ENCODERMODE_TI12;
	config_encoder.IC1Polarity = TIM_ICPOLARITY_RISING; 
	 
	config_encoder.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	config_encoder.IC1Prescaler = TIM_ICPSC_DIV1;
	config_encoder.IC1Filter = 0;
	 
	config_encoder.IC2Polarity = TIM_ICPOLARITY_RISING;
	config_encoder.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	config_encoder.IC2Prescaler = TIM_ICPSC_DIV1;
	config_encoder.IC2Filter = 0;
	
	
	// configure TIM3
    htim3.Instance = TIM3; // Timer 3
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 0xFFFF; // max counter value (has to be configured)
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.Prescaler = 0;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	
	// init TIM3
    if (HAL_TIM_Encoder_Init(&htim3, &config_encoder) != HAL_OK) {
        // Fehlerbehandlung hier
    } 
	
	// configure TIM1
    htim1.Instance = TIM1; // Timer 1
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 0xFFFF; // max counter value (has to be configured)
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.Prescaler = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	
	// init TIM1
    if (HAL_TIM_Encoder_Init(&htim1, &config_encoder) != HAL_OK) {
        // Fehlerbehandlung hier
    } 

}

/*******************************************************************************
* Start and Stop Timer
*******************************************************************************/
void Encoder_Start(void)
{
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
	
}
void Encoder_Stop(void)
{
    HAL_TIM_Encoder_Stop(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Stop(&htim1, TIM_CHANNEL_ALL);
}





/*******************************************************************************
* Read Hardware Counter
* Get counter value from TIMER (x4 Mode -> 4x Count with one cycle)
* -> 15 cycles for one round / 60 counts for one round
*******************************************************************************/
// TIM1 - ReadCounter Value
static int32_t Encoder_ReadCounter_Right(void)
{
	int32_t counter = __HAL_TIM_GET_COUNTER(&htim1);
	return (int32_t) counter;
}

// TIM12 - ReadCounter Value
static int32_t Encoder_ReadCounter_Left(void)
{
	int32_t counter = __HAL_TIM_GET_COUNTER(&htim3);
	return (int32_t) counter;
}



/*******************************************************************************
* Get Position (60 Counts per Cycle)
*******************************************************************************/
static double position_right = 0.0;
static double position_left = 0.0;

static int32_t last_count_right = 0;
static int32_t last_count_left = 0;

/*******************************************************************************
* Handle Over/Underflow
*******************************************************************************/
static int32_t calculate_encoder_delta(int32_t current_count, int32_t last_count)
{
    int32_t delta = current_count - last_count;
    
    // Overflow
    if (delta > COUNTER_MAX / 2) {
        delta -= COUNTER_MAX + 1;
    } 
    // Underflow
    else if (delta < -COUNTER_MAX / 2) {
        delta += COUNTER_MAX + 1;
    }

    return delta;
}



/*******************************************************************************
* Update Right Encoder Position
*******************************************************************************/
static void Update_Encoder_Position_Right(void)
{
    // get current counter
    int32_t current_count_right = Encoder_ReadCounter_Right();
    
    // caclulate difference
    int32_t delta = calculate_encoder_delta(current_count_right, last_count_right);
    
    // update position
    position_right += (double)delta;
    
    // store position
    last_count_right = current_count_right;
}

/*******************************************************************************
* Update Left Encoder Position
*******************************************************************************/
static void Update_Encoder_Position_Left(void)
{
    // get current counter
    int32_t current_count_left = Encoder_ReadCounter_Left();
    
    // caclulate difference
    int32_t delta = calculate_encoder_delta(current_count_left, last_count_left);
    
    // update position
    position_left += (double)delta;
    
    // store position
    last_count_left = current_count_left;
}

/*******************************************************************************
* Get counter value from TIMER (x4 Mode -> 4x Count with one cycle)
* Raw Counter Value (60 counts for one cycle)
*******************************************************************************/
double Get_Encoder_Position_Raw_Right(void)
{
    Update_Encoder_Position_Right();
    return (position_right * -1);
}
double Get_Encoder_Position_Raw_Left(void)
{
    Update_Encoder_Position_Left();
    return position_left;
}


/*******************************************************************************
* Get Position in Rounds
*******************************************************************************/
double Get_Encoder_Position_Rounds_Right(void)
{
	return Get_Encoder_Position_Raw_Right() / 60.00;
}
double Get_Encoder_Position_Rounds_Left(void)
{
	return Get_Encoder_Position_Raw_Left() / 60.00;
}

/*******************************************************************************
* Get Position in mm
*******************************************************************************/
double Get_Encoder_Position_mm_Right(void)
{
	return Get_Encoder_Position_Rounds_Right()*WHEEL_SIZE;
}
double Get_Encoder_Position_mm_Left(void)
{
	return Get_Encoder_Position_Rounds_Left()*WHEEL_SIZE;
}




/*******************************************************************************
* Calculate current speed
*******************************************************************************/

// Define separate previous times and positions for each motor
static uint32_t prev_time_ms_right = 0;
static uint32_t prev_time_ms_left = 0;
static double prev_position_right = 0;
static double prev_position_left = 0;

/*

float a_coeffs[IIR_ORDER + 1] = {1,	-3.98358125865852,	5.95087842926670,	-3.95101243657284,	0.983715267510480}; // Aus MATLAB exportieren
float b_coeffs[IIR_ORDER + 1] = {9.66139663398816e-11,	3.86455865359526e-10,	5.79683798039289e-10,	3.86455865359526e-10,	9.66139663398816e-11};

float input_buffer[IIR_ORDER + 1] = {0};
float output_buffer[IIR_ORDER + 1] = {0};

float apply_iir_filter(float input) {
    // Eingabe aktualisieren
    for (int i = IIR_ORDER; i > 0; i--) {
        input_buffer[i] = input_buffer[i - 1];
        output_buffer[i] = output_buffer[i - 1];
    }
    input_buffer[0] = input;

    // Berechnung
    float output = 0.0;
    for (int i = 0; i <= IIR_ORDER; i++) {
        output += b_coeffs[i] * input_buffer[i];
        if (i > 0) {
            output -= a_coeffs[i] * output_buffer[i];
        }
    }
    output_buffer[0] = output;

    return output;
}

void clear_iir_buffers() {
    for (int i = 0; i <= IIR_ORDER; i++) {
        input_buffer[i] = 0;
        output_buffer[i] = 0;
    }
}
*/
#define FILTER_ORDER 16
static float filter_coeffs[FILTER_ORDER+1] = {0.008855, 0.012860, 0.024217, 0.041348, 0.061713, 0.082210, 0.099672, 0.111378, 0.115495, 0.111378, 0.099672, 0.082210, 0.061713, 0.041348, 0.024217, 0.012860, 0.008855};
static float buffer_vel[FILTER_ORDER] = {0}; // Ringpuffer
static int buffer_index_vel = 0; // Index für Ringpuffer

static float buffer_ang[FILTER_ORDER] = {0}; // Ringpuffer
static int buffer_index_ang = 0; // Index für Ringpuffer

float apply_fir_filter_velocity(float input) {
    // Neue Eingabe in den Ringpuffer schreiben
    buffer_vel[buffer_index_vel] = input;

    // Faltung durchführen
    float output = 0.0;
    int index = buffer_index_vel;
    for (int i = 0; i < FILTER_ORDER; i++) {
        output += buffer_vel[index] * filter_coeffs[i];
        index = (index - 1 + FILTER_ORDER) % FILTER_ORDER; // Rückwärts zählen, mit Wrap-Around
    }

    // Ringpuffer-Index aktualisieren
    buffer_index_vel = (buffer_index_vel + 1) % FILTER_ORDER;

    return output;
}

float apply_fir_filter_angle_vel(float input) {
    // Neue Eingabe in den Ringpuffer schreiben
    buffer_ang[buffer_index_ang] = input;

    // Faltung durchführen
    float output = 0.0;
    int index = buffer_index_ang;
    for (int i = 0; i < FILTER_ORDER; i++) {
        output += buffer_ang[index] * filter_coeffs[i];
        index = (index - 1 + FILTER_ORDER) % FILTER_ORDER; // Rückwärts zählen, mit Wrap-Around
    }

    // Ringpuffer-Index aktualisieren
    buffer_index_ang = (buffer_index_ang + 1) % FILTER_ORDER;

    return output;
}

void clear_fir_buffers() {
    for (int i = 0; i < FILTER_ORDER; i++) {
        buffer_vel[i] = 0;
		buffer_ang[i] = 0;
    }
    buffer_index_vel = 0;
	buffer_index_ang = 0;
}


/*******************************************************************************
* Calculate current speed for right motor
*******************************************************************************/
double Encoder_GetSpeed_Right()
{
    // get time
    uint32_t current_time_ms = HAL_GetTick();

    // get position
    double current_position_counts = Get_Encoder_Position_mm_Right();

    // calculate time difference
    uint32_t elapsedTime = current_time_ms - prev_time_ms_right;
	
	// avoid division by zero
    if (elapsedTime == 0) return 0.0;

    // position difference
    double delta_position_counts = current_position_counts - prev_position_right;

    // calculate speed
    double speed_counts_per_sec = (delta_position_counts / elapsedTime)*1000;

    // Debug Print
    //PY_Printf("SPEED RIGHT: %f mm/s", speed_mm_per_sec);

    // update previous values
    prev_position_right = current_position_counts;
    prev_time_ms_right = current_time_ms;

    
	//speed_counts_per_sec = apply_fir_filter(speed_counts_per_sec);
    return speed_counts_per_sec;
}

/*******************************************************************************
* Calculate current speed for left motor
*******************************************************************************/
double Encoder_GetSpeed_Left()
{
    // get time
    uint32_t current_time_ms = HAL_GetTick();

    // get position
    double current_position_counts = Get_Encoder_Position_mm_Left();

    // calculate time difference
    uint32_t elapsedTime = current_time_ms - prev_time_ms_left;
	
	// avoid division by zero
    if (elapsedTime == 0) return 0.0;

    // position difference
    double delta_position_counts = current_position_counts - prev_position_left;

    // calculate speed
    double speed_counts_per_sec = (delta_position_counts / elapsedTime)*1000;

    // Debug Print
    //PY_Printf("SPEED LEFT: %f mm/s", speed_mm_per_sec);

    // update previous values
    prev_position_left = current_position_counts;
    prev_time_ms_left = current_time_ms;

    
	//speed_counts_per_sec = apply_fir_filter(speed_counts_per_sec);
    return speed_counts_per_sec;
}


/*******************************************************************************
* Reset Encoders
*******************************************************************************/
void Encoder_Reset(void){
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	
	prev_position_right = 0;
	prev_position_left = 0;
	clear_fir_buffers();
	
}


