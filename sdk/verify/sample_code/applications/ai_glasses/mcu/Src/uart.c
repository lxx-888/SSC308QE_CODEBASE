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
#include "ss_datatype.h"

struct UARTOpStruct
{
    uint8_t *     TxBuf; //发送数据指针
    uint8_t       TxLen; //待发送数据长度
    uint8_t       TxOpc; //已发送数据长度
    uint8_t *     RxBuf; //接收数据指针
    uint8_t       RxLen; //待接收数据长度
    uint8_t       RxOpc; //已接收数据长度
    volatile BOOL bWait; // remain msg to send
};
struct UARTOpStruct UARTxOp;

#if CONFIG_CMD_UART == UART5_DEF
void UART5_IRQHandler(void)
{
    uint8_t tmp;
    // FL_UART_DisableIT_TXBuffEmpty(UART0);
    //接收中断处理
    if ((ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART5)) && (SET == FL_UART_IsActiveFlag_RXBuffFull(UART5)))
    {
        //中断转发接收到的数据
        tmp = FL_UART_ReadRXBuff(UART5); //接收中断标志可通过读取rxreg寄存器清除

        // junqiang.bi write rx data to ring_buffer
        // FL_UART_WriteTXBuff(UART5, tmp);
        SS_Ring_Buffer_Write(tmp);
    }

    //发送中断处理
    if ((ENABLE == FL_UART_IsEnabledIT_TXShiftBuffEmpty(UART5))
        && (SET == FL_UART_IsActiveFlag_TXShiftBuffEmpty(UART5)))
    {
        //发送中断标志可通过写txreg寄存器清除或txif写1清除
        //发送指定长度的数据
        if (UARTxOp.TxOpc < UARTxOp.TxLen)
        {
            FL_UART_WriteTXBuff(UART5, UARTxOp.TxBuf[UARTxOp.TxOpc]); //发送一个数据
            DEBUG_INFO("send uart5 data[%d]:%x\r\n", UARTxOp.TxOpc, UARTxOp.TxBuf[UARTxOp.TxOpc]);
            UARTxOp.TxOpc++;
        }
        FL_UART_ClearFlag_TXShiftBuffEmpty(UART5); //清除发送中断标志
    }
}

void Uart5_Init(void)
{
    FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    FL_UART_InitTypeDef UART_InitStruct = {0};

    // PD0:UART5-RX   PD1:UART5-TX
    GPIO_InitStruct.pin        = FL_GPIO_PIN_0 | FL_GPIO_PIN_1;
    GPIO_InitStruct.mode       = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL; //推挽输出
    GPIO_InitStruct.pull       = DISABLE;
    GPIO_InitStruct.remapPin   = DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    UART_InitStruct.clockSrc          = FL_RCC_UART0_CLK_SOURCE_APB1CLK;
    UART_InitStruct.baudRate          = 115200;                    //波特率
    UART_InitStruct.dataWidth         = FL_UART_DATA_WIDTH_8B;     //数据位数
    UART_InitStruct.stopBits          = FL_UART_STOP_BIT_WIDTH_1B; //停止位
    UART_InitStruct.parity            = FL_UART_PARITY_NONE;       //奇偶校验
    UART_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;   //接收-发送使能

    FL_UART_Init(UART5, &UART_InitStruct);

    FL_UART_ClearFlag_RXBuffFull(UART5);
    FL_UART_EnableIT_RXBuffFull(UART5);

    /*NVIC中断配置*/
    NVIC_DisableIRQ(UART5_IRQn);
    NVIC_SetPriority(UART5_IRQn, 2); //中断优先级配置
    NVIC_EnableIRQ(UART5_IRQn);
}
#endif

#if CONFIG_CMD_UART == UART0_DEF
void UART0_IRQHandler(void)
{
    uint8_t tmp = 0;
    // FL_UART_DisableIT_TXBuffEmpty(UART0);
    //接收中断处理
    if ((ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART0)) && (SET == FL_UART_IsActiveFlag_RXBuffFull(UART0)))
    {
        //中断转发接收到的数据
        tmp = FL_UART_ReadRXBuff(UART0); //接收中断标志可通过读取rxreg寄存器清除
                                         // junqiang.bi write rx data to ring_buffer
                                         // FL_UART_WriteTXBuff(UART0, tmp);
        SS_Ring_Buffer_Write(tmp);
        // while(FL_UART_IsActiveFlag_RXBuffEmpty(UART0) != SET);
    }

    //发送中断处理
    if ((ENABLE == FL_UART_IsEnabledIT_TXShiftBuffEmpty(UART0))
        && (SET == FL_UART_IsActiveFlag_TXShiftBuffEmpty(UART0)))
    {
        //发送中断标志可通过写txreg寄存器清除或txif写1清除
        //发送指定长度的数据
        if (UARTxOp.TxOpc < UARTxOp.TxLen)
        {
            FL_UART_WriteTXBuff(UART0, UARTxOp.TxBuf[UARTxOp.TxOpc]); //发送一个数据
            DEBUG_INFO("send uart0 data[%d]:%x\r\n", UARTxOp.TxOpc, UARTxOp.TxBuf[UARTxOp.TxOpc]);
            UARTxOp.TxOpc++;
            if (UARTxOp.TxOpc == UARTxOp.TxLen - 1)
            {
                UARTxOp.bWait = FALSE; // msg had send out, so no need to wait
            }
        }
        FL_UART_ClearFlag_TXShiftBuffEmpty(UART0); //清除发送中断标志
    }
}

void Uart0_Init(void)
{
    FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    FL_UART_InitTypeDef UART_InitStruct = {0};

    // PA13:UART0-RX   PA14:UART0-TX
    GPIO_InitStruct.pin        = FL_GPIO_PIN_13;
    GPIO_InitStruct.mode       = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL; //推挽输出
    GPIO_InitStruct.pull       = ENABLE;
    GPIO_InitStruct.remapPin   = DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.pin = FL_GPIO_PIN_14;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    UART_InitStruct.clockSrc          = FL_RCC_UART0_CLK_SOURCE_APB1CLK;
    UART_InitStruct.baudRate          = 115200;                    //波特率
    UART_InitStruct.dataWidth         = FL_UART_DATA_WIDTH_8B;     //数据位数
    UART_InitStruct.stopBits          = FL_UART_STOP_BIT_WIDTH_1B; //停止位
    UART_InitStruct.parity            = FL_UART_PARITY_NONE;       //奇偶校验
    UART_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;   //接收-发送使能

    FL_UART_Init(UART0, &UART_InitStruct);

    FL_UART_ClearFlag_RXBuffFull(UART0);
    FL_UART_EnableIT_RXBuffFull(UART0);

    /*NVIC中断配置*/
    NVIC_DisableIRQ(UART0_IRQn);
    NVIC_SetPriority(UART0_IRQn, 2); //中断优先级配置
    NVIC_EnableIRQ(UART0_IRQn);

    UARTxOp.bWait = FALSE; // no need to wait when init done
}
#endif

void SS_Send_Cmd_to_Def_UART(uint8_t *TestTxData, int len)
{
    // waitting for the pre msg to be send out
    while (UARTxOp.bWait == TRUE)
    {
        DelayMs(1);
    }

    //中断发送数组
    UARTxOp.bWait = TRUE;
    UARTxOp.TxBuf = TestTxData;
    UARTxOp.TxLen = len;
    UARTxOp.TxOpc = 0 + 1;

    FL_UART_ClearFlag_TXShiftBuffEmpty(SS_UART_HANDLE);
    FL_UART_EnableIT_TXShiftBuffEmpty(SS_UART_HANDLE);
    FL_UART_WriteTXBuff(SS_UART_HANDLE, UARTxOp.TxBuf[0]);
    DelayMs(50); //软件延时
    FL_UART_DisableIT_TXShiftBuffEmpty(SS_UART_HANDLE);
    FL_UART_EnableIT_RXBuffFull(SS_UART_HANDLE);
}

// if use dma for uart Rx
#if 0
void Uart0DMA_Config(uint8_t *buffer,uint32_t length)
{
    FL_DMA_ConfigTypeDef DMA_ConfigStruct={0};


    DMA_ConfigStruct.memoryAddress = (uint32_t)buffer;
    DMA_ConfigStruct.transmissionCount = length-1;
    FL_DMA_StartTransmission(DMA, &DMA_ConfigStruct,FL_DMA_CHANNEL_1);
}

uint8_t DMARxData[SS_TASK_CMD_PROTRCOL];
void SS_Uart0_Rx_Dma_Init(void)
{
    //junqaing.bi init uart rx dma
    FL_DMA_Enable(DMA);
    Uart0DMA_Config(DMARxData, SS_TASK_CMD_PROTRCOL);
    FL_UART_EnableRX(UART0);

    FL_DMA_ClearFlag_TransferComplete(DMA, FL_DMA_CHANNEL_1);
}

void SS_Uart0_Rx_Dma_BufferFull_Cp_To_RingBuffer(void)
{
   if (FL_DMA_IsActiveFlag_TransferComplete(DMA, FL_DMA_CHANNEL_1))
   {
        for (int cnt = 0; cnt < SS_TASK_CMD_PROTRCOL; cnt++)
       {
            SS_Ring_Buffer_Write(DMARxData[cnt]);
            printf("data[%d]:%x\r\n", cnt, DMARxData[cnt]);
       }
        FL_DMA_ClearFlag_TransferComplete(DMA, FL_DMA_CHANNEL_1);
   }
}
#endif
