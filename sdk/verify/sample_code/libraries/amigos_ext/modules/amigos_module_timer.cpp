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

#include "amigos_module_timer.h"
#include <cassert>
#include <codecvt>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <ctime>
#include <cwchar>
#include <string>
#include "amigos_module_base.h"
#include "amigos_module_init.h"
#include "amigos_surface_timer.h"
#include "ss_linker.h"
#include "ss_thread.h"
#include "amigos_module_rgn_metadata_define.h"

AmigosModuleTimer::AmigosModuleTimer(const std::string &strInSection)
    : AmigosSurfaceTimer(strInSection), AmigosModuleBase(this)
{
}
AmigosModuleTimer::~AmigosModuleTimer() {}

unsigned int AmigosModuleTimer::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleTimer::GetInputType(unsigned int port) const
{
    return 0;
}
unsigned int AmigosModuleTimer::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
void AmigosModuleTimer::_Init()
{

}
void AmigosModuleTimer::_Deinit()
{

}
int AmigosModuleTimer::_Connected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    auto outportIt = this->mapPortOut.find(outPortId);
    if (outportIt == this->mapPortOut.end())
    {
        AMIGOS_ERR("Could not find output port %d.\n", outPortId);
        return -1;
    }
    TimerReaderDesc desc;
    desc.linker = &outportIt->second.positive;
    if (outportIt->second.positive.empty())
    {
        AMIGOS_ERR("Timer Can't run witout reader!\n");
        return -1;
    }
    ss_thread_attr ss_attr;
    memset(&ss_attr, 0, sizeof(ss_thread_attr));
    ss_attr.do_signal          = NULL;
    ss_attr.do_monitor         = TimerReader;
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
    AMIGOS_INFO("Timer reader [%d] start\n", outPortId);
    return 0;
}
int AmigosModuleTimer::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    auto it = this->map_timer_desc.find(outPortId);
    if (it == this->map_timer_desc.end())
    {
        AMIGOS_ERR("Could not find desc port %d.\n", outPortId);
        return -1;
    }
    if (it->second.linker->empty())
    {
        return -1;
    }
    ss_thread_stop(it->second.threadHandle);
    ss_thread_close(it->second.threadHandle);
    this->map_timer_desc.erase(it);
    AMIGOS_INFO("Timer reader [%d] stop\n", outPortId);
    return 0;
}

void *AmigosModuleTimer::TimerReader(struct ss_thread_buffer *thread_buf)
{
    TimerReaderDesc *pDesc = (TimerReaderDesc *)thread_buf->buf;

    auto now = std::chrono::system_clock::now();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(now);
    std::tm* localtime = std::localtime(&timestamp);

    stream_packet_info packet_info;
    packet_info.en_type              = EN_USER_META_DATA;
    packet_info.meta_data_i.reserved = E_META_DATA_TEXTS;
    packet_info.meta_data_i.size     = sizeof(AmigosRgnMetaDataTexts) + sizeof(AmigosRgnMetaDataText);
    stream_packet_obj packet = std::make_shared<stream_packet>(packet_info);
    assert(packet);

    AmigosRgnMetaDataTexts &texts = *(AmigosRgnMetaDataTexts*)packet->meta_data.data;
    memset(&texts, 0, sizeof(AmigosRgnMetaDataTexts) + sizeof(AmigosRgnMetaDataText));

    texts.count = 1;
    texts.texts[0].pt.x = AMIGOS_RGN_METADATA_COORDINATE_MAX_W;
    texts.texts[0].pt.y = AMIGOS_RGN_METADATA_COORDINATE_MAX_H;

    char mbstr[64];

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> utf8WcCvt;
    snprintf(mbstr, 64, "%4d年%02d月%02d日%02d:%02d:%02d", localtime->tm_year + 1900, localtime->tm_mon + 1,
            localtime->tm_mday, localtime->tm_hour,
            localtime->tm_min, localtime->tm_sec);
    std::wstring wstr = utf8WcCvt.from_bytes(mbstr);
    wstr.copy(texts.texts[0].str, AMIGOS_RGN_METADATA_TEXT_MAX_SIZE);

#if 0
    packet.meta_data_i.reserved = E_META_DATA_LINES;
    AmigosRgnMetaDataLines lines;
    memset(&lines, 0, sizeof(AmigosRgnMetaDataLines));

    srand(time(NULL));

    lines.count = 8;
    for (unsigned int i = 0; i < lines.count; ++i)
    {
        lines.lines[i].pt0.x  = rand() % 8192;
        lines.lines[i].pt0.y  = rand() % 8192;
        lines.lines[i].pt1.x  = rand() % 8192;
        lines.lines[i].pt1.y  = rand() % 8192;
        lines.lines[i].state  = (AmigosRgnMetaDataState)(rand() % 2);
    }

    packet.meta_data.data = (char*)&lines;
    packet.meta_data.size = sizeof(AmigosRgnMetaDataLines);
#endif
#if 0
    packet.meta_data_i.reserved = E_META_DATA_FRAMES;
    AmigosRgnMetaDataFrames frames;
    memset(&frames, 0, sizeof(AmigosRgnMetaDataFrames));

    srand(time(NULL));

    frames.count = 8;
    for (unsigned int i = 0; i < frames.count; ++i)
    {
        frames.frames[i].rect.x = rand() % 8192;
        frames.frames[i].rect.y = rand() % 8192;
        frames.frames[i].rect.w = rand() % 8192;
        frames.frames[i].rect.h = rand() % 8192;
        frames.frames[i].state  = (AmigosRgnMetaDataState)(rand() % 2);
    }

    packet.meta_data.data = (char*)&frames;
    packet.meta_data.size = sizeof(AmigosRgnMetaDataFrames);
#endif
#if 0
    packet.meta_data_i.reserved = E_META_DATA_COVERS;
    AmigosRgnMetaDataCovers covers;
    memset(&covers, 0, sizeof(AmigosRgnMetaDataCovers));

    srand(time(NULL));

    covers.count = 4;
    for (unsigned int i = 0; i < covers.count; ++i)
    {
        covers.covers[i].rect.x = rand() % 8192;
        covers.covers[i].rect.y = rand() % 8192;
        covers.covers[i].rect.w = rand() % 8192;
        covers.covers[i].rect.h = rand() % 8192;
    }

    packet.meta_data.data = (char*)&covers;
    packet.meta_data.size = sizeof(AmigosRgnMetaDataCovers);
#endif
#if 0
    srand(time(NULL));
    const char *strs[] = {
        "abcdefg",
        "hello",
        "efefefeff",
        "ABCDEFGAAAA",
        "ABCDBBBDEFC",
    };
    static int idx = 0;
    packet.meta_data_i.reserved = E_META_DATA_TEXTS;
    AmigosRgnMetaDataTexts texts;
    memset(&texts, 0, sizeof(AmigosRgnMetaDataTexts));

    texts.count = 8;
    for (unsigned int i = 0; i < 8; ++i)
    {
        texts.texts[i].pt.x = rand() % 16384;
        texts.texts[i].pt.y = rand() % 16384;
        strcpy(texts.texts[i].str, strs[idx]);
    }
    idx++;
    if (idx > 4)
    {
        idx = 0;
    }

    packet.meta_data.data = (char*)&texts;
    packet.meta_data.size = sizeof(AmigosRgnMetaDataTexts);

#endif
    pDesc->linker->enqueue(packet);
    return NULL;
}
AMIGOS_MODULE_INIT("TIMER", AmigosModuleTimer);

