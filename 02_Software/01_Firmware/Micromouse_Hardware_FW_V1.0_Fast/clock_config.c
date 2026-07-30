/*************************************************************
Project     : Micromouse
File        : clock_config.c
Description : Board dependent system clock configuration
Date        : 15.04.2026
*************************************************************/

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "clock_config.h"
#include "config.h"

static HAL_StatusTypeDef Clock_EnableOverDrive(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();

    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

static HAL_StatusTypeDef Clock_Config_180MHz_HSI(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /*
     * HSI = 16 MHz
     * PLLM = 16   -> 1 MHz PLL input
     * PLLN = 360  -> 360 MHz VCO
     * PLLP = 2    -> 180 MHz SYSCLK
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 360;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    RCC_OscInitStruct.PLL.PLLR = 2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        return HAL_ERROR;
    }

    if (Clock_EnableOverDrive() != HAL_OK) {
        return HAL_ERROR;
    }

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_HCLK   |
        RCC_CLOCKTYPE_PCLK1  |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        return HAL_ERROR;
    }

    SystemCoreClockUpdate();
    return HAL_OK;
}

static HAL_StatusTypeDef Clock_Config_84MHz_HSI(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    /*
     * HSI = 16 MHz
     * PLLM = 16   -> 1 MHz PLL input
     * PLLN = 168  -> 168 MHz VCO
     * PLLP = 2    -> 84 MHz SYSCLK
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    RCC_OscInitStruct.PLL.PLLR = 2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        return HAL_ERROR;
    }

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_HCLK   |
        RCC_CLOCKTYPE_PCLK1  |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2; /* 42 MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1; /* 84 MHz */

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        return HAL_ERROR;
    }

    SystemCoreClockUpdate();
    return HAL_OK;
}

HAL_StatusTypeDef Clock_Config_BoardDefault(void)
{
#if TARGET_BOARD == BOARD_HW_V1
    return Clock_Config_84MHz_HSI();
#elif TARGET_BOARD == BOARD_HW_V2
    return Clock_Config_180MHz_HSI();
#else
    return HAL_ERROR;
#endif
}