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
#include <sys/stat.h>
#include <sys/time.h>

#include "ssapp_glasses_misc.h"
#include "st_common_aov.h"

MI_VIRT SSAPP_GLASSES_MISC_GetPts(void)
{
    struct timeval tv;
    long           milliseconds;

    gettimeofday(&tv, NULL);
    milliseconds = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return milliseconds;
}

MI_S32 SSAPP_GLASSES_MISC_IsModuleLoaded(const char *moduleName)
{
    char  cmd[128];
    FILE *fp;
    char  buffer[128];

    snprintf(cmd, sizeof(cmd), "lsmod | grep %s", moduleName);

    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        perror("popen failed");
        return -1;
    }

    // check return result
    if (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        // module had insmod already if got one line
        pclose(fp);
        return 1;
    }

    pclose(fp);
    return 0;
}

MI_S32 SSAPP_GLASSES_MISC_CheckFtpPath(void)
{
    struct stat st = {0};

    // check floder path exist
    if (stat(FTP_SERVER_PATH, &st) == -1)
    {
        // if not exist, creat floder
        if (mkdir(FTP_SERVER_PATH, 0755) == -1)
        {
            printf("mkdir ftp file path failed\n");
            return -1;
        }
        printf("creat ftp file path success\n");
    }
    else
    {
        // floder had existed
        printf("ftp file path had exist\n");
    }

    return MI_SUCCESS;
}

#define SUSPEND_RETRY_CNT 1
MI_S32 SSAPP_GLASSES_MISC_Suspend(void)
{
    MI_S32 s32Ret     = MI_SUCCESS;
    MI_U8  u8RetryCnt = 0;

SUSPEND_RETRY:
    s32Ret = __WriteFile(POWER_STATE, "mem");
    printf("\033[34m=>\n\033[0m");
    if (s32Ret != MI_SUCCESS)
    {
        if (u8RetryCnt++ < SUSPEND_RETRY_CNT)
        {
            printf("Enter suspend fail, will retry after 1s!");
            sleep(1);
            goto SUSPEND_RETRY;
        }
        printf("Enter suspend fail!\n");
    }
    return s32Ret;
}

MI_S32 SSAPP_GLASSES_MISC_Init(void)
{
    return SSAPP_GLASSES_MISC_CheckFtpPath();
}

MI_S32 SSAPP_GLASSES_MISC_DeInit(void)
{
    return MI_SUCCESS;
}
