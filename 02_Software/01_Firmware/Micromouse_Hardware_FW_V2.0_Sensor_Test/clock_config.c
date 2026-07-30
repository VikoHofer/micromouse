/*************************************************************
Project		: Micromouse V2.0
File		: clock_config.h
Description	: Configurate systemClock (HSI) -> 180 Mhz
Date		: 11.03.2026
*************************************************************/

#include "stm32f4xx.h" 
#include "clock_config.h"

#include "clock_config.h"

static HAL_StatusTypeDef Clock_EnableOverDrive(void)
{
    /* Over-Drive required for 180 MHz on STM32F446 */
    __HAL_RCC_PWR_CLK_ENABLE();

    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef Clock_Config_Max_HSI(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Power control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Voltage scaling for high frequency operation */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Start from HSI and configure PLL:
       HSI = 16 MHz
       PLLM = 16   -> 1 MHz PLL input
       PLLN = 360  -> 360 MHz VCO
       PLLP = 2    -> 180 MHz SYSCLK
       PLLQ = 7    -> 48 MHz domain not exact, but acceptable if USB not needed
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

    /* Bus clocks:
       SYSCLK = 180 MHz
       HCLK   = 180 MHz
       PCLK1  = 45 MHz  (max)
       PCLK2  = 90 MHz  (max)
    */
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

HAL_StatusTypeDef Clock_Config_Max_HSE_8MHz_Bypass(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* On many NUCLEO boards HSE comes from ST-LINK MCO => BYPASS, 8 MHz */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
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