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

#include "user_init.h"

void ClockInit(uint32_t clock)
{
    switch (clock)
    {
        case FL_RCC_RCHF_FREQUENCY_8MHZ:
            FL_RCC_RCHF_WriteTrimValue(RCHF8M_TRIM);
            break;

        case FL_RCC_RCHF_FREQUENCY_16MHZ:
            FL_RCC_RCHF_WriteTrimValue(RCHF16M_TRIM);
            break;

        case FL_RCC_RCHF_FREQUENCY_24MHZ:
            FL_RCC_RCHF_WriteTrimValue(RCHF24M_TRIM);
            break;

        default:
            FL_RCC_RCHF_WriteTrimValue(RCHF8M_TRIM);
            break;
    }

    FL_RCC_RCHF_SetFrequency(clock);
    FL_RCC_SetSystemClockSource(FL_RCC_SYSTEM_CLK_SOURCE_RCHF);
}

static void SystickInit(void)
{
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

void FoutInit(void)
{
    FL_GPIO_InitTypeDef stGpioType = {0};

    stGpioType.pin        = FL_GPIO_PIN_11;
    stGpioType.mode       = FL_GPIO_MODE_DIGITAL;
    stGpioType.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    stGpioType.pull       = DISABLE;
    FL_GPIO_Init(GPIOD, &stGpioType);

    FL_GPIO_SetFOUT0(GPIO, FL_GPIO_FOUT0_SELECT_AHBCLK_DIV64);
}

//标准库需要的支持函数
struct __FILE
{
    int handle;
};
FILE __stdout;

//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
    x = x;
}

//重定义fputc函数
int fputc(int ch, FILE *f)
{
    FL_UART_WriteTXBuff(UART0, (uint8_t)ch);
    while (FL_UART_IsActiveFlag_TXBuffEmpty(UART0) != SET)
        ;
    return ch;
}

#ifndef MFANG
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
    int handle;
};
FILE __stdout;

//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
    x = x;
}

//重定义fputc函数
int fputc(int ch, FILE *f)
{
    FL_UART_WriteTXBuff(UART0, (uint8_t)ch);
    while (FL_UART_IsActiveFlag_TXBuffEmpty(UART0) != SET)
        ;
    return ch;
}

void DebugUartInit(void)
{
    FL_GPIO_InitTypeDef stGpioType = {0};
    FL_UART_InitTypeDef stUartType = {0};

    // PA13:UART0-RX   PA14:UART0-TX
    stGpioType.pin        = FL_GPIO_PIN_13 | FL_GPIO_PIN_14;
    stGpioType.mode       = FL_GPIO_MODE_DIGITAL;
    stGpioType.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    stGpioType.pull       = ENABLE;
    stGpioType.remapPin   = DISABLE;
    FL_GPIO_Init(GPIOA, &stGpioType);

    stUartType.clockSrc = FL_RCC_UART0_CLK_SOURCE_APB1CLK;

    stUartType.baudRate          = 115200;                    //波特率
    stUartType.dataWidth         = FL_UART_DATA_WIDTH_8B;     //数据位数
    stUartType.stopBits          = FL_UART_STOP_BIT_WIDTH_1B; //停止位
    stUartType.parity            = FL_UART_PARITY_EVEN;       //奇偶校验
    stUartType.transferDirection = FL_UART_DIRECTION_TX_RX;   //接收-发送使能
    FL_UART_Init(UART0, &stUartType);
}

void LedInit(void)
{
    uint8_t u8Count = 5;

    FL_GPIO_InitTypeDef stGpioType = {0};

    FL_GPIO_ResetOutputPin(LED0_GPIO, LED0_PIN);

    stGpioType.pin        = LED0_PIN;
    stGpioType.mode       = FL_GPIO_MODE_OUTPUT;
    stGpioType.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    stGpioType.pull       = DISABLE;
    FL_GPIO_Init(LED0_GPIO, &stGpioType);

    while (u8Count--)
    {
        LED0_ON();
        DelayMs(100);
        LED0_OFF();
        DelayMs(100);
    }
}

#endif

void UserInit(void)
{
    SystickInit();

#ifndef MFANG
    LedInit();
    DebugUartInit();
#endif
}

void DelayUs(uint32_t count)
{
    count = (uint64_t)FL_RCC_GetSystemClockFreq() * count / 1000000;
    count = count > 16777216 ? 16777216 : count;

    SysTick->LOAD = count - 1;
    SysTick->VAL  = 0;
    while (!((SysTick->CTRL >> 16) & 0x1))
        ;
}

void DelayMs(uint32_t count)
{
    while (count--)
    {
        DelayUs(1000);
    }
}
