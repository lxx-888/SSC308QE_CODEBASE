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
#include <string>
#include "amigos_module_init.h"
#include "amigos_surface_base.h"
#include "amigos_module_mi_base.h"
#include "amigos_module_scl_base.h"
#include "amigos_module_scl_stretch.h"
#include "amigos_surface_scl_stretch.h"
#include "mi_common_datatype.h"
#include "mi_sys_datatype.h"
#include "mi_sys.h"
#include "mi_scl_datatype.h"
#include "mi_scl.h"
#include "ss_enum_cast.hpp"
#include "ss_packet.h"
#include "ss_thread.h"

#define SCL_STRETCH_OSD_W    (16384)
#define SCL_STRETCH_OSD_H    (64)
#define SclStretchOsdPixel_t short

static MI_SYS_PixelFormat_e _ConvertRawVideoFmtToPixelFormat(raw_video_fmt fmt, unsigned int y_div[2])
{
    y_div[0] = 1;
    y_div[1] = 1;
    switch (fmt)
    {
        case RAW_FORMAT_YUV422_YUYV:
            return E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        case RAW_FORMAT_YUV422_UYVY:
            return E_MI_SYS_PIXEL_FRAME_YUV422_UYVY;
        case RAW_FORMAT_YUV422_YVYU:
            return E_MI_SYS_PIXEL_FRAME_YUV422_YVYU;
        case RAW_FORMAT_YUV422_VYUY:
            return E_MI_SYS_PIXEL_FRAME_YUV422_VYUY;
        case RAW_FORMAT_YUV422SP:
            return E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR;
        case RAW_FORMAT_YUV420SP:
            y_div[0] = 1;
            y_div[1] = 2;
            return E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        case RAW_FORMAT_YUV420SP_NV21:
            y_div[0] = 1;
            y_div[1] = 2;
            return E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21;
        case RAW_FORMAT_RGB888:
            return E_MI_SYS_PIXEL_FRAME_RGB888;
        case RAW_FORMAT_BGR888:
            return E_MI_SYS_PIXEL_FRAME_BGR888;
        case RAW_FORMAT_ARGB8888:
            return E_MI_SYS_PIXEL_FRAME_ARGB8888;
        case RAW_FORMAT_ABGR8888:
            return E_MI_SYS_PIXEL_FRAME_ABGR8888;
        case RAW_FORMAT_BGRA8888:
            return E_MI_SYS_PIXEL_FRAME_BGRA8888;
        case RAW_FORMAT_RGB565:
            return E_MI_SYS_PIXEL_FRAME_RGB565;
        case RAW_FORMAT_ARGB1555:
            return E_MI_SYS_PIXEL_FRAME_ARGB1555;
        case RAW_FORMAT_ARGB4444:
            return E_MI_SYS_PIXEL_FRAME_ARGB4444;
        case RAW_FORMAT_I2:
            return E_MI_SYS_PIXEL_FRAME_I2;
        case RAW_FORMAT_I4:
            return E_MI_SYS_PIXEL_FRAME_I4;
        case RAW_FORMAT_I8:
            return E_MI_SYS_PIXEL_FRAME_I8;
        case RAW_FORMAT_BAYER_BASE...RAW_FORMAT_BAYER_NUM:
            return MI_SYS_PixelFormat_e(E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE + fmt - RAW_FORMAT_BAYER_BASE);
        default:
            break;
    }
    return E_MI_SYS_PIXEL_FRAME_FORMAT_MAX;
}

AmigosModuleSclStretch::AmigosModuleSclStretch(const std::string &strSection)
    : AmigosSurfaceSclStretch(strSection), AmigosModuleSclBase(this),phyOsdBuf(0)
{
}
AmigosModuleSclStretch::~AmigosModuleSclStretch() {}

unsigned int AmigosModuleSclStretch::GetModId() const
{
    return E_MI_MODULE_ID_SCL;
}
unsigned int AmigosModuleSclStretch::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleSclStretch::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
void AmigosModuleSclStretch::_Init()
{
    MI_SCL_DevAttr_t stSclDevAttr;
    memset(&stSclDevAttr, 0, sizeof(MI_SCL_DevAttr_t));
    stSclDevAttr.u32NeedUseHWOutPortMask = this->stSclStretchInfo.uintHwPortMode;
    this->_CreateDevice(this->stModInfo.devId, stSclDevAttr);
    if (!this->stSclStretchInfo.uintOsdEn)
    {
        return;
    }
    // Alloc and fill osd buffer
    if (MI_SUCCESS
        != MI_SYS_MMA_Alloc(0, NULL, SCL_STRETCH_OSD_W * SCL_STRETCH_OSD_H * sizeof(SclStretchOsdPixel_t),
                            &this->phyOsdBuf))
    {
        AMIGOS_ERR("MI_SYS_Mma_Alloc failed.\n");
        return;
    }
    SclStretchOsdPixel_t *virt = NULL;
    if (MI_SUCCESS
        != MI_SYS_Mmap(this->phyOsdBuf, SCL_STRETCH_OSD_W * SCL_STRETCH_OSD_H * sizeof(SclStretchOsdPixel_t),
                       (void **)&virt, TRUE))
    {
        AMIGOS_ERR("MI_SYS_Mmap failed\n");
        return;
    }
    SclStretchOsdPixel_t val = 0;
    for (unsigned int i = 0; i < SCL_STRETCH_OSD_H; ++i)
    {
        val = 0;
        for (unsigned int j = 0; j < SCL_STRETCH_OSD_W; ++j)
        {
            virt[i * SCL_STRETCH_OSD_W + j] = val;
            val += 2;
        }
    }
    MI_SYS_FlushInvCache(virt, SCL_STRETCH_OSD_W * SCL_STRETCH_OSD_H * sizeof(SclStretchOsdPixel_t));
    MI_SYS_Munmap(virt, SCL_STRETCH_OSD_W * SCL_STRETCH_OSD_H * sizeof(SclStretchOsdPixel_t));
}
void AmigosModuleSclStretch::_Deinit()
{
    this->_DestroyDevice(this->stModInfo.devId);
    if (this->phyOsdBuf)
    {
        MI_SYS_MMA_Free(0, this->phyOsdBuf);
        this->phyOsdBuf = NULL;
    }
}
int AmigosModuleSclStretch::_FillPacket(unsigned int outPortId, stream_packet_obj &packet)
{
    auto itOut = this->mapPortOut.find(outPortId);
    if (itOut == this->mapPortOut.end())
    {
        AMIGOS_ERR("Base out port %d is not found\n", outPortId);
        return -1;
    }
    auto itSclStretchOut = this->mapSclStretchOut.find(outPortId);
    if (itSclStretchOut == this->mapSclStretchOut.end())
    {
        AMIGOS_ERR("SclStretch out port %d is not found\n", outPortId);
        return -1;
    }
    unsigned int inPortId = outPortId;
    auto itIn = this->mapPortIn.find(inPortId);
    if (itIn == this->mapPortIn.end())
    {
        AMIGOS_ERR("Base in port %d is not found\n", inPortId);
        return -1;
    }
    auto itSclStretchIn = this->mapSclStretchIn.find(inPortId);
    if (itSclStretchIn == this->mapSclStretchIn.end())
    {
        AMIGOS_ERR("SclStretch in port %d is not found\n", inPortId);
        return -1;
    }
    if (itSclStretchOut->second.uintRowNum == 0 || itSclStretchOut->second.uintColNum == 0)
    {
        AMIGOS_ERR("SclStretch layout error [ %dx%d ]\n", itSclStretchOut->second.uintRowNum,
                   itSclStretchOut->second.uintColNum);
        return -1;
    }
    for (unsigned int i = 0; i < itSclStretchOut->second.uintRowNum; ++i)
    {
        for (unsigned int j = 0; j < itSclStretchOut->second.uintColNum; ++j)
        {
            LinkerAsyncNegative *asyncLinker = static_cast<LinkerAsyncNegative *>(itIn->second.negative);
            auto packetSrc = asyncLinker->WaitPacket(-1);
            if (!packetSrc)
            {
                return -1;
            }
            MI_SCL_DirectBuf_t stSrcBuf;
            MI_SCL_DirectBuf_t stDstBuf;
            MI_SYS_WindowRect_t stSrcRect;
            MI_SCL_FilterType_e eFilterType;

            memset(&stSrcBuf, 0, sizeof(MI_SCL_DirectBuf_t));
            memset(&stDstBuf, 0, sizeof(MI_SCL_DirectBuf_t));
            memset(&stSrcRect, 0, sizeof(MI_SYS_WindowRect_t));

            stSrcRect.u16X      = itSclStretchIn->second.uintCropX;
            stSrcRect.u16Y      = itSclStretchIn->second.uintCropY;
            stSrcRect.u16Width  = itSclStretchIn->second.uintCropW;
            stSrcRect.u16Height = itSclStretchIn->second.uintCropH;

            unsigned int y_div[2] = {1, 1};
            stSrcBuf.ePixelFormat = _ConvertRawVideoFmtToPixelFormat(packetSrc->raw_vid_i.plane_info[0].fmt, y_div);
            stSrcBuf.u32Width     = packetSrc->raw_vid_i.plane_info[0].width;
            stSrcBuf.u32Height    = packetSrc->raw_vid_i.plane_info[0].height;
            stSrcBuf.u32BuffSize  = packetSrc->raw_vid_pa.plane_data[0].size[0] + packetSrc->raw_vid_pa.plane_data[0].size[1];
            stSrcBuf.u32Stride[0] = packetSrc->raw_vid_pa.plane_data[0].stride[0];
            stSrcBuf.u32Stride[1] = packetSrc->raw_vid_pa.plane_data[0].stride[1];
            stSrcBuf.phyAddr[0]   = packetSrc->raw_vid_pa.plane_data[0].phy[0];
            stSrcBuf.phyAddr[1]   = packetSrc->raw_vid_pa.plane_data[0].phy[1];

            stDstBuf.ePixelFormat = _ConvertRawVideoFmtToPixelFormat(packet->raw_vid_i.plane_info[0].fmt, y_div);
            stDstBuf.u32Width     = itSclStretchIn->second.uintCropW;
            stDstBuf.u32Height    = itSclStretchIn->second.uintCropH;
            stDstBuf.u32BuffSize  = stSrcBuf.u32BuffSize;
            stDstBuf.u32Stride[0] = packet->raw_vid_pa.plane_data[0].stride[0];
            stDstBuf.u32Stride[1] = packet->raw_vid_pa.plane_data[0].stride[1];
            stDstBuf.phyAddr[0]   = packet->raw_vid_pa.plane_data[0].phy[0]
                                  + stDstBuf.u32Stride[0] * itSclStretchIn->second.uintCropH * i / y_div[0]
                                  + stSrcBuf.u32Stride[0] * j;
            stDstBuf.phyAddr[1]   = packet->raw_vid_pa.plane_data[0].phy[1]
                                  + stDstBuf.u32Stride[1] * itSclStretchIn->second.uintCropH * i / y_div[1]
                                  + stSrcBuf.u32Stride[1] * j;

            eFilterType = E_MI_SCL_FILTER_TYPE_AUTO;

            if (this->phyOsdBuf && i == 0)
            {
                MI_SCL_DirectOsdBuf_t stDstOsdBuf;
                memset(&stDstOsdBuf, 0, sizeof(MI_SCL_DirectOsdBuf_t));
                stDstOsdBuf.ePixelFormat                           = E_MI_SYS_PIXEL_FRAME_ARGB1555;
                stDstOsdBuf.u8OsdBufCnt                            = 1;
                stDstOsdBuf.astOsdBuf[0].phyAddr =
                    this->phyOsdBuf + itSclStretchIn->second.uintCropW * sizeof(SclStretchOsdPixel_t) * j;
                stDstOsdBuf.astOsdBuf[0].u32X                      = 0;
                stDstOsdBuf.astOsdBuf[0].u32Y                      = 0;
                stDstOsdBuf.astOsdBuf[0].u32Width                  = SCL_STRETCH_OSD_W;
                stDstOsdBuf.astOsdBuf[0].u32Height                 = SCL_STRETCH_OSD_H;
                stDstOsdBuf.astOsdBuf[0].u32Stride                 = SCL_STRETCH_OSD_W * sizeof(SclStretchOsdPixel_t);
                stDstOsdBuf.astOsdBuf[0].stOsdAlphaAttr.eAlphaMode = E_MI_SCL_DIRECT_BUF_OSD_CONSTANT_ALPHT;
                stDstOsdBuf.astOsdBuf[0].stOsdAlphaAttr.stAlphaPara.u8ConstantAlpha = 0xff;
                if (MI_SUCCESS != MI_SCL_StretchBufOsd(&stSrcBuf, &stSrcRect, &stDstBuf, &stDstOsdBuf, eFilterType))
                {
                    AMIGOS_ERR("MI_SCL_StretchBufOsd Failed\n");
                    return -1;
                }
            }
            else
            {
                if (MI_SUCCESS != MI_SCL_StretchBuf(&stSrcBuf, &stSrcRect, &stDstBuf, eFilterType))
                {
                    AMIGOS_ERR("MI_SCL_StretchBuf Failed\n");
                    return -1;
                }
            }
            packetSrc = nullptr;
        }
    }
    return 0;
}
stream_packet_obj AmigosModuleSclStretch::_DequeueFromInside(unsigned int outPortId, unsigned int ms)
{
    stream_packet_info packet_info;
    unsigned int inPortId = outPortId;
    auto itSclStretchOut = this->mapSclStretchOut.find(outPortId);
    if (itSclStretchOut == this->mapSclStretchOut.end())
    {
        AMIGOS_ERR("SclStretch out port %d is not found\n", outPortId);
        return NULL;
    }
    auto itSclStretchIn = this->mapSclStretchIn.find(inPortId);
    if (itSclStretchIn == this->mapSclStretchIn.end())
    {
        AMIGOS_ERR("SclStretch in port %d is not found\n", inPortId);
        return NULL;
    }
    if (itSclStretchOut->second.uintRowNum == 0 || itSclStretchOut->second.uintColNum == 0)
    {
        AMIGOS_ERR("SclStretch layout error [ %dx%d ]\n", itSclStretchOut->second.uintRowNum,
                   itSclStretchOut->second.uintColNum);
        return NULL;
    }
    packet_info.en_type = EN_RAW_FRAME_DATA_PA;
    packet_info.raw_vid_i.plane_num = 1;
    packet_info.raw_vid_i.plane_info[0].fmt = ss_enum_cast<raw_video_fmt>::from_str(itSclStretchOut->second.strOutFmt);
    packet_info.raw_vid_i.plane_info[0].width  = itSclStretchIn->second.uintCropW * itSclStretchOut->second.uintColNum;
    packet_info.raw_vid_i.plane_info[0].height = itSclStretchIn->second.uintCropH * itSclStretchOut->second.uintRowNum;
    stream_packet_obj packet = stream_packet_base::make<StreamPacketSysMma>(packet_info.raw_vid_i);
    if (!packet)
    {
        AMIGOS_ERR("Make packet error in output port %d from post module.\n", outPortId);
        return nullptr;
    }
    return (this->_FillPacket(outPortId, packet) == 0) ? packet : nullptr;
}
int AmigosModuleSclStretch::_InConnect(unsigned int inPortId)
{
    auto itOut = this->mapPortOut.find(inPortId);
    if (itOut == this->mapPortOut.end())
    {
        AMIGOS_ERR("Base out port %d is not found\n", inPortId);
        return -1;
    }
    if (this->mapReaderDesc.find(inPortId) != this->mapReaderDesc.end())
    {
        AMIGOS_ERR("in port reader %d is started\n", inPortId);
        return -1;
    }
    SclStretchReaderDesc desc;
    desc.linker  = &itOut->second.positive;
    desc.packer  = &itOut->second.outPacker;
    desc.pModule = this;
    if (itOut->second.positive.empty())
    {
        AMIGOS_INFO("SclStretch reader is not needed\n");
        mapReaderDesc[inPortId] = desc;
        return 0;
    }
    ss_thread_attr ss_attr;
    memset(&ss_attr, 0, sizeof(ss_thread_attr));
    ss_attr.do_signal          = NULL;
    ss_attr.do_monitor         = SclStretchReader;
    ss_attr.monitor_cycle_sec  = 0;
    ss_attr.monitor_cycle_nsec = 0;
    ss_attr.is_reset_timer     = 0;
    ss_attr.in_buf.buf         = (&this->mapReaderDesc[inPortId]);
    ss_attr.in_buf.size        = 0;
    snprintf(ss_attr.thread_name, 128, "%s", this->GetInPortIdStr(inPortId).c_str());
    desc.threadHandle = ss_thread_open(&ss_attr);
    if (!desc.threadHandle)
    {
        this->mapReaderDesc.erase(inPortId);
        AMIGOS_ERR("ss_thread_open error!\n");
        return -1;
    }
    mapReaderDesc[inPortId] = desc;
    ss_thread_start_monitor(desc.threadHandle);
    return 0;
}
int AmigosModuleSclStretch::_InDisconnect(unsigned int inPortId)
{
    auto iter = this->mapReaderDesc.find(inPortId);
    if (iter == this->mapReaderDesc.end())
    {
        AMIGOS_ERR("out port reader %d is not started\n", inPortId);
        return -1;
    }
    if (iter->second.linker->empty())
    {
        this->mapReaderDesc.erase(iter);
        AMIGOS_INFO("SclStretch reader is not needed\n");
        return 0;
    }
    auto itIn = this->mapPortIn.find(inPortId);
    if (itIn == this->mapPortIn.end())
    {
        AMIGOS_ERR("Base in port %d is not found\n", inPortId);
        return -1;
    }
    LinkerAsyncNegative *asyncLinker = static_cast<LinkerAsyncNegative *>(itIn->second.negative);
    asyncLinker->EndMonitor();
    ss_thread_stop(iter->second.threadHandle);
    ss_thread_close(iter->second.threadHandle);
    this->mapReaderDesc.erase(iter);
    return 0;
}

ss_linker_base *AmigosModuleSclStretch::_CreateInputNegativeLinker(unsigned int inPortId)
{
    return new LinkerAsyncNegative(false, 8);
}

stream_packer *AmigosModuleSclStretch::_CreateInputStreamPacker(unsigned int inPortId, bool &bFast)
{
    return new StreamPackerSysMma();
}
void *AmigosModuleSclStretch::SclStretchReader(struct ss_thread_buffer *thread_buf)
{
    stream_packet_info packet_info;
    SclStretchReaderDesc *pDesc = (SclStretchReaderDesc *)thread_buf->buf;
    unsigned int inPortId = pDesc->outPortId;
    auto itSclStretchOut = pDesc->pModule->mapSclStretchOut.find(pDesc->outPortId);
    if (itSclStretchOut == pDesc->pModule->mapSclStretchOut.end())
    {
        AMIGOS_ERR("SclStretch out port %d is not found\n", pDesc->outPortId);
        return NULL;
    }
    auto itSclStretchIn = pDesc->pModule->mapSclStretchIn.find(inPortId);
    if (itSclStretchIn == pDesc->pModule->mapSclStretchIn.end())
    {
        AMIGOS_ERR("SclStretch in port %d is not found\n", inPortId);
        return NULL;
    }
    if (itSclStretchOut->second.uintRowNum == 0 || itSclStretchOut->second.uintColNum == 0)
    {
        AMIGOS_ERR("SclStretch layout error [ %dx%d ]\n", itSclStretchOut->second.uintRowNum,
                   itSclStretchOut->second.uintColNum);
        return NULL;
    }
    packet_info.en_type = EN_RAW_FRAME_DATA_PA;
    packet_info.raw_vid_i.plane_num = 1;
    packet_info.raw_vid_i.plane_info[0].fmt = ss_enum_cast<raw_video_fmt>::from_str(itSclStretchOut->second.strOutFmt);
    packet_info.raw_vid_i.plane_info[0].width  = itSclStretchIn->second.uintCropW * itSclStretchOut->second.uintColNum;
    packet_info.raw_vid_i.plane_info[0].height = itSclStretchIn->second.uintCropH * itSclStretchOut->second.uintRowNum;
    auto packet = pDesc->packer->make(packet_info);
    if (!packet)
    {
        AMIGOS_ERR("Make packet error in output port %d from post module.\n", pDesc->outPortId);
        return NULL;
    }
    if (pDesc->pModule->_FillPacket(pDesc->outPortId, packet) == 0)
    {
        pDesc->linker->enqueue(packet);
    }
    return NULL;
}

AMIGOS_MODULE_INIT("SCL_STRETCH", AmigosModuleSclStretch);

