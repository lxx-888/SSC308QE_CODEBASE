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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mi_hvp.h"
#include "mi_hdmirx.h"
#include "ss_thread.h"
#include "amigos_module_init.h"
#include "amigos_module_signal_monitor.h"

static MI_HVP_SignalTiming_t signal_timing;

typedef enum
{
    EN_SIGNAL_MONITOR_STATE_IDLE = 0,
    EN_SIGNAL_MONITOR_STATE_UNSTABLE,
    EN_SIGNAL_MONITOR_STATE_STABLE,
    EN_SIGNAL_MONITOR_STATE_TIMING_CHANGE
} SignalMonitorState_e;

typedef struct stMonitorDesc_s
{
    unsigned int uintHdmiPort;
    unsigned int uintHvpDev;
    unsigned int uintHvpChn;
    AmigosModuleSignalMonitor *pThis;
} stMonitorDesc_t;


AmigosModuleSignalMonitor::AmigosModuleSignalMonitor(const std::string &strInSection)
    : AmigosSurfaceSignalMonitor(strInSection), AmigosModuleBase(this), threadHandle(nullptr)
{
}
AmigosModuleSignalMonitor::~AmigosModuleSignalMonitor()
{
}
unsigned int AmigosModuleSignalMonitor::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleSignalMonitor::GetInputType(unsigned int port) const
{
    return 0;
}
unsigned int AmigosModuleSignalMonitor::GetOutputType(unsigned int port) const
{
    return 0;
}
#define IS_VALUE_CHANGE(__left, __right)                                                            \
                       (((__left ) == (__right)) ? FALSE                                            \
                       : ((__left = __right), printf("Change: %s=%s\n", #__left, #__right), TRUE))
#define SIGNAL_MONITOR_COLOR_FORMAT_HDMI_2_HVP(__hvp, __hdmirx, __val_check)         \
    do                                                                               \
    {                                                                                \
        switch (__hdmirx)                                                            \
        {                                                                            \
            case E_MI_HDMIRX_PIXEL_FORMAT_RGB:                                       \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_COLOR_FORMAT_RGB444); \
                break;                                                               \
            case E_MI_HDMIRX_PIXEL_FORMAT_YUV444:                                    \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_COLOR_FORMAT_YUV444); \
                break;                                                               \
            case E_MI_HDMIRX_PIXEL_FORMAT_YUV422:                                    \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_COLOR_FORMAT_YUV422); \
                break;                                                               \
            case E_MI_HDMIRX_PIXEL_FORMAT_YUV420:                                    \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_COLOR_FORMAT_YUV420); \
                break;                                                               \
            default:                                                                 \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_COLOR_FORMAT_MAX);    \
                break;                                                               \
        }                                                                            \
    } while(0)
#define SIGNAL_MONITOR_COLOR_DEPTH_HDMI_2_HVP(__hvp, __hdmirx, __val_check)      \
do                                                                               \
    {                                                                            \
        switch (__hdmirx)                                                        \
        {                                                                        \
            case E_MI_HDMIRX_PIXEL_BITWIDTH_8BIT:                                \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_COLOR_DEPTH_8);   \
                break;                                                           \
            case E_MI_HDMIRX_PIXEL_BITWIDTH_10BIT:                               \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_COLOR_DEPTH_10);  \
                break;                                                           \
            case E_MI_HDMIRX_PIXEL_BITWIDTH_12BIT:                               \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_COLOR_DEPTH_12);  \
                break;                                                           \
            default:                                                             \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_COLOR_DEPTH_MAX); \
                break;                                                           \
        }                                                                        \
    } while(0)
#define SIGNAL_MONITOR_PIXEX_REPETITIVE_HDMI_2_HVP(__hvp, __hdmirx, __val_check)      \
do                                                                                    \
    {                                                                                 \
        switch (__hdmirx)                                                             \
        {                                                                             \
            case E_MI_HDMIRX_OVERSAMPLE_1X:                                           \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_PIX_REP_TYPE_1X);      \
                break;                                                                \
            case E_MI_HDMIRX_OVERSAMPLE_2X:                                           \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_PIX_REP_TYPE_2X);      \
                break;                                                                \
            case E_MI_HDMIRX_OVERSAMPLE_3X:                                           \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_PIX_REP_TYPE_3X);      \
                break;                                                                \
            case E_MI_HDMIRX_OVERSAMPLE_4X:                                           \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_PIX_REP_TYPE_4X);      \
                break;                                                                \
            default:                                                                  \
                __val_check |= IS_VALUE_CHANGE(__hvp, E_MI_HVP_PIX_REP_TYPE_MAX);     \
                break;                                                                \
        }                                                                             \
    } while(0)
static int SyncHdmiParamter2Hvp(unsigned int uintHdmiPort, unsigned int uintHvpDev, unsigned int uintHvpChn)
{
    MI_S32 ret = 0;
    MI_HVP_ChannelParam_t stChnParam;
    MI_HDMIRX_PortId_e ePortId = (MI_HDMIRX_PortId_e)uintHdmiPort;
    MI_HDMIRX_TimingInfo_t stTimingInfo;
    MI_HDMIRX_SigStatus_e eSignalStatus = E_MI_HDMIRX_SIG_NO_SIGNAL;
    MI_HVP_DEV dev = (MI_HVP_DEV)uintHvpDev;
    MI_HVP_CHN chn = (MI_HVP_CHN)uintHvpChn;
    MI_BOOL isChanged = FALSE;

    memset(&stChnParam, 0, sizeof(MI_HVP_ChannelParam_t));
    ret = MI_HVP_GetChannelParam(dev, chn, &stChnParam);
    if (ret != MI_SUCCESS)
    {
        std::cout << "HVP DEV" << dev << " CHN" << chn << " GetChnParam ERR!!" << std::endl;
        return -1;
    }
    ret = MI_HDMIRX_GetSignalStatus(ePortId, &eSignalStatus);
    if (ret != MI_SUCCESS)
    {
        std::cout << "HDMIRX" << (int)ePortId << " GetSignalStatus ERR!!" << std::endl;
        return -1;
    }
    if (eSignalStatus != E_MI_HDMIRX_SIG_SUPPORT)
    {
        return 0;
    }
    ret = MI_HDMIRX_GetTimingInfo(ePortId, &stTimingInfo);
    if (ret != MI_SUCCESS)
    {
        std::cout << "HDMIRX" << (int)ePortId << " GetTiming Info ERR!!" << std::endl;
        return -1;
    }
    SIGNAL_MONITOR_PIXEX_REPETITIVE_HDMI_2_HVP(stChnParam.stChnSrcParam.enPixRepType, stTimingInfo.eOverSample, isChanged);
    SIGNAL_MONITOR_COLOR_FORMAT_HDMI_2_HVP(stChnParam.stChnSrcParam.enInputColor, stTimingInfo.ePixelFmt, isChanged);
    SIGNAL_MONITOR_COLOR_DEPTH_HDMI_2_HVP(stChnParam.stChnSrcParam.enColorDepth, stTimingInfo.eBitWidth, isChanged);
    if (isChanged)
    {
        ret = MI_HVP_SetChannelParam(dev, chn, &stChnParam);
        if (ret != MI_SUCCESS)
        {
            std::cout << "HVP DEV" << dev << " CHN" << chn << " SetChnParam ERR!!" << std::endl;
            return -1;
        }
    }
    return ret;
}
static void DoEvent(SignalMonitorState_e enState, int uintHdmiPort, unsigned int uintHvpDev, unsigned int uintHvpChn)
{
    switch (enState)
    {
        case EN_SIGNAL_MONITOR_STATE_IDLE:
            SyncHdmiParamter2Hvp(uintHdmiPort, uintHvpDev, uintHvpChn);
            break;
        case EN_SIGNAL_MONITOR_STATE_UNSTABLE:
            AMIGOS_INFO("UNSTABLE: SET VIDEO MUTE\n");
            MI_HVP_SetVideoMute(uintHvpDev, uintHvpChn, TRUE, 0);
            break;
        case EN_SIGNAL_MONITOR_STATE_STABLE:
            SyncHdmiParamter2Hvp(uintHdmiPort, uintHvpDev, uintHvpChn);
            MI_HVP_SetVideoMute(uintHvpDev, uintHvpChn, FALSE, 0);
            AMIGOS_INFO("STABLE: SET VIDEO UNMUTE\n");
            break;
        case EN_SIGNAL_MONITOR_STATE_TIMING_CHANGE:
            AMIGOS_INFO("TIMING CHANGE: SET VIDEO MUTE\n");
            MI_HVP_SetVideoMute(uintHvpDev, uintHvpChn, TRUE, 0);
            SyncHdmiParamter2Hvp(uintHdmiPort, uintHvpDev, uintHvpChn);
            MI_HVP_SetVideoMute(uintHvpDev, uintHvpChn, FALSE, 0);
            AMIGOS_INFO("TIMING CHANGE: SET VIDEO UNMUTE\n");
            break;
        default:
            AMIGOS_ERR("HVP EVENT %d ERROR!!!!!\n", enState);
            break;
    }
}
static void *MonitorMain(struct ss_thread_buffer *thread_buf)
{
    int select_ret = 0;
    MI_S32 ret = 0;
    MI_S32 s32Fd = -1;
    MI_BOOL bTrigger;
    fd_set read_fds;
    struct timeval tv;
    stMonitorDesc_t *pDesc = (stMonitorDesc_t *)thread_buf->buf;
    MI_HDMIRX_SigStatus_e cur_signal_status_hdmirx = E_MI_HDMIRX_SIG_NO_SIGNAL;
    MI_HVP_SignalTiming_t cur_signal_timing = {0, 0, 0, 0};
    MI_HVP_SignalStatus_e cur_signal_status_hvp = E_MI_HVP_SIGNAL_UNSTABLE;

    assert(pDesc);
    assert(sizeof(stMonitorDesc_t) == thread_buf->size);
    ret = MI_HVP_GetResetEventFd(pDesc->uintHvpDev, &s32Fd);
    if (ret != MI_SUCCESS)
    {
        AMIGOS_ERR("Get Reset fd Errr, Hvp Dev%d!\n", pDesc->uintHvpDev);
        return NULL;
    }
    FD_ZERO(&read_fds);
    FD_SET(s32Fd, &read_fds);
    tv.tv_sec = 0;
    tv.tv_usec = 100 * 1000;
    select_ret = select(s32Fd + 1, &read_fds, NULL, NULL, &tv);
    if (select_ret < 0)
    {
        AMIGOS_ERR("Reset fd select error!!\n");
        goto EXIT_FD;
    }
    ret = MI_HVP_GetResetEvent((MI_HVP_DEV)pDesc->uintHvpDev, &bTrigger);
    if (ret != MI_SUCCESS)
    {
        AMIGOS_ERR("Get Reset cnt Errr, Hvp Dev%d!\n",
                    pDesc->uintHvpDev);
        goto EXIT_FD;
    }
    ret = MI_HDMIRX_GetSignalStatus((MI_HDMIRX_PortId_e)pDesc->uintHdmiPort, &cur_signal_status_hdmirx);
    if (ret != MI_SUCCESS)
    {
        AMIGOS_ERR("Get Signal Status Errr, Hvp Dev%d!\n",
                    pDesc->uintHvpDev);
        goto EXIT_CLEAR_TRIGGER;
    }
    if (cur_signal_status_hdmirx == E_MI_HDMIRX_SIG_SUPPORT)
    {
        ret = MI_HVP_GetSourceSignalStatus((MI_HVP_DEV)pDesc->uintHvpDev, &cur_signal_status_hvp);
        if (ret != MI_SUCCESS)
        {
            AMIGOS_ERR("Get Signal Status Errr, Hvp Dev%d!\n",
                        pDesc->uintHvpDev);
            goto EXIT_CLEAR_TRIGGER;

        }
        ret = MI_HVP_GetSourceSignalTiming((MI_HVP_DEV)pDesc->uintHvpDev, &cur_signal_timing);
        if (ret != MI_SUCCESS)
        {
            AMIGOS_ERR("Get Signal Timing Errr, Hvp Dev%d!\n",
                        pDesc->uintHvpDev);
            goto EXIT_CLEAR_TRIGGER;
        }
    }
    ret = EN_SIGNAL_MONITOR_STATE_IDLE;
    if (cur_signal_timing.u16Width != signal_timing.u16Width
        || cur_signal_timing.u16Height!= signal_timing.u16Height
        || cur_signal_timing.bInterlace!= signal_timing.bInterlace)
    {
        std::cout << "Get Signal St: " << ((cur_signal_status_hdmirx == E_MI_HDMIRX_SIG_SUPPORT && cur_signal_status_hvp == E_MI_HVP_SIGNAL_STABLE)
            ? " stable" : " unstable")
            << std::dec << " W: " << cur_signal_timing.u16Width << " H: " << cur_signal_timing.u16Height
            << " Fps: " << cur_signal_timing.u16Fpsx100 << " bInterlace: " << (int)cur_signal_timing.bInterlace
            << std::endl;
        if (signal_timing.u16Fpsx100 && !cur_signal_timing.u16Fpsx100)
        {
            std::cout << "Signal unstable." << std::endl;
            ret = EN_SIGNAL_MONITOR_STATE_UNSTABLE;
        }
        else if (!signal_timing.u16Fpsx100  && cur_signal_timing.u16Fpsx100)
        {
            std::cout << "Signal stable." << std::endl;
            ret = EN_SIGNAL_MONITOR_STATE_STABLE;
        }
        else
        {
            std::cout << "Timing change." << std::endl;
            ret = EN_SIGNAL_MONITOR_STATE_TIMING_CHANGE;
        }
    }
    signal_timing = cur_signal_timing;
    DoEvent((SignalMonitorState_e)ret, pDesc->uintHdmiPort, pDesc->uintHvpDev, pDesc->uintHvpChn);
EXIT_CLEAR_TRIGGER:
    if (bTrigger)
    {
        cout << "Clear Trigger." << " Select Ret: " << select_ret << endl;
        MI_HVP_ClearResetEvent(pDesc->uintHvpDev);
    }
EXIT_FD:
    MI_HVP_CloseResetEventFd(pDesc->uintHvpDev, s32Fd);
    return NULL;
}
void AmigosModuleSignalMonitor::_Init()
{
    memset(&signal_timing, 0, sizeof(MI_HVP_SignalTiming_t));
}

void AmigosModuleSignalMonitor::_Deinit()
{
}

void AmigosModuleSignalMonitor::_Start()
{
    std::stringstream ss;
    stMonitorDesc_t stMonitorDesc;
    ss_thread_attr ss_attr;

    ss << "SigMonitor_hdmirx_" << stMonitorPara.uintHdmiPort << "hvp_" << stMonitorPara.uintHvpDev << '_' << stMonitorPara.uintHvpChn;
    memset(&ss_attr, 0, sizeof(ss_thread_attr));
    memset(&stMonitorDesc, 0, sizeof(stMonitorDesc_t));
    stMonitorDesc.uintHdmiPort = stMonitorPara.uintHdmiPort;
    stMonitorDesc.uintHvpDev   = stMonitorPara.uintHvpDev;
    stMonitorDesc.uintHvpChn   = stMonitorPara.uintHvpChn;
    stMonitorDesc.pThis = this;
    ss_attr.do_signal          = NULL;
    ss_attr.do_monitor         = MonitorMain;
    ss_attr.monitor_cycle_sec  = 0;
    ss_attr.monitor_cycle_nsec = 0;
    ss_attr.is_reset_timer     = 0;
    ss_attr.in_buf.buf         = &stMonitorDesc;
    ss_attr.in_buf.size        = sizeof(stMonitorDesc_t);
    snprintf(ss_attr.thread_name, 128, "%s", ss.str().c_str());
    threadHandle = ss_thread_open(&ss_attr);
    if (!threadHandle)
    {
        AMIGOS_ERR("Monitor return error!\n");
        return;
    }
    ss_thread_start_monitor(threadHandle);
}

void AmigosModuleSignalMonitor::_Stop()
{
    std::stringstream ss;

    ss << "SigMonitor_hdmirx_" << stMonitorPara.uintHdmiPort << "hvp_" << stMonitorPara.uintHvpDev << '_' << stMonitorPara.uintHvpChn;
    ss_thread_close(threadHandle);
    threadHandle = nullptr;
}

AMIGOS_MODULE_INIT("SIGNAL_MONITOR", AmigosModuleSignalMonitor);
