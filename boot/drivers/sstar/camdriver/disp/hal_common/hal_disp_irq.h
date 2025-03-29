/*
 * hal_disp_irq.h- Sigmastar
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

#ifndef _HAL_DISP_IRQ_H_
#define _HAL_DISP_IRQ_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define HAL_DISP_IS_IRQ_EN(msk, irq) ((msk & irq) == 0)

//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_DISP_IRQ_IOCTL_ENABLE,
    E_HAL_DISP_IRQ_IOCTL_GET_FLAG,
    E_HAL_DISP_IRQ_IOCTL_CLEAR,
    E_HAL_DISP_IRQ_IOCTL_GET_ID,

    E_HAL_DISP_IRQ_IOCTL_LCD_ENABLE,
    E_HAL_DISP_IRQ_IOCTL_LCD_GET_FLAG,
    E_HAL_DISP_IRQ_IOCTL_LCD_CLEAR,
    E_HAL_DISP_IRQ_IOCTL_LCD_GET_ID,

    E_HAL_DISP_IRQ_IOCTL_DMA_ENABLE,
    E_HAL_DISP_IRQ_IOCTL_DMA_GET_FLAG,
    E_HAL_DISP_IRQ_IOCTL_DMA_CLEAR,
    E_HAL_DISP_IRQ_IOCTL_DMA_GET_ID,

    E_HAL_DISP_IRQ_IOCTL_TIMEZONE_SUPPORTED,
    E_HAL_DISP_IRQ_IOCTL_TIMEZONE_GET_ID,
    E_HAL_DISP_IRQ_IOCTL_TIMEZONE_ENABLE,
    E_HAL_DISP_IRQ_IOCTL_TIMEZONE_GET_FLAG,
    E_HAL_DISP_IRQ_IOCTL_TIMEZONE_CLEAR,
    E_HAL_DISP_IRQ_IOCTL_TIMEZONE_GET_STATUS,

    E_HAL_DISP_IRQ_IOCTL_VGA_HPD_SUPPORTED,
    E_HAL_DISP_IRQ_IOCTL_VGA_HPD_GET_ID,
    E_HAL_DISP_IRQ_IOCTL_VGA_HPD_ENABLE,
    E_HAL_DISP_IRQ_IOCTL_VGA_HPD_GET_FLAG,
    E_HAL_DISP_IRQ_IOCTL_VGA_HPD_CLEAR,
    E_HAL_DISP_IRQ_IOCTL_VGA_HPD_GET_STATUS,
    E_HAL_DISP_IRQ_IOCTL_NUM,
} HAL_DISP_IRQ_IoCtlType_e;

typedef enum
{
    E_HAL_DISP_IRQ_TYPE_NONE                  = 0x00000000,
    E_HAL_DISP_IRQ_TYPE_INPORT_VSYNC          = 0x00000001,
    E_HAL_DISP_IRQ_TYPE_INPORT_VDE            = 0x00000002,
    E_HAL_DISP_IRQ_TYPE_TIMEZONE_VDE_NEGATIVE = 0x00000010,
    E_HAL_DISP_IRQ_TYPE_TIMEZONE_VS_POSITIVE  = 0x00000020,
    E_HAL_DISP_IRQ_TYPE_TIMEZONE_FPLL_UNLOCK  = 0x00000040,
    E_HAL_DISP_IRQ_TYPE_TIMEZONE_FPLL_LOCKED  = 0x00000080,
    E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON            = 0x00010000,
    E_HAL_DISP_IRQ_TYPE_VGA_HPD_OFF           = 0x00020000,
    E_HAL_DISP_IRQ_TYPE_VGA_FIFOFULL          = 0x00040000,
    E_HAL_DISP_IRQ_TYPE_VGA_FIFOEMPTY         = 0x00080000,
    E_HAL_DISP_IRQ_TYPE_CVBS_HPD_ON           = 0x00100000,
    E_HAL_DISP_IRQ_TYPE_CVBS_HPD_OFF          = 0x00200000,
    E_HAL_DISP_IRQ_TYPE_DMA_REGSETFAIL        = 0x01000000,
    E_HAL_DISP_IRQ_TYPE_DMA_FIFOFULL          = 0x02000000,
    E_HAL_DISP_IRQ_TYPE_DMA_ACTIVE_ON         = 0x04000000,
    E_HAL_DISP_IRQ_TYPE_DMA_ACTIVE_OFF        = 0x08000000,
    E_HAL_DISP_IRQ_TYPE_DMA_REALDONE          = 0x10000000,
    E_HAL_DISP_IRQ_TYPE_DMA_DONE              = 0x20000000,
    E_HAL_DISP_IRQ_TYPE_DMA_MISC              = 0x40000000,
    E_HAL_DISP_IRQ_TYPE_DMA_TRIG_FAIL         = 0x80000000,
} HAL_DISP_IRQ_Type_e;

typedef enum
{
    E_HAL_DISP_IRQ_TYPE_LCD_NONE    = 0x00000000,
    E_HAL_DISP_IRQ_TYPE_LCD_FRM_END = 0x00000001,
    E_HAL_DISP_IRQ_TYPE_LCD_IDX_RDY = 0x00000002,
    E_HAL_DISP_IRQ_TYPE_LCD_CMD_RDY = 0x00000004,
    E_HAL_DISP_IRQ_TYPE_LCD_FLM     = 0x00000200,
} HAL_DISP_IRQ_LcdIrqType_e;

typedef enum
{
    E_HAL_DISP_IRQ_TIMEZONE_TIMESTAMP_VSYNC_POSITIVE = 0x00,
    E_HAL_DISP_IRQ_TIMEZONE_TIMESTAMP_VSYNC_NEGATIVE = 0x01,
    E_HAL_DISP_IRQ_TIMEZONE_TIMESTAMP_VDE_POSITIVE   = 0x02,
    E_HAL_DISP_IRQ_TIMEZONE_TIMESTAMP_VDE_NEGATIVE   = 0x03,
    E_HAL_DISP_IRQ_TIMEZONE_TIMESTAMP_NUM            = 0x04,
} HAL_DISP_IRQ_TimeStampType_e;

typedef struct HAL_DISP_IRQ_DeviceStatus_s
{
    MI_U16 u16LockStatus[HAL_DISP_FPLL_CNT];
    MI_U16 u16PhaseErr[HAL_DISP_FPLL_CNT];
    MI_U16 u16FreqErr[HAL_DISP_FPLL_CNT];
    MI_U16 u16DacAffStatus;
} HAL_DISP_IRQ_DeviceStatus_t;

typedef struct
{
    MI_U32 u32IrqFlags;
    MI_U64 u64TimeStamp[E_HAL_DISP_IRQ_TIMEZONE_TIMESTAMP_NUM];
} HAL_DISP_IRQ_TimeZoneStatus_t;

typedef struct
{
    HAL_DISP_IRQ_IoCtlType_e  enIoctlType;
    HAL_DISP_IRQ_Type_e       enIrqType;
    HAL_DISP_IRQ_LcdIrqType_e enLcdIrqType;
    void *                    pDispCtx;
    void *                    pParam;
} HAL_DISP_IRQ_IoctlConfig_t;

typedef struct
{
    MI_U32 u32FifoFullCnt;
    MI_U32 u32RcmdHitCnt;
    MI_U32 u32Error;
    MI_U64 u64FFTimeStamp;
    MI_U64 u64RcmdHitTimeStamp;
    MI_U64 u64ErrTimeStamp;
} HAL_DISP_IRQ_VidLayerWorkingHist_t;

typedef struct
{
    MI_U32                             u32VsCnt;
    MI_U32                             u32ChangeTimingCnt;
    MI_U64                             u64TimeStamp;
    MI_U64                             u64ChangeTimingTimeStamp;
    HAL_DISP_IRQ_VidLayerWorkingHist_t stVidLayerHist[HAL_DISP_VIDLAYER_CNT];
} HAL_DISP_IRQ_DevWorkingHist_t;

typedef struct
{
    MI_U8 u8Deviceused;
    MI_U8 u8VidLayerEn[HAL_DISP_VIDLAYER_CNT];
    MI_U8 u8bStartTimegen;
} HAL_DISP_IRQ_DevWorkingStatus_t;
typedef enum
{
    E_HAL_DISP_IRQ_TIMEZONE_VDE_NEGATIVE,
    E_HAL_DISP_IRQ_TIMEZONE_VS_POSTIVE,
    E_HAL_DISP_IRQ_TIMEZONE_FPLL_UNLOCK,
    E_HAL_DISP_IRQ_TIMEZONE_FPLL_LOCKED,
    E_HAL_DISP_IRQ_TIMEZONE_MAX,
} HAL_DISP_IRQ_TimeZoneType_e;
typedef enum
{
    E_HAL_DISP_IRQ_TIMEZONE_FPLL_ST_DISABLE = 0,
    E_HAL_DISP_IRQ_TIMEZONE_FPLL_ST_LOCKING,
    E_HAL_DISP_IRQ_TIMEZONE_FPLL_ST_LOCKED,
    E_HAL_DISP_IRQ_TIMEZONE_FPLL_ST_STATBLE,
    E_HAL_DISP_IRQ_TIMEZONE_FPLL_ST_FREE,
} HAL_DISP_IRQ_TimeZoneFpllStatus_e;
typedef struct
{
    MI_U32 u32UnLockRstCnt;
    MI_U32 u32PhaseErrRstCnt;
    MI_U32 u32FpllLostLockCnt;
    MI_U64 u64UnLockRstTime;
    MI_U64 u64PhaseErrRstTime;
    MI_U64 u64FpllLostLockTime;
    MI_U64 u64FpllStableTime;
} HAL_DISP_IRQ_FpllWorkingHist_t;
typedef struct
{
    HAL_DISP_IRQ_TimeZoneFpllStatus_e enFpllStm;
    MI_U8                             u8FpllLocked;
    MI_U8                             u8FpllEn;
    MI_U8                             u8FpllLockedIrqEn;
    MI_U8                             u8FpllUnLockIrqEn;
    MI_U32                            u32UnstableFrmCnt;
    MI_U32                            u32PhaseErrCnt;
    MI_U32                            u32LockedFrmCnt;
} HAL_DISP_IRQ_FpllWorkingStatus_t;
typedef struct
{
    MI_U64 u64TimeStamp[E_HAL_DISP_IRQ_TIMEZONE_MAX];
    MI_U64 u64TimeStamp_Vde0;
    MI_U32 u32fps;
    MI_U32 u32VDeDoneCnt;
    MI_U32 u32VSCnt;
    MI_U32 u32FpllUnlockCnt;
    MI_U32 u32FplllockCnt;
} HAL_DISP_IRQ_TimeZoneWorkingHist_t;
typedef struct
{
    MI_U8  u8TimeZoneused;
    MI_U32 u32IrqFlags;
} HAL_DISP_IRQ_TimeZoneWorkingStatus_t;
typedef enum
{
    E_HAL_DISP_IRQ_VGA_HPD_ON,
    E_HAL_DISP_IRQ_VGA_HPD_OFF,
    E_HAL_DISP_IRQ_VGA_FIFOFULL,
    E_HAL_DISP_IRQ_VGA_FIFOEMPTY,
    E_HAL_DISP_IRQ_VGA_MAX,
} HAL_DISP_IRQ_VgaType_e;
typedef struct
{
    MI_U32 u32HPDonCnt;
    MI_U32 u32HPDoffCnt;
    MI_U32 u32affCnt;
    MI_U32 u32afeCnt;
    MI_U32 u32DacRstCnt;
    MI_U64 u64TimeStamp[E_HAL_DISP_IRQ_VGA_MAX];
} HAL_DISP_IRQ_VgaWorkingHist_t;
typedef struct
{
    MI_U8  u8Vgaused;
    MI_U8  u8DacSwEn;
    MI_U8  u8DacMute;
    MI_U8  u8DacRst;
    MI_U32 u32DacBindDev;
} HAL_DISP_IRQ_VgaWorkingStatus_t;
typedef enum
{
    E_HAL_DISP_IRQ_CVBS_HPD_ON,
    E_HAL_DISP_IRQ_CVBS_HPD_OFF,
    E_HAL_DISP_IRQ_CVBS_MAX,
} HAL_DISP_IRQ_CvbsType_e;
typedef struct
{
    MI_U32 u32HPDonCnt;
    MI_U32 u32HPDoffCnt;
    MI_U64 u64TimeStamp[E_HAL_DISP_IRQ_CVBS_MAX];
} HAL_DISP_IRQ_CvbsWorkingHist_t;
typedef struct
{
    MI_U8 u8Cvsbused;
} HAL_DISP_IRQ_CvbsWorkingStatus_t;
typedef struct
{
    MI_U64 u64TimeStamp;
} HAL_DISP_IRQ_LcdWorkingHist_t;
typedef struct
{
    MI_U8 u8Lcdused;
} HAL_DISP_IRQ_LcdWorkingStatus_t;
typedef enum
{
    E_HAL_DISP_IRQ_DMA_REGSETFAIL,
    E_HAL_DISP_IRQ_DMA_FIFOFULL,
    E_HAL_DISP_IRQ_DMA_ACTIVE_ON,
    E_HAL_DISP_IRQ_DMA_REALDONE,
    E_HAL_DISP_IRQ_DMA_TRIGFAIL,
    E_HAL_DISP_IRQ_DMA_TRIG,
    E_HAL_DISP_IRQ_DMA_DOUBLEVSYNC,
    E_HAL_DISP_IRQ_DMA_RST,
    E_HAL_DISP_IRQ_DMA_OVERWRITE,
    E_HAL_DISP_IRQ_DMA_CMDEMPTY,
    E_HAL_DISP_IRQ_DMA_ACTIVETIME,
    E_HAL_DISP_IRQ_DMA_MAX,
} HAL_DISP_IRQ_DmaType_e;
typedef struct
{
    MI_U32 u32FrDoneCnt;
    MI_U32 u32FrActCnt;
    MI_U32 u32FrTrigCnt;
    MI_U32 u32affCnt;
    MI_U32 u32SetFailCnt;
    MI_U32 u32TrigFailCnt;
    MI_U32 u32DoubleVsyncCnt;
    MI_U32 u32DmaRstCnt;
    MI_U32 u32OverWriteCnt;
    MI_U32 u32CmdFIFOEmptyCnt;
    MI_U64 u64TimeStamp[E_HAL_DISP_IRQ_DMA_MAX];
} HAL_DISP_IRQ_DmaWorkingHist_t;
typedef struct
{
    MI_U8  u8Dmaused;
    MI_U8  u8DmaBindDevSrc;
    MI_U8  u8DmaBindDevDst;
    MI_U8  u8bRstWdma;
    MI_U8  u8bDmaIdle;
    MI_U16 u16Crc16Md;
    MI_U32 u32CRC16[3];
    MI_U8  u8DmaDvStop;
} HAL_DISP_IRQ_DmaWorkingStatus_t;
typedef struct
{
    HAL_DISP_IRQ_DevWorkingHist_t      stDevHist[HAL_DISP_DEVICE_MAX];
    HAL_DISP_IRQ_TimeZoneWorkingHist_t stTimeZoneHist[HAL_DISP_DEVICE_MAX];
    HAL_DISP_IRQ_DmaWorkingHist_t      stDmaHist[HAL_DISP_DMA_MAX];
    HAL_DISP_IRQ_VgaWorkingHist_t      stVgaHist;
    HAL_DISP_IRQ_CvbsWorkingHist_t     stCvbsHist;
    HAL_DISP_IRQ_LcdWorkingHist_t      stLcdHist;
    HAL_DISP_IRQ_FpllWorkingHist_t     stFpllHist[HAL_DISP_FPLL_CNT];
} HAL_DISP_IRQ_WorkingHist_t;
typedef struct
{
    HAL_DISP_IRQ_DevWorkingStatus_t      stDevStatus[HAL_DISP_DEVICE_MAX];
    HAL_DISP_IRQ_TimeZoneWorkingStatus_t stTimeZoneStatus[HAL_DISP_DEVICE_MAX];
    HAL_DISP_IRQ_DmaWorkingStatus_t      stDmaStatus[HAL_DISP_DMA_MAX];
    HAL_DISP_IRQ_VgaWorkingStatus_t      stVgaStatus;
    HAL_DISP_IRQ_CvbsWorkingStatus_t     stCvbsStatus;
    HAL_DISP_IRQ_LcdWorkingStatus_t      stLcdStatus;
    HAL_DISP_IRQ_FpllWorkingStatus_t     stFpllStatus[HAL_DISP_FPLL_CNT];
} HAL_DISP_IRQ_WorkingStatus_t;

typedef struct
{
#ifndef DISP_STATISTIC_DISABLE
    HAL_DISP_IRQ_WorkingHist_t stWorkingHist;
#endif
    HAL_DISP_IRQ_WorkingStatus_t stWorkingStatus;
} HAL_DISP_IRQ_StatusHist_t;
extern HAL_DISP_IRQ_StatusHist_t g_stDispIrqHist;
//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------
#ifdef _HAL_DISP_IRQ_C_
#define INTERFACE

#else
#define INTERFACE extern
#endif
INTERFACE MI_U8 HAL_DISP_IRQ_IoCtl(HAL_DISP_IRQ_IoctlConfig_t *pCfg);
INTERFACE void  HAL_DISP_IRQ_TimeZoneOnOff(MI_U8 u8Val, void *pCtx);
INTERFACE void  HAL_DISP_IRQ_EnableTimeZoneIsr(HAL_DISP_IRQ_Type_e enType, MI_U8 *pbEn, MI_U32 u32DevId);
INTERFACE void  HAL_DISP_IRQ_SetDevIntClr(MI_U32 u32DevId, MI_U16 val);
INTERFACE void  HAL_DISP_IRQ_GetDeviceStatus(HAL_DISP_IRQ_DeviceStatus_t *pstDevStatus);
INTERFACE void  HAL_DISP_IRQ_Init(void);

#undef INTERFACE

#endif
