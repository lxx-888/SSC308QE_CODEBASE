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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <pthread.h>

#include "cam_os_wrapper.h"
#include "st_common_ai_glasses_uart.h"
#include "st_common_ai_glasses_protrcol.h"

#define BAUDRATE B115200
#define DEVICE   "/dev/ttyS2"

static MI_S32   u32UartFd = -1;
static MI_BOOL  bModeExit = FALSE;
pthread_t       pUartReadWorkThread_handle;
CamOsSpinlock_t g_uartWriteSpinlock;

static MI_S32 (*protcolTaskHandlerFun)(ST_Protrcol_Task_t* pstProtTask) = NULL;

static MI_BOOL bDmaSupport        = FALSE;
static MI_BOOL bReadThreadCanExit = FALSE;

#define SS_UART_MSG_BUFFER_SZIE 64
typedef struct ST_Glasses_Uart_Msg_Buffer_s
{
    char buffer[SS_UART_MSG_BUFFER_SZIE];
    int  bufsize;
} ST_Glasses_Uart_Msg_Buffer_t;

MI_S32 _ST_Common_AiGlasses_Uart_Printf_Hex(char* printBuf, MI_S32 len)
{
    int cnt = 0;

    if (!printBuf)
    {
        return -1;
    }

    printf("hex:\n");
    for (cnt = 0; cnt <= len; cnt++)
    {
        printf("%x ", printBuf[cnt]);
    }
    printf("\nchar:\n");
    for (cnt = 0; cnt <= len; cnt++)
    {
        printf("%c ", printBuf[cnt]);
    }
    printf("\n");
    return MI_SUCCESS;
}

MI_S32 _ST_Common_AiGlasses_Reset_Uart_Msg_Buffer(ST_Glasses_Uart_Msg_Buffer_t* pstGlassUartBuffer, BOOL bEnableDebugLog)
{
    if (!pstGlassUartBuffer)
    {
        printf("pstGlassUartBuffer is NULL\n");
        return -1;
    }
    if (bEnableDebugLog)
    {
        // for debug
        printf("uart msg buffer:\n");
        _ST_Common_AiGlasses_Uart_Printf_Hex(pstGlassUartBuffer->buffer, pstGlassUartBuffer->bufsize);
    }
    memset(pstGlassUartBuffer->buffer, 0x0, pstGlassUartBuffer->bufsize);
    pstGlassUartBuffer->bufsize = 0;
    return MI_SUCCESS;
}

void* ST_Common_AiGlasses_Uart_Read_Select(void* args)
{
    ST_Glasses_Uart_Msg_Buffer_t stGlassUartBuffer = {0};
    char*                        buf               = NULL;
    MI_U32                       u32BufSize        = 0;

    fd_set         readfds;
    struct timeval timeout;
    if (u32UartFd < 0)
    {
        printf("close u32UartFd:%d error\n", u32UartFd);
        return (void*)-1;
    }
    if (bDmaSupport)
    {
        u32BufSize = sizeof(char) * SS_UART_MSG_BUFFER_SZIE * 16;
    }
    else
    {
        u32BufSize = sizeof(char) * SS_UART_MSG_BUFFER_SZIE / 2;
    }
    buf = malloc(u32BufSize);
    memset(buf, 0x0, u32BufSize);

    while (!bReadThreadCanExit)
    {
        MI_U32 len = 0;

        FD_ZERO(&readfds);
        FD_SET(u32UartFd, &readfds);

        // set timeout
        timeout.tv_sec  = 1;
        timeout.tv_usec = 0;

        int ret = select(u32UartFd + 1, &readfds, NULL, NULL, &timeout);
        if (ret < 0)
        {
            printf("select failed, break\n");
            break;
        }
        else if (ret == 0)
        {
            continue;
        }

        len = read(u32UartFd, buf, u32BufSize);
        if (len > 0)
        {
            if (protcolTaskHandlerFun == NULL)
            {
                printf("please assign protcolTaskHandlerFun when uart init!!!\n");
                return (void*)-1;
            }
            if (bDmaSupport)
            {
                ST_Common_AiGlasses_Prot_Deal_Uart_Buffer(buf, len, TRUE, protcolTaskHandlerFun);
            }
            else
            {
                // maybe ringbuf more useful?
                if ((stGlassUartBuffer.bufsize + len) < SS_UART_MSG_BUFFER_SZIE)
                {
                    memcpy(&stGlassUartBuffer.buffer[stGlassUartBuffer.bufsize], buf, len);
                    stGlassUartBuffer.bufsize += len;
                }
                else
                {
                    printf("you need check mcu msg what happend!!!\n");
                    _ST_Common_AiGlasses_Reset_Uart_Msg_Buffer(&stGlassUartBuffer, TRUE);
                    printf("uart read buf:\n");
                    _ST_Common_AiGlasses_Uart_Printf_Hex(buf, len);
                    goto BUF_MEMSET;
                }
                if (stGlassUartBuffer.bufsize >= SS_TASK_CMD_PROTRCOL)
                {
                    if (ST_Common_AiGlasses_Prot_Deal_Uart_Buffer(stGlassUartBuffer.buffer, stGlassUartBuffer.bufsize, FALSE,
                                                         protcolTaskHandlerFun)
                        == MI_SUCCESS)
                    {
                        _ST_Common_AiGlasses_Reset_Uart_Msg_Buffer(&stGlassUartBuffer, FALSE);
                        goto BUF_MEMSET;
                    }
                }
                if (stGlassUartBuffer.bufsize >= SS_UART_MSG_BUFFER_SZIE - 1)
                {
                    printf("buffer full, but not find useful msg!!!\n");
                    _ST_Common_AiGlasses_Reset_Uart_Msg_Buffer(&stGlassUartBuffer, TRUE);
                }
            }
        BUF_MEMSET:
            memset(buf, 0x0, sizeof(*buf));
        }
    }
    free(buf);

    return 0;
}

/* MI_BOOL bEnableDma: if enable dma or not in fuart dts config */
MI_S32 ST_Common_AiGlasses_Uart_Init(void* protcolTaskHandler, MI_BOOL bEnableDma)
{
    struct termios options;

    if (protcolTaskHandler == NULL)
    {
        printf("protcolTaskHandler is NULL\n");
        return -1;
    }
    protcolTaskHandlerFun = protcolTaskHandler;
    bDmaSupport           = bEnableDma;

    u32UartFd = open(DEVICE, O_RDWR | O_NOCTTY | O_NDELAY);
    if (u32UartFd == -1)
    {
        printf("open failed\n");
        return -1;
    }
    CamOsSpinInit(&g_uartWriteSpinlock);

    tcgetattr(u32UartFd, &options);
    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);

    options.c_cflag |= (CLOCAL | CREAD); // set local model, enable recive
    options.c_cflag &= ~PARENB;          // no parity bits
    options.c_cflag &= ~CSTOPB;          // one stop bit
    options.c_cflag &= ~CSIZE;           // clear data bit config
    options.c_cflag |= CS8;              // 8 data bit

    /* stty -ixon -ixoff -icanon -echo -echoe -echok -echonl -isig -brkint -inpck -istrip -inlcr -ocrnl -onlcr -opost
     * -tabs */
    // close echo fun
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    // disable Tx terminal conversion
    options.c_oflag &= ~(OCRNL | ONLCR | OPOST | XTABS);
    // disable Rx terminal conversion
    options.c_iflag &= ~(ICRNL | INLCR | IXOFF | IXON | BRKINT | INPCK | ISTRIP); // IGNCR (we not need ignore CR)
    printf("close uart ECHO!\n");

    tcsetattr(u32UartFd, TCSANOW, &options);

    pthread_create(&pUartReadWorkThread_handle, NULL, ST_Common_AiGlasses_Uart_Read_Select, NULL);

    return MI_SUCCESS;
}

MI_S32 ST_Common_AiGlasses_Uart_DeInit(void)
{
    if (u32UartFd < 0)
    {
        printf("close u32UartFd:%d error\n", u32UartFd);
        return -1;
    }
    pthread_join(pUartReadWorkThread_handle, NULL);
    printf("uart thread exit\n");
    bModeExit = TRUE;
    CamOsSpinDeinit(&g_uartWriteSpinlock);
    printf("uart read close\n");
    return close(u32UartFd);
}

MI_S32 ST_Common_AiGlasses_Uart_Write(char* buf, int len)
{
    MI_S32 ret;
    if (u32UartFd < 0)
    {
        printf("close u32UartFd:%d error\n", u32UartFd);
        return -1;
    }
    if (bModeExit)
    {
        return MI_SUCCESS;
    }
    CamOsSpinLock(&g_uartWriteSpinlock);
    ret = write(u32UartFd, buf, len);
    CamOsSpinUnlock(&g_uartWriteSpinlock);
    return ret;
}

MI_S32 ST_Common_AiGlasses_Uart_ReadThread_CanExit(void)
{
    bReadThreadCanExit = TRUE;
    return MI_SUCCESS;
}
