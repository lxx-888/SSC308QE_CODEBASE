/*
 * pnl_sysfs.c- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
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

#define _PNL_SYSFS_C_
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
//#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/delay.h>
#include <linux/device.h>

#include "drv_pnl_os.h"
#include "cam_sysfs.h"
#include "hal_pnl_common.h"
#include "pnl_debug.h"
#include "hal_pnl_chip.h"
#include "hal_pnl_st.h"
#include "hal_pnl.h"
#include "hal_pnl_reg.h"
#include "mhal_pnl_datatype.h"
#include "mhal_pnl.h"
#include "drv_pnl_os.h"
#include "drv_pnl_if.h"
#include "drv_pnl_ctx.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define PNLSYSFS_SPRINTF_STRCAT(str, _fmt, _args...) \
    do                                               \
    {                                                \
        char tmpstr[1024];                           \
        sprintf(tmpstr, _fmt, ##_args);              \
        strcat(str, tmpstr);                         \
    } while (0)

#define PNLSYSFS_SPRINTF(str, _fmt, _args...) sprintf(str, _fmt, ##_args)

#define PNLSYSFS_DBG(_fmt, _args...)                       \
    do                                                     \
    {                                                      \
        CamOsPrintf(PRINT_GREEN _fmt PRINT_NONE, ##_args); \
    } while (0)

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int   argc;
    char *argv[200];
} PnlSysFsStrConfig_t;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
u32 g_gu32DbgLevel = 0;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
int PnlSysFsSplit(char **arr, char *str, char *del)
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

void PnlSysFsParsingString(char *str, PnlSysFsStrConfig_t *pstStrCfg)
{
    char del[] = " ";
    int  len;

    pstStrCfg->argc                               = PnlSysFsSplit(pstStrCfg->argv, str, del);
    len                                           = strlen(pstStrCfg->argv[pstStrCfg->argc - 1]);
    pstStrCfg->argv[pstStrCfg->argc - 1][len - 1] = '\0';
}

void PnlDbgmgStore(PnlSysFsStrConfig_t *pstStringCfg)
{
    int  ret;
    bool bParamSet = 0;
    u32  u32Level  = 0;

    if (pstStringCfg->argc == 1)
    {
        ret       = kstrtou32(pstStringCfg->argv[0], 16, &u32Level);
        bParamSet = 1;
    }

    g_gu32DbgLevel = u32Level;

    if (bParamSet)
    {
        MhalPnlSetDebugLevel((void *)&g_gu32DbgLevel);
        printk("dbg level=%x\n", g_gu32DbgLevel);
    }
    else
    {
        PNLSYSFS_DBG("[LEVEL]\n");
    }
}

int PnlDbgmgShow(char *DstBuf)
{
    int   retSprintf = -1;
    char *memSrcBuf;

    memSrcBuf = (char *)DrvPnlOsMemAlloc(1024 * 3);

    if (memSrcBuf)
    {
        PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "------------------- PNL DBGMG ------------------- \n");
        PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "DbgLvl: 0x%08x\n", g_gu32DbgLevel);
        PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "  NONE:   0x00000000\n");
        PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "   DRV:   0x00000001\n");
        PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "   HAL:   0x00000002\n");
        PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "MODULE:   0x00000004\n");
        PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "  CTX :   0x00000008\n");

        retSprintf = PNLSYSFS_SPRINTF(DstBuf, "%s", memSrcBuf);
        DrvPnlOsMemRelease(memSrcBuf);
    }
    return retSprintf;
}

//-----------------------------------------------------------------------------
void PnlClkStore(PnlSysFsStrConfig_t *pstStringCfg)
{
    int               ret, idx;
    char *            pClkName   = NULL;
    long              bEn        = 0;
    long              u32ClkRate = 0;
    bool              bParamSet  = 0;
    DrvPnlCtxConfig_t stPnlCtxCfg;
    bool *            pbClkEn     = NULL;
    u32 *             pu32ClkRate = NULL;
    u8                i;

    if (HAL_PNL_CLK_NUM)
    {
        pbClkEn     = DrvPnlOsMemAlloc(sizeof(bool) * HAL_PNL_CLK_NUM);
        pu32ClkRate = DrvPnlOsMemAlloc(sizeof(u32) * HAL_PNL_CLK_NUM);

        if (pbClkEn && pu32ClkRate)
        {
            if (strcmp(pstStringCfg->argv[0], "clktree") == 0)
            {
                pClkName = pstStringCfg->argv[0];
                ret      = kstrtol(pstStringCfg->argv[1], 10, &bEn);
                for (i = 0; i < HAL_PNL_CLK_NUM; i++)
                {
                    ret            = kstrtol(pstStringCfg->argv[2 + i], 10, &u32ClkRate);
                    pu32ClkRate[i] = u32ClkRate;
                }
                bParamSet = 1;
            }
            else if (pstStringCfg->argc == 3)
            {
                pClkName  = pstStringCfg->argv[0];
                ret       = kstrtol(pstStringCfg->argv[1], 10, &bEn);
                ret       = kstrtol(pstStringCfg->argv[2], 10, &u32ClkRate);
                bParamSet = 1;
            }
            else
            {
                char *memSrcBuf = NULL;
                memSrcBuf       = (char *)DrvPnlOsMemAlloc(1024 * 3);
                PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "----------------- CLK TREE  -----------------\n");
                PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "clktree [En] [VALUE0 ~ %d]\n", HAL_PNL_CLK_NUM - 1);
                PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "----------------- DRV Update -----------------\n");
                PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "[CLK Type] [En] [VALUE]\n");
                PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "----------------- CLK Type ----------------- \n");
                for (i = 0; i < HAL_PNL_CLK_NUM; i++)
                {
                    PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "%s, %s\n", HAL_PNL_CLK_GET_NAME(i),
                                            HAL_PNL_CLK_GET_MUX_ATTR(i) ? "CLKIdx" : "ClkRate");
                }
                PNLSYSFS_DBG("%s\n", memSrcBuf);
                DrvPnlOsMemRelease(memSrcBuf);
                return;
            }

            if (bParamSet)
            {
                if (strcmp(pClkName, "clktree") == 0)
                {
                    if (bEn)
                    {
                        for (i = 0; i < HAL_PNL_CLK_NUM; i++)
                        {
                            pbClkEn[i] = HAL_PNL_CLK_GET_OFF_SETTING(i);
                        }

                        if (DrvPnlOsSetClkOn(pbClkEn, pu32ClkRate, HAL_PNL_CLK_NUM) == 0)
                        {
                            PNL_ERR("%s %d Set Clk Tree On Fail\n", __FUNCTION__, __LINE__);
                        }
                    }
                    else
                    {
                        for (i = 0; i < HAL_PNL_CLK_NUM; i++)
                        {
                            pbClkEn[i] = HAL_PNL_CLK_GET_ON_SETTING(i);
                        }
                        if (DrvPnlOsSetClkOff(pbClkEn, pu32ClkRate, HAL_PNL_CLK_NUM) == 0)
                        {
                            PNL_ERR("%s %d Set Clk Tree Off Fail\n", __FUNCTION__, __LINE__);
                        }
                    }
                }
                else
                {
                    if (DrvPnlIfGetClkConfig((void *)&stPnlCtxCfg, pbClkEn, pu32ClkRate, HAL_PNL_CLK_NUM) == 0)
                    {
                        PNL_ERR("%s %d, Get Clk Fail\n", __FUNCTION__, __LINE__);
                    }

                    for (idx = 0; idx < HAL_PNL_CLK_NUM; idx++)
                    {
                        if (strcmp(pClkName, HAL_PNL_CLK_GET_NAME(idx)) == 0)
                        {
                            pbClkEn[idx]     = bEn ? 1 : 0;
                            pu32ClkRate[idx] = u32ClkRate;
                            if (DrvPnlIfSetClkConfig((void *)&stPnlCtxCfg, pbClkEn, pu32ClkRate, HAL_PNL_CLK_NUM) == 0)
                            {
                                PNL_ERR("%s %d, Set Clk Fail\n", __FUNCTION__, __LINE__);
                            }
                            break;
                        }
                    }

                    if (idx == HAL_PNL_CLK_NUM)
                    {
                        PNL_ERR("%s %d, Unknown clk type: %s\n", __FUNCTION__, __LINE__, pClkName);
                    }
                }
            }
        }

        if (pbClkEn)
        {
            DrvPnlOsMemRelease(pbClkEn);
        }
        if (pu32ClkRate)
        {
            DrvPnlOsMemRelease(pu32ClkRate);
        }
    }
}

int PnlClkShow(char *DstBuf)
{
    bool *            pbClkEn     = NULL;
    u32 *             pu32ClkRate = NULL;
    int               retSprintf  = -1;
    char *            memSrcBuf;
    DrvPnlCtxConfig_t stPnlCtxCfg;
    u8                i;

    if (HAL_PNL_CLK_NUM)
    {
        memSrcBuf   = (char *)DrvPnlOsMemAlloc(1024 * 3);
        pbClkEn     = DrvPnlOsMemAlloc(sizeof(bool) * HAL_PNL_CLK_NUM);
        pu32ClkRate = DrvPnlOsMemAlloc(sizeof(bool) * HAL_PNL_CLK_NUM);

        if (memSrcBuf && pbClkEn && pu32ClkRate)
        {
            if (DrvPnlIfGetClkConfig((void *)&stPnlCtxCfg, pbClkEn, pu32ClkRate, HAL_PNL_CLK_NUM) == 0)
            {
                PNL_ERR("%s %d, Get Clk Fail\n", __FUNCTION__, __LINE__);
            }

            for (i = 0; i < HAL_PNL_CLK_NUM; i++)
            {
                PNLSYSFS_SPRINTF_STRCAT(memSrcBuf, "%-20s: En:%d %s:%d\n", HAL_PNL_CLK_GET_NAME(i), pbClkEn[i],
                                        HAL_PNL_CLK_GET_MUX_ATTR(i) ? "ClkIdx" : "ClkRate", pu32ClkRate[i]);
            }

            retSprintf = PNLSYSFS_SPRINTF(DstBuf, "%s", memSrcBuf);
        }

        if (memSrcBuf)
        {
            DrvPnlOsMemRelease(memSrcBuf);
        }

        if (pbClkEn)
        {
            DrvPnlOsMemRelease(pbClkEn);
        }

        if (pu32ClkRate)
        {
            DrvPnlOsMemRelease(pu32ClkRate);
        }
    }

    return retSprintf;
}

//-----------------------------------------------------------------------------

ssize_t CheckPnldbgmgStore(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if (NULL != buf)
    {
        PnlSysFsStrConfig_t stStrCfg;

        PnlSysFsParsingString((char *)buf, &stStrCfg);
        PnlDbgmgStore(&stStrCfg);
        return n;
    }
    return 0;
}

ssize_t CheckPnldbgmgShow(struct device *dev, struct device_attribute *attr, char *buf)
{
    return PnlDbgmgShow(buf);
}

static DEVICE_ATTR(dbgmg, 0644, CheckPnldbgmgShow, CheckPnldbgmgStore);

ssize_t CheckPnlclkStore(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if (NULL != buf)
    {
        PnlSysFsStrConfig_t stStrCfg;

        PnlSysFsParsingString((char *)buf, &stStrCfg);
        PnlClkStore(&stStrCfg);
        return n;
    }
    return 0;
}

ssize_t CheckPnlclkShow(struct device *dev, struct device_attribute *attr, char *buf)
{
    return PnlClkShow(buf);
}

static DEVICE_ATTR(clk, 0644, CheckPnlclkShow, CheckPnlclkStore);

//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------

void DrvPnlSysfsInit(struct device *device)
{
    CamDeviceCreateFile(device, &dev_attr_dbgmg);
    CamDeviceCreateFile(device, &dev_attr_clk);
}
