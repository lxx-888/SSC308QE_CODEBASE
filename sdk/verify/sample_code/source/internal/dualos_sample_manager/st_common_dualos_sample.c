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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>

#include "sstar_rpmsg.h"
#include "rpmsg_dualos_common.h"
#include "st_common_dualos_sample.h"

static MI_S32 s32RpmsgFd       = 0;
static MI_S32 s32RpmsgCtrlFd   = 0;
static pthread_t pRpmsgCheckResultThread;

static MI_S32 __LinuxRpmsgInit(void)
{
    struct ss_rpmsg_endpoint_info info;
    char   devPath[64];

    info.src = EPT_ADDR_PREFIX(EPT_TYPE_SIGMASTAR, EPT_SOC_DEFAULT, EPT_OS_LINUX) | EPT_CLASS(EPT_CLASS_RESERVED) | 0x88;
    info.dst = EPT_ADDR_PREFIX(EPT_TYPE_SIGMASTAR, EPT_SOC_DEFAULT, EPT_OS_RTOS) | EPT_CLASS(EPT_CLASS_RESERVED) | 0x31;
    snprintf(info.name, sizeof(info.name), "dualos_sample_linux");

    info.mode      = 5;
    info.target_id = 0;

    s32RpmsgCtrlFd = open("/dev/rpmsg_ctrl0", O_RDWR);
    if (s32RpmsgCtrlFd < 0)
    {
        printf("dualos_sample case fail: open /dev/rpmsg_ctrl0 failed");
        return -1;
    }

    if (ioctl(s32RpmsgCtrlFd, SS_RPMSG_CREATE_EPT_IOCTL, &info) < 0)
    {
        printf("dualos_sample case fail: ioctl SS_RPMSG_CREATE_EPT_IOCTL failed");
        return -1;
    }
    // waiting for Rpmsg Init
    usleep(100*1000);

    snprintf(devPath, sizeof(devPath), "/dev/rpmsg%d", info.id);
    s32RpmsgFd = open(devPath, O_RDWR);
    if (s32RpmsgFd < 0)
    {
        printf("dualos_sample case fail: Failed to open endpoint!\n");
        return -1;
    }
    // printf("fd %d %d\n", s32RpmsgCtrlFd, *s32RpmsgFd);
    return MI_SUCCESS;
}

static MI_S32 __RpmsgSendDataL2R(char* send_common)
{
    ST_CHECK_POINTER(send_common);

    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret = write(s32RpmsgFd, send_common, strlen(send_common) + 1);
    if (s32Ret < 0)
    {
        printf("dualos_sample case fail: Failed to write endpoint!\n");
        return -1;
    }

    return 0;
}

void ST_DUALOS_SAMPLE_StopDemo(void)
{
    if(s32RpmsgFd >= 0)
    {
        close(s32RpmsgFd);
    }
    if(s32RpmsgCtrlFd >= 0)
    {
        close(s32RpmsgCtrlFd);
    }
}

static int __RpmsgCheckResult(char *common)
{
    MI_S32                        s32Ret            = 0;
    char                          send_data[256]    = {0};
    char                          recv_data[256]    = {0};
    char                          pass_data[264]    = {0};
    char                          fail_data[264]    = {0};
    struct timeval                timeout;
    fd_set                        readfds;

    // set timeout
    timeout.tv_sec  = 10;
    timeout.tv_usec = 0;
    if(strlen(common) > 256)
    {
        printf("common size is too long! \n");
    }
    strncpy(send_data, common, strlen(common));

    FD_ZERO(&readfds);
    FD_SET(s32RpmsgFd, &readfds);

    s32Ret = select(s32RpmsgFd + 1, &readfds, NULL, NULL, &timeout);

    if (s32Ret == -1)
    {
        printf("dualos_sample %s fail: select fail", common);
        return -1;
    }
    else if (s32Ret == 0)
    {
        printf("dualos_sample %s fail: Timeout \n", common);
        return -1;
    }
    else
    {
        memset(recv_data, 0, sizeof(recv_data));
        s32Ret = read(s32RpmsgFd, recv_data, sizeof(recv_data));
        if (s32Ret > 0)
        {
            sprintf(pass_data, "%s pass", send_data);
            sprintf(fail_data, "%s fail", send_data);
            if (strcmp(pass_data, recv_data) == 0)
            {
                printf("dualos_sample %s pass\n", common);
            }
            else if (strcmp(fail_data, recv_data) == 0)
            {
                printf("dualos_sample %s fail: dualos_sample case fail \n", common);
                return -1;
            }
            else
            {
                printf("dualos_sample %s fail: read date not find \n", common);
                return -1;
            }
        }
        else
        {
            printf("dualos_sample %s fail: read ept fail:%d\n", common, s32Ret);
            return -1;
        }
    }

    return 0;
}

static void *ST_RpmsgCheckResultThread(void *args)
{
    __RpmsgCheckResult((char*)args);
    return NULL;
}

void ST_HandleSig(MI_S32 signo)
{
    ST_DUALOS_SAMPLE_StopDemo();
    ST_INFO("%s exit\n", __FUNCTION__);
    _exit(0);
}

static void __AutoExitFunc(void)
{
    struct sigaction sigAction;

    sigAction.sa_handler = ST_HandleSig;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    sigaction(SIGINT, &sigAction, NULL);  //-2
    sigaction(SIGKILL, &sigAction, NULL); //-9
    sigaction(SIGTERM, &sigAction, NULL); //-15
}

int ST_DUALOS_SAMPLE_StartDemo(char *common)
{
    __AutoExitFunc();
    if(__LinuxRpmsgInit() == 0)
    {
        __RpmsgSendDataL2R(common);
        pthread_create(&pRpmsgCheckResultThread, NULL, ST_RpmsgCheckResultThread, (void*)common);
        pthread_join(pRpmsgCheckResultThread, NULL);
    }
    return 0;
}
