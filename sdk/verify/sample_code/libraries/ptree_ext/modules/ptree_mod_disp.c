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

#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_sys.h"
#include "ptree_mod_sys.h"
#include "ptree_mod_disp.h"
#include "ptree_sur_disp.h"
#include "ptree_sur_sys.h"
#include "ptree_maker.h"
#include "ssos_mem.h"
#include "ptree_log.h"

#ifdef INTERFACE_HDMI
#include "mi_hdmi.h"
#endif

#define PTREE_MOD_DISP_DEV_NUM (3)

#define MAKE_YUYV_VALUE(y, u, v) ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK               MAKE_YUYV_VALUE(0, 128, 128)
#define YUYV_WHITE               MAKE_YUYV_VALUE(255, 128, 128)
#define YUYV_RED                 MAKE_YUYV_VALUE(76, 84, 255)
#define YUYV_GREEN               MAKE_YUYV_VALUE(149, 43, 21)
#define YUYV_BLUE                MAKE_YUYV_VALUE(29, 225, 107)

PTREE_ENUM_DEFINE(MI_DISP_Interface_e,               // MI_DISP_Interface_e
                  {E_MI_DISP_INTF_TTL, "ttl"},       // ttl
                  {E_MI_DISP_INTF_MCU, "mcu"},       // mcu
                  {E_MI_DISP_INTF_SRGB, "srgb"},     // srgb
                  {E_MI_DISP_INTF_BT1120, "bt1120"}, // bi1120
                  {E_MI_DISP_INTF_BT656, "bt656"},   // bt656
                  {E_MI_DISP_INTF_MIPIDSI, "mipi"},  // mipi
                  {E_MI_DISP_INTF_CVBS, "cvbs"},     // cvbs
                  {E_MI_DISP_INTF_VGA, "vga"},       // vga
)

PTREE_ENUM_DEFINE(MI_DISP_OutputTiming_e,                          // MI_DISP_OutputTiming_e
                  {E_MI_DISP_OUTPUT_USER, "user"},                 // user
                  {E_MI_DISP_OUTPUT_NTSC, "ntsc"},                 // ntsc
                  {E_MI_DISP_OUTPUT_PAL, "pal"},                   // pal
                  {E_MI_DISP_OUTPUT_720P30, "720p30"},             // 720p30
                  {E_MI_DISP_OUTPUT_720P50, "720p50"},             // 720p50
                  {E_MI_DISP_OUTPUT_720P60, "720p60"},             // 720p60
                  {E_MI_DISP_OUTPUT_1024x768_60, "1024x768p60"},   // 1024x768p60
                  {E_MI_DISP_OUTPUT_1280x1024_60, "1280x1024p60"}, // 1280x1024p60
                  {E_MI_DISP_OUTPUT_1366x768_60, "1366x768p60"},   // 1366x768p60
                  {E_MI_DISP_OUTPUT_1080P24, "1080p24"},           // 1080p24
                  {E_MI_DISP_OUTPUT_1080P30, "1080p30"},           // 1080p30
                  {E_MI_DISP_OUTPUT_1080P50, "1080p50"},           // 1080p50
                  {E_MI_DISP_OUTPUT_1080P60, "1080p60"},           // 1080p60
                  {E_MI_DISP_OUTPUT_3840x2160_30, "3840x2160p30"}, // 3840x2160p30
                  {E_MI_DISP_OUTPUT_3840x2160_60, "3840x2160p60"}, // 3840x2160p60
)

PTREE_ENUM_DEFINE(int,                   // yuuv colors
                  {YUYV_BLACK, "black"}, // black
                  {YUYV_WHITE, "white"}, // white
                  {YUYV_RED, "red"},     // red
                  {YUYV_GREEN, "green"}, // green
                  {YUYV_BLUE, "blue"},   // blue
)

typedef struct PTREE_MOD_DISP_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
} PTREE_MOD_DISP_Obj_t;

typedef struct PTREE_MOD_DISP_InObj_s
{
    PTREE_MOD_SYS_InObj_t base;
} PTREE_MOD_DISP_InObj_t;

static int  _PTREE_MOD_DISP_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int  _PTREE_MOD_DISP_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static int  _PTREE_MOD_DISP_Start(PTREE_MOD_SYS_Obj_t *sysMod);
static int  _PTREE_MOD_DISP_Stop(PTREE_MOD_SYS_Obj_t *sysMod);
static void _PTREE_MOD_DISP_Free(PTREE_MOD_SYS_Obj_t *sysMod);

static PTREE_MOD_InObj_t *_PTREE_MOD_DISP_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void               _PTREE_MOD_DISP_InFree(PTREE_MOD_SYS_InObj_t *sysModIn);
static PTREE_MOD_InObj_t *_PTREE_MOD_DISP_InNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId);
static int                _PTREE_MOD_DISP_Linked(PTREE_MOD_SYS_InObj_t *sysModIn, unsigned int ref);
static int                _PTREE_MOD_DISP_Unlinked(PTREE_MOD_SYS_InObj_t *sysModIn, unsigned int ref);
static int                _PTREE_MOD_DISP_DirectBind(PTREE_MOD_SYS_InObj_t *modIn);
static int                _PTREE_MOD_DISP_DirectUnBind(PTREE_MOD_SYS_InObj_t *modIn);

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_DISP_SYS_OPS = {
    .init        = _PTREE_MOD_DISP_Init,
    .deinit      = _PTREE_MOD_DISP_Deinit,
    .start       = _PTREE_MOD_DISP_Start,
    .stop        = _PTREE_MOD_DISP_Stop,
    .createModIn = _PTREE_MOD_DISP_CreateModIn,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_DISP_SYS_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_DISP_Free,
};

static const PTREE_MOD_SYS_InOps_t G_PTREE_MOD_DISP_SYS_IN_OPS = {
    .linked       = _PTREE_MOD_DISP_Linked,
    .unlinked     = _PTREE_MOD_DISP_Unlinked,
    .directBind   = _PTREE_MOD_DISP_DirectBind,
    .directUnbind = _PTREE_MOD_DISP_DirectUnBind,
};
static const PTREE_MOD_SYS_InHook_t G_PTREE_MOD_DISP_SYS_IN_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_DISP_InFree,
};

static unsigned int g_dispCreateDev[PTREE_MOD_DISP_DEV_NUM] = {0};

#ifdef INTERFACE_HDMI
static int _PTREE_MOD_DISP_HdmiCb(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_EventType_e event, void *pEventParam,
                                  void *pUsrParam)
{
    (void)eHdmi;
    (void)pEventParam;
    (void)pUsrParam;
    switch (event)
    {
        case E_MI_HDMI_EVENT_HOTPLUG:
            PTREE_DBG("E_MI_HDMI_EVENT_HOTPLUG.\n");
            break;
        case E_MI_HDMI_EVENT_NO_PLUG:
            PTREE_DBG("E_MI_HDMI_EVENT_NO_PLUG.\n");
            break;
        default:
            PTREE_ERR("Unsupport event.\n");
            break;
    }

    return MI_SUCCESS;
}
#endif

static void _PTREE_MOD_DISP_HdmiInit(MI_DISP_OutputTiming_e eIntfSync)
{
#ifdef INTERFACE_HDMI
    MI_HDMI_InitParam_t stInitParam;
    MI_HDMI_Attr_t      stAttr;

    stInitParam.pCallBackArgs        = NULL;
    stInitParam.pfnHdmiEventCallback = _PTREE_MOD_DISP_HdmiCb;
    MI_HDMI_Init(&stInitParam);
    MI_HDMI_Open(E_MI_HDMI_ID_0);

    memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
    stAttr.stEnInfoFrame.bEnableAudInfoFrame = FALSE;
    stAttr.stEnInfoFrame.bEnableAviInfoFrame = FALSE;
    stAttr.stEnInfoFrame.bEnableSpdInfoFrame = FALSE;
    stAttr.stAudioAttr.bEnableAudio          = TRUE;
    stAttr.stAudioAttr.bIsMultiChannel       = 0;
    stAttr.stAudioAttr.eBitDepth             = E_MI_HDMI_BIT_DEPTH_16;
    stAttr.stAudioAttr.eCodeType             = E_MI_HDMI_ACODE_PCM;
    stAttr.stAudioAttr.eSampleRate           = E_MI_HDMI_AUDIO_SAMPLERATE_48K;
    stAttr.stVideoAttr.bEnableVideo          = TRUE;
    stAttr.stVideoAttr.eColorType            = E_MI_HDMI_COLOR_TYPE_RGB444; // default color type
    stAttr.stVideoAttr.eDeepColorMode        = E_MI_HDMI_DEEP_COLOR_MAX;
    switch (eIntfSync)
    {
        case E_MI_DISP_OUTPUT_NTSC:
        case E_MI_DISP_OUTPUT_480P60:
            stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_480_60P;
            break;
        case E_MI_DISP_OUTPUT_PAL:
        case E_MI_DISP_OUTPUT_576P50:
            stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_576_50P;
            break;
        case E_MI_DISP_OUTPUT_720P50:
            stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_720_50P;
            break;
        case E_MI_DISP_OUTPUT_720P60:
            stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_720_60P;
            break;
        case E_MI_DISP_OUTPUT_1080P60:
            stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
            break;
        case E_MI_DISP_OUTPUT_3840x2160_30:
            stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_4K2K_30P;
            break;
        case E_MI_DISP_OUTPUT_3840x2160_60:
            stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_4K2K_60P;
            break;
        default:
            stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
            break;
    }
    stAttr.stVideoAttr.eOutputMode = E_MI_HDMI_OUTPUT_MODE_HDMI;
    MI_HDMI_SetAttr(E_MI_HDMI_ID_0, &stAttr);
    MI_HDMI_Start(E_MI_HDMI_ID_0);
#endif
}

static void _PTREE_MOD_DISP_HdmiDeinit(void)
{
#ifdef INTERFACE_HDMI
    MI_HDMI_Stop(E_MI_HDMI_ID_0);
    MI_HDMI_Close(E_MI_HDMI_ID_0);
    MI_HDMI_DeInit();
#endif
}

static void _PTREE_MOD_DISP_InPortEnable(PTREE_SUR_DISP_InInfo_t *dispInInfo)
{
    MI_DISP_InputPortAttr_t stInputPortAttr;
    if (!dispInInfo->uintSrcWidth || !dispInInfo->uintSrcHeight || !dispInInfo->uintDstWidth
        || !dispInInfo->uintDstHeight)
    {
        return;
    }
    memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
    stInputPortAttr.u16SrcWidth         = dispInInfo->uintSrcWidth;
    stInputPortAttr.u16SrcHeight        = dispInInfo->uintSrcHeight;
    stInputPortAttr.stDispWin.u16X      = dispInInfo->uintDstXpos;
    stInputPortAttr.stDispWin.u16Y      = dispInInfo->uintDstYpos;
    stInputPortAttr.stDispWin.u16Width  = dispInInfo->uintDstWidth;
    stInputPortAttr.stDispWin.u16Height = dispInInfo->uintDstHeight;
    MI_DISP_SetInputPortAttr(dispInInfo->uintLayId, dispInInfo->uintLayPortId, &stInputPortAttr);
    MI_DISP_EnableInputPort(dispInInfo->uintLayId, dispInInfo->uintLayPortId);
}

static void _PTREE_MOD_DISP_InPortDisable(PTREE_SUR_DISP_InInfo_t *dispInInfo)
{
    if (!dispInInfo->uintSrcWidth || !dispInInfo->uintSrcHeight || !dispInInfo->uintDstWidth
        || !dispInInfo->uintDstHeight)
    {
        return;
    }
    MI_DISP_DisableInputPort(dispInInfo->uintLayId, dispInInfo->uintLayPortId);
}

static void _PTREE_MOD_DISP_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_DISP_Obj_t, base));
}

static int _PTREE_MOD_DISP_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    MI_DISP_LAYER            layerId = 0;
    int                      idx     = 0;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_RotateConfig_t   stRotateCfg;
    PTREE_SUR_DISP_Info_t *  dispInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_DISP_Info_t, base.base);

    do
    {
        // pub attr
        MI_DISP_PubAttr_t stPubAttr;
        if (g_dispCreateDev[sysMod->base.info->devId])
        {
            break;
        }
        memset(&stPubAttr, 0, sizeof(MI_DISP_PubAttr_t));
        stPubAttr.u32BgColor = PTREE_ENUM_FROM_STR(int, dispInfo->chBackGroundColor);
        stPubAttr.eIntfSync  = PTREE_ENUM_FROM_STR(MI_DISP_OutputTiming_e, dispInfo->chOutTiming);
        if (0 == strcmp(dispInfo->chDevType, "panel"))
        {
            stPubAttr.eIntfType = PTREE_ENUM_FROM_STR(MI_DISP_Interface_e, dispInfo->chPnlLinkType);
        }
        else if (0 == strcmp(dispInfo->chDevType, "vga"))
        {
            stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;
        }
        else if (0 == strcmp(dispInfo->chDevType, "cvbs_out"))
        {
            stPubAttr.eIntfType = E_MI_DISP_INTF_CVBS;
        }
        else if (0 == strcmp(dispInfo->chDevType, "hdmi"))
        {
            _PTREE_MOD_DISP_HdmiInit(stPubAttr.eIntfSync);
            stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
        }
        else
        {
            PTREE_ERR("Not support current device type!\n");
            return SSOS_DEF_FAIL;
        }

        // set disp pub
        if (MI_SUCCESS != MI_DISP_SetPubAttr((MI_DISP_DEV)dispInfo->base.base.devId, &stPubAttr))
        {
            PTREE_ERR("MI_DISP_SetPubAttr failed");
            return SSOS_DEF_FAIL;
        }
        if (MI_SUCCESS != MI_DISP_Enable((MI_DISP_DEV)dispInfo->base.base.devId))
        {
            PTREE_ERR("MI_DISP_Enable failed");
            return SSOS_DEF_FAIL;
        }
    } while (0);
    ++g_dispCreateDev[sysMod->base.info->devId];

    memset(&stLayerAttr, 0, sizeof(MI_DISP_VideoLayerAttr_t));
    memset(&stRotateCfg, 0, sizeof(MI_DISP_RotateConfig_t));

    // set inputport
    for (idx = 0; idx < dispInfo->uintLayerCount; idx++)
    {
        layerId = dispInfo->stDispLayerInfo[idx].uintId;
        // set layer
        MI_DISP_BindVideoLayer(layerId, (MI_DISP_DEV)dispInfo->base.base.devId);
        stLayerAttr.ePixFormat                  = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        stLayerAttr.stVidLayerSize.u16Width     = dispInfo->stDispLayerInfo[idx].uintWidth;
        stLayerAttr.stVidLayerSize.u16Height    = dispInfo->stDispLayerInfo[idx].uintHeight;
        stLayerAttr.stVidLayerDispWin.u16Width  = dispInfo->stDispLayerInfo[idx].uintDispWidth;
        stLayerAttr.stVidLayerDispWin.u16Height = dispInfo->stDispLayerInfo[idx].uintDispHeight;
        stLayerAttr.stVidLayerDispWin.u16X      = dispInfo->stDispLayerInfo[idx].uintDispXpos;
        stLayerAttr.stVidLayerDispWin.u16Y      = dispInfo->stDispLayerInfo[idx].uintDispYpos;
        MI_DISP_SetVideoLayerAttr(layerId, &stLayerAttr);
        MI_DISP_EnableVideoLayer(layerId);
        // rotate
        stRotateCfg.eRotateMode = (MI_DISP_RotateMode_e)dispInfo->stDispLayerInfo[idx].uintRot;
        MI_DISP_SetVideoLayerRotateMode(layerId, &stRotateCfg);
        MI_DISP_SetVideoLayerAttrBatchBegin(layerId);
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_DISP_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    MI_DISP_LAYER          layerId  = 0;
    int                    idx      = 0;
    PTREE_SUR_DISP_Info_t *dispInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_DISP_Info_t, base.base);

    for (idx = 0; idx < dispInfo->uintLayerCount; idx++)
    {
        layerId = dispInfo->stDispLayerInfo[idx].uintId;
        MI_DISP_SetVideoLayerAttrBatchEnd(layerId);
        MI_DISP_DisableVideoLayer(layerId);
        MI_DISP_UnBindVideoLayer(layerId, (MI_DISP_DEV)dispInfo->base.base.devId);
    }

    --g_dispCreateDev[sysMod->base.info->devId];
    do
    {
        if (g_dispCreateDev[sysMod->base.info->devId])
        {
            break;
        }
        MI_DISP_Disable((MI_DISP_DEV)dispInfo->base.base.devId);
        if (0 == strcmp(dispInfo->chDevType, "hdmi"))
        {
            _PTREE_MOD_DISP_HdmiDeinit();
        }
    } while (0);

    return SSOS_DEF_OK;
}

static int _PTREE_MOD_DISP_Start(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_DISP_Info_t *dispInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_DISP_Info_t, base.base);
    int                    idx;
    for (idx = 0; idx < dispInfo->uintLayerCount; idx++)
    {
        MI_DISP_SetVideoLayerAttrBatchEnd(dispInfo->stDispLayerInfo[idx].uintId);
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_DISP_Stop(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_DISP_Info_t *dispInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_DISP_Info_t, base.base);
    int                    idx;
    for (idx = 0; idx < dispInfo->uintLayerCount; idx++)
    {
        MI_DISP_SetVideoLayerAttrBatchBegin(dispInfo->stDispLayerInfo[idx].uintId);
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_DISP_Linked(PTREE_MOD_SYS_InObj_t *sysModIn, unsigned int ref)
{
    PTREE_SUR_DISP_InInfo_t *dispInInfo = CONTAINER_OF(sysModIn->base.info, PTREE_SUR_DISP_InInfo_t, base.base);
    if (ref)
    {
        return SSOS_DEF_OK;
    }
    _PTREE_MOD_DISP_InPortEnable(dispInInfo);
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_DISP_Unlinked(PTREE_MOD_SYS_InObj_t *sysModIn, unsigned int ref)
{
    PTREE_SUR_DISP_InInfo_t *dispInInfo = CONTAINER_OF(sysModIn->base.info, PTREE_SUR_DISP_InInfo_t, base.base);
    if (ref)
    {
        return SSOS_DEF_OK;
    }
    _PTREE_MOD_DISP_InPortDisable(dispInInfo);
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_DISP_DirectBind(PTREE_MOD_SYS_InObj_t *modIn)
{
    MI_S32 ret = MI_SUCCESS;

    MI_SYS_ChnPort_t srcChnPort;
    MI_SYS_ChnPort_t dstChnPort;

    MI_U32 srcFrmRate;
    MI_U32 dstFrmRate;

    MI_SYS_BindType_e bindType;
    MI_U32            bindParam;

    PTREE_MOD_DISP_InObj_t * sysModIn   = CONTAINER_OF(modIn, PTREE_MOD_DISP_InObj_t, base);
    PTREE_MOD_SYS_OutObj_t * sysModOut  = CONTAINER_OF(modIn->base.prevModOut, PTREE_MOD_SYS_OutObj_t, base);
    PTREE_SUR_DISP_InInfo_t *dispInInfo = CONTAINER_OF(modIn->base.info, PTREE_SUR_DISP_InInfo_t, base.base);

    _PTREE_MOD_DISP_InPortEnable(dispInInfo);

    memset(&srcChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&dstChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));

    srcChnPort.eModId    = sysModOut->thisSysMod->modId;
    srcChnPort.u32DevId  = sysModOut->base.thisMod->info->devId;
    srcChnPort.u32ChnId  = sysModOut->base.thisMod->info->chnId;
    srcChnPort.u32PortId = sysModOut->base.info->port;

    dstChnPort.eModId    = sysModIn->base.thisSysMod->modId;
    dstChnPort.u32DevId  = sysModIn->base.base.thisMod->info->devId;
    dstChnPort.u32ChnId  = dispInInfo->uintSysChn;
    dstChnPort.u32PortId = 0;

    srcFrmRate = sysModOut->base.info->fps;
    dstFrmRate = sysModIn->base.base.info->fps;
    bindType   = dispInInfo->base.bindType;
    bindParam  = dispInInfo->base.bindParam;

    ret = MI_SYS_BindChnPort2(0, &srcChnPort, &dstChnPort, srcFrmRate, dstFrmRate, bindType, bindParam);
    if (ret != MI_SUCCESS)
    {
        PTREE_ERR("MI_SYS_BindChnPort2 ret %d, %d-%d-%d-%d@%d --[%d, %d]--> %d-%d-%d-%d@%d", ret, srcChnPort.eModId,
                  srcChnPort.u32DevId, srcChnPort.u32ChnId, srcChnPort.u32PortId, srcFrmRate, bindType, bindParam,
                  dstChnPort.eModId, dstChnPort.u32DevId, dstChnPort.u32ChnId, dstChnPort.u32PortId, dstFrmRate);
        return SSOS_DEF_FAIL;
    }

    PTREE_DBG("Bind %d-%d-%d-%d@%d --[%d, %d]--> %d-%d-%d-%d@%d", srcChnPort.eModId, srcChnPort.u32DevId,
              srcChnPort.u32ChnId, srcChnPort.u32PortId, srcFrmRate, bindType, bindParam, dstChnPort.eModId,
              dstChnPort.u32DevId, dstChnPort.u32ChnId, dstChnPort.u32PortId, dstFrmRate);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_DISP_DirectUnBind(PTREE_MOD_SYS_InObj_t *modIn)
{
    MI_S32 ret = MI_SUCCESS;

    MI_SYS_ChnPort_t srcChnPort;
    MI_SYS_ChnPort_t dstChnPort;

    PTREE_MOD_DISP_InObj_t * sysModIn   = CONTAINER_OF(modIn, PTREE_MOD_DISP_InObj_t, base);
    PTREE_MOD_SYS_OutObj_t * sysModOut  = CONTAINER_OF(modIn->base.prevModOut, PTREE_MOD_SYS_OutObj_t, base);
    PTREE_SUR_DISP_InInfo_t *dispInInfo = CONTAINER_OF(modIn->base.info, PTREE_SUR_DISP_InInfo_t, base.base);

    memset(&srcChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&dstChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));

    srcChnPort.eModId    = sysModOut->thisSysMod->modId;
    srcChnPort.u32DevId  = sysModOut->base.thisMod->info->devId;
    srcChnPort.u32ChnId  = sysModOut->base.thisMod->info->chnId;
    srcChnPort.u32PortId = sysModOut->base.info->port;

    dstChnPort.eModId    = sysModIn->base.thisSysMod->modId;
    dstChnPort.u32DevId  = sysModIn->base.base.thisMod->info->devId;
    dstChnPort.u32ChnId  = dispInInfo->uintSysChn;
    dstChnPort.u32PortId = 0;

    ret = MI_SYS_UnBindChnPort(0, &srcChnPort, &dstChnPort);
    if (ret != MI_SUCCESS)
    {
        PTREE_ERR("MI_SYS_UnBindChnPort ret %d, %d-%d-%d-%d --> %d-%d-%d-%d", ret, srcChnPort.eModId,
                  srcChnPort.u32DevId, srcChnPort.u32ChnId, srcChnPort.u32PortId, dstChnPort.eModId,
                  dstChnPort.u32DevId, dstChnPort.u32ChnId, dstChnPort.u32PortId);
        return SSOS_DEF_FAIL;
    }

    PTREE_DBG("Unbind %d-%d-%d-%d -/ /-> %d-%d-%d-%d", srcChnPort.eModId, srcChnPort.u32DevId, srcChnPort.u32ChnId,
              srcChnPort.u32PortId, dstChnPort.eModId, dstChnPort.u32DevId, dstChnPort.u32ChnId, dstChnPort.u32PortId);

    _PTREE_MOD_DISP_InPortDisable(dispInInfo);
    return SSOS_DEF_OK;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_DISP_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    (void)loopId;
    return _PTREE_MOD_DISP_InNew(sysMod, loopId);
}

static void _PTREE_MOD_DISP_InFree(PTREE_MOD_SYS_InObj_t *sysModIn)
{
    SSOS_MEM_Free(CONTAINER_OF(sysModIn, PTREE_MOD_DISP_InObj_t, base));
}
static PTREE_MOD_InObj_t *_PTREE_MOD_DISP_InNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId)
{
    PTREE_MOD_DISP_InObj_t *dispModIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_DISP_InObj_t));
    if (!dispModIn)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(dispModIn, 0, sizeof(PTREE_MOD_DISP_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_InObjInit(&dispModIn->base, &G_PTREE_MOD_DISP_SYS_IN_OPS, sysMod, loopId))
    {
        SSOS_MEM_Free(dispModIn);
        return NULL;
    }
    PTREE_MOD_SYS_InObjRegister(&dispModIn->base, &G_PTREE_MOD_DISP_SYS_IN_HOOK);
    return &dispModIn->base.base;
}

PTREE_MOD_Obj_t *PTREE_MOD_DISP_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_DISP_Obj_t *dispMod = NULL;

    dispMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_DISP_Obj_t));
    if (!dispMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(dispMod, 0, sizeof(PTREE_MOD_DISP_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&dispMod->base, &G_PTREE_MOD_DISP_SYS_OPS, tag, E_MI_MODULE_ID_DISP))
    {
        goto ERR_MOD_SYS_INIT;
    }
    if (dispMod->base.base.info->devId >= PTREE_MOD_DISP_DEV_NUM)
    {
        PTREE_ERR("Dev id %d is not support, max number is %d", dispMod->base.base.info->devId, PTREE_MOD_DISP_DEV_NUM);
        goto ERR_DEV_OUT_OF_RANGE;
    }

    PTREE_MOD_SYS_ObjRegister(&dispMod->base, &G_PTREE_MOD_DISP_SYS_HOOK);
    return &dispMod->base.base;

ERR_DEV_OUT_OF_RANGE:
    PTREE_MOD_ObjDel(&dispMod->base.base);
ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(dispMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(DISP, PTREE_MOD_DISP_New);
