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

#include <cassert>
#include <codecvt>
#include <cwchar>
#include <memory>
#include <ostream>
#include "amigos_env.h"
#include "amigos_log.h"
#include "amigos_module_base.h"
#include "amigos_module_init.h"
#include "amigos_module_env_monitor.h"
#include "amigos_surface_env_monitor.h"
#include "ss_linker.h"
#include "ss_packet.h"
#include "ss_thread.h"
#include "amigos_module_rgn_metadata_define.h"

static void FormattingVencInfo(AmigosEnv &env, wchar_t *str, unsigned int n, unsigned int inPort,
                               unsigned int outPort);

AmigosModuleEnvMonitor::AmigosModuleEnvMonitor(const std::string &strInSection)
    : AmigosSurfaceEnvMonitor(strInSection), AmigosModuleBase(this)
{
}
AmigosModuleEnvMonitor::~AmigosModuleEnvMonitor() {}

unsigned int AmigosModuleEnvMonitor::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleEnvMonitor::GetInputType(unsigned int port) const
{
    return 0;
}
unsigned int AmigosModuleEnvMonitor::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
void AmigosModuleEnvMonitor::_Init()
{

}
void AmigosModuleEnvMonitor::_Deinit()
{

}
int AmigosModuleEnvMonitor::_Connected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    if (this->map_timer_desc.end() != this->map_timer_desc.find(outPortId))
    {
        AMIGOS_ERR("Timer desc had beed created.\n");
        return -1;
    }
    auto outportIt = this->mapPortOut.find(outPortId);
    if (outportIt == this->mapPortOut.end())
    {
        AMIGOS_ERR("Could not find output port %d.\n", outPortId);
        return -1;
    }
    EnvMonitorReaderDesc desc;
    desc.linker    = &outportIt->second.positive;
    desc.strModule = this->mapEnvMonitorOutputInfo[outPortId].strModule;
    desc.inPort    = this->mapEnvMonitorOutputInfo[outPortId].intInPort;
    desc.outPort   = this->mapEnvMonitorOutputInfo[outPortId].intOutPort;
    desc.env       = &this->env;
    if (desc.linker->empty())
    {
        AMIGOS_ERR("EnvMonitor Can't run witout reader!\n");
        return -1;
    }
    if (this->mapEnvMonitorOutputInfo[outPortId].strMode == "venc_info")
    {
        desc.formatting = FormattingVencInfo;
    }
    else
    {
        AMIGOS_WRN("Not supported mode %s\n", this->mapEnvMonitorOutputInfo[outPortId].strMode.c_str());
    }
    ss_thread_attr ss_attr;
    memset(&ss_attr, 0, sizeof(ss_thread_attr));
    ss_attr.do_signal          = NULL;
    ss_attr.do_monitor         = EnvMonitorReader;
    ss_attr.monitor_cycle_sec  = 1;
    ss_attr.monitor_cycle_nsec = 0;
    ss_attr.is_reset_timer     = 0;
    ss_attr.in_buf.buf         = (void *)&(this->map_timer_desc[outPortId]);
    ss_attr.in_buf.size        = 0;
    snprintf(ss_attr.thread_name, 128, "%s", this->GetOutPortIdStr(outPortId).c_str());
    desc.threadHandle = ss_thread_open(&ss_attr);
    if (!desc.threadHandle)
    {
        this->map_timer_desc.erase(outPortId);
        AMIGOS_ERR("Monitor return error!\n");
        return -1;
    }
    this->map_timer_desc[outPortId] = desc;
    ss_thread_start_monitor(desc.threadHandle);
    AMIGOS_INFO("EnvMonitor reader [%d] start\n", outPortId);
    return 0;
}
int AmigosModuleEnvMonitor::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    auto it = this->map_timer_desc.find(outPortId);
    if (it == this->map_timer_desc.end())
    {
        return -1;
    }
    if (it->second.linker->empty())
    {
        return -1;
    }
    ss_thread_stop(it->second.threadHandle);
    ss_thread_close(it->second.threadHandle);
    this->map_timer_desc.erase(it);
    return 0;
}

void *AmigosModuleEnvMonitor::EnvMonitorReader(struct ss_thread_buffer *thread_buf)
{
    EnvMonitorReaderDesc *pDesc = (EnvMonitorReaderDesc *)thread_buf->buf;
    if (!pDesc->formatting)
    {
        return NULL;
    }

    stream_packet_info packet_info;
    packet_info.en_type              = EN_USER_META_DATA;
    packet_info.meta_data_i.reserved = E_META_DATA_TEXTS;
    packet_info.meta_data_i.size     = sizeof(AmigosRgnMetaDataTexts) + sizeof(AmigosRgnMetaDataText);
    stream_packet_obj packet = std::make_shared<stream_packet>(packet_info);
    assert(packet);

    AmigosRgnMetaDataTexts &texts = *(AmigosRgnMetaDataTexts*)packet->meta_data.data;
    memset(&texts, 0, sizeof(AmigosRgnMetaDataTexts) + sizeof(AmigosRgnMetaDataText));

    texts.count = 1;
    pDesc->formatting(pDesc->env->Ext(pDesc->strModule.c_str()), texts.texts[0].str, AMIGOS_RGN_METADATA_TEXT_MAX_SIZE,
                      pDesc->inPort, pDesc->outPort);
    texts.texts[0].pt.x = AMIGOS_RGN_METADATA_COORDINATE_MAX_W;
    texts.texts[0].pt.y = AMIGOS_RGN_METADATA_COORDINATE_MAX_H;

    pDesc->linker->enqueue(packet);
    return NULL;
}

static void FormattingVencInfo(AmigosEnv &env, wchar_t *str, unsigned int n, unsigned int inPort,
                               unsigned int outPort)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> utf8WcCvt;
    std::stringstream ss;
    ss << env.Out(outPort)["EN_TYPE"] << " 帧率:" << env.Out(outPort)["STD_FPS"] << std::endl;
    ss << "分辨率:" << env.Out(outPort)["STD_RES"] << std::endl;
    ss << "瞬时码率:"<< env.Out(outPort)["STD_BITRATE"] << std::endl;
    ss << "平均码率:" << env.Out(outPort)["STD_AVERAGE_BITRATE"] << std::endl;
    ss << "GOP:" << env.Out(outPort)["STD_GOP"];
    std::wstring wstr = utf8WcCvt.from_bytes(ss.str());
    std::size_t len = wstr.copy(str, AMIGOS_RGN_METADATA_TEXT_MAX_SIZE - 1);
    str[len] = '\0';
}

AMIGOS_MODULE_INIT("ENV_MONITOR", AmigosModuleEnvMonitor);

