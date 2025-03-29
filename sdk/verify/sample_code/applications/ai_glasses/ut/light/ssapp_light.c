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
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include "light_misc_control_api.h"

static int    g_testExit         = 0;
unsigned char g_bOnlyLightSensor = 1;

static int _SSAPP_LIGHT_SensorLightInit()
{
    int fd = -1;

    fd = Dev_Light_Misc_Device_GetFd();
    if (fd < 0)
    {
        printf("get light device fd fail\n");
        return -1;
    }
    printf("get fd success, fd: %d\n", fd);

    if (Dev_Light_Misc_Device_Init(fd, 0))
    {
        printf("init light device fail\n");
        Dev_Light_Misc_Device_CloseFd(fd);
        fd = -1;
    }

    printf("leave SensorLightInit\n");
    return fd;
}

static void _SSAPP_LIGHT_SensorLightDeinit(int *pFd)
{
    if (*pFd >= 0)
    {
        Dev_Light_Misc_Device_DeInit(*pFd, 0);
        Dev_Light_Misc_Device_CloseFd(*pFd);
        *pFd = -1;
    }
}

static int _SSAPP_LIGHT_SensorLightGetLuxValue(int fd)
{
    return Dev_Light_Misc_Device_Get_LightSensor_Value(fd);
}

void SSAPP_LIGHT_SignalHandler(int signal)
{
    (void)signal;
    printf("Received SIGINT signal.\n");
    g_testExit = 1;
}

int SSAPP_LIGHT_UtTest()
{
    int fd  = -1;
    int lux = 0;

    fd = _SSAPP_LIGHT_SensorLightInit();
    if (fd < 0)
    {
        printf("sensor light init fail\n");
        return -1;
    }

    usleep(5000000);

    while (!g_testExit)
    {
        lux = _SSAPP_LIGHT_SensorLightGetLuxValue(fd);
        printf("get lux is %d, fd:%d\n", lux, fd);
        if (lux <= 0)
        {
            usleep(1000);
            lux = _SSAPP_LIGHT_SensorLightGetLuxValue(fd);
            printf("retry get lux is %d, fd:%d\n", lux, fd);
        }
        usleep(150000);
    }

    printf("deinit light dev\n");

    _SSAPP_LIGHT_SensorLightDeinit(&fd);

    return 0;
}

int main(int argc, char **argv)
{
    int ret = 0;

    (void)argc;
    (void)argv;
    signal(SIGINT, SSAPP_LIGHT_SignalHandler);

    // ut test
    ret = SSAPP_LIGHT_UtTest();

    return ret;
}
