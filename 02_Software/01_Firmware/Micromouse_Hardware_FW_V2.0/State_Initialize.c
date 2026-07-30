/*************************************************************
Project		: Micromouse V2.0
File		: State_Initialze.c
Description	: State Initialize -> set everything up
Date		: 11.03.2026
*************************************************************/

#include "StateMachine.h"

BNO055_Handle_t bno;
IMU_MotionState_t imu;

State_t const State_Initialize = {State_Initialize_Enter, State_Initialize_Process, State_Initialize_Exit};

static void I2C_Scan(void)
{
    printf("I2C scan start...\r\n");

    for (uint8_t addr = 1; addr < 128; addr++) {
        HAL_StatusTypeDef s = HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(addr << 1), 2, 20);
        if (s == HAL_OK) {
            printf("I2C device found at 0x%02X\r\n", addr);
        }
    }

    printf("I2C scan done.\r\n");
}

// initialize everthing
void State_Initialize_Enter(void)
{
	// initialize HAL-Library
	HAL_Init();
	
	// config clock
	if (Clock_Config_Max_HSI() != HAL_OK) {
        while (1) {
            /* clock error */
        }
    }
	// imu
	HAL_I2C_MspInit(&hi2c2);
	MX_I2C2_Init();

	VL6180X_InitAll();

	for (int i = 0; i < NUM_SENSORS; i++)
	{
		if ((TOF_ACTIVE_MASK & (1U << i)) == 0U) {
			continue;
		}

		VL6180X_StartContinuousAt(tof_sensor_addr[i], 11);
	}
	
	
	
	// peripheral init
	BUTTON_Init();
	Board_LED_Init();
	
	Uart6_Init();
	
	Motor_Init();
	
	encoder_init();
	encoder_start();

	MotionController_Init();
    MotionController_Reset();
	
	Motion_Init();
	
	InitMouseMaze(mouseMaze);
    InitMaze(maze);
	
	WallSensors45_Init();
	I2C_Scan();

//    if (BNO055_Init(&bno, &hi2c2, BNO055_ADDRESS_B) != HAL_OK) {
//        printf("BNO055_Init FAILED err=0x%08lX\r\n", HAL_I2C_GetError(&hi2c2));
//    } else {
//        printf("BNO055_Init OK\r\n");
//    }

//    if (IMU_Motion_Init(&bno, &imu) != HAL_OK) {
//        printf("IMU_Motion_Init FAILED\r\n");
//    } else {
//        printf("IMU_Motion_Init OK\r\n");
//    }

//    // Direkt nach Init testen
//    HAL_StatusTypeDef s;
//    uint8_t chip = 0;
//    bno055_opmode_t mode = OPERATION_MODE_CONFIG;
//    bno055_vector_t a = {0}, g = {0}, e = {0};

//    s = BNO055_GetChipID(&bno, &chip);
//    printf("ChipID read: status=%d err=0x%08lX id=0x%02X\r\n", s, HAL_I2C_GetError(&hi2c2), chip);

//    s = BNO055_GetOperationMode(&bno, &mode);
//    printf("Mode read: status=%d err=0x%08lX mode=%d\r\n", s, HAL_I2C_GetError(&hi2c2), mode);

//    s = BNO055_ReadAccelerometer(&bno, &a);
//    printf("ACC test: status=%d err=0x%08lX ax=%d ay=%d az=%d\r\n", s, HAL_I2C_GetError(&hi2c2), a.x, a.y, a.z);

//    s = BNO055_ReadGyroscope(&bno, &g);
//    printf("GYR test: status=%d err=0x%08lX gx=%d gy=%d gz=%d\r\n", s, HAL_I2C_GetError(&hi2c2), g.x, g.y, g.z);

//    s = BNO055_ReadEuler(&bno, &e);
//    printf("EUL test: status=%d err=0x%08lX ex=%d ey=%d ez=%d\r\n", s, HAL_I2C_GetError(&hi2c2), e.x, e.y, e.z);

//    printf("Initialize Done!!\r\n");
//    printf("Micromouse Running on: %lu Hz\r\n", SystemCoreClock);

//	
	
	printf("Initialize Done!!\n");
	printf("Micromouse Running on: %d Hz\n", SystemCoreClock);
	
	// Test OPT
	uint32_t v;

	opt_read24(0x59, 0x27, &v);
	printf("REG 0x27 = 0x%06lX\r\n", v);

	opt_read24(0x59, 0x9F, &v);
	printf("REG 0x9F = 0x%06lX\r\n", v);

	opt_read24(0x59, 0x2A, &v);
	printf("REG 0x2A = 0x%06lX\r\n", v);

	opt_read24(0x59, 0x29, &v);
	printf("REG 0x29 = 0x%06lX\r\n", v);

	opt_read24(0x59, 0x79, &v);
	printf("REG 0x79 = 0x%06lX\r\n", v);
	
	opt_read24(0x59, 0x6E, &v);
	printf("REG 0x6E = 0x%06lX\r\n", v);

	opt_read24(0x59, 0x80, &v);
	printf("REG 0x80 = 0x%06lX\r\n", v);
	
	opt_read24(0x59, 0x78, &v);
	printf("REG 0x78 = 0x%06lX\r\n", v);
	
	opt_read24(0x59, 0x7A, &v);
	printf("REG 0x7A = 0x%06lX\r\n", v);

}

// initialization process
State_t const* State_Initialize_Process(void)
{
    if (BUTTON2_Pressed()) {

//        if (IMU_Motion_Calibrate(&bno, &imu, 200) != HAL_OK) {
//            printf("Calibration ERROR\n");
//            return &State_Initialize;   // hier bleiben
//        } else {
//            printf("Calibration OK\n");
//            return &State_SolveMaze;
//        }
		return &State_SolveMaze;
		
	}

    return &State_Initialize;
}

// ---------------------
void State_Initialize_Exit(void)
{
	// Turn all Leds off
}

