/*************************************************************
Project		: Micromouse V1.0
File		: clock_config.c
Description	: Configurate systemClock (HSI) -> 180 Mhz // Implementation
Date		: 09.09.2024
*************************************************************/

#include "stm32f4xx.h" 
#include "clock_config.h"

/*----------------------------------------------------------------------------
 * SystemCoreClockConfigure: configure SystemCoreClock using HSI
                             (HSE is not populated on Nucleo board)
 *----------------------------------------------------------------------------*/
void SystemCoreClockConfigure(void) {

  RCC->CR |= ((uint32_t)RCC_CR_HSION);                     /* Enable HSI */
  while ((RCC->CR & RCC_CR_HSIRDY) == 0);                  /* Wait for HSI Ready */

  RCC->CFGR = RCC_CFGR_SW_HSI;                             /* HSI is system clock */
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);  /* Wait for HSI used as system clock */

  FLASH->ACR  = (FLASH_ACR_PRFTEN     |                    /* Enable Prefetch Buffer */
                 FLASH_ACR_ICEN       |                    /* Instruction cache enable */
                 FLASH_ACR_DCEN       |                    /* Data cache enable */
                 FLASH_ACR_LATENCY_5WS );                  /* Flash 5 wait state */

  RCC->CFGR |= (RCC_CFGR_HPRE_DIV1  |                      /* HCLK = SYSCLK */
                RCC_CFGR_PPRE1_DIV2 |                      /* APB1 = HCLK/2 */
                RCC_CFGR_PPRE2_DIV1  );                    /* APB2 = HCLK/1 */

  RCC->CR &= ~RCC_CR_PLLON;                                /* Disable PLL */

  /* PLL configuration:  VCO = HSI/M * N,  Sysclk = VCO/P */
  RCC->PLLCFGR = ( 16ul                   |                /* PLL_M =  16 */
                 (360ul <<  6)            |                /* PLL_N = 200 */
                 (  0ul << 16)            |                /* PLL_P =   2 */
                 (RCC_PLLCFGR_PLLSRC_HSI) |                /* PLL_SRC = HSI */
                 (  7ul << 24)            |                /* PLL_Q =   7 */
                 (  2ul << 28)             );              /* PLL_R =   2 */

  RCC->CR |= RCC_CR_PLLON;                                 /* Enable PLL */
  while((RCC->CR & RCC_CR_PLLRDY) == 0) __NOP();           /* Wait till PLL is ready */

  RCC->CFGR &= ~RCC_CFGR_SW;                               /* Select PLL as system clock source */
  RCC->CFGR |=  RCC_CFGR_SW_PLL;
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  /* Wait till PLL is system clock src */
  
  SystemCoreClockUpdate();
}


// externer CLOCK
void SystemCoreClockConfigure_HSE(void) {
	
	__HAL_RCC_GPIOH_CLK_ENABLE();
	  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	*/
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 360;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	RCC_OscInitStruct.PLL.PLLR = 2;

	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;


	/** Enables the Clock Security System
	*/
	HAL_RCC_EnableCSS();


	SystemCoreClockUpdate();
}

#include "stm32f4xx_hal.h"

#include "stm32f4xx_hal.h"

// Überschreibt den falschen HSE-Wert aus hal_conf
#undef HSE_VALUE
#define HSE_VALUE 8000000   // 8 MHz externer Quarz

#include "stm32f4xx_hal.h"

#include "stm32f4xx_hal.h"

//void SystemClock_Config(void)
//{
//    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
//    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

//    /** 1. Oszillator-Konfiguration **/
//    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
//    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
//    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
//    RCC_OscInitStruct.PLL.PLLM = 8;       // HSE 8 MHz / 8 = 1 MHz
//    RCC_OscInitStruct.PLL.PLLN = 100;     // 1 MHz * 100 = 100 MHz VCO
//    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // 100 / 2 = 50 MHz SYSCLK
//    RCC_OscInitStruct.PLL.PLLQ = 7;       // USB/SDIO (optional)
//    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//    {
//        
//    }

//    /** 2. Taktverteilung auf Busse **/
//    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
//                                  RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
//    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
//    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;  // HCLK = 50 MHz
//    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;    // APB1 = 50 MHz max 45 MHz OK ggf DIV2
//    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;    // APB2 = 50 MHz
//    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
//    {
//        
//    }
//}
void SystemCoreClockConfigure_HSE_120Mhz(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Set voltage scaling to Scale1 (required for 180 MHz) */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSE Oscillator and configure PLL */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 240;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;  // SYSCLK = 180 MHz
    RCC_OscInitStruct.PLL.PLLQ = 7;              // USB OTG FS, SDIO, RNG = 48 MHz
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        //Error_Handler();
    }

    /* Activate the OverDrive mode to reach 180 MHz */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        //Error_Handler();
    }

    /* Select PLL as system clock source and configure buses */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   // 180 MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;    // 45 MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;    // 90 MHz

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        //Error_Handler();
    }

    /* Update SystemCoreClock variable */
    SystemCoreClockUpdate();

    /* Enable Clock Security System */
    HAL_RCC_EnableCSS();
}

void SystemCoreClockConfigure_HSI_120Mhz(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Enable power control and voltage scaling */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Configure HSI and PLL */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;

    // HSI = 16 MHz ? PLLM, PLLN, PLLP berechnen:
    // fSYS = (16 MHz / PLLM) * PLLN / PLLP = 120 MHz
    // Beispiel: 16/8 * 240 / 4 = 120 MHz
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 240;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 5;
    RCC_OscInitStruct.PLL.PLLR = 2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        // Error_Handler();
    }

    /** Over-Drive ist bei 120 MHz NICHT nötig */
    // HAL_PWREx_EnableOverDrive();

    /** Configure CPU, AHB and APB clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
    {
        // Error_Handler();
    }
}


