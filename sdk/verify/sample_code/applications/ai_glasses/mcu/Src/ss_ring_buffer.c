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


#include <stdio.h>
#include "ss_ring_buffer.h"

/* FIXME: if volatile struct with mcu v6 compiler, param's value will not update immediately */
static uart_ring_buffer_t ring_buffer;

void SS_Ring_Buffer_Init(void)
{
    ring_buffer.head        = 0;
    ring_buffer.tail        = 0;
    ring_buffer.uart_length = 0;
    memset(ring_buffer.recv_buf, 0, sizeof(ring_buffer.recv_buf));
}

/* 读队列 */
BOOL SS_Ring_Buffer_Read(uint8_t *data)
{
    if (ring_buffer.uart_length == 0)
    {
        return FALSE;
    }
    *data                                  = ring_buffer.recv_buf[ring_buffer.head];
    ring_buffer.recv_buf[ring_buffer.head] = 0;
    ring_buffer.head++; /* 防止数组溢出 */
    if (ring_buffer.head - UART_DATA_MAX >= 0)
    {
        ring_buffer.head = ring_buffer.head - UART_DATA_MAX;
    }
    ring_buffer.uart_length--;
    return TRUE;
}

/* 往队列里面写 */
BOOL SS_Ring_Buffer_Write(uint8_t data)
{
    if (ring_buffer.uart_length >= UART_DATA_MAX)
    {
        return FALSE;
    }
    ring_buffer.recv_buf[ring_buffer.tail] = data;

    ring_buffer.tail++; /* 防止数组溢出 */
    if (ring_buffer.tail - UART_DATA_MAX >= 0)
    {
        ring_buffer.tail = ring_buffer.tail - UART_DATA_MAX;
    }
    ring_buffer.uart_length++;
    return TRUE;
}

/* 获取当前队列有效数据（未处理数据）长度 */
uint8_t SS_Ring_Buffer_Get_Uart_Length(void)
{
    return ring_buffer.uart_length;
}

uint8_t handler_buf[SS_TASK_CMD_PROTRCOL] = {0};

int8_t recv_flag = 0;
void   SS_Deal_Ring_Buffer_Data(void)
{
    uint8_t  check_sum = 0;
    uint32_t data      = 0;

    while (SS_Ring_Buffer_Read(&data)) /* 串口数据接收后处理 */
    {
        DEBUG_INFO("data:%x\r\n", data);
        switch (recv_flag)
        {
            case 0:
            case 1:
            case 2:
            case 3:
                if (data == 0x5A)
                {
                    handler_buf[recv_flag] = data;
                    recv_flag++;
                }
                else
                {
                    recv_flag = 0;
                }
                break;
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
                handler_buf[recv_flag] = data;
                recv_flag++;
                break;

            case 10:
            case 11:
            case 12:
            case 13:
                if (data == 0xA5)
                {
                    handler_buf[recv_flag] = data;
                    if (recv_flag < SS_TASK_CMD_PROTRCOL - 1)
                    {
                        recv_flag++;
                        break;
                    }
                    else
                    {
                        DEBUG_INFO("reveiced data!!!\r\n");
                        SS_Factory_Uart_Data_Handle(handler_buf);
                    }
                }
                memset(handler_buf, 0, sizeof(handler_buf));
                recv_flag = 0;
                break;
        }
#if DEBUG_LOG_LEVEL >= DEBUG_LOG_INFO
        for (int i = 0; i < SS_TASK_CMD_PROTRCOL; i++)
        {
            if (i == 0)
                printf("\r\nhandler_buf:\r\n");

            printf("%x ", handler_buf[i]);

            if (i == SS_TASK_CMD_PROTRCOL - 1)
                printf("\r\n");
        }
#endif
    }
}
