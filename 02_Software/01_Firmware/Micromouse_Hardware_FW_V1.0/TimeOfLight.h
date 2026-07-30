#ifndef TIME_OF_FLIGHT_H 
#define TIME_OF_FLIGHT_H  

#include "stm32f4xx_hal.h"  

#define NUM_SENSORS 6


typedef enum {
  TOF_OK = 0,
  TOF_ERR_I2C = 1,
  TOF_ERR_TIMEOUT = 2
} tof_err_t;


// Single global I2C handle (defined in TimeOfLight.c) 
extern I2C_HandleTypeDef hi2c2;
extern uint8_t ADDR_VL6180X;

// global counter for sensors
static uint8_t sens_count = 0;

static uint8_t sens_addr[NUM_SENSORS] = {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F};

// board pin mapping for shutdown ports and pins
#define SHDN_PORT_B GPIOB
#define SHDN_PORT_C GPIOC

// TOF sensor RR----------------------------
#define SENSOR_RR_PORT  SHDN_PORT_B 
#define SENSOR_RR_PIN   GPIO_PIN_7
// TOF sensor RM ---------------------------
#define SENSOR_RM_PORT  SHDN_PORT_C 
#define SENSOR_RM_PIN   GPIO_PIN_13
// TOF sensor RL ---------------------------
#define SENSOR_RL_PORT  SHDN_PORT_C
#define SENSOR_RL_PIN   GPIO_PIN_4
// TOF sensor LR ---------------------------
#define SENSOR_LR_PORT  SHDN_PORT_C
#define SENSOR_LR_PIN   GPIO_PIN_5
// TOF sensor LM ---------------------------
#define SENSOR_LM_PORT  SHDN_PORT_C
#define SENSOR_LM_PIN   GPIO_PIN_2
// TOF sensor LL ---------------------------
#define SENSOR_LL_PORT  SHDN_PORT_C
#define SENSOR_LL_PIN   GPIO_PIN_3

// I2C initialization 
void MX_I2C2_Init(void);  
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c);

// shutdown pins init
void Sensor_SHDN_Init(void);

//HAL_StatusTypeDef VL6180X_SetAddress(uint8_t newAddress); 
int VL6180X_Init(uint8_t newAddress);

int VL6180X_InitAll(void);

void TestDev(void);
 
// ===========================================================
// VL6180X single-shot read for a specific sensor (7-bit addr)
// ===========================================================
uint16_t VL6180X_ReadRangeSingleAt(uint8_t device_addr_7bit);
// ===========================================================
// Convenience: read one sensor by index (0..NUM_SENSORS-1)
// ===========================================================
uint16_t VL6180X_ReadSensorByIndex(uint8_t idx);

// ===========================================================
// Read all sensors into an array (out_mm must have NUM_SENSORS)
// Returns 0 if ok, else negative on first error
// ===========================================================
int VL6180X_ReadAll(uint16_t out_mm[NUM_SENSORS]);

tof_err_t VL6180X_ReadRangeSingleAtEx(uint8_t addr, uint16_t *out_mm);


#endif  
