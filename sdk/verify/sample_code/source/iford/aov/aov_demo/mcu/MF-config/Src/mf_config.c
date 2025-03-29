/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

/* Includes ------------------------------------------------------------------*/
#include "mf_config.h"

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  The application entry point.
 * @retval int
 */
void MF_Clock_Init(void)
{
    /* MCU Configuration--------------------------------------------------------*/
    // FL_RCC_EnableGroup1BusClock(FL_RCC_GROUP1_BUSCLK_RTC);
    // FL_RTC_WriteAdjustValue(RTC, 0);
    // FL_RCC_DisableGroup1BusClock(FL_RCC_GROUP1_BUSCLK_RTC);
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

    /* System interrupt init*/

    /* Initialize all configured peripherals */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void MF_SystemClock_Config(void) {}

void MF_BSTIM32_Init(void)
{
    /*IO CONFIG*/
    FL_BSTIM32_InitTypeDef defaultInitStruct;

    defaultInitStruct.prescaler       = 8000;
    defaultInitStruct.autoReload      = 1000;
    defaultInitStruct.autoReloadState = DISABLE;
    defaultInitStruct.clockSource     = FL_RCC_BSTIM32_CLK_SOURCE_APB1CLK;

    FL_BSTIM32_Init(BSTIM32, &defaultInitStruct);

    FL_BSTIM32_DisableIT_Update(BSTIM32);

    if (FL_BSTIM32_IsEnabled(BSTIM32))
    {
        FL_BSTIM32_Disable(BSTIM32);
    }
}

/**
 * @brief  NVIC Initialization function
 * @param  void
 * @retval None
 */
void MF_NVIC_Init(void)
{
    NVIC_ConfigTypeDef InterruptConfigStruct;

    InterruptConfigStruct.preemptPriority = 0x00;
    NVIC_Init(&InterruptConfigStruct, BSTIM_IRQn);
}

void MF_Config_Init(void)
{
    MF_BSTIM32_Init();

    MF_NVIC_Init();
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */

    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
