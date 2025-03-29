/*
 * drv_disp_os.c- Sigmastar
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
#define _DRV_DISP_OS_C_
#include "mi_disp.h"
#include "mi_disp_impl_datatype.h"

#include "drv_disp_os.h"
#include "disp_debug.h"
#include "mhal_common.h"
#include "mhal_cmdq.h"
#include "drv_gpio.h"
#include "drv_padmux.h"
#include "drv_puse.h"
#include "padmux.h"
#include "hal_disp_chip.h"
#include "hal_disp_st.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Internal Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------
#define DRV_DISP_OS_Printf(dbglv, _fmt, _args...)                 \
    do                                                            \
    {                                                             \
        if (g_u32DispDbgLevel & dbglv)                            \
        {                                                         \
            if (g_u32DispDbgLevel & DISP_DBG_LEVEL_DEBUG2WARNING) \
                DISP_PRINTF(KERN_WARNING _fmt, ##_args);          \
            else                                                  \
                DISP_PRINTF(KERN_DEBUG _fmt, ##_args);            \
        }                                                         \
    } while (0)

void *DRV_DISP_OS_MemAlloc(MI_U32 u32Size)
{
    void *pBuf = NULL;
    pBuf       = malloc(u32Size);
    if (pBuf)
    {
        memset(pBuf, 0, u32Size);
    }
    return pBuf;
}

void DRV_DISP_OS_MemRelease(void *pPtr)
{
    free(pPtr);
}

void DRV_DISP_OS_MsSleep(MI_U32 u32Msec)
{
    mdelay(u32Msec);
}

void DRV_DISP_OS_UsSleep(MI_U32 u32Usec)
{
    udelay(u32Usec);
}

void DRV_DISP_OS_MsDelay(MI_U32 u32Msec)
{
    mdelay(u32Msec);
}

void DRV_DISP_OS_UsDelay(MI_U32 u32Usec)
{
    udelay(u32Usec);
}

MI_S32 DRV_DISP_OS_ImiHeapAlloc(MI_U32 u32Size, void **ppAddr)
{
    return 0;
}

void DRV_DISP_OS_ImiHeapFree(void *pAddr) {}

MI_U8 DRV_DISP_OS_PadMuxActive(void)
{
    return 0;
}

MI_U8 DRV_DISP_OS_CreateTask(DRV_DISP_OS_TaskConfig_t *pstTaskCfg, DISP_TASK_ENTRY_CB pTaskEntry, void *pDataPtr,
                             MI_U8 *pTaskName, MI_U8 bAuotStart)
{
    return 0;
}

MI_U8 DRV_DISP_OS_DestroyTask(DRV_DISP_OS_TaskConfig_t *pstTaskCfg)
{
    return 0;
}

MI_U8 DRV_DISP_OS_SetDeviceNode(void *pPlatFormDev)
{
    return 1;
}

MI_U8 DRV_DISP_OS_SetClkOn(void *pClkEn, void *pClkRate, MI_U32 u32ClkRateSize)
{
    return 0;
}

MI_U8 DRV_DISP_OS_SetClkOff(void *pClkEn, void *pClkRate, MI_U32 u32ClkRateSize)
{
    return 0;
}

MI_U32 DRV_DISP_OS_GetSystemTime(void)
{
    return 0;
}

MI_U64 DRV_DISP_OS_GetSystemTimeStamp(void)
{
    return 0;
}

MI_S32 DRV_DISP_OS_Split(MI_U8 **arr, MI_U8 *str, MI_U8 *del)
{
    return 0;
}

MI_U32 DRV_DISP_OS_GetFileSize(DRV_DISP_OS_FileConfig_t *pFileCfg)
{
    return 0;
}

MI_S32 DRV_DISP_OS_SetFileSeek(DRV_DISP_OS_FileConfig_t *pFileCfg, MI_U32 u32Flag, MI_U32 u32Pos)
{
    return 0;
}

MI_S32 DRV_DISP_OS_OpenFile(MI_U8 *path, MI_S32 flag, MI_S32 mode, DRV_DISP_OS_FileConfig_t *pstFileCfg)
{
    return 0;
}

MI_S32 DRV_DISP_OS_WriteFile(DRV_DISP_OS_FileConfig_t *pFileCfg, MI_U8 *buf, MI_S32 writelen)
{
    return 0;
}

MI_S32 DRV_DISP_OS_ReadFile(DRV_DISP_OS_FileConfig_t *pFileCfg, MI_U8 *buf, MI_S32 readlen)
{
    return 0;
}

MI_S32 DRV_DISP_OS_CloseFile(DRV_DISP_OS_FileConfig_t *pFileCfg)
{
    return 0;
}

void DRV_DISP_OS_SplitByToken(DRV_DISP_OS_StrConfig_t *pstStrCfg, MI_U8 *pBuf, MI_U8 *token) {}

MI_S32 DRV_DISP_OS_StrCmp(MI_U8 *str, const char *dstr)

{
    MI_U8 ret;
    ret = strcmp((char *)str, dstr);
    return ret;
}

MI_U8 DRV_DISP_OS_CreateMutex(DRV_DISP_OS_MutexConfig_t *pstMutexCfg)

{
    return 1;
}

MI_U8 DRV_DISP_OS_DestroyMutex(DRV_DISP_OS_MutexConfig_t *pstMutexCfg)
{
    return 1;
}

MI_U8 DRV_DISP_OS_ObtainMutex(DRV_DISP_OS_MutexConfig_t *pstMutexCfg)
{
    return 1;
}

MI_U8 DRV_DISP_OS_ReleaseMutex(DRV_DISP_OS_MutexConfig_t *pstMutexCfg)
{
    return 1;
}
MI_U32 DRV_DISP_OS_StrToL(MI_U8 *str, u16 u16Base, MI_U32 *pValue)
{
    *pValue = ustrtoul((char *)str, NULL, u16Base);
    return 0;
}
MI_U32 DRV_DISP_OS_MathDivU64(MI_U64 nDividend, MI_U64 nDivisor, MI_U64 *pRemainder)
{
    MI_U64 u64idx = 0;
    while (1)
    {
        if (nDivisor * u64idx < nDividend)
        {
            u64idx++;
        }
        else
        {
            *pRemainder = nDividend - (nDivisor * (u64idx - 1));
            break;
        }
    }
    return (MI_U32)(u64idx - 1);
}
MI_U8 DRV_DISP_OS_PadMuxGetMode(MI_U32 u32Interface, MI_U32 *pu32PadMode)
{
    MI_U32 u32PadMode;

    u32PadMode = drv_padmux_getmode(PARSING_LINKTYPE_TO_PUSETYPE(u32Interface));

    if (PINMUX_FOR_UNKNOWN_MODE == u32PadMode)
    {
        DISP_ERR("%s %d: Fail to get pad mod:%d\n", __FUNCTION__, __LINE__, u32PadMode);
        return 0;
    }

    *pu32PadMode = u32PadMode;

    return 1;
}

MI_U32 *DRV_DISP_OS_GpioPadModeToPadIndex(MI_U32 u32PadMode)
{
    return sstar_gpio_padmode_to_padindex(u32PadMode);
}

MI_U8 DRV_DISP_OS_GpioDrvSet(MI_U8 u8GpioIdx, MI_U8 u8Level)
{
    return sstar_gpio_drv_set(u8GpioIdx, u8Level);
}

MI_U8 DRV_DISP_OS_GpioDrvGet(MI_U8 u8GpioIdx, MI_U8 *pu8Level)
{
    return sstar_gpio_drv_get(u8GpioIdx, pu8Level);
}
