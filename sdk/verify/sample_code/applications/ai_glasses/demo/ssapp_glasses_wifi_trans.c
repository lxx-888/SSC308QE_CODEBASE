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
/* #include <modprobe.h> */

#include "ssapp_glasses_wifi_trans.h"
#include "ssapp_glasses_misc.h"

#define WIFI_MODULE_NAME    "ssw6x5x"
#define WIFI_KO_NAME        "./" WIFI_MODULE_NAME "_4bit.ko"
#define WIFI_CFG_FILE       "./" WIFI_MODULE_NAME "-wifi.cfg"
#define FTP_SERVER_CMD      "tcpsvd 0 21 ftpd -w " FTP_SERVER_PATH
#define PKILL_FTP_SERVER    "pkill -9 tcpsvd"
#define IFCONFIG_NET_DEVICE "ifconfig ssw "
#define NETWORK_IP          "192.168.30.1"

MI_S32 SSAPP_GLASSES_WIFI_TRANS_InsmodWifiMoudle(void)
{
    int ret = MI_SUCCESS;

    ret = SSAPP_GLASSES_MISC_IsModuleLoaded(WIFI_MODULE_NAME);
    if (ret == 1)
    {
        printf("Module ssw6x5x is already loaded.\n");
        return -1;
    }

    ret = system("insmod " WIFI_KO_NAME " stacfgpath=" WIFI_CFG_FILE);
    if (ret != 0)
    {
        printf("modprobe failed:%d\n", ret);
    }
    return ret;
}

MI_S32 SSAPP_GLASSES_WIFI_TRANS_RmmodWifiMoudle(void)
{
    int ret = 0;
    ret     = system("rmmod " WIFI_MODULE_NAME);
    if (ret != 0)
    {
        printf("remove_module fail:%d\n", ret);
    }
    ret = SSAPP_GLASSES_MISC_IsModuleLoaded(WIFI_MODULE_NAME);
    if (ret == 0)
    {
        printf("remove %s success\n", WIFI_MODULE_NAME);
    }

    return ret;
}

MI_S32 SSAPP_GLASSES_WIFI_TRANS_StartFtpServer(void)
{
    MI_S32 ret;
    ret = system(FTP_SERVER_CMD " &");
    if (ret != 0)
    {
        printf("start ftp server failed%d\n", ret);
        return -1;
    }
    return ret;
}

MI_S32 SSAPP_GLASSES_WIFI_TRANS_StopFtpServer(void)
{
    MI_S32 ret = MI_SUCCESS;

    printf("going to pkill ftp server\n");
    ret = system(PKILL_FTP_SERVER);
    if (ret == 0)
    {
        printf("stop ftp server success\n");
    }
    return ret;
}

MI_S32 SSAPP_GLASSES_WIFI_TRANS_ConfigNetwork(void)
{
    FILE *fp = popen(IFCONFIG_NET_DEVICE "up", "r");
    pclose(fp);

    // set ip address
    fp = popen(IFCONFIG_NET_DEVICE NETWORK_IP, "r");
    pclose(fp);

    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_WIFI_TRANS_Init(void)
{
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_WIFI_TRANS_Start(void)
{
    SSAPP_GLASSES_WIFI_TRANS_InsmodWifiMoudle();
    SSAPP_GLASSES_WIFI_TRANS_ConfigNetwork();

    SSAPP_GLASSES_WIFI_TRANS_StartFtpServer();

    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_WIFI_TRANS_Stop(void)
{
    SSAPP_GLASSES_WIFI_TRANS_StopFtpServer();

    SSAPP_GLASSES_WIFI_TRANS_RmmodWifiMoudle();
    return MI_SUCCESS;
}
