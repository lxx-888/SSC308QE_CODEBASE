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
#include "amigos_surface_rtsp.h"
AmigosSurfaceRtsp::AmigosSurfaceRtsp(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false) {}

AmigosSurfaceRtsp::~AmigosSurfaceRtsp() {}

void AmigosSurfaceRtsp::_LoadDb()
{
    for (auto itMapRtspIn = this->mapModInputInfo.begin(); itMapRtspIn != this->mapModInputInfo.end(); itMapRtspIn++)
    {
        mapRtspInInfo[itMapRtspIn->first].url   = this->pDb->GetIn<std::string>(itMapRtspIn->second.curLoopId, "STREAM_NAME");
        mapRtspInInfo[itMapRtspIn->first].depth = this->pDb->GetIn<unsigned int>(itMapRtspIn->second.curLoopId, "DEPTH_IN");
    }
    bOpenOnvif = (unsigned char)this->pDb->GetMod<unsigned int>("ONVIF");
    for (auto itMapRtspOut = this->mapModOutputInfo.begin(); itMapRtspOut != this->mapModOutputInfo.end();
         itMapRtspOut++)
    {
        mapPortToCfg[itMapRtspOut->first].url      = this->pDb->GetOut<std::string>(itMapRtspOut->second.curLoopId, "URL");
        size_t strStart = mapPortToCfg[itMapRtspOut->first].url.find_first_of('<');
        size_t strEnd = mapPortToCfg[itMapRtspOut->first].url.find('>');
        if (strStart != std::string::npos && strEnd != std::string::npos && strEnd > strStart)
        {
            mapPortToCfg[itMapRtspOut->first].userName = mapPortToCfg[itMapRtspOut->first].url.substr(strStart + 1, strEnd - strStart - 1);
            std::istringstream iss(mapPortToCfg[itMapRtspOut->first].userName);
            std::getline(iss, mapPortToCfg[itMapRtspOut->first].userName, ':');
            std::getline(iss, mapPortToCfg[itMapRtspOut->first].passwd);
            mapPortToCfg[itMapRtspOut->first].url = mapPortToCfg[itMapRtspOut->first].url.substr(0, strStart);
        }
        mapPortToCfg[itMapRtspOut->first].portType = this->pDb->GetOut<std::string>(itMapRtspOut->second.curLoopId, "TYPE");
        mapPortToCfg[itMapRtspOut->first].fmt      = this->pDb->GetOut<std::string>(itMapRtspOut->second.curLoopId, "OUT_FMT");
        if (mapPortToCfg[itMapRtspOut->first].portType == "video")
        {
            mapPortToCfg[itMapRtspOut->first].width = this->pDb->GetOut<unsigned int>(itMapRtspOut->second.curLoopId, "TYPE_PARAM", "VID_W");
            mapPortToCfg[itMapRtspOut->first].height = this->pDb->GetOut<unsigned int>(itMapRtspOut->second.curLoopId, "TYPE_PARAM", "VID_H");
        }
        else if (mapPortToCfg[itMapRtspOut->first].portType == "audio")
        {
            mapPortToCfg[itMapRtspOut->first].channel = this->pDb->GetOut<std::string>(itMapRtspOut->second.curLoopId, "TYPE_PARAM", "CHN");
            mapPortToCfg[itMapRtspOut->first].samplerate = this->pDb->GetOut<unsigned int>(itMapRtspOut->second.curLoopId, "TYPE_PARAM", "SAMPLE_RATE");
        }
    }
}

void AmigosSurfaceRtsp::_UnloadDb()
{
    mapRtspInInfo.clear();
    mapPortToCfg.clear();
    bOpenOnvif = false;
}

bool AmigosSurfaceRtsp::GetRtspInInfo(unsigned int inPortId, struct RtspInInfo &stIn) const
{
    auto iter = this->mapRtspInInfo.find(inPortId);
    if (iter == this->mapRtspInInfo.end())
    {
        return false;
    }
    stIn = iter->second;
    return true;
}
void AmigosSurfaceRtsp::SetRtspInInfo(unsigned int inPortId, const struct RtspInInfo &stIn)
{
    auto iter = this->mapRtspInInfo.find(inPortId);
    if (iter == this->mapRtspInInfo.end())
    {
        return ;
    }
    iter->second = stIn;
}
bool AmigosSurfaceRtsp::GetRtspOutInfo(unsigned int outPortId, struct RtspOutInfo &stOut) const
{
    auto iter = this->mapPortToCfg.find(outPortId);
    if (iter == this->mapPortToCfg.end())
    {
        return false;
    }
    stOut = iter->second;
    return true;
}
void AmigosSurfaceRtsp::SetRtspOutInfo(unsigned int outPortId, const struct RtspOutInfo &stOut)
{
    auto iter = this->mapPortToCfg.find(outPortId);
    if (iter == this->mapPortToCfg.end())
    {
        return;
    }
    iter->second = stOut;
}
bool AmigosSurfaceRtsp::GetRtspOnvif(bool &bOpenOnvif) const
{
    bOpenOnvif = this->bOpenOnvif;
    return true;
}
void AmigosSurfaceRtsp::SetRtspOnvif(const bool &bOpenOnvif)
{
    this->bOpenOnvif = bOpenOnvif;
}
