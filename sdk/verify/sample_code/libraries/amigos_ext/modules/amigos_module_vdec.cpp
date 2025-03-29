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
#include <assert.h>

#include <vector>
#include <string>

#include "mi_sys.h"
#include "mi_vdec.h"
#include "mi_common.h"
#include "mi_vdec_datatype.h"
#include "ss_linker.h"
#include "amigos_module_init.h"
#include "amigos_module_vdec.h"
#include "mi_common_datatype.h"

#define VDEC_ALIGN_2xUP(x)               (((x+1) / 2) * 2)
#define VDEC_ALIGN_16xUP(x)              (((x+15) / 16) * 16)
static MI_VDEC_CodecType_e _ConvertVideoFmtToVdecCodecType(es_video_fmt fmt)
{
    switch (fmt)
    {
        case ES_STREAM_H264:
            return E_MI_VDEC_CODEC_TYPE_H264;
        case ES_STREAM_H265:
            return E_MI_VDEC_CODEC_TYPE_H265;
        case ES_STREAM_JPEG:
            return E_MI_VDEC_CODEC_TYPE_JPEG;
        default:
            break;
    }
    return E_MI_VDEC_CODEC_TYPE_MAX;
}
AmigosModuleVdec::AmigosModuleVdec(const std::string &strSection)
    : AmigosSurfaceVdec(strSection), AmigosModuleMiBase(this)
{
}
AmigosModuleVdec::~AmigosModuleVdec()
{
}

unsigned int AmigosModuleVdec::GetModId() const
{
    return E_MI_MODULE_ID_VDEC;
}
unsigned int AmigosModuleVdec::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleVdec::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
void AmigosModuleVdec::_Init()
{
    MI_VDEC_ChnAttr_t        stVdecChnAttr;
    MI_VDEC_OutputPortAttr_t stOutputPortAttr;
    MI_VDEC_CropCfg_t        stCropCfg;

    memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = stVdecInfo.refFrameNum;
    stVdecChnAttr.eVideoMode                     = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize                     = stVdecInfo.bitstreamSize * 1024 * 1024;
    stVdecChnAttr.u32PicWidth                    = stVdecInfo.uintBufWidth;
    stVdecChnAttr.u32PicHeight                   = stVdecInfo.uintBufHeight;
    stVdecChnAttr.u32Priority                    = 0;
    stVdecChnAttr.eCodecType                     = E_MI_VDEC_CODEC_TYPE_H265;
    stVdecChnAttr.eDpbBufMode                    = (MI_VDEC_DPB_BufMode_e)stVdecInfo.dpBufMode;
    MI_VDEC_CreateChn((MI_VDEC_DEV)this->stModInfo.devId, this->stModInfo.chnId, &stVdecChnAttr);
    MI_VDEC_StartChn((MI_VDEC_DEV)this->stModInfo.devId, this->stModInfo.chnId);
    for (auto itVdecOut = vVdecOutInfo.begin(); itVdecOut != vVdecOutInfo.end(); itVdecOut++)
    {
        //scaling
        memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
        stOutputPortAttr.u16Width  = itVdecOut->uintDecOutWidth;
        stOutputPortAttr.u16Height = itVdecOut->uintDecOutHeight;
        MI_VDEC_SetOutputPortAttr((MI_VDEC_DEV)this->stModInfo.devId, (MI_VDEC_CHN)this->stModInfo.chnId, &stOutputPortAttr);
        // set crop attr
        if (itVdecOut->bEnable)
        {
            stCropCfg.bEnable          = TRUE;
            stCropCfg.stRect.u16X      = VDEC_ALIGN_16xUP(itVdecOut->uintVdecCropX);
            stCropCfg.stRect.u16Y      = VDEC_ALIGN_2xUP(itVdecOut->uintVdecCropY);
            stCropCfg.stRect.u16Width  = VDEC_ALIGN_16xUP(itVdecOut->uintVdecCropW);
            stCropCfg.stRect.u16Height = VDEC_ALIGN_2xUP(itVdecOut->uintVdecCropH);
            if (MI_SUCCESS != MI_VDEC_SetDestCrop((MI_VDEC_DEV)this->stModInfo.devId, this->stModInfo.chnId, &stCropCfg))
            {
                AMIGOS_ERR("MI_VDEC_SetDestCrop failed, chn: %d\n", this->stModInfo.chnId);
            }
        }
    }
}

void AmigosModuleVdec::_Deinit()
{
    MI_VDEC_StopChn((MI_VDEC_DEV)this->stModInfo.devId, (MI_VDEC_CHN)this->stModInfo.chnId);
    MI_VDEC_DestroyChn((MI_VDEC_DEV)this->stModInfo.devId, (MI_VDEC_CHN)this->stModInfo.chnId);
}

int AmigosModuleVdec::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    if (packet->en_type != EN_VIDEO_CODEC_DATA)
    {
        AMIGOS_ERR("packet format error %d \n", packet->en_type);
        return -1;
    }
    if (!packet->es_vid_i.packet_count)
    {
        AMILOG_ERR << "Packet count is 0, dev:" << this->stModInfo.devId << " chn: " << this->stModInfo.chnId << COLOR_ENDL;
        return -1;
    }
    VdecWriterDesc *desc = &this->stWriterDesc;
    if (packet->es_vid_i.fmt != desc->fmt)
    {
        MI_VDEC_ChnAttr_t stVdecChnAttr;
        MI_VDEC_OutputPortAttr_t stOutputPortAttr;
        MI_VDEC_CropCfg_t stCropCfg;

        AMIGOS_INFO("Vdec in format change from %d to %d\n", desc->fmt , packet->es_vid_i.fmt);

        memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
        MI_VDEC_GetChnAttr((MI_VDEC_DEV)this->stModInfo.devId, (MI_VDEC_CHN)this->stModInfo.chnId, &stVdecChnAttr);
        memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
        MI_VDEC_GetOutputPortAttr((MI_VDEC_DEV)this->stModInfo.devId, (MI_VDEC_CHN)this->stModInfo.chnId, &stOutputPortAttr);

        /* Vdec need unbind then stop */
        for (auto itPortOut = this->mapPortOut.begin(); itPortOut != this->mapPortOut.end(); ++itPortOut)
        {
            for (auto itNext = itPortOut->second.mapNext.begin(); itNext != itPortOut->second.mapNext.end(); ++itNext)
            {
                for (auto itPortIn = itNext->second.begin(); itPortIn != itNext->second.end(); ++itPortIn)
                {
                    auto itIn = itNext->first->mapPortIn.find(*itPortIn);
                    if (itIn != itNext->first->mapPortIn.end() && (itIn->second.BindType() & E_MOD_PORT_TYPE_KERNEL))
                    {
                        itNext->first->UnBindBlock(*itPortIn);
                    }
                }
            }
        }
        MI_VDEC_StopChn((MI_VDEC_DEV)this->stModInfo.devId, (MI_VDEC_CHN)this->stModInfo.chnId);
        MI_VDEC_DestroyChn((MI_VDEC_DEV)this->stModInfo.devId, (MI_VDEC_CHN)this->stModInfo.chnId);

        stVdecChnAttr.eCodecType = _ConvertVideoFmtToVdecCodecType(packet->es_vid_i.fmt);
        MI_VDEC_CreateChn((MI_VDEC_DEV)this->stModInfo.devId, (MI_VDEC_CHN)this->stModInfo.chnId, &stVdecChnAttr);
        MI_VDEC_StartChn((MI_VDEC_DEV)this->stModInfo.devId, (MI_VDEC_CHN)this->stModInfo.chnId);
        desc->fmt = packet->es_vid_i.fmt;
        MI_VDEC_SetOutputPortAttr((MI_VDEC_DEV)this->stModInfo.devId, (MI_VDEC_CHN)this->stModInfo.chnId, &stOutputPortAttr);
        for (auto itVdecOut = vVdecOutInfo.begin(); itVdecOut != vVdecOutInfo.end(); itVdecOut++)
        {
            // set crop attr
            if (itVdecOut->bEnable)
            {
                stCropCfg.bEnable          = TRUE;
                stCropCfg.stRect.u16X      = VDEC_ALIGN_16xUP(itVdecOut->uintVdecCropX);
                stCropCfg.stRect.u16Y      = VDEC_ALIGN_2xUP(itVdecOut->uintVdecCropY);
                stCropCfg.stRect.u16Width  = VDEC_ALIGN_16xUP(itVdecOut->uintVdecCropW);
                stCropCfg.stRect.u16Height = VDEC_ALIGN_2xUP(itVdecOut->uintVdecCropH);
                if (MI_SUCCESS != MI_VDEC_SetDestCrop((MI_VDEC_DEV)this->stModInfo.devId, this->stModInfo.chnId, &stCropCfg))
                {
                    AMIGOS_ERR("MI_VDEC_SetDestCrop failed, chn: %d\n", this->stModInfo.chnId);
                }
            }
        }

        for (auto itPortOut = this->mapPortOut.begin(); itPortOut != this->mapPortOut.end(); ++itPortOut)
        {
            for (auto itNext = itPortOut->second.mapNext.begin(); itNext != itPortOut->second.mapNext.end(); ++itNext)
            {
                for (auto itPortIn = itNext->second.begin(); itPortIn != itNext->second.end(); ++itPortIn)
                {
                    auto itIn = itNext->first->mapPortIn.find(*itPortIn);
                    if (itIn != itNext->first->mapPortIn.end() && (itIn->second.BindType() & E_MOD_PORT_TYPE_KERNEL))
                    {
                        itNext->first->BindBlock(*itPortIn);
                    }
                }
            }
        }
    }
    char *       vdecSendData = NULL;
    unsigned int vdecSendSize = 0;
    if (packet->es_vid_i.packet_count == 1)
    {
        vdecSendData = packet->es_vid.packet_data[0].data;
        vdecSendSize = packet->es_vid_i.packet_info[0].size;
        return _SendStream2Hw(packet->get_time_stamp(), vdecSendData, vdecSendSize);
    }
    for (unsigned int i = 0; i < packet->es_vid_i.packet_count; i++)
    {
        vdecSendSize += packet->es_vid_i.packet_info[i].size;
    }
    vdecSendData = new char[vdecSendSize];
    assert(vdecSendData);
    for (unsigned int i = 0, shift = 0; i < packet->es_vid_i.packet_count; shift += packet->es_vid_i.packet_info[i].size, ++i)
    {
        memcpy(vdecSendData + shift,packet->es_vid.packet_data[i].data , packet->es_vid_i.packet_info[i].size);
    }
    int ret = _SendStream2Hw(packet->get_time_stamp(), vdecSendData, vdecSendSize);
    delete []vdecSendData;
    return ret;
}
int AmigosModuleVdec::_SendStream2Hw(const timeval &timeStamp, char *data, unsigned int size)
{
    MI_VDEC_VideoStream_t stVdecStream;
    memset(&stVdecStream, 0x0, sizeof(stVdecStream));
    stVdecStream.pu8Addr      = (MI_U8 *)data;
    stVdecStream.u32Len       = (MI_U32)size;
    stVdecStream.u64PTS       = timeStamp.tv_sec * 1000000 + timeStamp.tv_usec;
    stVdecStream.bEndOfFrame  = 1;
    stVdecStream.bEndOfStream = 0;
    int send_retry            = 5;
    do
    {
        if (MI_SUCCESS
            == MI_VDEC_SendStream((MI_VDEC_DEV)this->stModInfo.devId, (MI_VDEC_CHN)this->stModInfo.chnId, &stVdecStream, 20))
        {
            break;
        }
        send_retry--;
        AMILOG_WRN << "Rerty left: " << send_retry << " VDEC ES frame send error dev: " << this->stModInfo.devId
            << " chn: " << this->stModInfo.chnId << COLOR_ENDL;
        usleep(1000);
    } while (send_retry);
    return send_retry ? 0 : -1;
}
void AmigosModuleVdec::_StartIn(unsigned int inPortId)
{
    if (inPortId != 0)
    {
        return;
    }
    this->stWriterDesc.fmt           = ES_STREAM_H265;
}
void AmigosModuleVdec::_StopIn(unsigned int inPortId)
{
    if (inPortId != 0)
    {
        return;
    }
}
void AmigosModuleVdec::_ResetStreamOut(unsigned int outPortId, unsigned int width, unsigned int height)
{
    MI_VDEC_OutputPortAttr_t stOutputPortAttr;
    MI_VDEC_GetOutputPortAttr((MI_VDEC_DEV)this->stModInfo.devId, (MI_VDEC_CHN)this->stModInfo.chnId, &stOutputPortAttr);
    stOutputPortAttr.u16Width  = width;
    stOutputPortAttr.u16Height = height;
    MI_VDEC_SetOutputPortAttr((MI_VDEC_DEV)this->stModInfo.devId, (MI_VDEC_CHN)this->stModInfo.chnId, &stOutputPortAttr);
}
ss_linker_base *AmigosModuleVdec::_CreateInputNegativeLinker(unsigned int inportid)
{
    return new LinkerSyncNegative(inportid, this);
}
stream_packer *AmigosModuleVdec::_CreateInputStreamPacker(unsigned int inPortId, bool &bFast)
{
    bFast = false;
    return new StreamPackerNormal();
}
AMIGOS_MODULE_INIT("VDEC", AmigosModuleVdec);
