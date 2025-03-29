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
#include <cstring>
#include "amigos_surface_scl.h"

AmigosSurfaceScl::AmigosSurfaceScl(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, true, true) {}

AmigosSurfaceScl::~AmigosSurfaceScl() {}

void AmigosSurfaceScl::_LoadDb()
{
    SclInInfo stSclInInfo;
    SclOutInfo stSclOutInfo;

    stSclInfo.uintHwPortMode = this->pDb->GetMod<unsigned int>("PORT_MODE");
    stSclInfo.uintRotation   = this->pDb->GetMod<unsigned int>("ROT");
    for (auto itMapSclIn = this->mapModInputInfo.begin(); itMapSclIn != this->mapModInputInfo.end(); itMapSclIn++)
    {
        stSclInInfo.uintSclInCropX  = this->pDb->GetIn<unsigned int>(itMapSclIn->second.curLoopId, "VID_CROP_X");
        stSclInInfo.uintSclInCropY  = this->pDb->GetIn<unsigned int>(itMapSclIn->second.curLoopId, "VID_CROP_Y");
        stSclInInfo.uintSclInWidth  = this->pDb->GetIn<unsigned int>(itMapSclIn->second.curLoopId, "VID_CROP_W");
        stSclInInfo.uintSclInHeight = this->pDb->GetIn<unsigned int>(itMapSclIn->second.curLoopId, "VID_CROP_H");
        mapSclIn[itMapSclIn->first] = stSclInInfo;
    }
    for (auto itMapSclOut = this->mapModOutputInfo.begin(); itMapSclOut != this->mapModOutputInfo.end(); itMapSclOut++)
    {
        stSclOutInfo.uintIsMirror     = this->pDb->GetOut<unsigned int>(itMapSclOut->second.curLoopId, "IS_MIRROR");
        stSclOutInfo.uintIsFlip       = this->pDb->GetOut<unsigned int>(itMapSclOut->second.curLoopId, "IS_FLIP");
        stSclOutInfo.uintSclOutCropW  = this->pDb->GetOut<unsigned int>(itMapSclOut->second.curLoopId, "VID_CROP_W");
        stSclOutInfo.uintSclOutCropH  = this->pDb->GetOut<unsigned int>(itMapSclOut->second.curLoopId, "VID_CROP_H");
        stSclOutInfo.uintSclOutCropX  = this->pDb->GetOut<unsigned int>(itMapSclOut->second.curLoopId, "VID_CROP_X");
        stSclOutInfo.uintSclOutCropY  = this->pDb->GetOut<unsigned int>(itMapSclOut->second.curLoopId, "VID_CROP_Y");
        stSclOutInfo.uintSclOutWidth  = this->pDb->GetOut<unsigned int>(itMapSclOut->second.curLoopId, "VID_W");
        stSclOutInfo.uintSclOutHeight = this->pDb->GetOut<unsigned int>(itMapSclOut->second.curLoopId, "VID_H");
        stSclOutInfo.uintCompressMode = this->pDb->GetOut<unsigned int>(itMapSclOut->second.curLoopId, "COMPRESS_MODE");
        stSclOutInfo.strOutType       = this->pDb->GetOut<std::string>(itMapSclOut->second.curLoopId, "OUT_TYPE");
        stSclOutInfo.strOutFmt        = this->pDb->GetOut<std::string>(itMapSclOut->second.curLoopId, "OUT_FMT");
        mapSclOut[itMapSclOut->first] = stSclOutInfo;
    }
}

void AmigosSurfaceScl::_UnloadDb()
{
    mapSclIn.clear();
    mapSclOut.clear();
    stSclInfo.Clear();
}

void AmigosSurfaceScl::GetInfo(SclInfo &info) const
{
    info = this->stSclInfo;
}

void AmigosSurfaceScl::SetInfo(const SclInfo &info)
{
    this->stSclInfo = info;
}

void AmigosSurfaceScl::GetInInfo(unsigned int portId, SclInInfo &stIn) const
{
    auto it = this->mapSclIn.find(portId);
    if (it == this->mapSclIn.end())
    {
        return;
    }
    stIn = it->second;
}

void AmigosSurfaceScl::SetInInfo(unsigned int portId, const SclInInfo &stIn)
{
    auto it = this->mapSclIn.find(portId);
    if (it == this->mapSclIn.end())
    {
        return;
    }
    it->second = stIn;
}

void AmigosSurfaceScl::GetOutInfo(unsigned int portId, SclOutInfo &stOut) const
{
    auto it = this->mapSclOut.find(portId);
    if (it == this->mapSclOut.end())
    {
        return;
    }
    stOut = it->second;
}

void AmigosSurfaceScl::SetOutInfo(unsigned int portId, const SclOutInfo &stOut)
{
    auto it = this->mapSclOut.find(portId);
    if (it == this->mapSclOut.end())
    {
        return;
    }
    it->second = stOut;
}

