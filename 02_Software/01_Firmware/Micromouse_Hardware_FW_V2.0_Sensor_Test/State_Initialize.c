/*************************************************************
Project     : Micromouse V2.0
File        : State_Initialze.c
Description : State Initialize -> set everything up
Date        : 11.03.2026
*************************************************************/

#include "StateMachine.h"


BNO055_Handle_t bno;
IMU_MotionState_t imu;

State_t const State_Initialize =
{
    State_Initialize_Enter,
    State_Initialize_Process,
    State_Initialize_Exit
};

static void I2C_Scan(void)
{
    printf("I2C scan start...\r\n");

    for (uint8_t addr = 1U; addr < 128U; addr++)
    {
        HAL_StatusTypeDef s;

        s = HAL_I2C_IsDeviceReady(&hi2c2,
                                  (uint16_t)(addr << 1),
                                  2,
                                  20);

        if (s == HAL_OK)
        {
            printf("I2C device found at 0x%02X\r\n", addr);
        }
    }

    printf("I2C scan done.\r\n");
}

static void Init_System(void)
{
    HAL_Init();

    if (Clock_Config_Max_HSI() != HAL_OK)
    {
        while (1)
        {
            /* clock error */
        }
    }
}

static void Init_BasicPeripherals(void)
{
    Uart6_Init();

    BUTTON_Init();
    Board_LED_Init();
}

static void Init_I2C_Peripherals(void)
{
    HAL_I2C_MspInit(&hi2c2);
    MX_I2C2_Init();
}

static void Init_ToF_WallSensors(void)
{
    /*
     * ToF wird für die normalen Wandsensoren verwendet.
     * Diagonale Sensoren sind nicht mehr ToF, sondern ADC-IR.
     *
     * TOF_ACTIVE_MASK muss daher die diagonalen Sensoren ausschließen.
     */
    if (VL6180X_InitMask(TOF_ACTIVE_MASK) != 0)
    {
        printf("VL6180X_InitMask failed\r\n");
    }

    for (uint8_t i = 0U; i < NUM_SENSORS; i++)
    {
        if ((TOF_ACTIVE_MASK & (1UL << i)) == 0U)
        {
            continue;
        }

        if (VL6180X_StartContinuousAt(tof_sensor_addr[i], 11) != TOF_OK)
        {
            printf("VL6180X continuous start failed idx=%u addr=0x%02X\r\n",
                   i,
                   tof_sensor_addr[i]);
        }
    }
}

static void Init_DiagonalSensors(void)
{
    /*
     * Bei WALL_45_BACKEND_ADC_IR ruft WallSensors45_Init()
     * intern IR_Init() -> Sensor_Init() auf.
     *
     * Damit werden PC13/PB0 und PC2/PA4 initialisiert.
     */
    WallSensors45_Init();

#if WALL_45_BACKEND != WALL_45_BACKEND_ADC_IR
#warning "Diagonal wall sensors are not configured as ADC-IR"
#endif
}

static void Init_MotionSystem(void)
{
    Motor_Init();

    encoder_init();
    encoder_start();

    MotionController_Init();
    MotionController_Reset();

    Motion_Init();
}

static void Init_MazeData(void)
{
    InitMouseMaze(mouseMaze);
    InitMaze(maze);
}

/* initialize everything */
void State_Initialize_Enter(void)
{
    Init_System();

    Init_BasicPeripherals();

    Init_I2C_Peripherals();

    Init_ToF_WallSensors();

    Init_DiagonalSensors();

    Init_MotionSystem();

    Init_MazeData();

    //I2C_Scan();

    printf("Initialize Done!!\r\n");
    printf("Micromouse Running on: %lu Hz\r\n", SystemCoreClock);
}

/* initialization process */
State_t const* State_Initialize_Process(void)
{
    if (BUTTON2_Pressed())
    {
#if WALL_45_BACKEND == WALL_45_BACKEND_ADC_IR
        /*
         * Kalibriert nur die beiden diagonalen ADC-IR-Sensoren.
         * Maus dabei mittig bzw. in definierter Referenzposition platzieren.
         */
        IR_CalibrateReference();
        printf("Diagonal IR calibration done\r\n");
#endif

        return &State_SolveMaze;
    }

    return &State_Initialize;
}

void State_Initialize_Exit(void)
{
    /* Turn all LEDs off if needed */
}