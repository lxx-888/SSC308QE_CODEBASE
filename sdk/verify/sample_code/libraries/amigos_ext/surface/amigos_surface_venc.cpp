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
#include "amigos_surface_venc.h"

std::map<unsigned int, AmigosSurfaceVenc::VencDevInfo> AmigosSurfaceVenc::mapDevInfo = {
    {0, {0, 0, 0}},
    {8, {0, 0, 0}},
};
AmigosSurfaceVenc::AmigosSurfaceVenc(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, true, false)
{
}

AmigosSurfaceVenc::~AmigosSurfaceVenc() {}

void AmigosSurfaceVenc::_LoadDb()
{
    char                       strLayerName[30];
    stVencInfo.intMaxWidth    = this->pDb->GetMod<int>("MAX_STREAM_W");
    stVencInfo.intMaxHeight   = this->pDb->GetMod<int>("MAX_STREAM_H");
    stVencInfo.intWidth       = this->pDb->GetMod<int>("STREAM_W");
    stVencInfo.intHeight      = this->pDb->GetMod<int>("STREAM_H");
    stVencInfo.strEncodeType  = this->pDb->GetMod<std::string>("EN_TYPE");
    stVencInfo.intEncodeFps   = this->pDb->GetMod<int>("EN_FPS");
    stVencInfo.intMultiSlice  = this->pDb->GetMod<int>("MULTI_SLICE");
    stVencInfo.intSliceRowCnt = this->pDb->GetMod<int>("SLICE_ROW_CNT");
    if (mapDevInfo[this->stModInfo.devId].width < this->pDb->GetMod<unsigned int>("MAX_STREAM_W"))
    {
        mapDevInfo[this->stModInfo.devId].width = this->pDb->GetMod<int>("MAX_STREAM_W");
    }
    if (mapDevInfo[this->stModInfo.devId].height < this->pDb->GetMod<unsigned int>("MAX_STREAM_H"))
    {
        mapDevInfo[this->stModInfo.devId].height = this->pDb->GetMod<int>("MAX_STREAM_H");
    }
    mapDevInfo[this->stModInfo.devId].refCnt++;

    /*Max strem cnt enable*/
    stVencInfo.StreamCntEnable = this->pDb->GetMod<int>("STREAM_CNT_EN");
    if(stVencInfo.StreamCntEnable)
    {
        stVencInfo.intMaxStreamCnt = this->pDb->GetMod<int>("MAX_STREAM_CNT");
    }

    /*ltr param*/
    stVencInfo.LtrEnable   = this->pDb->GetMod<int>("LTR_EN");
    if(stVencInfo.LtrEnable)
    {
        stVencInfo.Base        = this->pDb->GetMod<int>("LTR_PARAM","BASE");
        stVencInfo.Enhance     = this->pDb->GetMod<int>("LTR_PARAM","ENHANCE");
        stVencInfo.bEnablePred = this->pDb->GetMod<int>("LTR_PARAM","BENABLEPRED");
    }

    /*DeBreath param*/
    stVencInfo.DeBreathEnable   = this->pDb->GetMod<int>("DEBREATH_ENABLE");
    if(stVencInfo.DeBreathEnable)
    {
        stVencInfo.Strength0     = this->pDb->GetMod<int>("DEBREATH_PARAM","STRENGTH0");
        stVencInfo.Strength1 = this->pDb->GetMod<int>("DEBREATH_PARAM","STRENGTH1");
    }

    /*roi param*/
    stVencInfo.RoiNum        = this->pDb->GetMod<int>("ROI_NUM");
    for(int i = 0; i<stVencInfo.RoiNum; i++)
    {
        snprintf(strLayerName, 30, "ROI_REGION_%d", i);
        stVencRoi[i].bEnable       = this->pDb->GetMod<int>(strLayerName,"ROI_EN");
        stVencRoi[i].index         = this->pDb->GetMod<int>(strLayerName,"INDEX");
        stVencRoi[i].bAbsQp        = this->pDb->GetMod<int>(strLayerName,"BABSQP");
        stVencRoi[i].s32Qp         = this->pDb->GetMod<int>(strLayerName,"QP");
        stVencRoi[i].u16RectX      = this->pDb->GetMod<int>(strLayerName,"RECTX");
        stVencRoi[i].u16RectY      = this->pDb->GetMod<int>(strLayerName,"RECTY");
        stVencRoi[i].u16RectWidth  = this->pDb->GetMod<int>(strLayerName,"RECTW");
        stVencRoi[i].u16RectHeight = this->pDb->GetMod<int>(strLayerName,"RECTH");
    }

    /*rc mode & param*/
    stVencInfo.strRcMode    = this->pDb->GetMod<std::string>("RC_MODE");
    if(stVencInfo.strRcMode == "cbr")
    {
        stVencInfo.stCbrCfg.intBitRate = this->pDb->GetMod<int>("RC_MODE_PARAM","BIT_RATE");
        stVencInfo.stCbrCfg.intGop     = this->pDb->GetMod<int>("RC_MODE_PARAM","GOP");
    }
    else if(stVencInfo.strRcMode == "vbr")
    {
        stVencInfo.stVbrCfg.intBitRate = this->pDb->GetMod<int>("RC_MODE_PARAM","BIT_RATE");
        stVencInfo.stVbrCfg.intGop     = this->pDb->GetMod<int>("RC_MODE_PARAM","GOP");
        stVencInfo.stVbrCfg.intMinQp   = this->pDb->GetMod<int>("RC_MODE_PARAM","MIN_QP");
        stVencInfo.stVbrCfg.intMaxQp   = this->pDb->GetMod<int>("RC_MODE_PARAM","MAX_QP");
    }
    else if(stVencInfo.strRcMode == "fixqp")
    {
        stVencInfo.stFixQpCfg.intGop = this->pDb->GetMod<int>("RC_MODE_PARAM","GOP");
        stVencInfo.stFixQpCfg.intIQp = this->pDb->GetMod<int>("RC_MODE_PARAM","IQP");
        stVencInfo.stFixQpCfg.intPQp = this->pDb->GetMod<int>("RC_MODE_PARAM","PQP");
        stVencInfo.stFixQpCfg.intQfactor = this->pDb->GetMod<int>("RC_MODE_PARAM","QFACTOR");
    }
    else if(stVencInfo.strRcMode == "avbr")
    {
        stVencInfo.stAvbrCfg.intBitRate = this->pDb->GetMod<int>("RC_MODE_PARAM","BIT_RATE");
        stVencInfo.stAvbrCfg.intGop     = this->pDb->GetMod<int>("RC_MODE_PARAM","GOP");
        stVencInfo.stAvbrCfg.intMinQp   = this->pDb->GetMod<int>("RC_MODE_PARAM","MIN_QP");
        stVencInfo.stAvbrCfg.intMaxQp   = this->pDb->GetMod<int>("RC_MODE_PARAM","MAX_QP");
    }
    else if(stVencInfo.strRcMode == "cvbr")
    {
        stVencInfo.stCvbrCfg.intMaxBitRate = this->pDb->GetMod<int>("RC_MODE_PARAM","BIT_RATE");
        stVencInfo.stCvbrCfg.intGop     = this->pDb->GetMod<int>("RC_MODE_PARAM","GOP");
        stVencInfo.stCvbrCfg.intShortTermStatsTime   = this->pDb->GetMod<int>("RC_MODE_PARAM","SHORT_STATS_TIMES");
        stVencInfo.stCvbrCfg.intLongTermStatsTime   = this->pDb->GetMod<int>("RC_MODE_PARAM","LONG_STATS_TIMES");
        stVencInfo.stCvbrCfg.intLongTermMaxBitRate   = this->pDb->GetMod<int>("RC_MODE_PARAM","LONG_MAX_BITRATE");
        stVencInfo.stCvbrCfg.intLongTermMinBitRate   = this->pDb->GetMod<int>("RC_MODE_PARAM","LONG_MIN_BITRATE");
    }
    else
    {
        AMIGOS_ERR("RC Mode error!\n");
        return;
    }
    stVencInfo.yuvEnable = this->pDb->GetMod<unsigned int>("YUV_EN");
    if (stVencInfo.yuvEnable)
    {
        stVencInfo.yuvWidth = this->pDb->GetMod<unsigned int>("YUV_PARAM", "VID_W");
        stVencInfo.yuvHeight = this->pDb->GetMod<unsigned int>("YUV_PARAM", "VID_H");
    }
}

void AmigosSurfaceVenc::_UnloadDb()
{
    stVencInfo.Clear();
    mapDevInfo[this->stModInfo.devId].refCnt--;
    if (!mapDevInfo[this->stModInfo.devId].refCnt)
    {
        mapDevInfo[this->stModInfo.devId].width = 0;
        mapDevInfo[this->stModInfo.devId].height = 0;
    }
}

void AmigosSurfaceVenc::GetInfo(VencInfo &info) const
{
    info = stVencInfo;
}

void AmigosSurfaceVenc::SetInfo(const VencInfo &info)
{
    stVencInfo = info;
}
