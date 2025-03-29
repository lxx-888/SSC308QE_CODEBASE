/*
 * drv_disp_os.h- Sigmastar
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

#ifndef _DRV_DISP_OS_H_
#define _DRV_DISP_OS_H_

#include "drv_disp_os_header.h"

#ifdef _DRV_DISP_OS_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

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

// void              DRV_DISP_OS_Printf(MI_U32 flag, const char *szFmt, ...);
INTERFACE MI_S32  DRV_DISP_OS_ImiHeapAlloc(MI_U32 u32Size, void **ppAddr);
INTERFACE void    DRV_DISP_OS_ImiHeapFree(void *pAddr);
INTERFACE MI_U8   DRV_DISP_OS_PadMuxActive(void);
INTERFACE MI_U8   DRV_DISP_OS_SetDeviceNode(void *pPlatFormDev);
INTERFACE MI_U8   DRV_DISP_OS_SetClkOn(void *pClkEn, void *pClkRate, MI_U32 u32ClkRateSize);
INTERFACE MI_U8   DRV_DISP_OS_SetClkOff(void *pClkEn, void *pClkRate, MI_U32 u32ClkRateSize);
INTERFACE MI_U8   DRV_DISP_OS_CreateTask(DRV_DISP_OS_TaskConfig_t *pstTaskCfg, DISP_TASK_ENTRY_CB pTaskEntry,
                                         void *pDataPtr, MI_U8 *pTaskName, MI_U8 bAuotStart);
INTERFACE MI_U8   DRV_DISP_OS_DestroyTask(DRV_DISP_OS_TaskConfig_t *pstTaskCfg);
INTERFACE MI_U32  DRV_DISP_OS_GetSystemTime(void);
INTERFACE MI_U64  DRV_DISP_OS_GetSystemTimeStamp(void);
INTERFACE MI_U8   DRV_DISP_OS_CreateMutex(DRV_DISP_OS_MutexConfig_t *pstMutexCfg);
INTERFACE MI_U8   DRV_DISP_OS_DestroyMutex(DRV_DISP_OS_MutexConfig_t *pstMutexCfg);
INTERFACE MI_U8   DRV_DISP_OS_ObtainMutex(DRV_DISP_OS_MutexConfig_t *pstMutexCfg);
INTERFACE MI_U8   DRV_DISP_OS_ReleaseMutex(DRV_DISP_OS_MutexConfig_t *pstMutexCfg);
INTERFACE MI_U32  DRV_DISP_OS_StrToL(MI_U8 *str, MI_U16 u16Base, MI_U32 *pValue);
INTERFACE MI_S32  DRV_DISP_OS_Split(MI_U8 **arr, MI_U8 *str, MI_U8 *del);
INTERFACE MI_U32  DRV_DISP_OS_GetFileSize(DRV_DISP_OS_FileConfig_t *pFileCfg);
INTERFACE MI_S32  DRV_DISP_OS_WriteFile(DRV_DISP_OS_FileConfig_t *pFileCfg, MI_U8 *buf, MI_S32 writelen);
INTERFACE MI_S32  DRV_DISP_OS_OpenFile(MI_U8 *path, MI_S32 flag, MI_S32 mode, DRV_DISP_OS_FileConfig_t *pstFileCfg);
INTERFACE MI_S32  DRV_DISP_OS_CloseFile(DRV_DISP_OS_FileConfig_t *pFileCfg);
INTERFACE MI_S32  DRV_DISP_OS_ReadFile(DRV_DISP_OS_FileConfig_t *pFileCfg, MI_U8 *buf, MI_S32 readlen);
INTERFACE void    DRV_DISP_OS_SplitByToken(DRV_DISP_OS_StrConfig_t *pstStrCfg, MI_U8 *pBuf, MI_U8 *token);
INTERFACE MI_S32  DRV_DISP_OS_StrCmp(MI_U8 *str, const char *dstr);
INTERFACE MI_S32  DRV_DISP_OS_SetFileSeek(DRV_DISP_OS_FileConfig_t *pFileCfg, MI_U32 u32Flag, MI_U32 u32Pos);
INTERFACE MI_U32  DRV_DISP_OS_MathDivU64(MI_U64 nDividend, MI_U64 nDivisor, MI_U64 *pRemainder);
INTERFACE MI_U32  DRV_DISP_OS_GetCrc16(MI_U32 u32DmaId, MI_U64 *p64Crc16);
INTERFACE MI_U32  DRV_DISP_OS_GetAffCnt(MI_U32 u32DevId, MI_U32 u32VidId, MI_U32 *p32Cnt);
INTERFACE MI_U32 *DRV_DISP_OS_GpioPadModeToPadIndex(MI_U32 u32PadMode);
INTERFACE MI_U8   DRV_DISP_OS_GpioDrvSet(MI_U8 u8GpioIdx, MI_U8 u8Leve);
INTERFACE MI_U8   DRV_DISP_OS_GpioDrvGet(MI_U8 u8GpioIdx, MI_U8 *pu8Level);
INTERFACE MI_U8   DRV_DISP_OS_SetLpllClkConfig(MI_U64 u64Dclk, MI_U8 bDsi);
INTERFACE MI_U8   DRV_DISP_OS_PadMuxGetMode(MI_U32 u32Interface, MI_U32 *pu32PadMode);

#undef INTERFACE

#endif
