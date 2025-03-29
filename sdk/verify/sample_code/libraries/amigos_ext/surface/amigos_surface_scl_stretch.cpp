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
#include "amigos_surface_scl_stretch.h"

AmigosSurfaceSclStretch::AmigosSurfaceSclStretch(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false)
{
}

AmigosSurfaceSclStretch::~AmigosSurfaceSclStretch() {}

void AmigosSurfaceSclStretch::_LoadDb()
{
    this->stSclStretchInfo.uintHwPortMode = this->pDb->GetMod<unsigned int>("PORT_MODE");
    this->stSclStretchInfo.uintOsdEn      = this->pDb->GetMod<unsigned int>("OSD_EN");
    for (auto itIn = this->mapModInputInfo.begin(); itIn != this->mapModInputInfo.end(); ++itIn)
    {
        SclStretchInInfo stSclStretchInInfo;
        stSclStretchInInfo.uintCropX       = this->pDb->GetIn<unsigned int>(itIn->second.curLoopId, "VID_CROP_X");
        stSclStretchInInfo.uintCropY       = this->pDb->GetIn<unsigned int>(itIn->second.curLoopId, "VID_CROP_Y");
        stSclStretchInInfo.uintCropW       = this->pDb->GetIn<unsigned int>(itIn->second.curLoopId, "VID_CROP_W");
        stSclStretchInInfo.uintCropH       = this->pDb->GetIn<unsigned int>(itIn->second.curLoopId, "VID_CROP_H");
        this->mapSclStretchIn[itIn->first] = stSclStretchInInfo;
    }
    for (auto itOut = this->mapModOutputInfo.begin(); itOut != this->mapModOutputInfo.end(); ++itOut)
    {
        SclStretchOutInfo stSclStretchOutInfo;
        stSclStretchOutInfo.uintRowNum = this->pDb->GetOut<unsigned int>(itOut->second.curLoopId, "ROW_NUM");
        stSclStretchOutInfo.uintColNum = this->pDb->GetOut<unsigned int>(itOut->second.curLoopId, "COL_NUM");
        stSclStretchOutInfo.strOutFmt  = this->pDb->GetOut<std::string>(itOut->second.curLoopId, "OUT_FMT");
        this->mapSclStretchOut[itOut->first] = stSclStretchOutInfo;
    }
}

void AmigosSurfaceSclStretch::_UnloadDb()
{
    this->mapSclStretchOut.clear();
    this->mapSclStretchIn.clear();
    this->stSclStretchInfo.Clear();
}

