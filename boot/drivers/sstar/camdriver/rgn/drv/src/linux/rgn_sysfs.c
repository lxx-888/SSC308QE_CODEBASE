/*
 * rgn_sysfs.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#define _RGN_SYSFS_C_
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "ms_msys.h"
#include "ms_platform.h"

#include "cam_os_wrapper.h"
#include "cam_sysfs.h"
#include "mhal_cmdq.h"
#include "mhal_rgn_datatype.h"
#include "mhal_rgn.h"
#include "rgn_debug.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int   argc;
    char *argv[200];
} RgnSysFsStrConfig_t;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
int RgnSysFsSplit(char **arr, char *str, char *del)
{
    char *cur   = str;
    char *token = NULL;
    int   cnt   = 0;

    token = strsep(&cur, del);
    while (token)
    {
        arr[cnt] = token;
        token    = strsep(&cur, del);
        cnt++;
    }
    return cnt;
}

void RgnSysFsParsingString(char *str, RgnSysFsStrConfig_t *pstStrCfg)
{
    char del[] = " ";
    int  len;

    pstStrCfg->argc                               = RgnSysFsSplit(pstStrCfg->argv, str, del);
    len                                           = strlen(pstStrCfg->argv[pstStrCfg->argc - 1]);
    pstStrCfg->argv[pstStrCfg->argc - 1][len - 1] = '\0';
}

ssize_t CheckRgnDbgmgStore(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    RgnSysFsStrConfig_t stStrCfg;
    int                 ret = 0;
    MS_U32              u32DebugLevel;
    if (buf == NULL)
    {
        RGN_ERR("Buf null");
        return n;
    }

    RgnSysFsParsingString((char *)buf, &stStrCfg);

    if (stStrCfg.argc != 1)
    {
        RGN_ERR("Not input parament");
        return n;
    }

    ret = kstrtou32(stStrCfg.argv[0], 16, &u32DebugLevel);

    if (ret != 0)
    {
        RGN_ERR("kstrtou32 failed");
        return n;
    }

    MHAL_RGN_SetDebugLevel(u32DebugLevel);
    CamOsPrintf("Level: 0x%08x\n", g_u32DebugLevel);
    return n;
}

ssize_t CheckRgnDbgmgSow(struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t ret = 0;
    if (buf == NULL)
    {
        return -1;
    }
    ret += sprintf(buf, "------------------- RGN DBGMG ------------------- \n");
    ret += sprintf(buf + ret, "  Debug Level: 0x%08x\n", g_u32DebugLevel);
    ret += sprintf(buf + ret, "    %20s: 0x%08x\n", "NONE", 0);
    ret += sprintf(buf + ret, "    %20s: 0x%08x\n", "MHAL", MHAL_RGN_DBG_LV_MHAL);
    ret += sprintf(buf + ret, "    %20s: 0x%08x\n", "HAL_IF", MHAL_RGN_DBG_LV_HAL_IF);
    ret += sprintf(buf + ret, "    %20s: 0x%08x\n", "HAL_GOP", MHAL_RGN_DBG_LV_HAL_GOP);
    ret += sprintf(buf + ret, "    %20s: 0x%08x\n", "HAL_COVER", MHAL_RGN_DBG_LV_HAL_COVER);
    ret += sprintf(buf + ret, "    %20s: 0x%08x\n", "HAL_FRAME", MHAL_RGN_DBG_LV_HAL_FRAME);
    ret += sprintf(buf + ret, "    %20s: 0x%08x\n", "HAL_CI", MHAL_RGN_DBG_LV_HAL_CI);
    ret += sprintf(buf + ret, "    %20s: 0x%08x\n", "REG", MHAL_RGN_DBG_LV_REG);
    CamOsPrintf("Ret string len %d\n", ret);
    return ret;
}

static DEVICE_ATTR(dbgmg, 0644, CheckRgnDbgmgSow, CheckRgnDbgmgStore);

void DrvRgnSysfsInit(struct device *device)
{
    CamDeviceCreateFile(device, &dev_attr_dbgmg);
}
