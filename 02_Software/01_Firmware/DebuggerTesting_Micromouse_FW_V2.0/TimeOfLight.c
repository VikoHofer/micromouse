#include "TimeOfLight.h"

/* ===================== Public globals ===================== */

I2C_HandleTypeDef hi2c2;

/**
 * Unique 7-bit addresses assigned to sensors during initialization.
 * These must not conflict with other devices on the I2C2 bus.
 */
const uint8_t tof_sensor_addr[NUM_SENSORS] = {
    0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F
};

/* ===================== Pin mapping ===================== */

#define SHDN_PORT_B GPIOB
#define SHDN_PORT_C GPIOC

/* Pin definitions for the 6 sensors (Right-Rear to Left-Left) */
#define SENSOR_RR_PORT  SHDN_PORT_B
#define SENSOR_RR_PIN   GPIO_PIN_7
#define SENSOR_RM_PORT  SHDN_PORT_C
#define SENSOR_RM_PIN   GPIO_PIN_13
#define SENSOR_RL_PORT  SHDN_PORT_C
#define SENSOR_RL_PIN   GPIO_PIN_4
#define SENSOR_LR_PORT  SHDN_PORT_C
#define SENSOR_LR_PIN   GPIO_PIN_5
#define SENSOR_LM_PORT  SHDN_PORT_C
#define SENSOR_LM_PIN   GPIO_PIN_2
#define SENSOR_LL_PORT  SHDN_PORT_C
#define SENSOR_LL_PIN   GPIO_PIN_3

/* ===================== Low level helpers ===================== */

/**
 * @brief Writes an 8-bit value to a 16-bit register address.
 * Note: VL6180X uses 16-bit index addresses.
 */
static HAL_StatusTypeDef VL6180X_Write8_at(uint8_t device_addr, uint16_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c2, (uint16_t)(device_addr << 1), reg,
                             I2C_MEMADD_SIZE_16BIT, &value, 1, 100);
}

/**
 * @brief Reads an 8-bit value from a 16-bit register address.
 */
static HAL_StatusTypeDef VL6180X_Read8_at(uint8_t device_addr, uint16_t reg, uint8_t *value)
{
    return HAL_I2C_Mem_Read(&hi2c2, (uint16_t)(device_addr << 1), reg,
                            I2C_MEMADD_SIZE_16BIT, value, 1, 100);
}

/**
 * @brief Changes the I2C slave address of a sensor.
 */
static HAL_StatusTypeDef VL6180X_SetAddressAt(uint8_t cur_addr, uint8_t new_addr)
{
    uint8_t addr7 = (uint8_t)(new_addr & 0x7F);
    if (addr7 == 0U) return HAL_ERROR;

    if (VL6180X_Write8_at(cur_addr, VL6180X_REG_I2C_SLAVE_DEVICE_ADDRESS, addr7) != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_Delay(2); // Small delay for address logic to stabilize
    return HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(addr7 << 1), 3, 100);
}

/**
 * @brief Loads the mandatory initialization settings from the datasheet.
 * These "magic" registers calibrate the internal sensor hardware.
 */
static int VL6180X_WriteMandatorySettingsAt(uint8_t device_addr)
{
    static const struct { uint16_t reg; uint8_t val; } init_seq[] = {
        {0x0207, 0x01}, {0x0208, 0x01}, {0x0096, 0x00}, {0x0097, 0xFD},
        {0x00E3, 0x00}, {0x00E4, 0x04}, {0x00E5, 0x02}, {0x00E6, 0x01},
        {0x00E7, 0x03}, {0x00F5, 0x02}, {0x00D9, 0x05}, {0x00DB, 0xCE},
        {0x00DC, 0x03}, {0x00DD, 0xF8}, {0x009F, 0x00}, {0x00A3, 0x3C},
        {0x00B7, 0x00}, {0x00BB, 0x3C}, {0x00B2, 0x09}, {0x00CA, 0x09},
        {0x0198, 0x01}, {0x01B0, 0x17}, {0x01AD, 0x00}, {0x00FF, 0x05},
        {0x0100, 0x05}, {0x0199, 0x05}, {0x01A6, 0x1B}, {0x01AC, 0x3E},
        {0x01A7, 0x1F}, {0x0030, 0x00}, {0x0133, 0x01}, {0x0109, 0x07},
        {0x010A, 0x30}, {0x003F, 0x46}, {0x0103, 0x01}, {0x001B, 0x0A},
        {0x003E, 0x0A}, {0x0131, 0x04}, {0x0011, 0x10}, {0x0014, 0x24},
        {0x0031, 0xFF}, {0x00D2, 0x01}, {0x00F2, 0x01}
    };

    for (uint32_t i = 0; i < (sizeof(init_seq) / sizeof(init_seq[0])); i++) {
        if (VL6180X_Write8_at(device_addr, init_seq[i].reg, init_seq[i].val) != HAL_OK) return -1;
    }

    // Clear interrupts and set convergence time
    VL6180X_Write8_at(device_addr, VL6180X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x24);
    VL6180X_Write8_at(device_addr, VL6180X_REG_SYSRANGE_MAX_CONVERGENCE_TIME, 0x0A); // 10 ms
    VL6180X_Write8_at(device_addr, VL6180X_REG_SYSTEM_INTERRUPT_CLEAR, 0x07);

    return 0;
}

/* ===================== SHDN helpers ===================== */

/**
 * @brief Pulls all XSHUT pins LOW (Putting all sensors into reset).
 */
static void All_SHDN_Reset(void)
{
    HAL_GPIO_WritePin(SENSOR_RR_PORT, SENSOR_RR_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SENSOR_RM_PORT, SENSOR_RM_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SENSOR_RL_PORT, SENSOR_RL_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SENSOR_LR_PORT, SENSOR_LR_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SENSOR_LM_PORT, SENSOR_LM_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SENSOR_LL_PORT, SENSOR_LL_PIN, GPIO_PIN_RESET);
}

/**
 * @brief Pulls one specific XSHUT pin HIGH (Waking up one sensor).
 */
static void One_SHDN_Set(uint8_t idx)
{
    switch (idx) {
        case 0: HAL_GPIO_WritePin(SENSOR_RR_PORT, SENSOR_RR_PIN, GPIO_PIN_SET); break;
        case 1: HAL_GPIO_WritePin(SENSOR_RM_PORT, SENSOR_RM_PIN, GPIO_PIN_SET); break;
        case 2: HAL_GPIO_WritePin(SENSOR_RL_PORT, SENSOR_RL_PIN, GPIO_PIN_SET); break;
        case 3: HAL_GPIO_WritePin(SENSOR_LR_PORT, SENSOR_LR_PIN, GPIO_PIN_SET); break;
        case 4: HAL_GPIO_WritePin(SENSOR_LM_PORT, SENSOR_LM_PIN, GPIO_PIN_SET); break;
        case 5: HAL_GPIO_WritePin(SENSOR_LL_PORT, SENSOR_LL_PIN, GPIO_PIN_SET); break;
        default: break;
    }
}

/* ===================== Public API ===================== */
 /* ===================== HAL init ===================== */


void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance != I2C2) {
        return;
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_I2C2_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;

    GPIO_InitStruct.Pin = GPIO_PIN_10; /* SCL */
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12; /* SDA */
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
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


void MX_I2C2_Init(void)
{
    hi2c2.Instance = I2C2;
    hi2c2.Init.ClockSpeed = 400000;
    hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;


    if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
        while (1) {}
    }
}


/**
 * @brief Initializes all connected VL6180X sensors and assigns unique I2C addresses.
 * @returns 0 on success, or a negative value indicating the failed sensor index.
 */
int VL6180X_InitAll(void)
{
    All_SHDN_Reset();
    HAL_Delay(20);

    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        uint8_t model = 0, fresh = 0;

        One_SHDN_Set(i); // Wake up only the current sensor
        HAL_Delay(50);   // Wait for boot

        // Verify communication at the default address (0x29)
        if (HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(VL6180X_DEFAULT_ADDR << 1), 5, 100) != HAL_OK) return -(10 + (int)i);

        // Check Model ID (Should be 0xB4)
        VL6180X_Read8_at(VL6180X_DEFAULT_ADDR, VL6180X_REG_IDENTIFICATION_MODEL_ID, &model);
        if (model != 0xB4U) return -(30 + (int)i);

        // Apply tuning settings
        if (VL6180X_WriteMandatorySettingsAt(VL6180X_DEFAULT_ADDR) != 0) return -(40 + (int)i);

        // Handle "Fresh out of reset" flag
        VL6180X_Read8_at(VL6180X_DEFAULT_ADDR, VL6180X_REG_SYSTEM_FRESH_OUT_OF_RESET, &fresh);
        if (fresh == 1U) VL6180X_Write8_at(VL6180X_DEFAULT_ADDR, VL6180X_REG_SYSTEM_FRESH_OUT_OF_RESET, 0x00);

        // Change from 0x29 to the unique address in tof_sensor_addr[i]
        if (VL6180X_SetAddressAt(VL6180X_DEFAULT_ADDR, tof_sensor_addr[i]) != HAL_OK) return -(50 + (int)i);
    }
    return 0;
}

/**
 * @brief Performs a single-shot range measurement.
 * @param device_addr_7bit: The 7-bit I2C address of the target sensor.
 * @returns Distance in mm (0..255), or 0xFFFF on error.
 */
uint16_t VL6180X_ReadRangeSingleAt(uint8_t device_addr_7bit)
{
    uint16_t range_mm = VL6180X_INVALID_RANGE;

    if (VL6180X_ReadRangeSingleAtEx(device_addr_7bit, &range_mm) != TOF_OK) {
        return VL6180X_INVALID_RANGE;
    }

    return range_mm;
}

/**
 * @brief Reads range from a sensor based on its array index.
 * @param idx: Sensor index (0 to NUM_SENSORS-1).
 * @returns Distance in mm, or 0xFFFF on error.
 */
uint16_t VL6180X_ReadSensorByIndex(uint8_t idx)
{
    if (idx >= NUM_SENSORS) {
        return VL6180X_INVALID_RANGE;
    }

    // Uses the global address array to find the hardware address for this index
    return VL6180X_ReadRangeSingleAt(tof_sensor_addr[idx]);
}

/**
 * @brief Reads distances from all sensors in a single pass.
 * @param out_mm: Array of size NUM_SENSORS to store results.
 * @returns 0 on success, or a negative index on first failure.
 */
int VL6180X_ReadAll(uint16_t out_mm[NUM_SENSORS])
{
    if (out_mm == NULL) {
        return -100;
    }

    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        tof_err_t err = VL6180X_ReadRangeSingleAtEx(tof_sensor_addr[i], &out_mm[i]);
        if (err != TOF_OK) {
            return -(int)(i + 1); // Returns 1-based negative index of the failed sensor
        }
    }

    return 0;
}

tof_err_t VL6180X_StartContinuousAt(uint8_t addr, uint8_t intermeasurement_ms)
{
    // 1. Convert ms to the register format (Value is LSB * 10ms, but typically 0-255 is safe)
    // For many VL6180X versions, the value is simply (ms / 10) - 1. 
    // We'll use the raw value for simplicity, as 20ms is a common standard.
    uint8_t period = (intermeasurement_ms > 10) ? (intermeasurement_ms / 10) : 0x00;

    if (VL6180X_Write8_at(addr, VL6180X_REG_SYSRANGE_INTERMEASUREMENT_PERIOD, period) != HAL_OK) {
        return TOF_ERR_I2C;
    }

    // 2. Clear any pending interrupts to ensure a clean start
    if (VL6180X_Write8_at(addr, VL6180X_REG_SYSTEM_INTERRUPT_CLEAR, 0x07) != HAL_OK) {
        return TOF_ERR_I2C;
    }

    // 3. Start Continuous Mode (Bit 1 and Bit 0 set to 1)
    // 0x03 = START_STOP bit set to 1 and MODE select set to Continuous
    if (VL6180X_Write8_at(addr, VL6180X_REG_SYSRANGE_START, 0x03) != HAL_OK) {
        return TOF_ERR_I2C;
    }

    return TOF_OK;
}

/**
 * @brief Stops continuous ranging and returns the sensor to standby.
 * @param addr: 7-bit I2C address.
 * @returns tof_err_t status.
 */
tof_err_t VL6180X_StopContinuousAt(uint8_t addr)
{
    // Writing 0x01 to SYSRANGE_START sets it back to Single-Shot/Standby mode
    // after the current measurement finishes.
    if (VL6180X_Write8_at(addr, VL6180X_REG_SYSRANGE_START, 0x01) != HAL_OK) {
        return TOF_ERR_I2C;
    }
    
    return TOF_OK;
}

/**
 * @brief Blocking read of the most recent continuous measurement.
 * @param addr: 7-bit I2C address.
 * @param out_mm: Pointer to store the distance result.
 * @returns tof_err_t status.
 */
tof_err_t VL6180X_ReadContinuousAt(uint8_t addr, uint16_t *out_mm)
{
    uint32_t t0 = HAL_GetTick();

    if (out_mm == NULL) return TOF_ERR_PARAM;

    // Polling loop to wait for data to become ready
    while (1) {
        tof_err_t err = VL6180X_ReadContinuousIfReadyAt(addr, out_mm);
        if (err == TOF_OK) return TOF_OK;
        
        // Return if I2C fails entirely
        if (err == TOF_ERR_I2C) return TOF_ERR_I2C;

        // Break on timeout (100ms)
        if ((HAL_GetTick() - t0) > 100U) {
            *out_mm = VL6180X_INVALID_RANGE;
            return TOF_ERR_TIMEOUT;
        }
    }
}

/**
 * @brief Extended version of single-shot read with explicit error reporting.
 * @param addr: 7-bit I2C address.
 * @param out_mm: Pointer to store the resulting distance.
 * @returns tof_err_t status.
 */
tof_err_t VL6180X_ReadRangeSingleAtEx(uint8_t addr, uint16_t *out_mm)
{
    uint8_t status = 0, range = 0;
    if (out_mm == NULL) return TOF_ERR_PARAM;
    *out_mm = VL6180X_INVALID_RANGE;

    // Trigger single measurement
    VL6180X_Write8_at(addr, VL6180X_REG_SYSRANGE_START, 0x01);

    // Wait for the interrupt status bit to indicate completion
    uint32_t t0 = HAL_GetTick();
    while (1) {
        VL6180X_Read8_at(addr, VL6180X_REG_RESULT_INTERRUPT_STATUS_GPIO, &status);
        if ((status & 0x07U) != 0U) break;
        if ((HAL_GetTick() - t0) > 100U) return TOF_ERR_TIMEOUT;
    }

    // Read result and clear the interrupt
    VL6180X_Read8_at(addr, VL6180X_REG_RESULT_RANGE_VAL, &range);
    VL6180X_Write8_at(addr, VL6180X_REG_SYSTEM_INTERRUPT_CLEAR, 0x07);

    *out_mm = (uint16_t)range;
    return TOF_OK;
}

/**
 * @brief Non-blocking read that only returns data if a new measurement is ready.
 * @param addr: 7-bit I2C address.
 * @param out_mm: Pointer to store the distance result.
 * @returns TOF_OK if new data read, TOF_ERR_TIMEOUT (Not Ready) if no update available.
 */
tof_err_t VL6180X_ReadContinuousIfReadyAt(uint8_t addr, uint16_t *out_mm)
{
    uint8_t irq_status = 0, range = 0;
    if (out_mm == NULL) return TOF_ERR_PARAM;

    VL6180X_Read8_at(addr, VL6180X_REG_RESULT_INTERRUPT_STATUS_GPIO, &irq_status);
    
    if ((irq_status & 0x04U) == 0U) return TOF_ERR_NOT_READY; // Measurement not finished yet

    VL6180X_Read8_at(addr, VL6180X_REG_RESULT_RANGE_VAL, &range);
    VL6180X_Write8_at(addr, VL6180X_REG_SYSTEM_INTERRUPT_CLEAR, 0x07);

    *out_mm = (uint16_t)range;

    return TOF_OK;
}