#include "imu_test.h"
#include "Delay.h"
#include <stdio.h>

#define BNO055_DEFAULT_TIMEOUT 1000 

HAL_StatusTypeDef BNO055_DiagnoseI2C(BNO055_Handle_t* bno)
{
    for (int timeout = 100; timeout <= 2000; timeout += 500) {
        for (int retries = 1; retries <= 5; retries++) {
            HAL_StatusTypeDef s = HAL_I2C_IsDeviceReady(bno->hi2c, bno->dev_address, retries, timeout);
            if (s == HAL_OK) return HAL_OK;
            HAL_Delay(10);
        }
    }
    return HAL_ERROR;
} 

/**
 * @brief Initialize BNO055 sensor with comprehensive I2C diagnostics
 */
HAL_StatusTypeDef BNO055_Init(BNO055_Handle_t* bno, I2C_HandleTypeDef* hi2c, uint8_t addr7)
{
    HAL_StatusTypeDef s;
    uint8_t chip_id = 0;
    uint8_t page0 = 0;
    uint8_t cfg = OPERATION_MODE_CONFIG;
    uint8_t pwr = POWER_MODE_NORMAL;
    uint8_t ndof = OPERATION_MODE_NDOF;
    uint8_t mode_read = 0;

    bno->hi2c = hi2c;
    bno->dev_address = ((uint16_t)addr7) << 1;
    bno->timeout = 1000;
    bno->mode = OPERATION_MODE_CONFIG;

    HAL_Delay(700);

    s = HAL_I2C_IsDeviceReady(bno->hi2c, bno->dev_address, 3, bno->timeout);
    if (s != HAL_OK) return s;

    s = HAL_I2C_Mem_Write(bno->hi2c, bno->dev_address, BNO055_PAGE_ID_ADDR,
                          I2C_MEMADD_SIZE_8BIT, &page0, 1, bno->timeout);
    if (s != HAL_OK) return s;

    s = HAL_I2C_Mem_Read(bno->hi2c, bno->dev_address, BNO055_CHIP_ID_ADDR,
                         I2C_MEMADD_SIZE_8BIT, &chip_id, 1, bno->timeout);
    if (s != HAL_OK) return s;
    if (chip_id != BNO055_ID) return HAL_ERROR;

    s = HAL_I2C_Mem_Write(bno->hi2c, bno->dev_address, BNO055_OPR_MODE_ADDR,
                          I2C_MEMADD_SIZE_8BIT, &cfg, 1, bno->timeout);
    if (s != HAL_OK) return s;
    HAL_Delay(30);

    s = HAL_I2C_Mem_Write(bno->hi2c, bno->dev_address, BNO055_PWR_MODE_ADDR,
                          I2C_MEMADD_SIZE_8BIT, &pwr, 1, bno->timeout);
    if (s != HAL_OK) return s;
    HAL_Delay(10);

    s = HAL_I2C_Mem_Write(bno->hi2c, bno->dev_address, BNO055_PAGE_ID_ADDR,
                          I2C_MEMADD_SIZE_8BIT, &page0, 1, bno->timeout);
    if (s != HAL_OK) return s;

    s = HAL_I2C_Mem_Write(bno->hi2c, bno->dev_address, BNO055_OPR_MODE_ADDR,
                          I2C_MEMADD_SIZE_8BIT, &ndof, 1, bno->timeout);
    if (s != HAL_OK) return s;
    HAL_Delay(50);

    s = HAL_I2C_Mem_Read(bno->hi2c, bno->dev_address, BNO055_OPR_MODE_ADDR,
                         I2C_MEMADD_SIZE_8BIT, &mode_read, 1, bno->timeout);
    if (s != HAL_OK) return s;
    if (mode_read != OPERATION_MODE_NDOF) return HAL_ERROR;

    bno->mode = OPERATION_MODE_NDOF;
    return HAL_OK;
}
/**
 * @brief Set operation mode using HAL_I2C_Mem_Write
 * @param bno: BNO055 handle
 * @param mode: Operation mode
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BNO055_SetOperationMode(BNO055_Handle_t *bno, bno055_opmode_t mode)
{
    HAL_StatusTypeDef status;
    uint8_t mode_value = (uint8_t)mode;
    
    status = HAL_I2C_Mem_Write(bno->hi2c, bno->dev_address, BNO055_OPR_MODE_ADDR,
                               I2C_MEMADD_SIZE_8BIT, &mode_value, 1, bno->timeout);
    if (status == HAL_OK) {
        bno->mode = mode;
        HAL_Delay(30); // Wait for mode change
    }
		
		volatile uint8_t current_mode = 0;
		status = HAL_I2C_Mem_Read(bno->hi2c, bno->dev_address, BNO055_OPR_MODE_ADDR,
                          I2C_MEMADD_SIZE_8BIT, &current_mode, 1, bno->timeout);
		
 
    return status;
}

/**
 * @brief Set power mode using HAL_I2C_Mem_Write
 * @param bno: BNO055 handle
 * @param mode: Power mode
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BNO055_SetPowerMode(BNO055_Handle_t *bno, bno055_powermode_t mode)
{
    HAL_StatusTypeDef status;
    uint8_t power_value = (uint8_t)mode;
    
    status = HAL_I2C_Mem_Write(bno->hi2c, bno->dev_address, BNO055_PWR_MODE_ADDR,
                               I2C_MEMADD_SIZE_8BIT, &power_value, 1, bno->timeout);
    if (status == HAL_OK) {
        HAL_Delay(10);
    }
    
    return status;
}

HAL_StatusTypeDef BNO055_GetPowerMode(BNO055_Handle_t *bno, bno055_powermode_t *mode)
{
    if(!bno || !mode) return HAL_ERROR;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        bno->hi2c,
        bno->dev_address,
        0x3E,                   // PWR_MODE Register
        I2C_MEMADD_SIZE_8BIT,
        mode,
        1,
        bno->timeout
    );

    return status;
}

HAL_StatusTypeDef BNO055_GetOperationMode(BNO055_Handle_t *bno, bno055_opmode_t *mode)
{
    if(!bno || !mode) return HAL_ERROR;

    uint8_t m = 0;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        bno->hi2c,
        bno->dev_address,
        0x3D,                   // OPR_MODE Register
        I2C_MEMADD_SIZE_8BIT,
        &m,
        1,
        bno->timeout
    );

    if(status == HAL_OK)
        *mode = (bno055_opmode_t)m;

    return status;
}


HAL_StatusTypeDef BNO055_ReadAccelerometer(BNO055_Handle_t* bno, bno055_vector_t* accel)
{
    uint8_t buf[6];
    HAL_StatusTypeDef s = HAL_I2C_Mem_Read(
        bno->hi2c,
        bno->dev_address,
        BNO055_ACCEL_DATA_X_LSB_ADDR,
        I2C_MEMADD_SIZE_8BIT,
        buf,
        6,
        bno->timeout
    );

    if (s != HAL_OK) {
        printf("ACC fail: status=%d err=0x%08lX reg=0x%02X\r\n",
               s, HAL_I2C_GetError(bno->hi2c), BNO055_ACCEL_DATA_X_LSB_ADDR);
        return s;
    }

    accel->x = (int16_t)((buf[1] << 8) | buf[0]);
    accel->y = (int16_t)((buf[3] << 8) | buf[2]);
    accel->z = (int16_t)((buf[5] << 8) | buf[4]);
    return HAL_OK;
}

HAL_StatusTypeDef BNO055_ReadLinearAcceleration(BNO055_Handle_t* bno, bno055_vector_t* accel)
{
    uint8_t buf[6];
    HAL_StatusTypeDef s = HAL_I2C_Mem_Read(
        bno->hi2c,
        bno->dev_address,
        BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR,
        I2C_MEMADD_SIZE_8BIT,
        buf,
        6,
        bno->timeout
    );

    if (s == HAL_OK) {
        accel->x = (int16_t)((buf[1] << 8) | buf[0]);
        accel->y = (int16_t)((buf[3] << 8) | buf[2]);
        accel->z = (int16_t)((buf[5] << 8) | buf[4]);
    }
    return s;
}

HAL_StatusTypeDef BNO055_ReadMagnetometer(BNO055_Handle_t* bno, bno055_vector_t* mag)
{
    uint8_t buf[6];
    HAL_StatusTypeDef s = HAL_I2C_Mem_Read(bno->hi2c, bno->dev_address,
                                           BNO055_MAG_DATA_X_LSB_ADDR,
                                           I2C_MEMADD_SIZE_8BIT, buf, 6, bno->timeout);
    if (s == HAL_OK) {
        mag->x = (int16_t)((buf[1] << 8) | buf[0]);
        mag->y = (int16_t)((buf[3] << 8) | buf[2]);
        mag->z = (int16_t)((buf[5] << 8) | buf[4]);
    }
    return s;
} 

HAL_StatusTypeDef BNO055_ReadGyroscope(BNO055_Handle_t* bno, bno055_vector_t* gyro)
{
    uint8_t buf[6];
    HAL_StatusTypeDef s = HAL_I2C_Mem_Read(
        bno->hi2c,
        bno->dev_address,
        BNO055_GYRO_DATA_X_LSB_ADDR,
        I2C_MEMADD_SIZE_8BIT,
        buf,
        6,
        bno->timeout
    );

    if (s != HAL_OK) {
        printf("GYR fail: status=%d err=0x%08lX reg=0x%02X\r\n",
               s, HAL_I2C_GetError(bno->hi2c), BNO055_GYRO_DATA_X_LSB_ADDR);
        return s;
    }

    gyro->x = (int16_t)((buf[1] << 8) | buf[0]);
    gyro->y = (int16_t)((buf[3] << 8) | buf[2]);
    gyro->z = (int16_t)((buf[5] << 8) | buf[4]);
    return HAL_OK;
}
HAL_StatusTypeDef BNO055_ReadEuler(BNO055_Handle_t* bno, bno055_vector_t* euler)
{
    uint8_t buf[6];
    HAL_StatusTypeDef s = HAL_I2C_Mem_Read(
        bno->hi2c,
        bno->dev_address,
        BNO055_EULER_H_LSB_ADDR,
        I2C_MEMADD_SIZE_8BIT,
        buf,
        6,
        bno->timeout
    );

    if (s != HAL_OK) {
        printf("EUL fail: status=%d err=0x%08lX reg=0x%02X\r\n",
               s, HAL_I2C_GetError(bno->hi2c), BNO055_EULER_H_LSB_ADDR);
        return s;
    }

    euler->x = (int16_t)((buf[1] << 8) | buf[0]);
    euler->y = (int16_t)((buf[3] << 8) | buf[2]);
    euler->z = (int16_t)((buf[5] << 8) | buf[4]);
    return HAL_OK;
}
HAL_StatusTypeDef BNO055_ReadQuaternion(BNO055_Handle_t* bno, bno055_quaternion_t* quat)
{
    uint8_t buf[8];
    HAL_StatusTypeDef s = HAL_I2C_Mem_Read(bno->hi2c, bno->dev_address,
                                           BNO055_QUATERNION_DATA_W_LSB_ADDR,
                                           I2C_MEMADD_SIZE_8BIT, buf, 8, bno->timeout);
    if (s == HAL_OK) {
        quat->w = (int16_t)((buf[1] << 8) | buf[0]);
        quat->x = (int16_t)((buf[3] << 8) | buf[2]);
        quat->y = (int16_t)((buf[5] << 8) | buf[4]);
        quat->z = (int16_t)((buf[7] << 8) | buf[6]);
    }
    return s;
} 

HAL_StatusTypeDef BNO055_GetCalibrationStatus(BNO055_Handle_t* bno, bno055_calibration_t* calib)
{
    uint8_t cal_status = 0;
    HAL_StatusTypeDef s = HAL_I2C_Mem_Read(bno->hi2c, bno->dev_address,
                                           BNO055_CALIB_STAT_ADDR,
                                           I2C_MEMADD_SIZE_8BIT, &cal_status, 1, bno->timeout);
    if (s == HAL_OK) {
        calib->system = (cal_status >> 6) & 0x03;
        calib->gyro   = (cal_status >> 4) & 0x03;
        calib->accel  = (cal_status >> 2) & 0x03;
        calib->mag    = cal_status & 0x03;
    }
    return s;
} 
/**
 * @brief Get system status using HAL_I2C_Mem_Read
 * @param bno: BNO055 handle
 * @param sys_stat: Pointer to store system status
 * @param sys_err: Pointer to store system error
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BNO055_GetSystemStatus(BNO055_Handle_t* bno, uint8_t* sys_stat, uint8_t* sys_err)
{
    HAL_StatusTypeDef s1 = HAL_I2C_Mem_Read(bno->hi2c, bno->dev_address, BNO055_SYS_STAT_ADDR,
                                            I2C_MEMADD_SIZE_8BIT, sys_stat, 1, bno->timeout);
    HAL_StatusTypeDef s2 = HAL_I2C_Mem_Read(bno->hi2c, bno->dev_address, BNO055_SYS_ERR_ADDR,
                                            I2C_MEMADD_SIZE_8BIT, sys_err, 1, bno->timeout);
    return (s1 == HAL_OK && s2 == HAL_OK) ? HAL_OK : HAL_ERROR;
}

/**
 * @brief Get chip ID using HAL_I2C_Mem_Read
 * @param bno: BNO055 handle
 * @param chip_id: Pointer to store chip ID
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BNO055_GetChipID(BNO055_Handle_t *bno, uint8_t *chip_id)
{
    return HAL_I2C_Mem_Read(bno->hi2c, bno->dev_address, BNO055_CHIP_ID_ADDR,
                            I2C_MEMADD_SIZE_8BIT, chip_id, 1, bno->timeout);
}

/**
 * @brief Check if BNO055 device is ready using HAL_I2C_IsDeviceReady
 * @param bno: BNO055 handle
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BNO055_IsDeviceReady(BNO055_Handle_t *bno)
{
    return HAL_I2C_IsDeviceReady(bno->hi2c, bno->dev_address, 3, bno->timeout);
}

/**
 * @brief Run comprehensive test of BNO055 using HAL I2C functions
 * @param bno: BNO055 handle
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BNO055_RunTest(BNO055_Handle_t *bno)
{
    HAL_StatusTypeDef status;
    uint8_t chip_id, sys_stat, sys_err;
    bno055_vector_t accel, gyro, mag, euler;
    bno055_quaternion_t quat;
    bno055_calibration_t calib;
    
    // Test 1: Check if device is ready
    status = BNO055_IsDeviceReady(bno);
    if (status != HAL_OK) {
        return status;
    }
    
    // Test 2: Check chip ID
    status = BNO055_GetChipID(bno, &chip_id);
    if (status != HAL_OK || chip_id != BNO055_ID) {
        return HAL_ERROR;
    }
    
    // Test 3: Check system status
    status = BNO055_GetSystemStatus(bno, &sys_stat, &sys_err);
    if (status != HAL_OK) {
        return status;
    }
    
    // Test 4: Read all sensor data
    status = BNO055_ReadAccelerometer(bno, &accel);
    if (status != HAL_OK) return status;
    
    status = BNO055_ReadGyroscope(bno, &gyro);
    if (status != HAL_OK) return status;
    
    status = BNO055_ReadMagnetometer(bno, &mag);
    if (status != HAL_OK) return status;
    
    status = BNO055_ReadEuler(bno, &euler);
    if (status != HAL_OK) return status;
    
    status = BNO055_ReadQuaternion(bno, &quat);
    if (status != HAL_OK) return status;
    
    // Test 5: Read calibration status
    status = BNO055_GetCalibrationStatus(bno, &calib);
    if (status != HAL_OK) return status;
		
		if (BNO055_SetOperationMode(bno, OPERATION_MODE_NDOF) != HAL_OK) return HAL_ERROR;
    
    return HAL_OK;
}

void Acceleration(BNO055_Handle_t *bno){
	//RHA Geschwindigkeitstest
	volatile static float x_velocity, y_velocity, z_velocity = {0,0,0};
	volatile bno055_vector_t accel;

	volatile uint32_t currentMsTicks = SysTick_Get();
	volatile static uint32_t lastTime;
	static int16_t accel_offset_x = 0;
	static int16_t accel_offset_y = 0;
	static int16_t accel_offset_z = 0;
	static bool doItOnce = true;
	
	if(doItOnce){
		doItOnce = false;
		lastTime = currentMsTicks;
	}
	volatile uint32_t now = currentMsTicks;
	volatile float dt = (now - lastTime) / 1000.0f; // Delta t in Sekunden
	lastTime = now;

	BNO055_ReadAccelerometer(bno,&accel); // m/s?

	// Nullpunktkorrektur, falls n?tig
	accel.x = (accel.x - accel_offset_x)/10;
	accel.y = (accel.y - accel_offset_y)/10;
	accel.z = (accel.z - accel_offset_z)/10;

	// Geschwindigkeit berechnen
	x_velocity += accel.x * dt;
	y_velocity += accel.y * dt;
	z_velocity += accel.z * dt;

	HAL_Delay(10); // optional, reduziert CPU-Last
	
}
void IMUDirection(BNO055_Handle_t *bno){
	
	bno055_vector_t currentEuler;
	BNO055_ReadEuler(bno, &currentEuler);
	float initialX= currentEuler.x;
	float initialY= currentEuler.y;
	float initialZ= currentEuler.z;
	
	volatile float deltaX;
	volatile float deltaY;
	volatile float deltaZ;
	
	while(1){
		BNO055_ReadEuler(bno, &currentEuler);
		deltaX = (currentEuler.x - initialX)/16;
		deltaY = (currentEuler.y - initialY)/16;
		deltaZ = (currentEuler.z - initialZ)/16;
		
	}
	
}

void AngularVelocity(BNO055_Handle_t *bno){

    bno055_vector_t gyro;

    // Optional: f?r ?t falls du es brauchst
    uint32_t lastTime = SysTick_Get();

    while(1){

        uint32_t now = SysTick_Get();
        float dt = (now - lastTime) / 1000.0f; // Sekunden
        lastTime = now;

        // Gyroskop auslesen
        BNO055_ReadGyroscope(bno, &gyro);

        // Winkelgeschwindigkeit in ?/s
        volatile float omega_x = gyro.x;
        volatile float omega_y = gyro.y;
        volatile float omega_z = gyro.z;

        // Optional: in rad/s umrechnen
        volatile float omega_x_rad = omega_x * (3.14159265f / 180.0f);
        volatile float omega_y_rad = omega_y * (3.14159265f / 180.0f);
        volatile float omega_z_rad = omega_z * (3.14159265f / 180.0f);

        HAL_Delay(10);
    }
}

