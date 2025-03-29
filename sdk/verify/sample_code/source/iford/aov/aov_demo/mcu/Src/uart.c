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

#include "main.h"
#include "user_init.h"
#include "uart.h"

struct UARTOpStruct
{
    uint8_t *TxBuf;
    uint8_t  TxLen;
    uint8_t  TxOpc;
    uint8_t *RxBuf;
    uint8_t  RxLen;
    uint8_t  RxOpc;
};
struct UARTOpStruct UARTxOp;

void UART5_IRQHandler(void)
{
    uint32_t u32Tmp;
    // FL_UART_DisableIT_TXBuffEmpty(UART0);

    if ((ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART5)) && (SET == FL_UART_IsActiveFlag_RXBuffFull(UART5)))
    {
        u32Tmp = FL_UART_ReadRXBuff(UART5);

        FL_UART_WriteTXBuff(UART5, u32Tmp);
    }

    if ((ENABLE == FL_UART_IsEnabledIT_TXShiftBuffEmpty(UART5))
        && (SET == FL_UART_IsActiveFlag_TXShiftBuffEmpty(UART5)))
    {
        if (UARTxOp.TxOpc < UARTxOp.TxLen)
        {
            FL_UART_WriteTXBuff(UART5, UARTxOp.TxBuf[UARTxOp.TxOpc]);
            UARTxOp.TxOpc++;
        }
        FL_UART_ClearFlag_TXShiftBuffEmpty(UART5);
    }
}

void UART0_IRQHandler(void)
{
    uint32_t u32Tmp;
    // FL_UART_DisableIT_TXBuffEmpty(UART0);

    if ((ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART0)) && (SET == FL_UART_IsActiveFlag_RXBuffFull(UART0)))
    {
        u32Tmp = FL_UART_ReadRXBuff(UART0);
        FL_UART_WriteTXBuff(UART0, u32Tmp);
    }

    if ((ENABLE == FL_UART_IsEnabledIT_TXShiftBuffEmpty(UART0))
        && (SET == FL_UART_IsActiveFlag_TXShiftBuffEmpty(UART0)))
    {
        if (UARTxOp.TxOpc < UARTxOp.TxLen)
        {
            FL_UART_WriteTXBuff(UART0, UARTxOp.TxBuf[UARTxOp.TxOpc]);
            UARTxOp.TxOpc++;
        }
        FL_UART_ClearFlag_TXShiftBuffEmpty(UART0);
    }
}

void Uart0_Init(void)
{
    FL_GPIO_InitTypeDef stGpioType = {0};
    FL_UART_InitTypeDef stUartType = {0};

    // PA13:UART0-RX   PA14:UART0-TX
    stGpioType.pin        = FL_GPIO_PIN_13 | FL_GPIO_PIN_14;
    stGpioType.mode       = FL_GPIO_MODE_DIGITAL;
    stGpioType.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    stGpioType.pull       = DISABLE;
    stGpioType.remapPin   = DISABLE;
    FL_GPIO_Init(GPIOA, &stGpioType);

    stUartType.clockSrc          = FL_RCC_UART0_CLK_SOURCE_APB1CLK;
    stUartType.baudRate          = 115200;
    stUartType.dataWidth         = FL_UART_DATA_WIDTH_8B;
    stUartType.stopBits          = FL_UART_STOP_BIT_WIDTH_1B;
    stUartType.parity            = FL_UART_PARITY_EVEN;
    stUartType.transferDirection = FL_UART_DIRECTION_TX_RX;

    FL_UART_Init(UART0, &stUartType);
    FL_UART_EnableIT_RXBuffFull(UART0);

    NVIC_DisableIRQ(UART0_IRQn);
    NVIC_SetPriority(UART0_IRQn, 2);
    NVIC_EnableIRQ(UART0_IRQn);
}

void Uart5_Init(void)
{
    FL_GPIO_InitTypeDef stGpioType = {0};
    FL_UART_InitTypeDef stUartType = {0};

    // PD0:UART5-RX   PD1:UART5-TX
    stGpioType.pin        = FL_GPIO_PIN_0 | FL_GPIO_PIN_1;
    stGpioType.mode       = FL_GPIO_MODE_DIGITAL;
    stGpioType.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    stGpioType.pull       = DISABLE;
    stGpioType.remapPin   = DISABLE;
    FL_GPIO_Init(GPIOD, &stGpioType);

    stUartType.clockSrc          = FL_RCC_UART0_CLK_SOURCE_APB1CLK;
    stUartType.baudRate          = 115200;
    stUartType.dataWidth         = FL_UART_DATA_WIDTH_8B;
    stUartType.stopBits          = FL_UART_STOP_BIT_WIDTH_1B;
    stUartType.parity            = FL_UART_PARITY_NONE;
    stUartType.transferDirection = FL_UART_DIRECTION_TX_RX;

    FL_UART_Init(UART5, &stUartType);
    FL_UART_EnableIT_RXBuffFull(UART5);

    NVIC_DisableIRQ(UART5_IRQn);
    NVIC_SetPriority(UART5_IRQn, 2);
    NVIC_EnableIRQ(UART5_IRQn);
}

void Test_Uart0(void)
{
    uint8_t u8TestTxData[13] = "TestUart-TX\r\n";
    Uart0_Init();

    UARTxOp.TxBuf = u8TestTxData;
    UARTxOp.TxLen = 13;
    UARTxOp.TxOpc = 0 + 1;

    FL_UART_ClearFlag_TXShiftBuffEmpty(UART0);
    FL_UART_EnableIT_TXShiftBuffEmpty(UART0);
    FL_UART_WriteTXBuff(UART0, UARTxOp.TxBuf[0]);
    DelayMs(50);
    FL_UART_DisableIT_TXShiftBuffEmpty(UART0);
    FL_UART_EnableIT_RXBuffFull(UART0);
}

void Test1_Uart0(void)
{
    uint8_t u8TestTxData[22] = "TestUart-TX123456789\r\n";
    Uart0_Init();

    UARTxOp.TxBuf = u8TestTxData;
    UARTxOp.TxLen = 22;
    UARTxOp.TxOpc = 0 + 1;

    FL_UART_ClearFlag_TXShiftBuffEmpty(UART0);
    FL_UART_EnableIT_TXShiftBuffEmpty(UART0);
    FL_UART_WriteTXBuff(UART0, UARTxOp.TxBuf[0]);
    DelayMs(50);
    FL_UART_DisableIT_TXShiftBuffEmpty(UART0);
    FL_UART_EnableIT_RXBuffFull(UART0);
}
