#include "TimeOfLight.h"
#include <string.h> 
#include "Python_Debugger.h"

uint8_t ADDR_VL6180X = 0x29;

// global I2C handle
I2C_HandleTypeDef hi2c2; 



// HAL MSP init: configure I2C1 pins and sensor reset pins once
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  if (hi2c->Instance != I2C2) return;


	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	__HAL_RCC_I2C2_CLK_ENABLE();

	GPIO_InitTypeDef g = {0};
	g.Mode = GPIO_MODE_AF_OD;
	g.Pull = GPIO_PULLUP;                 // zum Debug!
	g.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	g.Alternate = GPIO_AF4_I2C2;

	g.Pin = GPIO_PIN_10;                  // SCL
	HAL_GPIO_Init(GPIOB, &g);

	g.Pin = GPIO_PIN_12;                  // SDA (siehe Punkt 2)
	HAL_GPIO_Init(GPIOC, &g);
}

void MX_I2C2_Init(void)
{
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;       // erstmal 100k zum Debug
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(&hi2c2) != HAL_OK) while(1);
}

// inits every sensors shutdown pins
void Sensor_SHDN_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// Sensor reset pins
	GPIO_InitStruct.Pin = SENSOR_RR_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(SENSOR_RR_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SENSOR_RM_PIN;
	HAL_GPIO_Init(SENSOR_RM_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SENSOR_RL_PIN;
	HAL_GPIO_Init(SENSOR_RL_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SENSOR_LR_PIN;
	HAL_GPIO_Init(SENSOR_LR_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SENSOR_LM_PIN;
	HAL_GPIO_Init(SENSOR_LM_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SENSOR_LL_PIN;
	HAL_GPIO_Init(SENSOR_LL_PORT, &GPIO_InitStruct);

	// Keep all sensors in reset initially
	HAL_GPIO_WritePin(SENSOR_RR_PORT, SENSOR_RR_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SENSOR_RM_PORT, SENSOR_RM_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SENSOR_RL_PORT, SENSOR_RL_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SENSOR_LR_PORT, SENSOR_LR_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SENSOR_LM_PORT, SENSOR_LM_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SENSOR_LL_PORT, SENSOR_LL_PIN, GPIO_PIN_RESET);
}

/* ===================== VL6180X (16-bit register map) ===================== */ 

// VL6180X registers
#define VL6180X_REG_IDENTIFICATION_MODEL_ID 0x000 
#define VL6180X_REG_SYSTEM__INTERRUPT_CONFIG_GPIO 0x014 
#define VL6180X_REG_SYSTEM__INTERRUPT_CLEAR 0x015 
#define VL6180X_REG_SYSTEM__FRESH_OUT_OF_RESET 0x016 
#define VL6180X_REG_SYSRANGE__START 0x018 
#define VL6180X_REG_SYSRANGE__MAX_CONVERGENCE_TIME 0x01C 
#define VL6180X_REG_RESULT__INTERRUPT_STATUS_GPIO 0x04F 
#define VL6180X_REG_RESULT__RANGE_VAL 0x062 
#define VL6180X_REG_I2C_SLAVE_DEVICE_ADDRESS 0x212


// low-level helpers for arbitrary device address
static HAL_StatusTypeDef VL6180X_Write8_at(uint8_t device_addr, uint16_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c2, (device_addr << 1), reg, I2C_MEMADD_SIZE_16BIT, &value, 1, 100);
}
static HAL_StatusTypeDef VL6180X_Read8_at(uint8_t device_addr, uint16_t reg, uint8_t* value)
{
    return HAL_I2C_Mem_Read(&hi2c2, (device_addr << 1), reg, I2C_MEMADD_SIZE_16BIT, value, 1, 100);
}

/**
  * @brief  sets a new address for a sensor
  * @param  curAddr is the default address
  * @param  newAddress
  * @param  addr_var is the 7 bit version of newAddress
  * @retval The result of the operation (HAL).
*/
static HAL_StatusTypeDef VL6180X_SetAddressAt(uint8_t curAddr, uint8_t newAddress, uint8_t *addr_var)
{
    uint8_t na = newAddress & 0x7F;
    if (na == 0) return HAL_ERROR;

    if (VL6180X_Write8_at(curAddr, VL6180X_REG_I2C_SLAVE_DEVICE_ADDRESS, na) != HAL_OK) return HAL_ERROR;
    HAL_Delay(2);
    if (HAL_I2C_IsDeviceReady(&hi2c2, (na << 1), 3, 100) != HAL_OK) return HAL_ERROR;

    *addr_var = na;
    return HAL_OK;
}

/**
  * @brief  writes mandatory settings to tof registers
  * @param  device_addr
  * @retval The result of the operation.
*/
static int VL6180X_WriteMandatorySettingsAt(uint8_t device_addr) { 
  struct { uint16_t reg; uint8_t val; } init_seq[] = 
  { 
    {0x0207, 0x01}, {0x0208, 0x01}, {0x0096, 0x00}, {0x0097, 0xFD}, {0x00E3, 0x00}, 
    {0x00E4, 0x04}, {0x00E5, 0x02}, {0x00E6, 0x01}, {0x00E7, 0x03}, {0x00F5, 0x02}, 
    {0x00D9, 0x05}, {0x00DB, 0xCE}, {0x00DC, 0x03}, {0x00DD, 0xF8}, {0x009F, 0x00}, 
    {0x00A3, 0x3C}, {0x00B7, 0x00}, {0x00BB, 0x3C}, {0x00B2, 0x09}, {0x00CA, 0x09}, 
    {0x0198, 0x01}, {0x01B0, 0x17}, {0x01AD, 0x00}, {0x00FF, 0x05}, {0x0100, 0x05}, 
    {0x0199, 0x05}, {0x01A6, 0x1B}, {0x01AC, 0x3E}, {0x01A7, 0x1F}, {0x0030, 0x00},
    {0x0133, 0x01}, {0x0109, 0x07}, {0x010a, 0x30}, {0x003f, 0x46}, {0x0103, 0x01},
    {0x001b, 0x0A}, {0x003e, 0x0A}, {0x0131, 0x04}, {0x0011, 0x10}, {0x0014, 0x24},
    {0x0031, 0xFF}, {0x00d2, 0x01}, {0x00f2, 0x01}
  }; 
  
  for (unsigned i = 0; i < sizeof(init_seq)/sizeof(init_seq[0]); ++i) {
    if (VL6180X_Write8_at(device_addr,init_seq[i].reg, init_seq[i].val) != HAL_OK) return -1; 
  } 
  
    // Configure GPIO1 interrupt: new range sample ready 
  if (VL6180X_Write8_at(device_addr,VL6180X_REG_SYSTEM__INTERRUPT_CONFIG_GPIO, 0x24) != HAL_OK) 
    return -2; 
  
  // Shorter convergence time (optional) 
  if (VL6180X_Write8_at(device_addr,VL6180X_REG_SYSRANGE__MAX_CONVERGENCE_TIME, 0x31) != HAL_OK) 
    return -3; 
  
  // Clear pending interrupts 
  if (VL6180X_Write8_at(device_addr,VL6180X_REG_SYSTEM__INTERRUPT_CLEAR, 0x07) != HAL_OK) 
    return -4; 

  return 0; 
}

/**
  * @brief  init function for the sensors
  * @param  newAddress sets the new address for each individual sensor
  * @retval The result of the operation.
*/
int VL6180X_Init(uint8_t newAddress)
{
    switch (newAddress) {
        case 0x1A:
            HAL_GPIO_WritePin(SENSOR_RR_PORT, SENSOR_RR_PIN, GPIO_PIN_RESET);
            HAL_Delay(5);
            HAL_GPIO_WritePin(SENSOR_RR_PORT, SENSOR_RR_PIN, GPIO_PIN_SET);
            HAL_Delay(5);
            break;
        case 0x2B:
            HAL_GPIO_WritePin(SENSOR_RM_PORT, SENSOR_RM_PIN, GPIO_PIN_RESET);
            HAL_Delay(5);
            HAL_GPIO_WritePin(SENSOR_RM_PORT, SENSOR_RM_PIN, GPIO_PIN_SET);
            HAL_Delay(5);
            break;
        case 0x3C:
            HAL_GPIO_WritePin(SENSOR_RL_PORT, SENSOR_RL_PIN, GPIO_PIN_RESET);
            HAL_Delay(5);
            HAL_GPIO_WritePin(SENSOR_RL_PORT, SENSOR_RL_PIN, GPIO_PIN_SET);
            HAL_Delay(5);
            break;
        case 0x4D:
            HAL_GPIO_WritePin(SENSOR_LR_PORT, SENSOR_LR_PIN, GPIO_PIN_RESET);
            HAL_Delay(5);
            HAL_GPIO_WritePin(SENSOR_LR_PORT, SENSOR_LR_PIN, GPIO_PIN_SET);
            HAL_Delay(5);
            break;
        case 0x5E:
            HAL_GPIO_WritePin(SENSOR_LM_PORT, SENSOR_LM_PIN, GPIO_PIN_RESET);
            HAL_Delay(5);
            HAL_GPIO_WritePin(SENSOR_LM_PORT, SENSOR_LM_PIN, GPIO_PIN_SET);
            HAL_Delay(5);
            break;
        case 0x6F:
            HAL_GPIO_WritePin(SENSOR_LL_PORT, SENSOR_LL_PIN, GPIO_PIN_RESET);
            HAL_Delay(5);
            HAL_GPIO_WritePin(SENSOR_LL_PORT, SENSOR_LL_PIN, GPIO_PIN_SET);
            HAL_Delay(5);
            break;
    }
	
	
    

    ADDR_VL6180X = 0x29;
    if (HAL_I2C_IsDeviceReady(&hi2c2, (ADDR_VL6180X << 1), 3, 100) != HAL_OK) return -1;

    uint8_t model = 0;
    if (VL6180X_Read8_at(ADDR_VL6180X, VL6180X_REG_IDENTIFICATION_MODEL_ID, &model) != HAL_OK) return -2;
    if (model != 0xB4) return -3;

    if (VL6180X_WriteMandatorySettingsAt(ADDR_VL6180X) != 0) return -4;

    if ((newAddress != 0x29) && (VL6180X_SetAddressAt(ADDR_VL6180X, newAddress, &ADDR_VL6180X) != HAL_OK)) return -5;

    sens_count++;

    return 0;
}

static void All_SHDN_Reset(void)
{
  HAL_GPIO_WritePin(SENSOR_RR_PORT, SENSOR_RR_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SENSOR_RM_PORT, SENSOR_RM_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SENSOR_RL_PORT, SENSOR_RL_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SENSOR_LR_PORT, SENSOR_LR_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SENSOR_LM_PORT, SENSOR_LM_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SENSOR_LL_PORT, SENSOR_LL_PIN, GPIO_PIN_RESET);
}

static void One_SHDN_Set(uint8_t idx)
{
  switch(idx){
    case 0: HAL_GPIO_WritePin(SENSOR_RR_PORT, SENSOR_RR_PIN, GPIO_PIN_SET); break;
    case 1: HAL_GPIO_WritePin(SENSOR_RM_PORT, SENSOR_RM_PIN, GPIO_PIN_SET); break;
    case 2: HAL_GPIO_WritePin(SENSOR_RL_PORT, SENSOR_RL_PIN, GPIO_PIN_SET); break;
    case 3: HAL_GPIO_WritePin(SENSOR_LR_PORT, SENSOR_LR_PIN, GPIO_PIN_SET); break;
    case 4: HAL_GPIO_WritePin(SENSOR_LM_PORT, SENSOR_LM_PIN, GPIO_PIN_SET); break;
    case 5: HAL_GPIO_WritePin(SENSOR_LL_PORT, SENSOR_LL_PIN, GPIO_PIN_SET); break;
  }
}

int VL6180X_InitAll(void)
{
    All_SHDN_Reset();
    HAL_Delay(20);

    for (uint8_t i = 0; i < NUM_SENSORS; i++)
    {
        One_SHDN_Set(i);
        HAL_Delay(50);   // etwas mehr Boot-Zeit

        uint8_t addr = 0x29;
        uint8_t model = 0;

        if (HAL_I2C_IsDeviceReady(&hi2c2, addr << 1, 5, 100) != HAL_OK) {
            PY_Printf("Init %d: 0x29 not ready", i);
            return -(10 + i);
        }

        if (VL6180X_Read8_at(addr, VL6180X_REG_IDENTIFICATION_MODEL_ID, &model) != HAL_OK) {
            PY_Printf("Init %d: model read failed", i);
            return -(20 + i);
        }

        if (model != 0xB4) {
            PY_Printf("Init %d: wrong model = 0x%02X", i, model);
            return -(30 + i);
        }

        if (VL6180X_WriteMandatorySettingsAt(addr) != 0) {
            PY_Printf("Init %d: mandatory settings failed", i);
            return -(40 + i);
        }

        /* ----------- WICHTIGER FIX ----------- */
        uint8_t fresh = 0;
        if (VL6180X_Read8_at(addr, 0x016, &fresh) != HAL_OK) {
            PY_Printf("Init %d: read fresh failed", i);
            return -(45 + i);
        }

        if (fresh == 1) {
            if (VL6180X_Write8_at(addr, 0x016, 0x00) != HAL_OK) {
                PY_Printf("Init %d: clear fresh failed", i);
                return -(46 + i);
            }
        }
        /* ------------------------------------- */

        uint8_t dummy = addr;
        if (VL6180X_SetAddressAt(addr, sens_addr[i], &dummy) != HAL_OK) {
            PY_Printf("Init %d: set address 0x%02X failed", i, sens_addr[i]);
            return -(50 + i);
        }

        HAL_Delay(10);

        if (HAL_I2C_IsDeviceReady(&hi2c2, sens_addr[i] << 1, 5, 100) != HAL_OK) {
            PY_Printf("Init %d: new address 0x%02X not ready", i, sens_addr[i]);
            return -(60 + i);
        }

        PY_Printf("Init %d OK -> 0x%02X", i, sens_addr[i]);
    }

    return 0;
}
#define VL6180X_REG_RESULT__RANGE_STATUS 0x04D
void TestDev(void)
{
    for (int i = 0; i < NUM_SENSORS; i++) {
		PY_Printf("addr 0x%02X ready=%d",
			sens_addr[i],
			HAL_I2C_IsDeviceReady(&hi2c2, sens_addr[i] << 1, 2, 20) == HAL_OK);

		uint8_t rs = 0;
		VL6180X_Read8_at(sens_addr[i], VL6180X_REG_RESULT__RANGE_STATUS, &rs);

		PY_Printf("addr 0x%02X range_status=0x%02X", sens_addr[i], rs);
    }

    PY_Printf("default 0x29 ready=%d",
        HAL_I2C_IsDeviceReady(&hi2c2, 0x29 << 1, 2, 20) == HAL_OK);
	
	
}
// ===========================================================
// VL6180X single-shot read for a specific sensor (7-bit addr)
// ===========================================================
uint16_t VL6180X_ReadRangeSingleAt(uint8_t addr)
{
    uint8_t status;
    uint8_t range;
    uint32_t t0 = HAL_GetTick();

    // interrupt l?schen
    if (VL6180X_Write8_at(addr, 0x015, 0x07) != HAL_OK)
        return 0xFFFF;

    // Messung starten
    if (VL6180X_Write8_at(addr, 0x018, 0x01) != HAL_OK)
        return 0xFFFF;

    while (1)
    {
        if (VL6180X_Read8_at(addr, 0x04F, &status) != HAL_OK)
            return 0xFFFF;

        if (status & 0x07)   // irgendein Ergebnis fertig
            break;

        if ((HAL_GetTick() - t0) > 100)
            return 0xFFFF;
    }

    if (VL6180X_Read8_at(addr, 0x062, &range) != HAL_OK)
        return 0xFFFF;

    VL6180X_Write8_at(addr, 0x015, 0x07);

    return range;
}
// ===========================================================
// Convenience: read one sensor by index (0..NUM_SENSORS-1)
// ===========================================================
uint16_t VL6180X_ReadSensorByIndex(uint8_t idx)
{
    if (idx >= NUM_SENSORS) return 0xFFFF;
    return VL6180X_ReadRangeSingleAt(sens_addr[idx]);
}

// ===========================================================
// Read all sensors into an array (out_mm must have NUM_SENSORS)
// Returns 0 if ok, else negative on first error
// ===========================================================
int VL6180X_ReadAll(uint16_t out_mm[NUM_SENSORS])
{
    for (uint8_t i = 0; i < NUM_SENSORS; i++)
    {
        uint16_t v = VL6180X_ReadRangeSingleAt(sens_addr[i]);
        out_mm[i] = v;

        if (v == 0xFFFF)
            return -(int)(i + 1); // e.g. -1 means sensor 0 failed, -2 sensor 1, ...
    }
    return 0;
}

//tof_err_t VL6180X_ReadRangeSingleAtEx(uint8_t addr, uint16_t *out_mm)
//{
//	int ret = VL6180X_Write8_at(addr, VL6180X_REG_SYSRANGE__START, 0x01);
//	
//  if (ret != HAL_OK) {
//  
//	uint32_t e = HAL_I2C_GetError(&hi2c2);
//	  PY_Printf("I2C Error: %d\n", e);//return TOF_ERR_I2C;
//  }
//	  

//  uint32_t t0 = HAL_GetTick();
//  while (1) {
//    uint8_t st = 0;
//    if (VL6180X_Read8_at(addr, VL6180X_REG_RESULT__INTERRUPT_STATUS_GPIO, &st) != HAL_OK)
//      return TOF_ERR_I2C;

//    if ((st & 0x07) == 0x04) break;

//    if ((HAL_GetTick() - t0) > 50) return TOF_ERR_TIMEOUT;
//  }

//  uint8_t range_mm = 0;
//  if (VL6180X_Read8_at(addr, VL6180X_REG_RESULT__RANGE_VAL, &range_mm) != HAL_OK)
//    return TOF_ERR_I2C;

//  (void)VL6180X_Write8_at(addr, VL6180X_REG_SYSTEM__INTERRUPT_CLEAR, 0x07);

//  *out_mm = range_mm;
//  return TOF_OK;
//}
#define VL6180X_REG_RESULT__RANGE_STATUS 0x04D

tof_err_t VL6180X_ReadRangeSingleAtEx(uint8_t addr, uint16_t *out_mm)
{
    if (VL6180X_Write8_at(addr, VL6180X_REG_SYSRANGE__START, 0x01) != HAL_OK) {
        PY_Printf("addr 0x%02X: start failed, i2c_err=%lu", addr, HAL_I2C_GetError(&hi2c2));
        return TOF_ERR_I2C;
    }

    uint32_t t0 = HAL_GetTick();
    uint8_t st = 0;

    while (1) {
        if (VL6180X_Read8_at(addr, VL6180X_REG_RESULT__INTERRUPT_STATUS_GPIO, &st) != HAL_OK) {
            PY_Printf("addr 0x%02X: status read failed", addr);
            return TOF_ERR_I2C;
        }

        if (st & 0x07) break;   // nicht nur == 0x04

        if ((HAL_GetTick() - t0) > 100) {
            PY_Printf("addr 0x%02X: timeout, int_status=0x%02X", addr, st);
            return TOF_ERR_TIMEOUT;
        }
    }

    uint8_t range_mm = 0;
    uint8_t range_status = 0;

    VL6180X_Read8_at(addr, VL6180X_REG_RESULT__RANGE_STATUS, &range_status);

    if (VL6180X_Read8_at(addr, VL6180X_REG_RESULT__RANGE_VAL, &range_mm) != HAL_OK) {
        PY_Printf("addr 0x%02X: range read failed", addr);
        return TOF_ERR_I2C;
    }

    (void)VL6180X_Write8_at(addr, VL6180X_REG_SYSTEM__INTERRUPT_CLEAR, 0x07);

    PY_Printf("addr 0x%02X: int=0x%02X range=%u status=0x%02X",
              addr, st, range_mm, range_status);

    *out_mm = range_mm;
    return TOF_OK;
}
