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

#ifndef __SS_RING_BUFFER_H
#define __SS_RING_BUFFER_H

#include <stdint.h>
#include "ss_datatype.h"

#define UART_DATA_MAX 50

typedef struct
{
    uint8_t tail;                    /* 队列尾，入队时需要自加 */
    uint8_t head;                    /* 队列头，出队时需要自减 */
    uint8_t uart_length;             /* 保存的是当前队列有效数据的长度 */
    uint8_t recv_buf[UART_DATA_MAX]; /* 数据缓冲区 */
} uart_ring_buffer_t;

void    SS_Ring_Buffer_Init(void);
BOOL    SS_Ring_Buffer_Read(uint8_t *data);
BOOL    SS_Ring_Buffer_Write(uint8_t data);
uint8_t SS_Ring_Buffer_Get_Uart_Length(void);
#endif
