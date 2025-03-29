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

#include "amigos_surface_exp.h"
#include "ss_enum_cast.hpp"

SS_ENUM_CAST_STR(AmigosSurfaceExp::ExpInMode, // ExpInMode
                 {{AmigosSurfaceExp::E_EXP_IN_MODE_DIRECT, "direct"},
                  {AmigosSurfaceExp::E_EXP_IN_MODE_HAND_SHAKE, "handshake"}});

AmigosSurfaceExp::AmigosSurfaceExp(const std::string &strInSection) : AmigosSurfaceBase(strInSection, false, false) {}

AmigosSurfaceExp::~AmigosSurfaceExp() {}

void AmigosSurfaceExp::_LoadDb()
{
    const char *strParamKey = "MODE_PARAM";
    for (auto itInInfo = this->mapModInputInfo.begin(); itInInfo != this->mapModInputInfo.end(); ++itInInfo)
    {
        ExpInInfo expInInfo;
        expInInfo.mode = ss_enum_cast<ExpInMode>::from_str(this->pDb->GetIn<std::string>(itInInfo->first, "MODE"));
        if (expInInfo.mode == E_EXP_IN_MODE_HAND_SHAKE)
        {
            expInInfo.urlSuffix  = this->pDb->GetIn<std::string>(itInInfo->second.curLoopId, strParamKey, "URL_SUFFIX");
            expInInfo.workId     = this->pDb->GetIn<unsigned int>(itInInfo->second.curLoopId, strParamKey, "WORK_ID");
            expInInfo.socketPort = this->pDb->GetIn<unsigned int>(itInInfo->second.curLoopId, strParamKey, "SOCKET_PORT");
        }
        else if (expInInfo.mode == E_EXP_IN_MODE_DIRECT)
        {
            expInInfo.socketPort = this->pDb->GetIn<unsigned int>(itInInfo->second.curLoopId, strParamKey, "SOCKET_PORT");
        }
        this->mapExpInInfo[itInInfo->first] = expInInfo;
    }
    for (auto itOutInfo = this->mapModOutputInfo.begin(); itOutInfo != this->mapModOutputInfo.end(); ++itOutInfo)
    {
        ExpOutInfo expOutInfo;
        // exp://xxx.xxx.xxx.xxx:1234/aabbcc
        expOutInfo.url      = this->pDb->GetOut<std::string>(itOutInfo->second.curLoopId, "URL");
        expOutInfo.workId   = this->pDb->GetOut<unsigned int>(itOutInfo->second.curLoopId, "WORK_ID");
        expOutInfo.portType = this->pDb->GetOut<std::string>(itOutInfo->second.curLoopId, "TYPE");
        this->mapExpOutInfo[itOutInfo->first] = expOutInfo;
        if (expOutInfo.portType == "video")
        {
            expOutInfo.width  = this->pDb->GetOut<unsigned int>(itOutInfo->second.curLoopId, "TYPE_PARAM", "VID_W");
            expOutInfo.height = this->pDb->GetOut<unsigned int>(itOutInfo->second.curLoopId, "TYPE_PARAM", "VID_H");
            expOutInfo.fmt    = this->pDb->GetOut<std::string>(itOutInfo->second.curLoopId, "TYPE_PARAM", "OUT_FMT");
        }
        else if (expOutInfo.portType == "audio")
        {
            expOutInfo.channel    = this->pDb->GetOut<std::string>(itOutInfo->second.curLoopId, "TYPE_PARAM", "CHN");
            expOutInfo.samplerate = this->pDb->GetOut<unsigned int>(itOutInfo->second.curLoopId, "TYPE_PARAM", "SAMPLE_RATE");
            expOutInfo.fmt        = this->pDb->GetOut<std::string>(itOutInfo->second.curLoopId, "TYPE_PARAM", "OUT_FMT");
        }
    }
}

void AmigosSurfaceExp::_UnloadDb()
{
    this->mapExpInInfo.clear();
    this->mapExpOutInfo.clear();
}
