/*
 * drv_disp_if.h- Sigmastar
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

#ifndef _DRV_DISP_IF_H_
#define _DRV_DISP_IF_H_

#include "disp_debug.h"

// mi fb use only
///< Define display output timing
typedef struct
{
    MI_U16 u16Htotal;      ///< Hoizontal total size
    MI_U16 u16Vtotal;      ///< Vertical total size
    MI_U16 u16HdeStart;    ///< Horizontal DE start
    MI_U16 u16VdeStart;    ///< Vertical DE start
    MI_U16 u16Width;       ///< Horizontal size
    MI_U16 u16Height;      ///< Vertical size
    MI_U8  bInterlaceMode; ///< 1:interlace , 0: non-interlace
    MI_U8  bYuvOutput;     ///< 1: YUV color, 0: RGB color
} MHAL_DISP_DisplayInfo_t;

#ifdef _DRV_DISP_IF_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif
INTERFACE MI_U8 DRV_DISP_IF_InitPanelConfig(MI_DISP_IMPL_MhalPanelConfig_t *pstPanelConfig, MI_U8 u8Size);
INTERFACE MI_U8 DRV_DISP_IF_DeviceCreateInstance(MI_DISP_IMPL_MhalMemAllocConfig_t *pstAlloc, MI_U32 u32DeviceId,
                                                 void **pDevCtx);
INTERFACE MI_U8 DRV_DISP_IF_DeviceDestroyInstance(void *pDevCtx);
INTERFACE MI_U8 DRV_DISP_IF_DeviceEnable(void *pDevCtx, MI_U8 bEnable);
INTERFACE MI_U8 DRV_DISP_IF_DeviceSetBackGroundColor(void *pDevCtx, MI_U32 u32BgColor);
INTERFACE MI_U8 DRV_DISP_IF_DeviceAddOutInterface(void *pDevCtx, MI_U32 u32Interface);
INTERFACE MI_U8 DRV_DISP_IF_DeviceSetOutputTiming(void *pDevCtx, MI_U32 u32Interface,
                                                  MI_DISP_IMPL_MhalDeviceTimingInfo_t *pstTimingInfo);
INTERFACE MI_U8 DRV_DISP_IF_DeviceSetColortemp(void *pDevCtx, MI_DISP_ColorTemperature_t *pstcolorTemp);
INTERFACE MI_U8 DRV_DISP_IF_DeviceSetCvbsParam(void *pDevCtx, MI_DISP_CvbsParam_t *pstCvbsInfo);
INTERFACE MI_U8 DRV_DISP_IF_DeviceGetCvbsParam(void *pDevCtx, MI_DISP_CvbsParam_t *pstCvbsInfo);
INTERFACE MI_U8 DRV_DISP_IF_DeviceSetHdmiParam(void *pDevCtx, MI_DISP_HdmiParam_t *pstHdmiInfo);
INTERFACE MI_U8 DRV_DISP_IF_DeviceGetHdmiParam(void *pDevCtx, MI_DISP_HdmiParam_t *pstHdmiInfo);
INTERFACE MI_U8 DRV_DISP_IF_DeviceSetVgaParam(void *pDevCtx, MI_DISP_VgaParam_t *pstVgaInfo);
INTERFACE MI_U8 DRV_DISP_IF_DeviceGetVgaParam(void *pDevCtx, MI_DISP_VgaParam_t *pstVgaInfo);
INTERFACE MI_U8 DRV_DISP_IF_DeviceSetGammaParam(void *pDevCtx, MI_DISP_GammaParam_t *pstGammaInfo);
INTERFACE MI_U8 DRV_DISP_IF_DeviceSetLcdParam(void *pDevCtx, MI_DISP_LcdParam_t *pstLcdInfo);
INTERFACE MI_U8 DRV_DISP_IF_DeviceGetLcdParam(void *pDevCtx, MI_DISP_LcdParam_t *pstLcdInfo);
INTERFACE MI_U8 DRV_DISP_IF_DeviceAttach(void *pSrcDevCtx, void *pDstDevCtx);
INTERFACE MI_U8 DRV_DISP_IF_DeviceDetach(void *pSrcDevCtx, void *pDstDevCtx);
INTERFACE MI_U8 DRV_DISP_IF_DeviceGetTimeZone(void *pDevCtx, MI_DISP_IMPL_MhalTimeZone_t *pstTimeZone);
INTERFACE MI_U8 DRV_DISP_IF_DeviceSetTimeZoneConfig(void *pDevCtx, MI_DISP_IMPL_MhalTimeZoneConfig_t *pstTimeZoneCfg);
INTERFACE MI_U8 DRV_DISP_IF_DeviceGetInstance(MI_U32 u32DeviceId, void **pDevCtx);
INTERFACE MI_U8 DRV_DISP_IF_DeviceGetDisplayInfo(void *pDevCtx, MHAL_DISP_DisplayInfo_t *pstDisplayInfo); // for fb
INTERFACE MI_U8 DRV_DISP_IF_DeviceSetPqConfig(void *pDevCtx, MI_DISP_IMPL_MhalPqConfig_t *pstPqCfg);
INTERFACE MI_U8 DRV_DISP_IF_DeviceGetHwCount(MI_U32 *pu32Count);
INTERFACE MI_U8 DRV_DISP_IF_SetDeviceConfig(MI_U32 u32DevId, MI_DISP_IMPL_MhalDeviceConfig_t *pstDevCfg);
INTERFACE MI_U8 DRV_DISP_IF_GetDeviceConfig(MI_U32 u32DevId, MI_DISP_IMPL_MhalDeviceConfig_t *pstDevCfg);
INTERFACE MI_U8 DRV_DISP_IF_DeviceGetCapabilityConfig(MI_U32                                     u32DevId,
                                                      MI_DISP_IMPL_MhalDeviceCapabilityConfig_t *pstDevCapCfg);
INTERFACE MI_U8 DRV_DISP_IF_DeviceGetInterfaceCapabilityConfig(
    MI_U32 u32Interface, MI_DISP_IMPL_MhalInterfaceCapabilityConfig_t *pstInterfaceCapCfg);
INTERFACE MI_U8 DRV_DISP_IF_ClkOn(void);
INTERFACE MI_U8 DRV_DISP_IF_ClkOff(void);
INTERFACE MI_U8 DRV_DISP_IF_Suspend(MI_DISP_IMPL_MhalPreSuspendConfig_t *pCfg);
INTERFACE MI_U8 DRV_DISP_IF_Resume(void);
INTERFACE MI_U8 DRV_DISP_IF_GetClk(void *pDevCtx, MI_U8 *pbEn, MI_U32 *pu32ClkRate, MI_U32 u32ClkNum);
INTERFACE MI_U8 DRV_DISP_IF_SetClk(void *pDevCtx, MI_U8 *pbEn, MI_U32 *pu32ClkRate, MI_U32 u32ClkNum);
INTERFACE MI_U8 DRV_DISP_IF_SetRegAccessConfig(void *pDevCtx, MI_DISP_IMPL_MhalRegAccessConfig_t *pstRegAccessCfg);
INTERFACE MI_U8 DRV_DISP_IF_SetRegFlipConfig(void *pDevCtx, MI_DISP_IMPL_MhalRegFlipConfig_t *pstRegFlipCfg);
INTERFACE MI_U8 DRV_DISP_IF_SetRegWaitDoneConfig(void *                                pDevCtx,
                                                 MI_DISP_IMPL_MhalRegWaitDoneConfig_t *pstRegWaitDoneCfg);
INTERFACE MI_U8 DRV_DISP_IF_SetCmdqInterfaceConfig(MI_U32 u32DevId, MI_DISP_IMPL_MhalCmdqInfConfig_t *pstCmdqInfCfg);
INTERFACE MI_U8 DRV_DISP_IF_VideoLayerCreateInstance(MI_DISP_IMPL_MhalMemAllocConfig_t *pstAlloc,
                                                     MI_DISP_IMPL_MhalVideoLayerType_e  eVidLayerType,
                                                     void **                            pVidLayerCtx);
INTERFACE MI_U8 DRV_DISP_IF_VideoLayerDestoryInstance(void *pVidLayerCtx);
INTERFACE MI_U8 DRV_DISP_IF_VideoLayerEnable(void *pVidLayerCtx, MI_U8 bEnable);
INTERFACE MI_U8 DRV_DISP_IF_VideoLayerBind(void *pVidLayerCtx, void *pDevCtx);
INTERFACE MI_U8 DRV_DISP_IF_VideoLayerUnBind(void *pVidLayerCtx, void *pDevCtx);
INTERFACE MI_U8 DRV_DISP_IF_VideoLayerSetAttr(void *pVidLayerCtx, MI_DISP_VideoLayerAttr_t *pstAttr);
INTERFACE MI_U8 DRV_DISP_IF_VideoLayerBufferFire(void *pVidLayerCtx);
INTERFACE MI_U8 DRV_DISP_IF_VideoLayerCheckBufferFired(void *pVidLayerCtx);
INTERFACE MI_U8 DRV_DISP_IF_VideoLayerSetCompress(void *pVidLayerCtx, MI_BOOL bCompress);
INTERFACE MI_U8 DRV_DISP_IF_VideoLayerSetPriority(void *pVidLayerCtx, MI_U32 u32Priority);
INTERFACE MI_U8 DRV_DISP_IF_VideoLayerGetCapabilityConfig(
    MI_DISP_IMPL_MhalVideoLayerType_e eVidLayerType, MI_DISP_IMPL_MhalVideoLayerCapabilityConfig_t *pstVidLayerCapCfg);
INTERFACE MI_U8 DRV_DISP_IF_InputPortCreateInstance(MI_DISP_IMPL_MhalMemAllocConfig_t *pstAlloc, void *pVidLayerCtx,
                                                    MI_U32 u32PortId, void **pCtx);
INTERFACE MI_U8 DRV_DISP_IF_InputPortDestroyInstance(void *pInputPortCtx);
INTERFACE MI_U8 DRV_DISP_IF_InputPortFlip(void *pInputPortCtx, MI_DISP_IMPL_MhalVideoFrameData_t *pstVideoFrameData);
INTERFACE MI_U8 DRV_DISP_IF_InputPortEnable(void *pInputPortCtx, MI_U8 bEnable);
INTERFACE MI_U8 DRV_DISP_IF_InputPortSetAttr(void *pInputPortCtx, MI_DISP_InputPortAttr_t *pstAttr);
INTERFACE MI_U8 DRV_DISP_IF_InputPortShow(void *pInputPortCtx);
INTERFACE MI_U8 DRV_DISP_IF_InputPortHide(void *pInputPortCtx);
INTERFACE MI_U8 DRV_DISP_IF_InputPortAttrBegin(void *pVidLayerCtx);
INTERFACE MI_U8 DRV_DISP_IF_InputPortAttrEnd(void *pVidLayerCtx);
INTERFACE MI_U8 DRV_DISP_IF_InputPortRotate(void *pInputPortCtx, MI_DISP_RotateConfig_t *pstRotateCfg);
INTERFACE MI_U8 DRV_DISP_IF_InputPortSetCropAttr(void *pInputPortCtx, MI_DISP_VidWinRect_t *pstCropAttr);
INTERFACE MI_U8 DRV_DISP_IF_InputPortSetRingBuffAttr(void *                             pInputPortCtx,
                                                     MI_DISP_IMPL_MhalRingBufferAttr_t *pstRingBufAttr);
INTERFACE MI_U8 DRV_DISP_IF_InputPortSetImiAddr(void *pInputPortCtx, MI_DISP_IMPL_MhalVideoFrameData_t *pstImiAddr);
INTERFACE MI_U8 DRV_DISP_IF_SetDbgLevel(void *p);
INTERFACE MI_U8 DRV_DISP_IF_DmaCreateInstance(MI_DISP_IMPL_MhalMemAllocConfig_t *pstAlloc, MI_U32 u32DmaId,
                                              void **pDmaCtx);
INTERFACE MI_U8 DRV_DISP_IF_DmaDestoryInstance(void *pDmaCtx);
INTERFACE MI_U8 DRV_DISP_IF_DmaBind(void *pDmaCtx, MI_DISP_IMPL_MhalDmaBindConfig_t *pstDmaBindCfg);
INTERFACE MI_U8 DRV_DISP_IF_DmaUnBind(void *pDmaCtx, MI_DISP_IMPL_MhalDmaBindConfig_t *pstDmaBindCfg);
INTERFACE MI_U8 DRV_DISP_IF_DmaSetAttr(void *pDmaCtx, MI_DISP_IMPL_MhalDmaAttrConfig_t *pstDmatAttrCfg);
INTERFACE MI_U8 DRV_DISP_IF_DmaSetBufferAttr(void *                                  pDmaCtx,
                                             MI_DISP_IMPL_MhalDmaBufferAttrConfig_t *pstDmaBufferAttrCfg);
INTERFACE MI_U8 DRV_DISP_IF_DmaGetHwCount(MI_U32 *pu32Count);
INTERFACE MI_U8 DRV_DISP_IF_DmaGetCapabilityConfig(MI_U32                                  u32DmaId,
                                                   MI_DISP_IMPL_MhalDmaCapabiliytConfig_t *pstDmaCapCfg);

INTERFACE MI_U8 DRV_DISP_IF_SetPnlUnifiedParamConfig(void *pCtx, MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pUdParamCfg);
INTERFACE MI_U8 DRV_DISP_IF_GetPnlUnifiedParamConfig(void *pCtx, MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pUdParamCfg);
INTERFACE MI_U8 DRV_DISP_IF_SetPnlPowerConfig(void *pDevCtx, MI_DISP_IMPL_MhalPnlPowerConfig_t *pstPowerCfg);
INTERFACE MI_U8 DRV_DISP_IF_SetPnlMipiDsiWriteCmdConfig(
    void *pDevCtx, MI_DISP_IMPL_MhalPnlMipiDsiWriteCmdConfig_t *pstMipiDsiWriteCmdCfg);
INTERFACE MI_U8
DRV_DISP_IF_SetPnlMipiDsiReadCmdConfig(void *pDevCtx, MI_DISP_IMPL_MhalPnlMipiDsiReadCmdConfig_t *pstMipiDsiReadCmdCfg);
INTERFACE MI_U8 DRV_DISP_IF_GetFpllLockStatus(void *pDevCtx, MI_U8 *pu8LockStatus);

#if 0
INTERFACE void DRV_DISP_DEBUG_InPortStore(DRV_DISP_OS_StrConfig_t *pstStringCfg);
INTERFACE MI_U32 DRV_DISP_DEBUG_InPortShow(MI_U8 *p8DstBuf);
INTERFACE void DRV_DISP_DEBUG_VidLayerStore(DRV_DISP_OS_StrConfig_t *pstStringCfg);
INTERFACE MI_U32 DRV_DISP_DEBUG_VidLayerShow(MI_U8 *p8DstBuf);
#endif
INTERFACE MI_U8 DRV_DISP_DEBUG_SetClk(MI_U8 *p8ClkName, MI_U32 u32Enable, MI_U32 u32ClkRate, MI_U32 *pu32ClkRate);
INTERFACE MI_U8 DRV_DISP_DEBUG_GetClk(MI_U8 *pbClkEn, MI_U32 *pu32ClkRate);
INTERFACE MI_U8 DRV_DISP_DEBUG_SetDbgmgFlag(MI_U32 u32Level);
INTERFACE MI_U8 DRV_DISP_DEBUG_GetDbgmgFlag(void *m);
INTERFACE MI_U8 DRV_DISP_DEBUG_SetFunc(MI_U8 *p8FuncType, MI_U32 u32val, MI_U32 u32DevId);
INTERFACE MI_U8 DRV_DISP_DEBUG_GetFunc(MI_U8 u8DevIdx, void *pstDevCfg);
INTERFACE MI_U8 DRV_DISP_DEBUG_SetTurnDrv(MI_U32 u32Argc, MI_U8 *p8TrimName, MI_U16 *pu16Trim);
INTERFACE MI_U8 DRV_DISP_DEBUG_GetTurnDrv(MI_U16 *pu16VgaTrim, MI_U16 *pu16CvbsTrim);
#if DISP_SYSFS_PQ_EN
INTERFACE void   DRV_DISP_DEBUG_SetPq(DRV_DISP_OS_StrConfig_t *pstStringCfg);
INTERFACE MI_U32 DRV_DISP_DEBUG_GetPqBinName(MI_U8 *p8DstBuf);
#endif
INTERFACE MI_U8  DRV_DISP_DEBUG_GetIrqHist(void *pstDispIrqHist);
INTERFACE MI_U8  DRV_DISP_DEBUG_GetDeviceInstance(MI_U32 u32DeviceId, void **ppstDevCtx);
INTERFACE MI_U32 DRV_DISP_DEBUG_GetCrc16(MI_U32 u32DmaId, MI_U64 *p64Crc16);
INTERFACE MI_U32 DRV_DISP_DEBUG_GetAffCnt(MI_U32 u32Dev, MI_U32 u32Vid, MI_U32 *p32Cnt);
INTERFACE void   DRV_DISP_DEBUG_Str(void *pCtx, MI_U8 bResume);

#undef INTERFACE
#endif
