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
#include "amigos_env.h"
#include "amigos_module_init.h"
#include "amigos_module_uvc.h"
#include "ss_packet.h"

SS_ENUM_CAST_STR(UVC_IO_MODE_e,
{
    {UVC_MEMORY_MMAP,    "MMAP"},
    {UVC_MEMORY_USERPTR, "USERPTR"},
});
SS_ENUM_CAST_STR(Transfer_Mode_e,
{
    {USB_ISOC_MODE,    "ISOC"},
    {USB_BULK_MODE,    "BULK"},
});

static bool _GetH26xKeyFrame(bool bH264, unsigned char* pData, unsigned long length)
{
    bool ret = false;
    if (nullptr == pData || 4 > length)
    {
        return ret;
    }
    unsigned char naluType = pData[0];
    if (bH264)
    {
        unsigned char nt = pData[0] & 0x1F;
        unsigned char ft = pData[0] & 0x1F;
        if (nt == 0x1C)
        {
            ft = pData[1] & 0x1F;
        }
        if (ft == 5)
        {
            ret = true;
        }
    }
    else
    {
        naluType = ((naluType & 0x7E) >> 1);
        if ((naluType >= 0x10 && naluType <= 0x15) ||
            (naluType >= 0x23 && naluType <= 0x26))
        {
            ret = true;
        }
    }
    return ret;
}

AmigosModuleUvc::AmigosModuleUvc(const std::string &strInSection)
    : AmigosSurfaceUvc(strInSection), AmigosModuleBase(this), srcLinker(nullptr), curInport(0), handle(nullptr), streamType(0)
{
    AMILOG_INFO << "func: " << __FUNCTION__ <<COLOR_ENDL;
}
AmigosModuleUvc::~AmigosModuleUvc()
{
    AMILOG_INFO << "func: " << __FUNCTION__ <<COLOR_ENDL;
}
unsigned int AmigosModuleUvc::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleUvc::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleUvc::GetOutputType(unsigned int port) const
{
    return 0;
}

void AmigosModuleUvc::_Init()
{
}
void AmigosModuleUvc::_Deinit()
{
}
void AmigosModuleUvc::_Start()
{
    SS_UVC_Setting_t pstSet={ this->stUvcInfo.ucMaxCnt,
                              this->stUvcInfo.uintMaxPacket,
                              this->stUvcInfo.ucMult,
                              this->stUvcInfo.ucBurst,
                              this->stUvcInfo.ucCIntf,
                              this->stUvcInfo.ucSIntf,
                              (UVC_IO_MODE_e)ss_enum_cast<UVC_IO_MODE_e>::from_str(this->stUvcInfo.strMode),
                              (Transfer_Mode_e)ss_enum_cast<Transfer_Mode_e>::from_str(this->stUvcInfo.strType) };
    SS_UVC_MMAP_BufOpts_t m = {MmapFillBuffer};
    SS_UVC_USERPTR_BufOpts_t u = {UserPtrFillBuffer, UserPtrFinishBuffer};
    SS_UVC_OPS_t fops = { UvcInit,
                          UvcDeinit,
                          {{}},
                          StartCapture,
                          StopCapture,
                          ForceIdr};
    if (this->stUvcInfo.strMode == "MMAP")
    {
        fops.m = m;
    }
    else
    {
        fops.u = u;
    }
    SS_UVC_ChnAttr_t pstAttr = {pstSet, fops};

    std::stringstream ss;
    ss << "/dev/video" << this->stModInfo.devId;
    if (SS_UVC_SUCCESS != SS_UVC_Init(ss.str().c_str(), this, &handle))
    {
        AMILOG_ERR << "uvc init fail" << COLOR_ENDL;
        return;
    }
    if (SS_UVC_SUCCESS != SS_UVC_CreateDev(handle, &pstAttr))
    {
        AMILOG_ERR << "UVC Create fail" << COLOR_ENDL;
        SS_UVC_Uninit(handle);
        return;
    }
    if (SS_UVC_SUCCESS != SS_UVC_StartDev(handle))
    {
        AMILOG_ERR << "UVC Start fail" << COLOR_ENDL;
        SS_UVC_DestroyDev(handle);
        SS_UVC_Uninit(handle);
        return;
    }
    SaveEnvDevPath();
    AMILOG_INFO << __func__ << ", nodedev:" << ss.str() << ", mult:" << (int)this->stUvcInfo.ucMult <<
                   ", burst:" << (int)this->stUvcInfo.ucBurst << ", ci:" << (int)this->stUvcInfo.ucCIntf <<
                   ", si:" << (int)this->stUvcInfo.ucSIntf << ", Mode:" << this->stUvcInfo.strMode <<
                   ", Type:" << this->stUvcInfo.strType << COLOR_ENDL;
}
void AmigosModuleUvc::_Stop()
{
    ClearEnvDevPath();
    if (SS_UVC_SUCCESS != SS_UVC_StopDev(handle))
    {
        AMILOG_ERR << "Stop fail" << COLOR_ENDL;
    }
    if (SS_UVC_SUCCESS != SS_UVC_DestroyDev(handle))
    {
        AMILOG_ERR << "Destroy fail" << COLOR_ENDL;
    }
    if (SS_UVC_SUCCESS != SS_UVC_Uninit(handle))
    {
        AMILOG_ERR << "Uninit fail" << COLOR_ENDL;
    }
    handle = nullptr;
}
void AmigosModuleUvc::SaveEnvDevPath()
{
    std::stringstream ss, vid;
    ss << "UVC" << this->stModInfo.devId;
    vid << this->env.Ext("UVC_PERFIX")["UVC_PERFIX_NAME"] << "video" << this->stModInfo.devId;
    this->env.Ext("UVC_PREVIEW_WINDOWS")[ss.str()] = vid.str();
}
void AmigosModuleUvc::ClearEnvDevPath()
{
    std::stringstream ss;
    ss << "UVC" << this->stModInfo.devId;
    this->env.Ext("UVC_PREVIEW_WINDOWS")[ss.str()] = "";
}
int AmigosModuleUvc::StartCapture(void *uvc,Stream_Params_t format)
{
    AmigosModuleUvc *Obj = (AmigosModuleUvc*)((SS_UVC_Device_t*)uvc)->private_data;
    if (!Obj || Obj->srcLinker)
    {
        AMILOG_ERR << "Object is Null or Linker is not Null" << COLOR_ENDL;
        return -1;
    }

    Obj->streamType = format.fcc;
    Obj->width = format.width;
    Obj->height = format.height;
    AMILOG_INFO << "nodedev:/dev/video" << Obj->stModInfo.devId << ", streamType:" << format.fcc << ", width:" << Obj->width <<
                   ", height:" << Obj->height << " ,frameRate:" << format.frameRate << COLOR_ENDL;
    switch (Obj->streamType)
    {
        case V4L2_PIX_FMT_H264:
        {
            Obj->curInport = 0;
        }
        break;
        case V4L2_PIX_FMT_H265:
        {
            Obj->curInport = 1;
        }
        break;
        case V4L2_PIX_FMT_MJPEG:
        {
            Obj->curInport = 2;
        }
        break;
        case V4L2_PIX_FMT_NV12:
        {
            Obj->curInport = 3;
        }
        break;
        case V4L2_PIX_FMT_YUYV:
        {
            Obj->curInport = 4;
        }
        break;
        case V4L2_PIX_FMT_AV01:
        {
            Obj->curInport = 5;
        }
        break;
        default:
        {
            AMILOG_ERR << "streamType err " << Obj->streamType << COLOR_ENDL;
            return -1;
        }
        break;
    }
    Obj->CreateDelayPass(Obj->curInport);
    Obj->InitDelayPass(Obj->curInport);
    Obj->BindDelayPass(Obj->curInport);
    Obj->ResetStream(Obj->curInport, format.width, format.height);
    Obj->StartDelayPass(Obj->curInport);
    auto iter = Obj->mapPortIn.find(Obj->curInport);
    if(iter == Obj->mapPortIn.end())
    {
        AMILOG_ERR << "Base port " << Obj->curInport << " not found" << COLOR_ENDL;
        Obj->StopDelayPass(Obj->curInport);
        Obj->UnbindDelayPass(Obj->curInport);
        Obj->DeinitDelayPass(Obj->curInport);
        Obj->DestroyDelayPass(Obj->curInport);
        return -1;
    }
    iter->second.access();
    Obj->srcLinker = &iter->second.positive;
    return 0;
}
int AmigosModuleUvc::StopCapture(void *uvc)
{
    AmigosModuleUvc *Obj = (AmigosModuleUvc*)((SS_UVC_Device_t*)uvc)->private_data;
    if (!Obj || !Obj->srcLinker)
    {
        AMILOG_ERR << "Object or Linker is Null" << COLOR_ENDL;
        return -1;
    }
    auto iter = Obj->mapPortIn.find(Obj->curInport);
    if(iter == Obj->mapPortIn.end())
    {
        AMILOG_ERR << "Base port " << Obj->curInport << " not found" << COLOR_ENDL;
        return -1;
    }
    Obj->srcLinker = nullptr;
    iter->second.leave();
    Obj->StopDelayPass(Obj->curInport);
    Obj->UnbindDelayPass(Obj->curInport);
    Obj->DeinitDelayPass(Obj->curInport);
    Obj->DestroyDelayPass(Obj->curInport);
    Obj->curInport = -1;
    return 0;
}
int AmigosModuleUvc::MmapFillBuffer(void *uvc,SS_UVC_BufInfo_t *bufInfo)
{
    AmigosModuleUvc *Obj = (AmigosModuleUvc*)((SS_UVC_Device_t*)uvc)->private_data;
    if (!Obj || !Obj->srcLinker)
    {
        AMILOG_ERR << "Object or Linker is Null" << COLOR_ENDL;
        return -1;
    }
    unsigned char *u8CopyData = (unsigned char *)bufInfo->b.buf;
    unsigned int *pu32length = (unsigned int *)&bufInfo->length;

    auto packet = Obj->srcLinker->dequeue(100);
    if(packet == nullptr)
    {
        //Time out!
        return -1;
    }
    switch (Obj->streamType)
    {
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_NV12:
        {
            stream_packet_obj va = stream_packet_base::convert(packet, EN_RAW_FRAME_DATA);
            if (!va)
            {
                AMILOG_ERR << "Packet type: " << packet->get_type() << ", can not convert to va packet!" << COLOR_ENDL;
                return -1;
            }
            for (int i = 0; i < va->raw_vid_i.plane_num; i++)
            {
                if (va->raw_vid.plane_data[i].data[0])
                {
                    memcpy(u8CopyData, va->raw_vid.plane_data[i].data[0], va->raw_vid.plane_data[i].size[0]);
                    u8CopyData += va->raw_vid.plane_data[i].size[0];
                }
                if (va->raw_vid.plane_data[i].data[1])
                {
                    memcpy(u8CopyData, va->raw_vid.plane_data[i].data[1], va->raw_vid.plane_data[i].size[1]);
                    u8CopyData += va->raw_vid.plane_data[i].size[1];
                }
                if (va->raw_vid.plane_data[i].data[2])
                {
                    memcpy(u8CopyData, va->raw_vid.plane_data[i].data[2], va->raw_vid.plane_data[i].size[2]);
                    u8CopyData += va->raw_vid.plane_data[i].size[2];
                }
            }
            bufInfo->is_keyframe = false;
            *pu32length = u8CopyData - (unsigned char*)bufInfo->b.buf;
        }
        break;
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_AV01:
        {
            for (unsigned int i = 0; i < packet->es_vid_i.packet_count; i++)
            {
                memcpy(u8CopyData, packet->es_vid.packet_data[i].data, packet->es_vid_i.packet_info[i].size);
                u8CopyData += packet->es_vid_i.packet_info[i].size;
            }
            *pu32length = u8CopyData - (unsigned char*)bufInfo->b.buf;
            bufInfo->is_tail = true;
            if (V4L2_PIX_FMT_H264 == Obj->streamType || V4L2_PIX_FMT_H265 == Obj->streamType)
            {
                bufInfo->is_keyframe = _GetH26xKeyFrame(V4L2_PIX_FMT_H264 == Obj->streamType, u8CopyData, *pu32length);
            }
        }
        break;
        default:
        {
            AMILOG_ERR << "streamType err " << Obj->streamType << COLOR_ENDL;
        }
    }
    return 0;
}
int AmigosModuleUvc::UserPtrFillBuffer(void *uvc,SS_UVC_BufInfo_t *bufInfo)
{
    AmigosModuleUvc *Obj = (AmigosModuleUvc*)((SS_UVC_Device_t*)uvc)->private_data;
    if (!Obj || !Obj->srcLinker)
    {
        AMILOG_ERR << "Object or Linker is Null" << COLOR_ENDL;
        return -1;
    }
    stream_packet_obj *pUserPtrStream = new stream_packet_obj;
    auto packet = Obj->srcLinker->dequeue(100);
    if(packet == nullptr)
    {
        delete pUserPtrStream;
        pUserPtrStream = nullptr;
        return -1;
    }
    switch (Obj->streamType)
    {
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_NV12:
        {
            stream_packet_obj va = stream_packet_base::convert(packet, EN_RAW_FRAME_DATA);
            if (!va)
            {
                AMILOG_ERR << "Packet type: " << packet->get_type() << ", can not convert to va packet!" << COLOR_ENDL;
                delete pUserPtrStream;
                pUserPtrStream = nullptr;
                return -1;
            }
            *pUserPtrStream = va;
            bufInfo->b.start = (long unsigned int)va->raw_vid.plane_data[0].data[0];
            bufInfo->length = va->raw_vid.plane_data[0].size[0]
                            + va->raw_vid.plane_data[0].size[1]
                            + va->raw_vid.plane_data[0].size[2];
            bufInfo->is_keyframe = false;
        }
        break;
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_AV01:
        {
            *pUserPtrStream = packet;
            bufInfo->b.start = (long unsigned int)packet->es_vid.packet_data[0].data;
            bufInfo->length = packet->es_vid_i.packet_info[0].size;
            if (V4L2_PIX_FMT_H264 == Obj->streamType || V4L2_PIX_FMT_H265 == Obj->streamType)
            {
                bufInfo->is_keyframe = _GetH26xKeyFrame(V4L2_PIX_FMT_H264 == Obj->streamType,
                        (unsigned char *)packet->es_vid.packet_data[0].data, bufInfo->length);
            }
        }
        break;
        default:
        {
            AMILOG_ERR << "streamType err " << Obj->streamType << COLOR_ENDL;
        }
    }
    bufInfo->b.handle = (long unsigned int)pUserPtrStream;

    return 0;
}
int AmigosModuleUvc::UserPtrFinishBuffer(void *uvc,SS_UVC_BufInfo_t *bufInfo)
{
    AmigosModuleUvc *Obj = (AmigosModuleUvc*)((SS_UVC_Device_t*)uvc)->private_data;
    if (!Obj || !Obj->srcLinker)
    {
        AMILOG_ERR << "Object or Linker is Null" << COLOR_ENDL;
        return -1;
    }
    stream_packet_obj *pUserPtrStream = (stream_packet_obj*)bufInfo->b.handle;
    delete pUserPtrStream;
    pUserPtrStream = nullptr;
    return 0;
}
void AmigosModuleUvc::ForceIdr(void *uvc)
{
    AmigosModuleUvc *Obj = (AmigosModuleUvc*)((SS_UVC_Device_t*)uvc)->private_data;
    if (!Obj || !Obj->srcLinker)
    {
        AMILOG_ERR << "Object or Linker is Null" << COLOR_ENDL;
        return;
    }
    if ((Obj->streamType == V4L2_PIX_FMT_H264) || (Obj->streamType == V4L2_PIX_FMT_H265))
    {
        Obj->ResetStream(Obj->curInport, Obj->width, Obj->height);
    }
}
AMIGOS_MODULE_INIT("UVC", AmigosModuleUvc);
