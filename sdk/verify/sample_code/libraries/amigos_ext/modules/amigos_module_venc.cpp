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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>

#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "amigos_env.h"
#include "amigos_module_init.h"
#include "amigos_module_venc.h"
#include "mi_common_datatype.h"
#include "ss_linker.h"
#include "ss_enum_cast.hpp"
#include "ss_log.h"
#include "ss_packet.h"
#include "ss_thread.h"

void *VencReader(struct ss_thread_buffer *thread_buf);

class AmigosVencStreamPacket : public stream_packet_base
{
public:
    class VencErrBuf : public err_buf
    {
        void show() const override;
    };
public:
    AmigosVencStreamPacket(unsigned int devId, unsigned int chnId, unsigned int ms)
        : stream_packet_base(), dupPacket(nullptr)
    {
        struct timeval    tv;
        MI_VENC_ChnAttr_t stAttr;

        this->devId = devId;
        this->chnId = chnId;
        MI_S32 s32Fd = MI_VENC_GetFd(this->devId, this->chnId);
        if (s32Fd < 0)
        {
            throw VencErrBuf();
        }
        FD_ZERO(&this->read_fds);
        FD_SET(s32Fd, &this->read_fds);
        tv.tv_sec  = 0;
        tv.tv_usec = ms * 1000;
        MI_S32 s32Ret = select(s32Fd + 1, &this->read_fds, NULL, NULL, &tv);
        if (s32Ret < 0)
        {
            MI_VENC_CloseFd(this->devId, this->chnId);
            throw VencErrBuf();
        }
        if (0 == s32Ret)
        {
            MI_VENC_CloseFd(this->devId, this->chnId);
            throw VencErrBuf();
        }
        memset(&stStat, 0, sizeof(MI_VENC_ChnStat_t));
        s32Ret = MI_VENC_Query(devId, chnId, &stStat);
        if (s32Ret != MI_SUCCESS || stStat.u32CurPacks == 0)
        {
            throw VencErrBuf();
        }
        memset(&stStream, 0, sizeof(MI_VENC_Stream_t));
        memset(astPack, 0, sizeof(MI_VENC_Pack_t) * ES_SLICE_COUNT_MAX);
        memset(&stAttr, 0, sizeof(MI_VENC_ChnAttr_t));

        if (MI_SUCCESS != MI_VENC_GetChnAttr(devId, chnId, &stAttr))
        {
            AMIGOS_ERR("MI_VENC_GetChnAttr Failed\n");
            throw VencErrBuf();
        }

        stStream.pstPack      = astPack;
        stStream.u32PackCount = stStat.u32CurPacks;

        if (MI_SUCCESS != MI_VENC_GetStream(devId, chnId, &stStream, ms))
        {
            AMIGOS_ERR("MI_VENC_GetStream Failed\n");
            throw VencErrBuf();
        }
        this->en_type = EN_VIDEO_CODEC_DATA;
        if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
        {
            this->es_vid_i.fmt    = ES_STREAM_H264;
            this->es_vid_i.b_head = false;
            this->es_vid_i.width  = stAttr.stVeAttr.stAttrH264e.u32PicWidth;
            this->es_vid_i.height = stAttr.stVeAttr.stAttrH264e.u32PicHeight;
        }
        else if(stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
        {
            this->es_vid_i.fmt    = ES_STREAM_H265;
            this->es_vid_i.b_head = false;
            this->es_vid_i.width  = stAttr.stVeAttr.stAttrH265e.u32PicWidth;
            this->es_vid_i.height = stAttr.stVeAttr.stAttrH265e.u32PicHeight;
        }
        else if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
        {
            this->es_vid_i.fmt    = ES_STREAM_JPEG;
            this->es_vid_i.b_head = false;
            this->es_vid_i.width  = stAttr.stVeAttr.stAttrJpeg.u32PicWidth;
            this->es_vid_i.height = stAttr.stVeAttr.stAttrJpeg.u32PicHeight;
        }
        else if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_AV1)
        {
            this->es_vid_i.fmt    = ES_STREAM_AV1;
            this->es_vid_i.b_head = false;
            this->es_vid_i.width  = stAttr.stVeAttr.stAttrAv1.u32PicWidth;
            this->es_vid_i.height = stAttr.stVeAttr.stAttrAv1.u32PicHeight;
        }
        else
        {
            AMIGOS_ERR("Bad en_type %d\n", stAttr.stVeAttr.eType);
            this->en_type = EN_STREAM_TYPE_NONE;
            return;
        }
        this->es_vid_i.packet_count = stStream.u32PackCount;
        for (unsigned int i = 0; i < stStream.u32PackCount; ++i)
        {
            this->es_vid.packet_data[i].data    = (char *)stStream.pstPack[i].pu8Addr;
            this->es_vid_i.packet_info[i].size  = stStream.pstPack[i].u32Len;
            this->es_vid_i.packet_info[i].b_end = stStream.pstPack[i].bFrameEnd;
        }
        this->keyFrame = (this->es_vid_i.fmt == ES_STREAM_H264
            && E_MI_VENC_H264E_NALU_ISLICE == stStream.pstPack[0].stDataType.eH264EType)
            ||(this->es_vid_i.fmt == ES_STREAM_H265
            && E_MI_VENC_H265E_NALU_ISLICE == stStream.pstPack[0].stDataType.eH265EType)
            ||(this->es_vid_i.fmt == ES_STREAM_AV1
            && E_MI_VENC_BASE_IDR == stStream.stAv1Info.eRefType);
    }
    virtual ~AmigosVencStreamPacket()
    {
        if (!this->dupPacket)
        {
            MI_VENC_ReleaseStream((MI_VENC_DEV)devId, (MI_VENC_CHN)chnId, &stStream);
        }
    }
    stream_packet_obj dup() override
    {
        if (this->dupPacket)
        {
            return nullptr;
        }
        this->dupPacket = stream_packet_base::make<stream_packet_clone>(*this);
        assert(this->dupPacket);
        this->en_type  = this->dupPacket->en_type;
        this->es_vid_i = this->dupPacket->es_vid_i;
        this->es_vid   = this->dupPacket->es_vid;

        MI_VENC_ReleaseStream((MI_VENC_DEV)devId, (MI_VENC_CHN)chnId, &stStream);
        return nullptr;
    }
    bool isKeyFrame()
    {
        return this->keyFrame;
    }
private:
    void update_time_stamp() final
    {
        struct timeval stamp;
        stamp.tv_sec  = stStream.pstPack[0].u64PTS / 1000000;
        stamp.tv_usec = stStream.pstPack[0].u64PTS % 1000000;
        this->set_time_stamp(stamp);
    }
    stream_packet_obj dupPacket;
    bool              keyFrame;
    unsigned int      devId;
    unsigned int      chnId;
    MI_VENC_Stream_t  stStream;
    MI_VENC_ChnStat_t stStat;
    MI_VENC_Pack_t    astPack[ES_SLICE_COUNT_MAX];
    fd_set            read_fds;
};
void AmigosVencStreamPacket::VencErrBuf::show() const
{
}

AmigosModuleVenc::AmigosModuleVenc(const std::string &strSection)
    : AmigosSurfaceVenc(strSection), AmigosModuleMiBase(this)
{
}
AmigosModuleVenc::~AmigosModuleVenc() {}

void AmigosModuleVenc::SetRefParam()
{
    MI_S32             s32Ret = 0;
    MI_VENC_ParamRef_t stRefParam;
    s32Ret = MI_VENC_GetRefParam(this->stModInfo.devId, this->stModInfo.chnId, &stRefParam);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("MI_VENC_GetRefParam err0x%x\n", s32Ret);
        return;
    }
    stRefParam.u32Base     = this->stVencInfo.Base;
    stRefParam.u32Enhance  = this->stVencInfo.Enhance;
    stRefParam.bEnablePred = this->stVencInfo.bEnablePred;
    s32Ret                 = MI_VENC_SetRefParam(this->stModInfo.devId, this->stModInfo.chnId, &stRefParam);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("MI_VENC_SetRefParam err0x%x\n", s32Ret);
        return;
    }
    return;
}
void AmigosModuleVenc::SetDeBreathParam()
{
    MI_S32             s32Ret = 0;
    MI_VENC_DeBreathCfg_t  stDeBreathParam;
    s32Ret = MI_VENC_GetDeBreathCfg(this->stModInfo.devId, this->stModInfo.chnId, &stDeBreathParam);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("MI_VENC_GetDeBreath err0x%x\n", s32Ret);
        return;
    }
    stDeBreathParam.bEnable     = this->stVencInfo.DeBreathEnable;
    stDeBreathParam.u8Strength0  = this->stVencInfo.Strength0;
    stDeBreathParam.u8Strength1 = this->stVencInfo.Strength1;
    s32Ret                 = MI_VENC_SetDeBreathCfg(this->stModInfo.devId, this->stModInfo.chnId, &stDeBreathParam);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("MI_VENC_SetDeBreath err0x%x\n", s32Ret);
        return;
    }
    return;
}
void AmigosModuleVenc::SetRoiParam(int label)
{
    MI_S32           index  = 0;
    MI_S32           s32Ret = 0;
    MI_VENC_RoiCfg_t stRoiCfg;

    s32Ret =
        MI_VENC_GetRoiCfg((MI_VENC_DEV)this->stModInfo.devId, (MI_VENC_CHN)this->stModInfo.chnId, index, &stRoiCfg);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("MI_VENC_GetRoiCfg err0x%x\n", s32Ret);
        return;
    }

    stRoiCfg.bEnable          = true;
    stRoiCfg.bAbsQp           = stVencRoi[label].bAbsQp;
    stRoiCfg.s32Qp            = stVencRoi[label].s32Qp;
    stRoiCfg.stRect.u32Left   = stVencRoi[label].u16RectX;
    stRoiCfg.stRect.u32Top    = stVencRoi[label].u16RectY;
    stRoiCfg.stRect.u32Width  = stVencRoi[label].u16RectWidth;
    stRoiCfg.stRect.u32Height = stVencRoi[label].u16RectHeight;
    stRoiCfg.u32Index         = stVencRoi[label].index;
    s32Ret = MI_VENC_SetRoiCfg((MI_VENC_DEV)this->stModInfo.devId, (MI_VENC_CHN)this->stModInfo.chnId, &stRoiCfg);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("MI_VENC_SetRoiCfg err0x%x\n", s32Ret);
        return;
    }
    return;
}
void AmigosModuleVenc::_ResourceInit()
{
    this->env.Out(0)["WIDTH"]  = std::to_string(stVencInfo.intWidth);
    this->env.Out(0)["HEIGHT"] = std::to_string(stVencInfo.intHeight);
    this->env.In(0)["WIDTH"]   = std::to_string(stVencInfo.intWidth);
    this->env.In(0)["HEIGHT"]  = std::to_string(stVencInfo.intHeight);
    MI_VENC_DupChn(this->stModInfo.devId, this->stModInfo.chnId);
}
void AmigosModuleVenc::_ResourceDeinit()
{
    MI_VENC_DestroyChn(this->stModInfo.devId, this->stModInfo.chnId);
}
void AmigosModuleVenc::_Init()
{
    AMILOG_INFO << std::to_string(this->GetModId()) << " " << std::to_string(this->stModInfo.devId) << " "
                << std::to_string(this->stModInfo.chnId) << std::endl;

    MI_VENC_InitParam_t stVencInitPara;
    memset(&stVencInitPara, 0, sizeof(MI_VENC_InitParam_t));
    stVencInitPara.u32MaxWidth  = AmigosSurfaceVenc::mapDevInfo[this->stModInfo.devId].width;
    stVencInitPara.u32MaxHeight = AmigosSurfaceVenc::mapDevInfo[this->stModInfo.devId].height;
    if (MI_SUCCESS != MI_VENC_CreateDev(this->stModInfo.devId, &stVencInitPara))
    {
        AMIGOS_ERR("MI_VENC_CreateDev failed\n");
        return;
    }
    AMIGOS_INFO("VENC dev %d creat\n",this->stModInfo.devId);

    MI_VENC_ChnAttr_t stChnAttr;
    memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));

    es_video_fmt es_fmt = ss_enum_cast<es_video_fmt>::from_str(this->stVencInfo.strEncodeType);
    switch (es_fmt)
    {
        case ES_STREAM_H264:
        {
            stChnAttr.stVeAttr.eType                       = E_MI_VENC_MODTYPE_H264E;
            stChnAttr.stVeAttr.stAttrH264e.u32PicWidth     = (MI_U32)this->stVencInfo.intWidth;
            stChnAttr.stVeAttr.stAttrH264e.u32PicHeight    = (MI_U32)this->stVencInfo.intHeight;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth  = (MI_U32)this->stVencInfo.intMaxWidth;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = (MI_U32)this->stVencInfo.intMaxHeight;
            stChnAttr.stVeAttr.stAttrH264e.bByFrame =
                (this->stVencInfo.intMultiSlice != -1) ? !this->stVencInfo.intMultiSlice : TRUE;
            stChnAttr.stVeAttr.stAttrH264e.u32Profile = 2;
            if(stVencInfo.strRcMode == "cbr")
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
                stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate =
                    (MI_U32)this->stVencInfo.stCbrCfg.intBitRate * 1000;
                stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop           = stVencInfo.stCbrCfg.intGop;
                stChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime      = 0;
            }
            else if(stVencInfo.strRcMode == "vbr")
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264VBR;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate =
                    (MI_U32)this->stVencInfo.stVbrCfg.intBitRate * 1000;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxQp = this->stVencInfo.stVbrCfg.intMaxQp;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32MinQp = this->stVencInfo.stVbrCfg.intMinQp;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32Gop           = stVencInfo.stVbrCfg.intGop;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32StatTime      = 0;
            }
            else if(stVencInfo.strRcMode == "fixqp")
            {
                stChnAttr.stRcAttr.eRcMode                = E_MI_VENC_RC_MODE_H264FIXQP;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = this->stVencInfo.stFixQpCfg.intIQp;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = this->stVencInfo.stFixQpCfg.intPQp;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop           = stVencInfo.stFixQpCfg.intGop;
            }
            else if(stVencInfo.strRcMode == "avbr")
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264AVBR;
                stChnAttr.stRcAttr.stAttrH264Avbr.u32MaxBitRate =
                    (MI_U32)this->stVencInfo.stAvbrCfg.intBitRate * 1000;
                stChnAttr.stRcAttr.stAttrH264Avbr.u32MaxQp = this->stVencInfo.stAvbrCfg.intMaxQp;
                stChnAttr.stRcAttr.stAttrH264Avbr.u32MinQp = this->stVencInfo.stAvbrCfg.intMinQp;
                stChnAttr.stRcAttr.stAttrH264Avbr.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrH264Avbr.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH264Avbr.u32Gop           = stVencInfo.stAvbrCfg.intGop;
                stChnAttr.stRcAttr.stAttrH264Avbr.u32StatTime      = 0;
            }
            else if(stVencInfo.strRcMode == "cvbr")
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_CVBR;
                stChnAttr.stRcAttr.stAttrCvbr.u32MaxBitRate =
                    (MI_U32)this->stVencInfo.stCvbrCfg.intMaxBitRate * 1000;
                stChnAttr.stRcAttr.stAttrCvbr.u32ShortTermStatsTime = this->stVencInfo.stCvbrCfg.intShortTermStatsTime;
                stChnAttr.stRcAttr.stAttrCvbr.u32LongTermStatsTime = this->stVencInfo.stCvbrCfg.intLongTermStatsTime;
                stChnAttr.stRcAttr.stAttrCvbr.u32LongTermMaxBitRate = this->stVencInfo.stCvbrCfg.intLongTermMaxBitRate * 1000;
                stChnAttr.stRcAttr.stAttrCvbr.u32LongTermMinBitRate = this->stVencInfo.stCvbrCfg.intLongTermMinBitRate * 1000;
                stChnAttr.stRcAttr.stAttrCvbr.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrCvbr.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrCvbr.u32Gop           = stVencInfo.stCvbrCfg.intGop;
            }
            else
            {
                AMIGOS_ERR("RC Mode error!\n");
                return;
            }
        }
        break;
        case ES_STREAM_H265:
        {
            stChnAttr.stVeAttr.eType                       = E_MI_VENC_MODTYPE_H265E;
            stChnAttr.stVeAttr.stAttrH265e.u32PicWidth     = (MI_U32)this->stVencInfo.intWidth;
            stChnAttr.stVeAttr.stAttrH265e.u32PicHeight    = (MI_U32)this->stVencInfo.intHeight;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth  = (MI_U32)this->stVencInfo.intMaxWidth;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = (MI_U32)this->stVencInfo.intMaxHeight;
            stChnAttr.stVeAttr.stAttrH265e.bByFrame =
                (this->stVencInfo.intMultiSlice != -1) ? !this->stVencInfo.intMultiSlice : TRUE;
            if(stVencInfo.strRcMode == "cbr")
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
                stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate =
                    (MI_U32)this->stVencInfo.stCbrCfg.intBitRate * 1000;
                stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop           = stVencInfo.stCbrCfg.intGop;
                stChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime      = 0;
            }
            else if(stVencInfo.strRcMode == "vbr")
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate =
                    (MI_U32)this->stVencInfo.stVbrCfg.intBitRate * 1000;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp = this->stVencInfo.stVbrCfg.intMaxQp;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp = this->stVencInfo.stVbrCfg.intMinQp;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop           = stVencInfo.stVbrCfg.intGop;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32StatTime      = 0;
            }
            else if(stVencInfo.strRcMode == "fixqp")
            {
                stChnAttr.stRcAttr.eRcMode                = E_MI_VENC_RC_MODE_H265FIXQP;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32IQp = this->stVencInfo.stFixQpCfg.intIQp;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32PQp = this->stVencInfo.stFixQpCfg.intPQp;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32Gop           = stVencInfo.stFixQpCfg.intGop;
            }
            else if(stVencInfo.strRcMode == "avbr")
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265AVBR;
                stChnAttr.stRcAttr.stAttrH265Avbr.u32MaxBitRate =
                    (MI_U32)this->stVencInfo.stAvbrCfg.intBitRate * 1000;
                stChnAttr.stRcAttr.stAttrH265Avbr.u32MaxQp = this->stVencInfo.stAvbrCfg.intMaxQp;
                stChnAttr.stRcAttr.stAttrH265Avbr.u32MinQp = this->stVencInfo.stAvbrCfg.intMinQp;
                stChnAttr.stRcAttr.stAttrH265Avbr.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrH265Avbr.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH265Avbr.u32Gop           = stVencInfo.stAvbrCfg.intGop;
                stChnAttr.stRcAttr.stAttrH265Avbr.u32StatTime      = 0;
            }
            else if(stVencInfo.strRcMode == "cvbr")
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_CVBR;
                stChnAttr.stRcAttr.stAttrCvbr.u32MaxBitRate =
                    (MI_U32)this->stVencInfo.stCvbrCfg.intMaxBitRate * 1000;
                stChnAttr.stRcAttr.stAttrCvbr.u32ShortTermStatsTime = this->stVencInfo.stCvbrCfg.intShortTermStatsTime;
                stChnAttr.stRcAttr.stAttrCvbr.u32LongTermStatsTime = this->stVencInfo.stCvbrCfg.intLongTermStatsTime;
                stChnAttr.stRcAttr.stAttrCvbr.u32LongTermMaxBitRate = this->stVencInfo.stCvbrCfg.intLongTermMaxBitRate * 1000;
                stChnAttr.stRcAttr.stAttrCvbr.u32LongTermMinBitRate = this->stVencInfo.stCvbrCfg.intLongTermMinBitRate * 1000;
                stChnAttr.stRcAttr.stAttrCvbr.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrCvbr.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrCvbr.u32Gop           = stVencInfo.stCvbrCfg.intGop;
            }
            else
            {
                AMIGOS_ERR("RC Mode error!\n");
                return;
            }
        }
        break;
        case ES_STREAM_JPEG:
        {
            stChnAttr.stVeAttr.eType                      = E_MI_VENC_MODTYPE_JPEGE;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth     = (MI_U32)this->stVencInfo.intWidth;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight    = (MI_U32)this->stVencInfo.intHeight;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth  = (MI_U32)this->stVencInfo.intMaxWidth;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = (MI_U32)this->stVencInfo.intMaxHeight;

            if(stVencInfo.strRcMode == "cbr")
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGCBR;
                stChnAttr.stRcAttr.stAttrMjpegCbr.u32BitRate = (MI_U32)this->stVencInfo.stCbrCfg.intBitRate * 1000;
                stChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateDen = 1;
            }
            else if(stVencInfo.strRcMode == "vbr")
            {
                    stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGVBR;
                    stChnAttr.stRcAttr.stAttrMjpegVbr.u32MaxBitRate = (MI_U32)this->stVencInfo.stVbrCfg.intBitRate * 1000;
                    stChnAttr.stRcAttr.stAttrMjpegVbr.u32SrcFrmRateNum =
                        (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                    stChnAttr.stRcAttr.stAttrMjpegVbr.u32SrcFrmRateDen = 1;
                }
            else if(stVencInfo.strRcMode == "fixqp")
            {
                stChnAttr.stRcAttr.eRcMode                   = E_MI_VENC_RC_MODE_MJPEGFIXQP;
                stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrMjpegFixQp.u32Qfactor = this->stVencInfo.stFixQpCfg.intQfactor;
            }
            else
            {
                AMIGOS_ERR("RC Mode error!\n");
                return;
            }
        }
        break;
        case ES_STREAM_AV1:
        {
            stChnAttr.stVeAttr.eType                       = E_MI_VENC_MODTYPE_AV1;
            stChnAttr.stVeAttr.stAttrAv1.u32PicWidth     = (MI_U32)this->stVencInfo.intWidth;
            stChnAttr.stVeAttr.stAttrAv1.u32PicHeight    = (MI_U32)this->stVencInfo.intHeight;
            stChnAttr.stVeAttr.stAttrAv1.u32MaxPicWidth  = (MI_U32)this->stVencInfo.intMaxWidth > 4096 ? 4096 : (MI_U32)this->stVencInfo.intMaxWidth;
            stChnAttr.stVeAttr.stAttrAv1.u32MaxPicHeight = (MI_U32)this->stVencInfo.intMaxHeight > 4096 ? 4096 : (MI_U32)this->stVencInfo.intMaxHeight;
            stChnAttr.stVeAttr.stAttrAv1.bByFrame        = TRUE;
            if(stVencInfo.strRcMode == "cbr")
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_AV1CBR;
                stChnAttr.stRcAttr.stAttrAv1Cbr.u32BitRate =
                    (MI_U32)this->stVencInfo.stCbrCfg.intBitRate * 1000;
                stChnAttr.stRcAttr.stAttrAv1Cbr.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrAv1Cbr.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrAv1Cbr.u32Gop           = stVencInfo.stCbrCfg.intGop;
                stChnAttr.stRcAttr.stAttrAv1Cbr.u32StatTime      = 0;
            }
            else if(stVencInfo.strRcMode == "vbr")
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_AV1VBR;
                stChnAttr.stRcAttr.stAttrAv1Vbr.u32MaxBitRate =
                    (MI_U32)this->stVencInfo.stVbrCfg.intBitRate * 1000;
                stChnAttr.stRcAttr.stAttrAv1Vbr.u32MaxQp = this->stVencInfo.stVbrCfg.intMaxQp;
                stChnAttr.stRcAttr.stAttrAv1Vbr.u32MinQp = this->stVencInfo.stVbrCfg.intMinQp;
                stChnAttr.stRcAttr.stAttrAv1Vbr.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrAv1Vbr.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrAv1Vbr.u32Gop           = stVencInfo.stVbrCfg.intGop;
                stChnAttr.stRcAttr.stAttrAv1Vbr.u32StatTime      = 0;
            }
            else if(stVencInfo.strRcMode == "fixqp")
            {
                stChnAttr.stRcAttr.eRcMode                = E_MI_VENC_RC_MODE_AV1FIXQP;
                stChnAttr.stRcAttr.stAttrAv1FixQp.u32IQp = this->stVencInfo.stFixQpCfg.intIQp;
                stChnAttr.stRcAttr.stAttrAv1FixQp.u32PQp = this->stVencInfo.stFixQpCfg.intPQp;
                stChnAttr.stRcAttr.stAttrAv1FixQp.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrAv1FixQp.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrAv1FixQp.u32Gop           = stVencInfo.stFixQpCfg.intGop;
            }
            else if(stVencInfo.strRcMode == "avbr")
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_AV1AVBR;
                stChnAttr.stRcAttr.stAttrAv1Avbr.u32MaxBitRate =
                    (MI_U32)this->stVencInfo.stAvbrCfg.intBitRate * 1000;
                stChnAttr.stRcAttr.stAttrAv1Avbr.u32MaxQp = this->stVencInfo.stAvbrCfg.intMaxQp;
                stChnAttr.stRcAttr.stAttrAv1Avbr.u32MinQp = this->stVencInfo.stAvbrCfg.intMinQp;
                stChnAttr.stRcAttr.stAttrAv1Avbr.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrAv1Avbr.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrAv1Avbr.u32Gop           = stVencInfo.stAvbrCfg.intGop;
                stChnAttr.stRcAttr.stAttrAv1Avbr.u32StatTime      = 0;
            }
            else if(stVencInfo.strRcMode == "cvbr")
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_CVBR;
                stChnAttr.stRcAttr.stAttrCvbr.u32MaxBitRate =
                    (MI_U32)this->stVencInfo.stCvbrCfg.intMaxBitRate * 1000;
                stChnAttr.stRcAttr.stAttrCvbr.u32ShortTermStatsTime = this->stVencInfo.stCvbrCfg.intShortTermStatsTime;
                stChnAttr.stRcAttr.stAttrCvbr.u32LongTermStatsTime = this->stVencInfo.stCvbrCfg.intLongTermStatsTime;
                stChnAttr.stRcAttr.stAttrCvbr.u32LongTermMaxBitRate = this->stVencInfo.stCvbrCfg.intLongTermMaxBitRate * 1000;
                stChnAttr.stRcAttr.stAttrCvbr.u32LongTermMinBitRate = this->stVencInfo.stCvbrCfg.intLongTermMinBitRate * 1000;
                stChnAttr.stRcAttr.stAttrCvbr.u32SrcFrmRateNum =
                    (MI_U32)((this->stVencInfo.intEncodeFps != -1) ? this->stVencInfo.intEncodeFps : 30);
                stChnAttr.stRcAttr.stAttrCvbr.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrCvbr.u32Gop           = stVencInfo.stCvbrCfg.intGop;
            }
            else
            {
                AMIGOS_ERR("RC Mode error!\n");
                return;
            }
        }
        break;

        default:
            assert(0);
    }
    if (MI_SUCCESS != MI_VENC_CreateChn(this->stModInfo.devId, this->stModInfo.chnId, &stChnAttr))
    {
        AMIGOS_ERR("MI_VENC_CreateChn failed\n");
        goto CREATE_CHN_ERR;
    }
    this->env.Out(0)["WIDTH"]  = std::to_string(stVencInfo.intWidth);
    this->env.Out(0)["HEIGHT"] = std::to_string(stVencInfo.intHeight);
    this->env.In(0)["WIDTH"]   = std::to_string(stVencInfo.intWidth);
    this->env.In(0)["HEIGHT"]  = std::to_string(stVencInfo.intHeight);
    AMIGOS_INFO("Create Chn success\n");
    if ((this->stVencInfo.intMultiSlice != -1) ? this->stVencInfo.intMultiSlice : FALSE)
    {
        switch (es_fmt)
        {
            case ES_STREAM_H264:
                MI_VENC_ParamH264SliceSplit_t stH264SliceSplit;

                stH264SliceSplit.bSplitEnable     = this->stVencInfo.intMultiSlice;
                stH264SliceSplit.u32SliceRowCount = this->stVencInfo.intSliceRowCnt;
                MI_VENC_SetH264SliceSplit((MI_VENC_DEV)this->stModInfo.devId, (MI_VENC_CHN)this->stModInfo.chnId,
                                          &stH264SliceSplit);
                break;
            case ES_STREAM_H265:
                MI_VENC_ParamH265SliceSplit_t stH265SliceSplit;

                stH265SliceSplit.bSplitEnable     = this->stVencInfo.intMultiSlice;
                stH265SliceSplit.u32SliceRowCount = this->stVencInfo.intSliceRowCnt;
                MI_VENC_SetH265SliceSplit((MI_VENC_DEV)this->stModInfo.devId, (MI_VENC_CHN)this->stModInfo.chnId,
                                          &stH265SliceSplit);
                break;
            case ES_STREAM_AV1:
                MI_VENC_ParamAv1TileSplit_t stAv1TileSplit;

                stAv1TileSplit.bSplitEnable     = this->stVencInfo.intMultiSlice;
                stAv1TileSplit.u32TileRowCount = this->stVencInfo.intSliceRowCnt;
                MI_VENC_SetAv1TileSplit((MI_VENC_DEV)this->stModInfo.devId, (MI_VENC_CHN)this->stModInfo.chnId,
                                          &stAv1TileSplit);
                break;
            default:
                break;
        }
    }

    if (this->stVencInfo.StreamCntEnable)
    {
        MI_VENC_SetMaxStreamCnt((MI_VENC_DEV)this->stModInfo.devId, (MI_VENC_CHN)this->stModInfo.chnId,
                                this->stVencInfo.intMaxStreamCnt);
    }

    {
        MI_VENC_InputSourceConfig_t stVenInSrc;
        MI_SYS_BindType_e           eBindType    = E_MI_SYS_BIND_TYPE_FRAME_BASE;
        MI_U32                      u32BindParam = 0;
        MI_S32                      s32Ret       = 0;
        AmigosSurfaceBase::ModPortInInfo stInInfo;
        this->GetPortInInfo(0, stInInfo);

        memset(&stVenInSrc, 0, sizeof(MI_VENC_InputSourceConfig_t));
        eBindType = (MI_SYS_BindType_e)stInInfo.bindType;
        if (eBindType == E_MI_SYS_BIND_TYPE_HW_RING)
        {
            u32BindParam = stInInfo.bindPara;
            if (u32BindParam == (MI_U32)this->stVencInfo.intHeight)
            {
                stVenInSrc.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_ONE_FRM;
                s32Ret                         = MI_VENC_SetInputSourceConfig((MI_VENC_DEV)this->stModInfo.devId,
                                                                              (MI_VENC_CHN)this->stModInfo.chnId, &stVenInSrc);
                AMIGOS_INFO("Set ring one frame mode! Chn %d height %d ret %d\n", this->stModInfo.chnId,
                            this->stVencInfo.intHeight, s32Ret);
            }
            else if (u32BindParam == (MI_U32)stVencInfo.intHeight / 2)
            {
                stVenInSrc.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_HALF_FRM;
                s32Ret                         = MI_VENC_SetInputSourceConfig((MI_VENC_DEV)this->stModInfo.devId,
                                                                              (MI_VENC_CHN)this->stModInfo.chnId, &stVenInSrc);
                AMIGOS_INFO("Set ring half frame mode! Chn %d height %d ret %d\n", this->stModInfo.chnId,
                            this->stVencInfo.intHeight, s32Ret);
            }
            else
            {
                if (u32BindParam == 1)
                {
                    stVenInSrc.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_UNIFIED_DMA;
                    s32Ret = MI_VENC_SetInputSourceConfig((MI_VENC_DEV)this->stModInfo.devId,
                                                          (MI_VENC_CHN)this->stModInfo.chnId, &stVenInSrc);
                    AMIGOS_INFO("Set dma ring mode! Chn %d height %d ret %d\n", this->stModInfo.chnId, this->stVencInfo.intHeight, s32Ret);
                }
            }
        }
        else if(eBindType == E_MI_SYS_BIND_TYPE_FRAME_BASE)
        {
            stVenInSrc.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_NORMAL_FRMBASE;
            s32Ret                         = MI_VENC_SetInputSourceConfig((MI_VENC_DEV)this->stModInfo.devId,
                                                                          (MI_VENC_CHN)this->stModInfo.chnId, &stVenInSrc);
            AMIGOS_INFO("Set frame mode! ret %d\n", s32Ret);
        }
    }

    // set REF_LTR :NormalP、LTR_I、LTR_VI、TSVC-2、TSVC-3
    if (stVencInfo.LtrEnable)
    {
        SetRefParam();
    }

    // set Debreathing effect
    if (stVencInfo.DeBreathEnable)
    {
        SetDeBreathParam();
    }

    // roi bEnable default 0
    for (int num = 0; num<stVencInfo.RoiNum; num++)
    {
        SetRoiParam(num);
    }

    return;

    MI_VENC_DestroyChn(this->stModInfo.devId, this->stModInfo.chnId);
CREATE_CHN_ERR:
    MI_VENC_DestroyDev(this->stModInfo.devId);
}

void AmigosModuleVenc::_Deinit()
{
    MI_VENC_DestroyChn(this->stModInfo.devId, this->stModInfo.chnId);
    AMIGOS_INFO("VENC dev %d chn %d, stop and destroy\n",this->stModInfo.devId,this->stModInfo.chnId);

    MI_VENC_DestroyDev(this->stModInfo.devId);
    AMIGOS_INFO("VENC dev %d destroy\n",this->stModInfo.devId);
}

unsigned int AmigosModuleVenc::GetModId() const
{
    return E_MI_MODULE_ID_VENC;
}
unsigned int AmigosModuleVenc::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleVenc::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
void AmigosModuleVenc::_StartOut(unsigned int outPortId)
{
    if (this->mapModOutputInfo.size() > 1 && outPortId == 0)
    {
        if (this->stVencInfo.yuvEnable)
        {
            MI_VENC_OutPortParam_t outPortParam;
            memset(&outPortParam, 0, sizeof(MI_VENC_OutPortParam_t));
            outPortParam.u32Width = this->stVencInfo.yuvWidth;
            outPortParam.u32Height = this->stVencInfo.yuvHeight;
            MI_VENC_SetOutputPortParam(this->stModInfo.devId, this->stModInfo.chnId, 0, &outPortParam);
            MI_VENC_EnableOutputPort(this->stModInfo.devId, this->stModInfo.chnId, 0);
        }
        AmigosModuleMiBase::_StartOut(outPortId);
        return;
    }
    if (MI_SUCCESS != MI_VENC_StartRecvPic(this->stModInfo.devId, this->stModInfo.chnId))
    {
        AMIGOS_ERR("MI_VENC_StartRecvPic failed\n");
    }
}
void AmigosModuleVenc::_StopOut(unsigned int outPortId)
{
    if (this->mapModOutputInfo.size() > 1 && outPortId == 0)
    {
        AmigosModuleMiBase::_StopOut(outPortId);
        if (this->stVencInfo.yuvEnable)
        {
            MI_VENC_DisableOutputPort(this->stModInfo.devId, this->stModInfo.chnId, 0);
        }
        return;
    }
    MI_VENC_StopRecvPic(this->stModInfo.devId, this->stModInfo.chnId);
}

int AmigosModuleVenc::_Connected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        if (this->stVencInfo.yuvEnable && outPortId == 0 && this->mapModOutputInfo.size() > 1)
        {
            return AmigosModuleMiBase::_Connected(outPortId, ref);
        }
        if (this->stVencInfo.strEncodeType == "h264" || this->stVencInfo.strEncodeType == "h265"
            || this->stVencInfo.strEncodeType == "av1")
        {
            MI_VENC_RequestIdr(this->stModInfo.devId, this->stModInfo.chnId, TRUE);
        }
        return 0;
    }
    if (this->stVencInfo.yuvEnable && outPortId == 0 && this->mapModOutputInfo.size() > 1)
    {
        return AmigosModuleMiBase::_Connected(outPortId, ref);
    }
    auto iter = this->mapPortOut.find(outPortId);
    if (iter == this->mapPortOut.end())
    {
        AMIGOS_ERR("Base port %d is not found\n", outPortId);
        return -1;
    }
    this->env.Out(0)["EN_TYPE"] = this->stVencInfo.strEncodeType;
    std::stringstream ss;
    ss << this->stVencInfo.intWidth << "x" << this->stVencInfo.intHeight;
    this->env.Out(0)["STD_RES"] = ss.str();
    if (iter->second.positive.empty())
    {
        /* Direct + post reader  */
        if (this->stVencInfo.strEncodeType == "h264" || this->stVencInfo.strEncodeType == "h265"
            || this->stVencInfo.strEncodeType == "av1")
        {
            MI_VENC_RequestIdr(this->stModInfo.devId, this->stModInfo.chnId, TRUE);
        }
        AMIGOS_INFO("VENC Reader is not needed\n");
        return 0;
    }
    if (this->stVencInfo.strEncodeType == "h264" || this->stVencInfo.strEncodeType == "h265"
        || this->stVencInfo.strEncodeType == "av1")
    {
        MI_VENC_RequestIdr(this->stModInfo.devId, this->stModInfo.chnId, TRUE);
    }
    VencReaderDesc readerDesc;
    readerDesc.linker    = &iter->second.positive;
    readerDesc.pModule   = this;
    ss_thread_attr ss_attr;
    memset(&ss_attr, 0, sizeof(ss_thread_attr));
    ss_attr.do_signal          = NULL;
    ss_attr.do_monitor         = VencReader;
    ss_attr.monitor_cycle_sec  = 0;
    ss_attr.monitor_cycle_nsec = 0;
    ss_attr.is_reset_timer     = 0;
    ss_attr.in_buf.buf         = (&this->mapReaderDesc[outPortId]);
    ss_attr.in_buf.size        = 0;
    snprintf(ss_attr.thread_name, 128, "%s", this->GetOutPortIdStr(outPortId).c_str());
    readerDesc.threadHandle = ss_thread_open(&ss_attr);
    if (!readerDesc.threadHandle)
    {
        this->mapReaderDesc.erase(outPortId);
        AMIGOS_ERR("Monitor return error!\n");
        return -1;
    }
    this->mapReaderDesc[outPortId] = readerDesc;
    ss_thread_start_monitor(readerDesc.threadHandle);
    AMIGOS_INFO("Venc reader %d started\n", outPortId);
    return 0;
}
int AmigosModuleVenc::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if (this->stVencInfo.yuvEnable && outPortId == 0 && this->mapModOutputInfo.size() > 1)
    {
        return AmigosModuleMiBase::_Disconnected(outPortId, ref);
    }
    if (ref > 0)
    {
        return 0;
    }
    auto iter = this->mapPortOut.find(outPortId);
    if (iter == this->mapPortOut.end())
    {
        AMIGOS_ERR("Base port %d is not found\n", outPortId);
        return -1;
    }
    if (iter->second.positive.empty())
    {
        /* Direct + post reader  */
        AMIGOS_INFO("VENC Reader is not needed\n");
        return 0;
    }
    auto iterDesc = this->mapReaderDesc.find(outPortId);
    if (iterDesc == this->mapReaderDesc.end())
    {
        AMIGOS_ERR("Not found desc, output %d\n", outPortId);
        return -1;
    }
    if (!iterDesc->second.threadHandle)
    {
        AMIGOS_ERR("Thread handle is null, port %d!\n", outPortId);
        return -1;
    }
    ss_thread_stop(iterDesc->second.threadHandle);
    ss_thread_close(iterDesc->second.threadHandle);
    this->mapReaderDesc.erase(iterDesc);
    AMIGOS_INFO("Venc reader %d stopped\n", outPortId);
    return 0;
}

int AmigosModuleVenc::_EnqueueOut(unsigned int outPortId, stream_packet_obj &packet)
{
    if (this->stVencInfo.yuvEnable && outPortId == 0 && this->mapModOutputInfo.size() > 1)
    {
        return AmigosModuleMiBase::_EnqueueOut(outPortId, packet);
    }
    if(!packet)
    {
        return -1;
    }
    if (packet->en_type != EN_VIDEO_CODEC_DATA)
    {
        return -1;
    }
    auto packetIn = this->_OutPacket(300);
    if (!packetIn)
    {
        return -1;
    }
    return packet->compare_copy(packetIn);
}
int AmigosModuleVenc::_DequeueOut(unsigned int outPortId, stream_packet_obj &packet)
{
    if (this->stVencInfo.yuvEnable && outPortId == 0 && this->mapModOutputInfo.size() > 1)
    {
        return AmigosModuleMiBase::_DequeueOut(outPortId, packet);
    }
    return 0;
}

stream_packet_info AmigosModuleVenc::_GetStreamInfo(unsigned int outPortId)
{
    struct stream_packet_info streamInfo;
    if (this->stVencInfo.yuvEnable && outPortId == 0 && this->mapModOutputInfo.size() > 1)
    {
        streamInfo.en_type = EN_RAW_FRAME_DATA;
        streamInfo.raw_vid_i.plane_num = 1;
        streamInfo.raw_vid_i.plane_info[0].fmt = RAW_FORMAT_YUV420SP;
        streamInfo.raw_vid_i.plane_info[0].width = this->stVencInfo.yuvWidth;
        streamInfo.raw_vid_i.plane_info[0].height = this->stVencInfo.yuvHeight;
        return streamInfo;
    }
    streamInfo.en_type         = EN_VIDEO_CODEC_DATA;
    streamInfo.es_vid_i.fmt    = ss_enum_cast<es_video_fmt>::from_str(this->stVencInfo.strEncodeType);
    streamInfo.es_vid_i.width  = this->stVencInfo.intWidth;
    streamInfo.es_vid_i.height = this->stVencInfo.intHeight;
    return streamInfo;
}
stream_packet_obj AmigosModuleVenc::_DequeueFromInside(unsigned int outPortId, unsigned int ms)
{
    if (this->stVencInfo.yuvEnable && outPortId == 0 && this->mapModOutputInfo.size() > 1)
    {
        return AmigosModuleMiBase::_DequeueFromInside(outPortId, ms);
    }
    return this->_OutPacket(ms);
}
stream_packet_obj AmigosModuleVenc::_OutPacket(unsigned int ms)
{
    unsigned int  totalSize = 0;
    bool bFrameEnd = false;
    auto packet = stream_packet_base::make<AmigosVencStreamPacket>(this->stModInfo.devId, this->stModInfo.chnId, ms);
    if(!packet)
    {
        return nullptr;
    }
    if ((unsigned int)this->stVencInfo.intWidth != packet->es_vid_i.width
        || (unsigned int)this->stVencInfo.intHeight != packet->es_vid_i.height)
    {
        this->stVencInfo.intWidth = packet->es_vid_i.width;
        this->stVencInfo.intHeight = packet->es_vid_i.height;
        std::stringstream ss;
        ss << packet->es_vid_i.width << "x" << packet->es_vid_i.height;
        this->env.Out(0)["STD_RES"] = ss.str();
    }
    for (unsigned int i = 0; i < packet->es_vid_i.packet_count; ++i)
    {
        totalSize += packet->es_vid_i.packet_info[i].size;
        bFrameEnd = packet->es_vid_i.packet_info[i].b_end ? true : bFrameEnd;
    }
    if (packet->es_vid_i.fmt == ES_STREAM_H265 || packet->es_vid_i.fmt == ES_STREAM_H264
        || packet->es_vid_i.fmt == ES_STREAM_AV1)
    {
        this->env.MonitorOut(0, totalSize, packet->isKeyFrame(), bFrameEnd);
    }
    else
    {
        this->env.MonitorOut(0, totalSize);
    }
    return packet;
}
bool AmigosModuleVenc::_NeedResetStreamTraverse(unsigned int outPortId, unsigned int bindType,
                                                unsigned int &inPortId)
{
    auto iter = this->mapPortIn.begin();
    if (iter == this->mapPortIn.end())
    {
        return false;
    }
    if (bindType & E_MOD_PORT_TYPE_KERNEL)
    {
        /* Venc output is direct bind to another venc, and it can change stream output by the post venc. */
        return false;
    }
    inPortId = iter->first;
    return true;
}
void AmigosModuleVenc::_ResetStreamOut(unsigned int outPortId, unsigned int width, unsigned int height)
{
    auto itOut = this->mapPortOut.find(outPortId);
    if (itOut == this->mapPortOut.end())
    {
        AMILOG_ERR << "can not find outPortId " << outPortId << COLOR_ENDL;
        return;
    }
    if (this->state.Get() != this->state.MODULE_STATE_NONE)
    {
        MI_VENC_ChnAttr_t stAttr;
        memset(&stAttr, 0, sizeof(MI_VENC_ChnAttr_t));
        if (MI_SUCCESS != MI_VENC_GetChnAttr(this->stModInfo.devId, this->stModInfo.chnId, &stAttr))
        {
            AMIGOS_ERR("MI_VENC_GetChnAttr Failed\n");
            return;
        }
        if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
        {
            stAttr.stVeAttr.stAttrH264e.u32PicWidth = width;
            stAttr.stVeAttr.stAttrH264e.u32PicHeight = height;
        }
        else if(stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
        {
            stAttr.stVeAttr.stAttrH265e.u32PicWidth = width;
            stAttr.stVeAttr.stAttrH265e.u32PicHeight = height;
        }
        else if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
        {
            stAttr.stVeAttr.stAttrJpeg.u32PicWidth = width;
            stAttr.stVeAttr.stAttrJpeg.u32PicHeight = height;
        }
        else if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_AV1)
        {
            stAttr.stVeAttr.stAttrAv1.u32PicWidth = width;
            stAttr.stVeAttr.stAttrAv1.u32PicHeight = height;
        }
        else
        {
            AMIGOS_ERR("Enc type %d is not support\n", stAttr.stVeAttr.eType);
            return;
        }
        if (MI_SUCCESS != MI_VENC_SetChnAttr(this->stModInfo.devId, this->stModInfo.chnId, &stAttr))
        {
            AMIGOS_ERR("MI_VENC_SetChnAttr Failed\n");
            return;
        }
        this->env.Out(0)["WIDTH"]  = std::to_string(width);
        this->env.Out(0)["HEIGHT"] = std::to_string(height);
        this->env.In(0)["WIDTH"]   = std::to_string(width);
        this->env.In(0)["HEIGHT"]  = std::to_string(height);
        return;
    }
    auto it= this->mapDevInfo.find(outPortId);
    if (it == this->mapDevInfo.end())
    {
        AMILOG_ERR << "can not find outPortId " << outPortId << COLOR_ENDL;
        return;
    }
    this->env.Out(0)["WIDTH"]  = std::to_string(width);
    this->env.Out(0)["HEIGHT"] = std::to_string(height);
    this->env.In(0)["WIDTH"]   = std::to_string(width);
    this->env.In(0)["HEIGHT"]  = std::to_string(height);
    it->second.width  = width;
    it->second.height = height;
}

#ifdef CONFIG_MI_SYSCALL_L2R_DESTROY_SUPPORT
bool AmigosModuleVenc::_NeedMarkDeinitOnRtos()
{
    return true;
}
bool AmigosModuleVenc::_NeedMarkUnbindOnRtos(unsigned int inPortId)
{
    return true;
}
bool AmigosModuleVenc::_NeedMarkStopOutOnRtos(unsigned int outPortId)
{
    return true;
}
#endif

void *VencReader(struct ss_thread_buffer *thread_buf)
{
    AmigosModuleVenc::VencReaderDesc *desc = (AmigosModuleVenc::VencReaderDesc *)thread_buf->buf;
    if (!desc->packet)
    {
        desc->packet = desc->pModule->_OutPacket(300);
        if (!desc->packet)
        {
            return NULL;
        }
    }
    if (0 == desc->linker->enqueue(desc->packet))
    {
        desc->packet = nullptr;
        return NULL;
    }
    usleep(1000 * 10);
    return NULL;
}

AMIGOS_MODULE_INIT("VENC", AmigosModuleVenc);
