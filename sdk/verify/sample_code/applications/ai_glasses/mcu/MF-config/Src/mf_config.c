/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : mf_config.c
  * @brief          : MCU FUNCTION CONFIG
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 FMSH.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by FMSH under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

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
   //FL_RCC_EnableGroup1BusClock(FL_RCC_GROUP1_BUSCLK_RTC);
   //FL_RTC_WriteAdjustValue(RTC, 0);
   //FL_RCC_DisableGroup1BusClock(FL_RCC_GROUP1_BUSCLK_RTC);
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  /* System interrupt init*/

  /* Initialize all configured peripherals */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void MF_SystemClock_Config(void)
{

}

/**
  * @brief  DMA_Channel1 Initialization function
  * @param  void
  * @retval None
  */
void MF_DMA_Channel1_Init(void)
{

    /*IO CONFIG*/
    FL_DMA_InitTypeDef    defaultInitStruct;

    defaultInitStruct.periphAddress = FL_DMA_PERIPHERAL_FUNCTION1;
    defaultInitStruct.direction = FL_DMA_DIR_PERIPHERAL_TO_RAM;
    defaultInitStruct.memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE;
    defaultInitStruct.flashAddressIncMode = FL_DMA_CH7_FLASH_INC_MODE_INCREASE;
    defaultInitStruct.dataSize = FL_DMA_BANDWIDTH_8B;
    defaultInitStruct.priority = FL_DMA_PRIORITY_HIGH;
    defaultInitStruct.circMode = DISABLE;

    FL_DMA_Init(DMA,&defaultInitStruct,FL_DMA_CHANNEL_1 );
}


void MF_BSTIM32_Init(void)
{

    /*IO CONFIG*/
    FL_BSTIM32_InitTypeDef    defaultInitStruct;

    defaultInitStruct.prescaler = 8000;
    defaultInitStruct.autoReload = 50;                         // junqiang.bi 50ms timer
    defaultInitStruct.autoReloadState = DISABLE;
    defaultInitStruct.clockSource = FL_RCC_BSTIM32_CLK_SOURCE_APB1CLK;

    FL_BSTIM32_Init(BSTIM32,&defaultInitStruct );


    FL_BSTIM32_DisableIT_Update(BSTIM32);

    if(FL_BSTIM32_IsEnabled(BSTIM32))
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

    NVIC_ConfigTypeDef    InterruptConfigStruct;

    InterruptConfigStruct.preemptPriority = 0x00;
    NVIC_Init(&InterruptConfigStruct,BSTIM_IRQn );


}

void MF_Config_Init(void)
{
    /*     MF_DMA_Channel1_Init(); //junqiang.bi dma uart init */
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

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
