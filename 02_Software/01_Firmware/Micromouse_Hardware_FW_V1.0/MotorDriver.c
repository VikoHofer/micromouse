/*************************************************************
Project		: Micromouse V1.0
File		: MotorDriver.h
Description	: Create PWM Signals to controll Motor / Implemtation
Date		: 09.09.2024
*************************************************************/

#include "MotorDriver.h"
#include "config.h"

/************************************
* using TIM 4 to create PWM Signals
* TIM4 (AF2)
* CH3 -> PB8 (Motor Left)
* CH4 -> PB9 (Motor Right)
************************************
* Control PINS (Forward / Backward)
* Motor Left: AIN1 -> PA10
* Motor Left: AIN2 -> PC9
* Motor Right: BIN1 -> PB5
* Motor Right: BIN2 -> PC8
************************************
* CW: 	IN1 = HIGH, IN2 = LOW 
* CCW:	IN1 = LOW,  IN2 = HIGH
* Stop: IN1 = LOW,  IN2 = LOW
*************************************/
// CONFIGURED FOR 180Mhz


// struct to identifier GPIO
typedef struct {
    GPIO_TypeDef* GPIO_Port;
    uint16_t GPIO_Pin;
} GPIO_Pin_t;


// Motor GPIO Pins 
static const GPIO_Pin_t MOTOR_CTRL_PIN[] = 
{
    { GPIOA, GPIO_PIN_10 },
	{ GPIOC, GPIO_PIN_9 },
	{ GPIOB, GPIO_PIN_5 },
	{ GPIOC, GPIO_PIN_8 }
};

// initialize all GPIOs for Motor direction control
void Motor_Ctrl_Init(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	for (int i = 0; i < sizeof(MOTOR_CTRL_PIN) / sizeof(MOTOR_CTRL_PIN[0]); ++i) {
        GPIO_Pin_t pin = MOTOR_CTRL_PIN[i];
        
        // initialize GPIO-Pins as Input
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.Pin = pin.GPIO_Pin;
        GPIO_InitStruct.Mode = MODE_OUTPUT;
        HAL_GPIO_Init(pin.GPIO_Port, &GPIO_InitStruct);
    }


}

/**************************
* Direction Ctrl functions
* Set direction pins on motor controller
**************************/
// CW
void Motor_Left_Forwards(void)
{
	// AIN1 = H
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
	
	// AIN2 = L
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);

}
// direction controll
// CCW
void Motor_Left_Backwards (void)
{
	// AIN1 = L
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
	
	// AIN2 = H
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);

}
// direction controll
// STOP
void Motor_Left_Brake (void)
{
	// AIN1 = L
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
	
	// AIN2 = L
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
}

// direction controll
// CW
void Motor_Right_Forwards(void)
{
	// BIN1 = H
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	
	// BIN2 = L
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
}
// direction controll
// CCW
void Motor_Right_Backwards (void)
{
	// BIN1 = L
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
	
	// BIN2 = H
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);

}
// direction controll
// STOP
void Motor_Right_Brake (void)
{
	// BIN1 = L
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	
	// BIN2 = L
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);

}

/**************************
* PWM Control
**************************/

TIM_HandleTypeDef tim4 = {0};


// Start / Stop PWM Signals
static void Motor_Left_PWM_Start(void)
{
	HAL_TIM_PWM_Start(&tim4, TIM_CHANNEL_3);
}

static void Motor_Right_PWM_Start(void)
{
	HAL_TIM_PWM_Start(&tim4, TIM_CHANNEL_4);
}
static void Motor_Left_PWM_Stop(void)
{
	HAL_TIM_PWM_Stop(&tim4, TIM_CHANNEL_3);
}
static void Motor_Right_PWM_Stop(void)
{
	HAL_TIM_PWM_Stop(&tim4, TIM_CHANNEL_4);
}


// configure GPIO as PWM Output
static void GPIO_Motor_Init(void){
	
	// enable clock
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	// init gpio PB8 and PB9 -> TIM4
	GPIO_InitTypeDef GPIO_PWM_Init = {0};
	
	GPIO_PWM_Init.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_PWM_Init.Mode = GPIO_MODE_AF_PP;
	GPIO_PWM_Init.Pull = GPIO_PULLUP;
	GPIO_PWM_Init.Speed = GPIO_SPEED_HIGH;
	GPIO_PWM_Init.Alternate = GPIO_AF2_TIM4; // AlternateFunction TIM4
	
	HAL_GPIO_Init(GPIOB, &GPIO_PWM_Init);
	
}

// Timer TIM4 as PWM generator (Config: 100Khz, Duty: 0-255)
void Motor_Init(void){
	__HAL_RCC_TIM4_CLK_ENABLE();
	
	// Init all GPIOs
	GPIO_Motor_Init();
	Motor_Ctrl_Init();
	
	Motor_Left_Brake();
	Motor_Right_Brake();
	
	
    tim4.Instance = TIM4;
    tim4.Init.Prescaler = PWM_PRESCALER; 
    tim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim4.Init.Period = PWM_PERIOD; 
    tim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    tim4.Init.RepetitionCounter = 0;
    HAL_TIM_PWM_Init(&tim4);

    // PWM-Mode CH3
    TIM_OC_InitTypeDef sConfigOC1;
    sConfigOC1.OCMode = TIM_OCMODE_PWM1;
    sConfigOC1.Pulse = 0; 
    sConfigOC1.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC1.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&tim4, &sConfigOC1, TIM_CHANNEL_3);

    // PWM-Mode CH4
    TIM_OC_InitTypeDef sConfigOC2;
    sConfigOC2.OCMode = TIM_OCMODE_PWM1;
    sConfigOC2.Pulse = 0; 
    sConfigOC2.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC2.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&tim4, &sConfigOC2, TIM_CHANNEL_4);
	

	
	// Start PWM channels
    HAL_TIM_PWM_Start(&tim4, TIM_CHANNEL_3);  // Motor Left
    HAL_TIM_PWM_Start(&tim4, TIM_CHANNEL_4);  // Motor Right
	
	__HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_3, 0);
	__HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_4, 0);
	

}



/********************************************
* Only forward (0-255)
********************************************/
void Motor_Left_SetSpeed(uint16_t speed) {
    __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_4, speed);
}

void Motor_Right_SetSpeed(uint16_t speed) {
    __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_3, speed);
}

/********************************************
* Forward and Backward	(-255 - 255)
********************************************/
#define PWM_MAX 255

// PWM bordering
int16_t limitPWM(double pwm) {
	if (pwm > PWM_MAX)
		return PWM_MAX;
	else if (pwm < -PWM_MAX)
		return -PWM_MAX;
	return pwm;
}


void Motor_Left_SetPWM(double pwm) {


	if(pwm == 0) {
        Motor_Left_Brake(); 
        __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_3, 0);
        return;
    }
	
	if(pwm > 0){
		Motor_Left_Forwards();
		__HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_3, limitPWM(pwm));
		
	} else if(pwm < 0){
		Motor_Left_Backwards();
		__HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_3,(limitPWM(pwm)*-1));
	}

    
}

void Motor_Right_SetPWM(double pwm) {

	
	if(pwm == 0) {
        Motor_Right_Brake(); 
        __HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_4, 0);
        return;
    }
	
	if(pwm > 0){
		Motor_Right_Forwards();
		__HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_4, limitPWM(pwm));
		
	} else if(pwm < 0){
		Motor_Right_Backwards();
		__HAL_TIM_SET_COMPARE(&tim4, TIM_CHANNEL_4, (limitPWM(pwm)*-1));
	}
	
}

