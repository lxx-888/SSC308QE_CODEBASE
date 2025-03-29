/*
 * disp.c - Sigmastar
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

#include <command.h>
#include <common.h>
#include <malloc.h>
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#if defined(CONFIG_SSTAR_DISP)
#include "cam_os_wrapper.h"
#include "mi_common_datatype.h"
#include "mi_disp_impl_datatype.h"
#include "mhal_common.h"
#include "mhal_cmdq.h"
#include "drv_disp_os.h"
#include "hal_disp_chip.h"
#include "hal_disp_st.h"
#include "disp_debug.h"
#include "drv_disp_if.h"
#endif
#if defined(CONFIG_SSTAR_HDMITX)
#include "mhal_hdmitx_datatype.h"
#include "mhal_hdmitx.h"
#endif
typedef enum
{
    E_DISP_MODULE_NONE   = 0x0000,
    E_DISP_MODULE_DISP   = 0x0001,
    E_DISP_MODULE_HDMITX = 0x0002,
    E_DISP_MODULE_DAC    = 0x0004,
    E_DISP_MODULE_DMA    = 0x0008,
    E_DISP_MODULE_PNL    = 0x0010,
} DispModuleType_e;

typedef struct
{
    MI_U8  aPanelName[20];
    MI_U16 PanelType;
    MI_U16 u16Tgn_HSyncWidth;
    MI_U16 u16Tgn_HSyncBackPorch;
    MI_U16 u16Tgn_VSyncWidth;
    MI_U16 u16Tgn_VSyncBackPorch;
    MI_U16 u16Tgn_HStart;
    MI_U16 u16Tgn_VStart;
    MI_U16 u16Tgn_Width;
    MI_U16 u16Tgn_Height;
    MI_U16 u16Tgn_HTotal;
    MI_U16 u16Tgn_VTotal;
    MI_U32 u32Tgn_DCLK;
    MI_U16 u16Tgn_InvDCLK;
    MI_U16 u16Tgn_InvDE;
    MI_U16 u16Tgn_InvHSync;
    MI_U16 u16Tgn_InvVSync;
    MI_U16 u16Tgn_OutputFormatBitMode;
    MI_U16 u16Tgn_PadMux;
} BootDispPnlUnifiedParam_t;

typedef struct
{
    MI_U32                     u32DispDevIntf;
    MI_U32                     u32PathId;
    BootDispPnlUnifiedParam_t *pPnlUnfiedParam;
#if defined(CONFIG_SSTAR_DISP)
    MI_DISP_OutputTiming_e eDispTiming;
#endif
#if defined(CONFIG_SSTAR_HDMITX)
    MhaHdmitxTimingResType_e eHdmiTiming;
    MhalHdmitxColorType_e    eInColor;
    MhalHdmitxColorType_e    eOutColor;
#endif

    MI_U16 u16InputWidth;
    MI_U16 u16InputHeight;
    MI_U16 u16InputStide;
    MI_U16 u16OutputWidth;
    MI_U16 u16OutputHeight;
    MI_U64 u64PhyAddr;
} BootDispInputParam_t;

typedef struct
{
    void * pDevCtx;
    void * pVidLayerCtx[HAL_DISP_VIDLAYER_MAX];
    void * pInputPortCtx[HAL_DISP_INPUTPORT_MAX];
    MI_U32 u32Interface;
    MI_U32 u32DevId;
} BootDispContext_t;
typedef struct
{
    MI_S32                           s32DmaId;
    void *                           pDmaCtx; // DrvDispCtxConfig_t
    MI_S32                           s32Src;
    MI_S32                           s32Dest;
    MI_DISP_IMPL_MhalDmaOutputMode_e enMode;
} BootDispDmaCtxConfig_t;

typedef struct
{
#if defined(CONFIG_SSTAR_HDMITX)
    MhalHdmitxColorType_e        enInColor;
    MhalHdmitxColorType_e        enOutColor;
    MhalHdmitxOutpuModeType_e    enOuputMode;
    MhalHdmitxColorDepthType_e   enColorDepth;
    MhaHdmitxTimingResType_e     enOutputTiming;
    MhalHdmitxAudioFreqType_e    enAudioFrq;
    MhalHdmitxAudioChannelType_e enAudioCh;
#endif
    bool  bHpdDetect;
    void *pHdmitxCtx;
} BootDispHdmiTxContext_t;

typedef struct
{
#if defined(CONFIG_SSTAR_DISP)
    MI_DISP_OutputTiming_e enTiminId;
#endif
    MI_U16 u16HsyncWidht;
    MI_U16 u16HsyncBacPorch;

    MI_U16 u16VsyncWidht;
    MI_U16 u16VsyncBacPorch;

    MI_U16 u16Hstart;
    MI_U16 u16Vstart;
    MI_U16 u16Hactive;
    MI_U16 u16Vactive;

    MI_U16 u16Htotal;
    MI_U16 u16Vtotal;
    MI_U32 u32Dclkhz;
} BootDispTimingConfig_t;

static BootDispTimingConfig_t g_stDispTimingTable[] = {
    {E_MI_DISP_OUTPUT_1080P60, 44, 148, 5, 36, 192, 41, 1920, 1080, 2200, 1125, 148000000},

    {E_MI_DISP_OUTPUT_1080P50, 44, 148, 5, 36, 192, 41, 1920, 1080, 2640, 1125, 148000000},

    {E_MI_DISP_OUTPUT_720P50, 40, 220, 5, 20, 260, 25, 1280, 720, 1980, 750, 74000000},

    {E_MI_DISP_OUTPUT_720P60, 40, 220, 5, 20, 260, 25, 1280, 720, 1650, 750, 74000000},

    {E_MI_DISP_OUTPUT_480P60, 62, 60, 6, 30, 122, 36, 720, 480, 858, 525, 27000000},

    {E_MI_DISP_OUTPUT_576P50, 64, 68, 4, 39, 132, 44, 720, 5760, 864, 625, 27000000},

    {E_MI_DISP_OUTPUT_1024x768_60, 136, 160, 6, 29, 296, 35, 1024, 768, 1344, 806, 65000000},

    {E_MI_DISP_OUTPUT_1366x768_60, 143, 215, 3, 24, 358, 27, 1366, 768, 1792, 798, 86000000},

    {E_MI_DISP_OUTPUT_1440x900_60, 152, 232, 6, 25, 384, 31, 1440, 900, 1904, 934, 106000000},

    {E_MI_DISP_OUTPUT_1280x800_60, 128, 200, 6, 22, 328, 28, 1280, 800, 1680, 831, 84000000},

    {E_MI_DISP_OUTPUT_1280x1024_60, 112, 248, 3, 38, 360, 41, 1280, 1024, 1688, 1066, 108000000},

    {E_MI_DISP_OUTPUT_1680x1050_60, 176, 280, 6, 30, 456, 36, 1680, 1050, 2240, 1089, 146000000},

    {E_MI_DISP_OUTPUT_1600x1200_60, 192, 304, 3, 46, 496, 49, 1600, 1200, 2160, 1250, 162000000},

    {E_MI_DISP_OUTPUT_NTSC, 137, 1, 44, 1, 138, 45, 720, 480, 858, 525, 27000000},

    {E_MI_DISP_OUTPUT_PAL, 64, 68, 3, 59, 132, 44, 720, 576, 864, 625, 27000000},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {E_MI_DISP_OUTPUT_USER, 48, 46, 4, 23, 98, 27, 1024, 600, 1344, 635, 51000000},
};

BootDispContext_t       g_stDispCtx[HAL_DISP_DEVICE_MAX];
BootDispDmaCtxConfig_t  g_stDmaCtx[HAL_DISP_DMA_MAX];
BootDispHdmiTxContext_t g_stHdmitxCtx;
static MI_U8            g_bInit = 0;

#if defined(CONFIG_SSTAR_HDMITX)

MhalHdmitxColorType_e _DispTransHdmiColor(char *pColor)
{
    MhalHdmitxColorType_e eColor;

    eColor = !strcmp(pColor, "rgb")   ? E_MHAL_HDMITX_COLOR_RGB444
             : !strcmp(pColor, "yuv") ? E_MHAL_HDMITX_COLOR_YUV444
                                      : E_MHAL_HDMITX_COLOR_AUTO;
    return eColor;
}

MhaHdmitxTimingResType_e _DispTransHdmiTiming(char *pTiming)
{
    MhaHdmitxTimingResType_e eTiming;

    eTiming = !strcmp(pTiming, "480p60")     ? E_MHAL_HDMITX_RES_720X480P_60HZ
              : !strcmp(pTiming, "576p50")   ? E_MHAL_HDMITX_RES_720X576P_50HZ
              : !strcmp(pTiming, "720p50")   ? E_MHAL_HDMITX_RES_1280X720P_50HZ
              : !strcmp(pTiming, "720p60")   ? E_MHAL_HDMITX_RES_1280X720P_60HZ
              : !strcmp(pTiming, "1080p50")  ? E_MHAL_HDMITX_RES_1920X1080P_50HZ
              : !strcmp(pTiming, "1080p60")  ? E_MHAL_HDMITX_RES_1920X1080P_60HZ
              : !strcmp(pTiming, "1080p120") ? E_MHAL_HDMITX_RES_1920X1080P_60HZ
                                             : E_MHAL_HDMITX_RES_MAX;
    return eTiming;
}
void _DispHdmitxCreateInstance(void)
{
    if (g_stHdmitxCtx.pHdmitxCtx == NULL)
    {
        MhalHdmitxCreateInstance(&g_stHdmitxCtx.pHdmitxCtx, 0);
    }
}
void _DispHdmitxDestoryInstance(void)
{
    if (g_stHdmitxCtx.pHdmitxCtx)
    {
        MhalHdmitxDestroyInstance(g_stHdmitxCtx.pHdmitxCtx);
        g_stHdmitxCtx.pHdmitxCtx = NULL;
    }
}

void _DispHdmitxShow(BootDispInputParam_t *pstInputParam)
{
    BootDispHdmiTxContext_t *pstHdmitxCtx = &g_stHdmitxCtx;
    MhalHdmitxSignalConfig_t stSignalCfg;
    MhalHdmitxMuteConfig_t   stMuteCfg;
    MhalHdmitxAttrConfig_t   stAttrCfg;

    pstHdmitxCtx->bHpdDetect     = 1;
    pstHdmitxCtx->enInColor      = pstInputParam->eInColor;
    pstHdmitxCtx->enOutColor     = pstInputParam->eOutColor;
    pstHdmitxCtx->enOutputTiming = pstInputParam->eHdmiTiming;
    pstHdmitxCtx->enColorDepth   = E_MHAL_HDMITX_CD_24_BITS;
    pstHdmitxCtx->enOuputMode    = E_MHAL_HDMITX_OUTPUT_MODE_HDMI;
    pstHdmitxCtx->enAudioFrq     = E_MHAL_HDMITX_AUDIO_FREQ_48K;
    pstHdmitxCtx->enAudioCh      = E_MHAL_HDMITX_AUDIO_CH_2;

    _DispHdmitxCreateInstance();

    stMuteCfg.enType = E_MHAL_HDMITX_MUTE_AUDIO | E_MHAL_HDMITX_MUTE_VIDEO | E_MHAL_HDMITX_MUTE_AVMUTE;
    stMuteCfg.bMute  = 1;
    MhalHdmitxSetMute(pstHdmitxCtx->pHdmitxCtx, &stMuteCfg);

    stSignalCfg.bEn = 0;
    MhalHdmitxSetSignal(pstHdmitxCtx->pHdmitxCtx, &stSignalCfg);

    stAttrCfg.bVideoEn     = 1;
    stAttrCfg.enOutColor   = pstHdmitxCtx->enOuputMode;
    stAttrCfg.enInColor    = pstHdmitxCtx->enInColor;
    stAttrCfg.enOutColor   = pstHdmitxCtx->enOutColor;
    stAttrCfg.enOutputMode = pstHdmitxCtx->enOuputMode;
    stAttrCfg.enColorDepth = pstHdmitxCtx->enColorDepth;
    stAttrCfg.enTiming     = pstHdmitxCtx->enOutputTiming;

    stAttrCfg.bAudioEn    = 1;
    stAttrCfg.enAudioFreq = pstHdmitxCtx->enAudioFrq;
    stAttrCfg.enAudioCh   = pstHdmitxCtx->enAudioCh;
    stAttrCfg.enAudioFmt  = E_MHAL_HDMITX_AUDIO_FORMAT_PCM;
    stAttrCfg.enAudioCode = E_MHAL_HDMITX_AUDIO_CODING_PCM;

    MhalHdmitxSetAttr(pstHdmitxCtx->pHdmitxCtx, &stAttrCfg);

    stSignalCfg.bEn = 1;
    MhalHdmitxSetSignal(pstHdmitxCtx->pHdmitxCtx, &stSignalCfg);

    stMuteCfg.enType = E_MHAL_HDMITX_MUTE_AUDIO | E_MHAL_HDMITX_MUTE_VIDEO | E_MHAL_HDMITX_MUTE_AVMUTE;
    stMuteCfg.bMute  = 0;
    MhalHdmitxSetMute(pstHdmitxCtx->pHdmitxCtx, &stMuteCfg);
}
#endif

#if defined(CONFIG_SSTAR_DISP)
MI_DISP_OutputTiming_e _DispTransDispTimingId(char *pTiming)
{
    MI_DISP_OutputTiming_e eTiming;

    eTiming = !strcmp(pTiming, "480p60")    ? E_MI_DISP_OUTPUT_480P60
              : !strcmp(pTiming, "576p50")  ? E_MI_DISP_OUTPUT_576P50
              : !strcmp(pTiming, "720p50")  ? E_MI_DISP_OUTPUT_720P50
              : !strcmp(pTiming, "720p60")  ? E_MI_DISP_OUTPUT_720P60
              : !strcmp(pTiming, "1080p50") ? E_MI_DISP_OUTPUT_1080P50
              : !strcmp(pTiming, "1080p60") ? E_MI_DISP_OUTPUT_1080P60
              : !strcmp(pTiming, "ntsc")    ? E_MI_DISP_OUTPUT_NTSC
              : !strcmp(pTiming, "pal")     ? E_MI_DISP_OUTPUT_PAL
                                            : E_MI_DISP_OUTPUT_USER;
    return eTiming;
}
void _DispCreateInstance(MI_U32 u32Id)
{
    BootDispContext_t *pstDispCtx = &g_stDispCtx[u32Id];

    if (pstDispCtx->pDevCtx == NULL)
    {
        MI_DISP_IMPL_MhalMemAllocConfig_t stDispAlloc;

        stDispAlloc.alloc = NULL;
        stDispAlloc.free  = NULL;

        DRV_DISP_IF_DeviceCreateInstance(&stDispAlloc, u32Id, &pstDispCtx->pDevCtx);

        DRV_DISP_IF_VideoLayerCreateInstance(&stDispAlloc, E_MI_DISP_VIDEOLAYER_SINGLEWIN,
                                             &pstDispCtx->pVidLayerCtx[0]);

        DRV_DISP_IF_VideoLayerBind(pstDispCtx->pVidLayerCtx[0], pstDispCtx->pDevCtx);

        DRV_DISP_IF_InputPortCreateInstance(&stDispAlloc, pstDispCtx->pVidLayerCtx[0], 0,
                                            &pstDispCtx->pInputPortCtx[0]);
    }
}
void _DispDestoryInstance(MI_U32 u32Id)
{
    BootDispContext_t *pstDispCtx = &g_stDispCtx[u32Id];

    if (pstDispCtx->pDevCtx)
    {
        DRV_DISP_IF_InputPortDestroyInstance(pstDispCtx->pInputPortCtx[0]);
        DRV_DISP_IF_VideoLayerUnBind(pstDispCtx->pVidLayerCtx[0], pstDispCtx->pDevCtx);
        DRV_DISP_IF_VideoLayerDestoryInstance(pstDispCtx->pVidLayerCtx[0]);
        DRV_DISP_IF_DeviceDestroyInstance(pstDispCtx->pDevCtx);
        pstDispCtx->pDevCtx = NULL;
    }
}
void _DispRegFlipAndWaitDone(MI_U32 u32Id)
{
    MI_DISP_IMPL_MhalRegFlipConfig_t     stRegFlipCfg;
    MI_DISP_IMPL_MhalRegWaitDoneConfig_t stRegWaitDoneCfg;
    BootDispContext_t *                  pstDispCtx = NULL;

    pstDispCtx = &g_stDispCtx[u32Id];

    stRegFlipCfg.bEnable  = 1;
    stRegFlipCfg.pCmdqInf = NULL;
    DRV_DISP_IF_SetRegFlipConfig(pstDispCtx->pDevCtx, &stRegFlipCfg);

    stRegWaitDoneCfg.pCmdqInf    = NULL;
    stRegWaitDoneCfg.u32WaitType = 0;
    DRV_DISP_IF_SetRegWaitDoneConfig(pstDispCtx->pDevCtx, &stRegWaitDoneCfg);
}

void _DispPnlInit(void)
{
    MI_DISP_IMPL_MhalPanelConfig_t stPnlCfg[E_MI_DISP_OUTPUT_MAX];
    MI_U16                         i;
    MI_U32                         u32Cnt = 0;

    u32Cnt = sizeof(g_stDispTimingTable) / sizeof(BootDispTimingConfig_t);
    if (u32Cnt > E_MI_DISP_OUTPUT_MAX)
    {
        DISP_ERR("%s %d:: Timing Talbe is bigger than %d\n", __FUNCTION__, __LINE__, E_MI_DISP_OUTPUT_MAX);
        return;
    }
    memset(stPnlCfg, 0, sizeof(MI_DISP_IMPL_MhalPanelConfig_t) * E_MI_DISP_OUTPUT_MAX);
    for (i = 0; i < u32Cnt; i++)
    {
        stPnlCfg[i].bValid                                             = g_stDispTimingTable[i].u32Dclkhz ? 1 : 0;
        stPnlCfg[i].stPnlUniParamCfg.eDisplayTiming                    = g_stDispTimingTable[i].enTiminId;
        stPnlCfg[i].stPnlUniParamCfg.u8TgnTimingFlag                   = 1;
        stPnlCfg[i].stPnlUniParamCfg.stTgnTimingInfo.u16HSyncWidth     = g_stDispTimingTable[i].u16HsyncWidht;
        stPnlCfg[i].stPnlUniParamCfg.stTgnTimingInfo.u16HSyncBackPorch = g_stDispTimingTable[i].u16HsyncBacPorch;
        stPnlCfg[i].stPnlUniParamCfg.stTgnTimingInfo.u16VSyncWidth     = g_stDispTimingTable[i].u16VsyncWidht;
        stPnlCfg[i].stPnlUniParamCfg.stTgnTimingInfo.u16VSyncBackPorch = g_stDispTimingTable[i].u16VsyncBacPorch;
        stPnlCfg[i].stPnlUniParamCfg.stTgnTimingInfo.u16HStart         = g_stDispTimingTable[i].u16Hstart;
        stPnlCfg[i].stPnlUniParamCfg.stTgnTimingInfo.u16VStart         = g_stDispTimingTable[i].u16Vstart;
        stPnlCfg[i].stPnlUniParamCfg.stTgnTimingInfo.u16HActive        = g_stDispTimingTable[i].u16Hactive;
        stPnlCfg[i].stPnlUniParamCfg.stTgnTimingInfo.u16VActive        = g_stDispTimingTable[i].u16Vactive;
        stPnlCfg[i].stPnlUniParamCfg.stTgnTimingInfo.u16HTotal         = g_stDispTimingTable[i].u16Htotal;
        stPnlCfg[i].stPnlUniParamCfg.stTgnTimingInfo.u16VTotal         = g_stDispTimingTable[i].u16Vtotal;
        stPnlCfg[i].stPnlUniParamCfg.stTgnTimingInfo.u32Dclk           = g_stDispTimingTable[i].u32Dclkhz;
    }

    DRV_DISP_IF_InitPanelConfig(stPnlCfg, u32Cnt);
}

bool _DispGetTimingTblIdx(MI_DISP_OutputTiming_e eTiming, u8 *pu8Idx)
{
    MI_U16  i;
    MI_U32  u32Cnt = 0;
    MI_BOOL bFound = 0;
    u32Cnt         = sizeof(g_stDispTimingTable) / sizeof(BootDispTimingConfig_t);

    for (i = 0; i < u32Cnt; i++)
    {
        if (g_stDispTimingTable[i].enTiminId == eTiming)
        {
            bFound  = 1;
            *pu8Idx = i;
            break;
        }
    }
    return bFound;
}

void _DispShow(BootDispInputParam_t *pstInputParam)
{
    BootDispContext_t *                 pstDispCtx = NULL;
    MI_DISP_IMPL_MhalDeviceTimingInfo_t stDeviceTimingInfo;
    MI_U32                              u32DevId;
    static MI_U32                       u32OriInterface;
    static MI_U8                        u8Enable[HAL_DISP_DEVICE_MAX];
    MI_U8                               u8PnlIdx;
    MI_DISP_VideoLayerAttr_t            stVideoLayerAttr;
    MI_DISP_InputPortAttr_t             stInputPortAttr;
    MI_DISP_IMPL_MhalVideoFrameData_t   stVideoFrameData;

    u32DevId = HAL_DISP_MAPPING_DEVICEID_FROM_MI(pstInputParam->u32PathId);
    _DispCreateInstance(u32DevId);
    pstDispCtx = &g_stDispCtx[u32DevId];
    _DispPnlInit();

    memset(&stDeviceTimingInfo, 0, sizeof(MI_DISP_IMPL_MhalDeviceTimingInfo_t));
    stDeviceTimingInfo.eTimeType = pstInputParam->eDispTiming;
    pstDispCtx->u32DevId         = u32DevId;
    pstDispCtx->u32Interface     = pstInputParam->u32DispDevIntf;
    DRV_DISP_IF_DeviceSetBackGroundColor(pstDispCtx->pDevCtx, 0x800080);
    if (pstDispCtx->u32Interface != u32OriInterface && u8Enable[u32DevId])
    {
        DRV_DISP_IF_DeviceEnable(pstDispCtx->pDevCtx, 0);
    }
    DRV_DISP_IF_DeviceAddOutInterface(pstDispCtx->pDevCtx, pstInputParam->u32DispDevIntf);
    DRV_DISP_IF_DeviceSetOutputTiming(pstDispCtx->pDevCtx, pstInputParam->u32DispDevIntf, &stDeviceTimingInfo);

    if (pstInputParam->u32DispDevIntf == HAL_DISP_INTF_TTL
        && _DispGetTimingTblIdx(pstInputParam->eDispTiming, &u8PnlIdx))
    {
        MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t stUnifiedParamCfg;
        memset(&stUnifiedParamCfg, 0, sizeof(MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t));

        stUnifiedParamCfg.eIntfType      = E_MI_DISP_INTF_TTL;
        stUnifiedParamCfg.eDisplayTiming = g_stDispTimingTable[u8PnlIdx].enTiminId;

        stUnifiedParamCfg.u8TgnTimingFlag                   = 1;
        stUnifiedParamCfg.stTgnTimingInfo.u16HSyncWidth     = g_stDispTimingTable[u8PnlIdx].u16HsyncWidht;
        stUnifiedParamCfg.stTgnTimingInfo.u16HSyncBackPorch = g_stDispTimingTable[u8PnlIdx].u16HsyncBacPorch;
        stUnifiedParamCfg.stTgnTimingInfo.u16VSyncWidth     = g_stDispTimingTable[u8PnlIdx].u16VsyncWidht;
        stUnifiedParamCfg.stTgnTimingInfo.u16VSyncBackPorch = g_stDispTimingTable[u8PnlIdx].u16VsyncBacPorch;
        stUnifiedParamCfg.stTgnTimingInfo.u16HStart         = g_stDispTimingTable[u8PnlIdx].u16Hstart;
        stUnifiedParamCfg.stTgnTimingInfo.u16VStart         = g_stDispTimingTable[u8PnlIdx].u16Vstart;
        stUnifiedParamCfg.stTgnTimingInfo.u16HActive        = g_stDispTimingTable[u8PnlIdx].u16Hactive;
        stUnifiedParamCfg.stTgnTimingInfo.u16VActive        = g_stDispTimingTable[u8PnlIdx].u16Vactive;
        stUnifiedParamCfg.stTgnTimingInfo.u16HTotal         = g_stDispTimingTable[u8PnlIdx].u16Htotal;
        stUnifiedParamCfg.stTgnTimingInfo.u16VTotal         = g_stDispTimingTable[u8PnlIdx].u16Vtotal;
        stUnifiedParamCfg.stTgnTimingInfo.u32Dclk           = g_stDispTimingTable[u8PnlIdx].u32Dclkhz;

        stUnifiedParamCfg.u8TgnPadMuxFlag = 1;
        stUnifiedParamCfg.u16PadMux       = 1;
        DRV_DISP_IF_SetPnlUnifiedParamConfig(pstDispCtx->pDevCtx, &stUnifiedParamCfg);

        memset(&stVideoLayerAttr, 0, sizeof(MI_DISP_VideoLayerAttr_t));
        stVideoLayerAttr.ePixFormat                  = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        stVideoLayerAttr.stVidLayerDispWin.u16X      = 0;
        stVideoLayerAttr.stVidLayerDispWin.u16Y      = 0;
        stVideoLayerAttr.stVidLayerDispWin.u16Width  = g_stDispTimingTable[u8PnlIdx].u16Hactive;
        stVideoLayerAttr.stVidLayerDispWin.u16Height = g_stDispTimingTable[u8PnlIdx].u16Vactive;

        stVideoLayerAttr.stVidLayerSize.u16Width  = g_stDispTimingTable[u8PnlIdx].u16Hactive;
        stVideoLayerAttr.stVidLayerSize.u16Height = g_stDispTimingTable[u8PnlIdx].u16Vactive;
        DRV_DISP_IF_VideoLayerSetAttr(pstDispCtx->pVidLayerCtx[0], &stVideoLayerAttr);
        DRV_DISP_IF_VideoLayerEnable(pstDispCtx->pVidLayerCtx[0], 1);

        memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
        stInputPortAttr.eDecompressMode     = E_MI_SYS_COMPRESS_MODE_NONE;
        stInputPortAttr.stDispWin.u16X      = 0;
        stInputPortAttr.stDispWin.u16Y      = 0;
        stInputPortAttr.stDispWin.u16Width  = g_stDispTimingTable[u8PnlIdx].u16Hactive;
        stInputPortAttr.stDispWin.u16Height = g_stDispTimingTable[u8PnlIdx].u16Vactive;
        stInputPortAttr.u16SrcWidth         = g_stDispTimingTable[u8PnlIdx].u16Hactive;
        stInputPortAttr.u16SrcHeight        = g_stDispTimingTable[u8PnlIdx].u16Vactive;

        memset(&stVideoFrameData, 0, sizeof(MI_DISP_IMPL_MhalVideoFrameData_t));
        stVideoFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
        stVideoFrameData.ePixelFormat  = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        stVideoFrameData.eTileMode     = E_MI_SYS_FRAME_TILE_MODE_NONE;
        stVideoFrameData.aPhyAddr[0]   = 0x100000;
        stVideoFrameData.aPhyAddr[1] =
            0x100000 + g_stDispTimingTable[u8PnlIdx].u16Hactive * g_stDispTimingTable[u8PnlIdx].u16Vactive;
        stVideoFrameData.au32Stride[0] =
            g_stDispTimingTable[u8PnlIdx].u16Hactive * g_stDispTimingTable[u8PnlIdx].u16Vactive;
        stVideoFrameData.au32Stride[1] = stVideoFrameData.au32Stride[1];

        DRV_DISP_IF_InputPortSetAttr(pstDispCtx->pInputPortCtx[0], &stInputPortAttr);
        DRV_DISP_IF_InputPortFlip(pstDispCtx->pInputPortCtx[0], &stVideoFrameData);
        DRV_DISP_IF_InputPortEnable(pstDispCtx->pInputPortCtx[0], 1);
    }

    DRV_DISP_IF_DeviceEnable(pstDispCtx->pDevCtx, 1);

    _DispRegFlipAndWaitDone(u32DevId);
    u32OriInterface    = pstDispCtx->u32Interface;
    u8Enable[u32DevId] = 1;
}
void _DispParsingDmaParam(int argc, char *const argv[], BootDispInputParam_t *pstInputParam)
{
    if (argc >= 12)
    {
    }
    else
    {
        DISP_ERR(
            "disp dma [SrcId][DestId][In Type][In Widht] [In Height] [Out Type] [Out Mode] [Out Widht] [Out Height] "
            "[RING HEIHGT] {[Comp] [Fmt] [Cnt] }");
    }
}
#endif

bool _DispParsingHdmiParam(int argc, char *const argv[], BootDispInputParam_t *pstInputParam)
{
#define DISP_HDMI_PARAM_NUM   6
#define DISP_HDMI_PARAM_START 3

    bool bRet = 0;
    if (argc == DISP_HDMI_PARAM_NUM || argc == (DISP_HDMI_PARAM_NUM + 1))
    {
        pstInputParam->u32DispDevIntf = !strcmp(argv[DISP_HDMI_PARAM_START - 1], "vgahdmi")
                                            ? (HAL_DISP_INTF_HDMI) | HAL_DISP_INTF_VGA
                                            : HAL_DISP_INTF_HDMI;

#if defined(CONFIG_SSTAR_DISP)
        pstInputParam->eDispTiming = _DispTransDispTimingId(argv[DISP_HDMI_PARAM_START]);
#endif
#if defined(CONFIG_SSTAR_HDMITX)
        pstInputParam->eHdmiTiming = _DispTransHdmiTiming(argv[DISP_HDMI_PARAM_START]);
        pstInputParam->eInColor    = _DispTransHdmiColor(argv[DISP_HDMI_PARAM_START + 1]);
        pstInputParam->eOutColor   = _DispTransHdmiColor(argv[DISP_HDMI_PARAM_START + 2]);
#endif
        pstInputParam->u32PathId =
            (argc == (DISP_HDMI_PARAM_NUM + 1)) ? simple_strtol(argv[DISP_HDMI_PARAM_START + 3], NULL, 10) : 0;
        bRet = 1;
    }
    else
    {
        DISP_ERR("disp show hdmi  [Timing] [IN COLOR] [OUT COLOR] [PATH ID]\n");
    }
    return bRet;
}

bool _DispParsingVgaCvbsParam(int argc, char *const argv[], BootDispInputParam_t *pstInputParam)
{
#define DISP_VGA_CVBS_PARAM_NUM   4
#define DISP_VGA_CVBS_PARAM_START 3

    bool bRet = 0;

    if (argc == DISP_VGA_CVBS_PARAM_NUM || argc == (DISP_VGA_CVBS_PARAM_NUM + 1))
    {
        pstInputParam->u32DispDevIntf =
            !strcmp(argv[DISP_VGA_CVBS_PARAM_START - 1], "vga") ? HAL_DISP_INTF_VGA : HAL_DISP_INTF_CVBS;

        pstInputParam->eDispTiming = _DispTransDispTimingId(argv[DISP_VGA_CVBS_PARAM_START]);

        pstInputParam->u32PathId =
            (argc == (DISP_VGA_CVBS_PARAM_NUM + 1)) ? simple_strtol(argv[DISP_VGA_CVBS_PARAM_START + 1], NULL, 10) : 0;
        bRet = 1;
        DISP_ERR("%s path %d timing %d interface %d%s\n", __FUNCTION__, pstInputParam->u32PathId,
                 pstInputParam->eDispTiming, pstInputParam->u32DispDevIntf, argv[DISP_VGA_CVBS_PARAM_START - 1]);
    }
    else
    {
        DISP_ERR("disp show vga  [TIMING ID] [PATH ID]\n");
        DISP_ERR("disp show cvbs  [TIMING ID] [PATH ID]\n");
    }
    return bRet;
}

bool _DispParsingPnlParam(int argc, char *const argv[], BootDispInputParam_t *pstInputParam)
{
#define DISP_PNL_PARAM_NUM   4
#define DISP_PNL_PARAM_START 3

    bool bRet = 0;

    if (argc == DISP_PNL_PARAM_NUM || argc == (DISP_PNL_PARAM_NUM + 1))
    {
        pstInputParam->u32DispDevIntf =
            !strcmp(argv[DISP_PNL_PARAM_START - 1], "ttl") ? HAL_DISP_INTF_TTL : HAL_DISP_INTF_TTL;

        pstInputParam->eDispTiming = _DispTransDispTimingId(argv[DISP_PNL_PARAM_START]);

        pstInputParam->u32PathId =
            (argc == (DISP_PNL_PARAM_NUM + 1)) ? simple_strtol(argv[DISP_PNL_PARAM_START + 1], NULL, 10) : 0;
        bRet = 1;
        DISP_ERR("%s path %d timing %d interface %d%s\n", __FUNCTION__, pstInputParam->u32PathId,
                 pstInputParam->eDispTiming, pstInputParam->u32DispDevIntf, argv[DISP_PNL_PARAM_START - 1]);
    }
    else
    {
        DISP_ERR("disp show ttl  [TIMING ID] [PATH ID]\n");
    }
    return bRet;
}

bool _DispSetDbgmg(int argc, char *const argv[])
{
    bool   bRet = 0;
    MI_U32 u32DbgLevel;

    if (argc == 4)
    {
        u32DbgLevel = simple_strtol(argv[3], NULL, 16);
        if (!strcmp(argv[2], "disp"))
        {
#if defined(CONFIG_SSTAR_DISP)
            DRV_DISP_IF_SetDbgLevel(&u32DbgLevel);
            DISP_DBG(DISP_DBG_LEVEL_IO, "disp %x\n", u32DbgLevel);
            bRet = 1;
#endif
        }
        else if (!strcmp(argv[2], "hdmitx"))
        {
#if defined(CONFIG_SSTAR_HDMITX)
            if (g_stHdmitxCtx.pHdmitxCtx)
            {
                MhalHdmitxSetDebugLevel(g_stHdmitxCtx.pHdmitxCtx, u32DbgLevel);
            }
            DISP_DBG(DISP_DBG_LEVEL_IO, "hdmitx %x\n", u32DbgLevel);
            bRet = 1;
#endif
        }
    }
    else
    {
        DISP_ERR("dbgmg [MODULE] [LEVEL]\n");
    }
    return bRet;
}

bool _DispSetFunc(int argc, char *const argv[])
{
    bool                            bRet = 0;
    MI_U32                          u32Val;
    MI_U32                          u32Dev;
    MI_DISP_IMPL_MhalDeviceConfig_t stDevCfg;

    if (argc == 5)
    {
        u32Val = simple_strtol(argv[3], NULL, 16);
        u32Dev = simple_strtol(argv[4], NULL, 16);
        u32Dev = HAL_DISP_MAPPING_DEVICEID_FROM_MI(u32Dev);
        if (!strcmp(argv[2], "disppat"))
        {
#if defined(CONFIG_SSTAR_DISP)
            stDevCfg.bDispPat = u32Val ? 1 : 0;
            stDevCfg.u8PatMd  = u32Val;
            stDevCfg.eType    = E_MI_DISP_DEV_CFG_DISP_PAT;
            DRV_DISP_IF_SetDeviceConfig(u32Dev, &stDevCfg);
            DISP_DBG(DISP_DBG_LEVEL_IO, "disppat:%x disppath:%d\n", u32Val, u32Dev);
            bRet = 1;
#endif
        }
    }
    else
    {
        DISP_ERR("func [func] [val] [path]\n");
    }
    return bRet;
}

void _DispCtxInit(void)
{
    if (!g_bInit)
    {
        memset(&g_stDispCtx, 0, sizeof(BootDispContext_t) * HAL_DISP_DEVICE_MAX);
        memset(&g_stHdmitxCtx, 0, sizeof(BootDispHdmiTxContext_t));
        memset(&g_stDmaCtx, 0, sizeof(BootDispDmaCtxConfig_t) * HAL_DISP_DMA_MAX);
    }
}
static void _DispPrintBuf(MI_U8 *buf)
{
    char * cur   = (char *)buf;
    MI_U8 *token = NULL;
    MI_U8  del[] = "\n";
    do
    {
        token = (MI_U8 *)strsep(&(cur), (const char *)del);
        printf("%s\n", token);
    } while (token);
}
void _DispGetCfg(void)
{
#if defined(CONFIG_SSTAR_DISP)
    void *str = NULL;

    str = malloc(PAGE_SIZE);
    if (str)
    {
        // DrvDispDebugCfgShow(str);
        _DispPrintBuf((MI_U8 *)str);
        free(str);
    }
#endif
}

void _DispExit(void)
{
    MI_U32 u32DispPath = 0;

#if defined(CONFIG_SSTAR_DISP)
    for (u32DispPath = 0; u32DispPath < HAL_DISP_DEVICE_MAX; u32DispPath++)
    {
        _DispDestoryInstance(u32DispPath);
    }
#endif
#if defined(CONFIG_SSTAR_HDMITX)
    _DispHdmitxDestoryInstance();
#endif
    g_bInit = 0;
    _DispCtxInit();
}
void _DispParsingShowParam(int argc, char *const argv[], BootDispInputParam_t *pstInputParam)
{
    DispModuleType_e eModule = E_DISP_MODULE_NONE;

    if (argc >= 2)
    {
        if (!strcmp(argv[2], "hdmi"))
        {
            if (_DispParsingHdmiParam(argc, argv, pstInputParam))
            {
                eModule = E_DISP_MODULE_HDMITX | E_DISP_MODULE_DISP;
            }
            else
            {
                DISP_ERR("%s %d, ParsingCmd Fail\n", __FUNCTION__, __LINE__);
            }
        }
        else if (!strcmp(argv[2], "vga") || !strcmp(argv[2], "cvbs"))
        {
            if (_DispParsingVgaCvbsParam(argc, argv, pstInputParam))
            {
                eModule = E_DISP_MODULE_DISP | E_DISP_MODULE_DAC;
            }
            else
            {
                DISP_ERR("%s %d, ParsingCmd Fail\n", __FUNCTION__, __LINE__);
            }
        }
        else if (!strcmp(argv[2], "ttl"))
        {
            if (_DispParsingPnlParam(argc, argv, pstInputParam))
            {
                eModule = E_DISP_MODULE_DISP | E_DISP_MODULE_PNL;
            }
            else
            {
                DISP_ERR("%s %d, ParsingCmd Fail\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            DISP_ERR("Un-Support CMD: %s\n", argv[1]);
        }
    }

    if (eModule != E_DISP_MODULE_NONE)
    {
        if (eModule & E_DISP_MODULE_DISP)
        {
#if defined(CONFIG_SSTAR_DISP)
            _DispShow(pstInputParam);
#endif
        }
        if (eModule & E_DISP_MODULE_HDMITX)
        {
#if defined(CONFIG_SSTAR_HDMITX)
            _DispHdmitxShow(pstInputParam);
#endif
        }
    }
}

int do_disp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    BootDispInputParam_t stInputParam;

    memset(&stInputParam, 0, sizeof(BootDispInputParam_t));
    _DispCtxInit();
    g_bInit = 1;
    if (argc >= 2)
    {
        if (!strcmp(argv[1], "show"))
        {
            _DispParsingShowParam(argc, argv, &stInputParam);
        }
        else if (!strcmp(argv[1], "dma"))
        {
            _DispParsingDmaParam(argc, argv, &stInputParam);
        }
        else if (!strcmp(argv[1], "dbgmg"))
        {
            _DispSetDbgmg(argc, argv);
        }
        else if (!strcmp(argv[1], "func"))
        {
            _DispSetFunc(argc, argv);
        }
        else if (!strcmp(argv[1], "cfg"))
        {
            _DispGetCfg();
        }
        else if (!strcmp(argv[1], "exit"))
        {
            _DispExit();
        }
        else
        {
            DISP_ERR("UnSuporot Cmd: %s\n", argv[1]);
        }
    }

    return 0;
}

static char disp_help_text[] =
    "show [interface] [timing] {[inc] [outc]} [device id]\n"
    "dbgmg [mod] [level]\n"
    "dma [SrcId][DestId][In Type][In Widht] [In Height] [Out Type] [Out Mode] [Out Widht] [Out Height] [RING HEIHGT] { "
    "[Comp] [Fmt] [Cnt] }\n"
    "cfg\n"
    "func [func] [val] [path]\n"
    "exit\n";

U_BOOT_CMD(disp, CONFIG_SYS_MAXARGS, 1, do_disp, "disp command", disp_help_text);
