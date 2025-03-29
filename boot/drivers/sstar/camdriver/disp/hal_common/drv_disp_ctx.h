/*
 * drv_disp_ctx.h- Sigmastar
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

#ifndef _DRV_DISP_CTX_H_
#define _DRV_DISP_CTX_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_CTX_INVAILD_IDX 0xFFFF
//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_DRV_DISP_CTX_TYPE_DEVICE,
    E_DRV_DISP_CTX_TYPE_VIDLAYER,
    E_DRV_DISP_CTX_TYPE_INPUTPORT,
    E_DRV_DISP_CTX_TYPE_DMA,
    E_DRV_DISP_CTX_TYPE_MAX,
} DRV_DISP_CTX_Type_e;

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct DRV_DISP_CTX_List_s DRV_DISP_CTX_List_t;

struct DRV_DISP_CTX_List_s
{
    DRV_DISP_CTX_List_t *next;
    DRV_DISP_CTX_List_t *prev;
    void *               entry; // DRV_DISP_CTX_Config_t
};

typedef struct
{
    MI_U8                             bUsedInputPort;
    MI_U32                            u32PortId;
    MI_U8                             bEnInPort;
    MI_U8                             bChangeInPortSizeCfg;
    MI_U16                            u16Hratio;
    MI_U16                            u16Vratio;
    MI_DISP_IMPL_MhalVideoFrameData_t stFrameData;
    MI_DISP_IMPL_MhalVideoFrameData_t stImiData;
    MI_DISP_InputPortAttr_t           stAttr;
    MI_DISP_VidWinRect_t              stCrop;
    MI_DISP_RotateConfig_t            stRot;
    MI_DISP_IMPL_MhalRingBufferAttr_t stRingBuffAttr;
    void *                            pstVidLayerContain;
    MI_U32                            u32VidLayerBindType;
    MI_U16                            u16FlipFrontPorchCnt;
} DRV_DISP_CTX_InputPortContain_t;

typedef struct
{
    MI_U8                             bEnable;
    MI_U8                             u8MergeToDevId;
    MI_DISP_IMPL_MhalVideoLayerType_e eVidLayerType;
    MI_U32                            u32BindDevId;
    void *                            pstDevCtx; // DRV_DISP_CTX_DeviceContain_t
    MI_DISP_VideoLayerAttr_t          stAttr;
    MI_U32                            u32Priority;
    MI_U32                            u32RealOrder;
    DRV_DISP_CTX_InputPortContain_t * pstInputPortContain[HAL_DISP_INPUTPORT_NUM];
    HAL_DISP_ST_MopRotateConfig_t     stRotCfg;
    DRV_DISP_CTX_List_t               stVidList;
} DRV_DISP_CTX_VideoLayerContain_t;

typedef struct
{
    MI_U32                                 u32DmaId;
    MI_DISP_IMPL_MhalDmaInputConfig_t      stInputCfg;
    MI_DISP_IMPL_MhalDmaOutputConfig_t     stOutputCfg;
    MI_DISP_IMPL_MhalDmaBufferAttrConfig_t stBufferAttrCfg;
    void *                                 pSrcDevContain;
    void *                                 pDestDevContain;
    MI_U8                                  u8RingModeEn;
    MI_U16                                 u16RingMdChange;
} DRV_DISP_CTX_DmaContain_t;

typedef struct
{
    MI_U32                                   u32DevId;
    MI_U8                                    bEnable;
    MI_U8                                    bMergeCase;
    MI_U8                                    u8BindVideoLayerCnt;
    MI_U32                                   u32BgColor;
    MI_U32                                   u32Interface;
    MI_U16                                   u16McuCnt;
    MI_U16                                   u16McuFenceId;
    MI_U32                                   u32DeviceSynth[E_HAL_DISP_ST_DEV_SYNTH_MAX];
    MI_U32                                   eBindVideoLayer;
    HAL_DISP_ST_ColorSpaceType_e             eDeviceTopColor;
    HAL_DISP_ST_ColorSpaceType_e             eDeviceBotColor;
    MI_DISP_OutputTiming_e                   eTimeType;
    HAL_DISP_ST_DeviceTimingConfig_t         stDevTimingCfg;
    HAL_DISP_ST_DeviceParam_t                stDeviceParam;
    MI_DISP_GammaParam_t                     stGammaParam;
    MI_DISP_ColorTemperature_t               stColorTemp;
    MI_DISP_IMPL_MhalTimeZoneConfig_t        stTimeZoeCfg;
    HAL_DISP_ST_DeviceChangeTimingConfig_t   stDevChangeTimingCfg;
    DRV_DISP_CTX_VideoLayerContain_t *       pstVidLayeCtx[HAL_DISP_VIDLAYER_MAX];
    DRV_DISP_CTX_List_t                      stListHead;
    DRV_DISP_CTX_List_t                      stListTail;
    void *                                   pstDevAttachSrc;
    DRV_DISP_CTX_DmaContain_t *              pstDmaCtx;
    HAL_DISP_PICTURE_NonLinearCurveConfig_t  stPictureNonLinearSetting;
    HAL_DISP_PICTURE_PqConfig_t              stPicturePqCfg;
    HAL_DISP_COLOR_Config_t                  stColorCfg[E_HAL_DISP_COLOR_CSC_ID_NUM];
    MI_DISP_IMPL_MhalPqInputConfig_t         stPqInputCfg;
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t stPnlUdParamCfg;
} DRV_DISP_CTX_DeviceContain_t;

typedef struct
{
    MI_U8                             bDevContainUsed[HAL_DISP_DEVICE_MAX];
    MI_U8                             bVidLayerContainUsed[HAL_DISP_VIDLAYER_MAX];
    MI_U8                             bInputPortContainUsed[HAL_DISP_INPUTPORT_MAX];
    MI_U8                             bDmaContainUsed[HAL_DISP_DMA_MAX];
    DRV_DISP_CTX_DeviceContain_t *    pstDevContain[HAL_DISP_DEVICE_MAX];
    DRV_DISP_CTX_VideoLayerContain_t *pstVidLayerContain[HAL_DISP_VIDLAYER_MAX];
    DRV_DISP_CTX_InputPortContain_t * pstInputPortContain[HAL_DISP_INPUTPORT_MAX];
    DRV_DISP_CTX_DmaContain_t *       pstDmaContain[HAL_DISP_DMA_MAX];
    MI_DISP_IMPL_MhalMemAllocConfig_t stMemAllcCfg;
    HAL_DISP_HwContain_t *            pstHalHwContain;
    HAL_DISP_ST_StrConfig_t           stStr;
} DRV_DISP_CTX_Contain_t;

typedef struct
{
    DRV_DISP_CTX_Type_e     enCtxType;
    MI_U32                  u32ContainIdx;
    MI_U32                  u32CtxIdx;
    MI_U8                   u8bCpuDirectAccess;
    DRV_DISP_CTX_Contain_t *pstCtxContain;
} DRV_DISP_CTX_Config_t;

typedef struct
{
    DRV_DISP_CTX_Type_e               enType;
    MI_U32                            u32DevId;
    DRV_DISP_CTX_Config_t *           pstBindCtx;
    MI_DISP_IMPL_MhalMemAllocConfig_t stMemAllcCfg;
} DRV_DISP_CTX_AllocConfig_t;
//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------
#ifndef _DRV_DISP_CTX_C_
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE MI_U8  DRV_DISP_CTX_Init(void);
INTERFACE MI_U8  DRV_DISP_CTX_DeInit(void);
INTERFACE MI_U8  DRV_DISP_CTX_Allocate(DRV_DISP_CTX_AllocConfig_t *pAllocCfg, DRV_DISP_CTX_Config_t **pCtx);
INTERFACE MI_U8  DRV_DISP_CTX_Free(DRV_DISP_CTX_Config_t *pCtx);
INTERFACE MI_U8  DRV_DISP_CTX_IsAllFree(void);
INTERFACE MI_U8  DRV_DISP_CTX_IsLastDeviceCtx(DRV_DISP_CTX_Config_t *pCtx);
INTERFACE MI_U8  DRV_DISP_CTX_SetCurCtx(DRV_DISP_CTX_Config_t *pCtx, MI_U32 u32Idx);
INTERFACE MI_U8  DRV_DISP_CTX_GetDeviceCurCtx(MI_U32 u32Idx, DRV_DISP_CTX_Config_t **pCtx);
INTERFACE MI_U32 DRV_DISP_CTX_GetDeviceContainIdx(MI_U32 u32DevId);
INTERFACE void   DRV_DISP_CTX_ListAddList(DRV_DISP_CTX_List_t *pstNew, DRV_DISP_CTX_List_t *pstPrev);
INTERFACE void   DRV_DISP_CTX_ListDelList(DRV_DISP_CTX_List_t *pstDel);

#undef INTERFACE

#endif
