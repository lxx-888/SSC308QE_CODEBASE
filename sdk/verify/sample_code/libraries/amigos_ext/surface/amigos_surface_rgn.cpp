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


#include <cstdio>
#include "amigos_surface_rgn.h"
#include "amigos_surface_base.h"

AmigosSurfaceRgn::~AmigosSurfaceRgn()
{
}

AmigosSurfaceRgn::AmigosSurfaceRgn(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false)
{
}

void AmigosSurfaceRgn::_LoadDb()
{
    this->stRgnInfo.intColorInvertEn         = this->pDb->GetMod<unsigned int>("COLOR_INVERT_EN");
    this->stRgnInfo.strColorInvertMode       = this->pDb->GetMod<std::string>("COLOR_INVERT_MODE");
    this->stRgnInfo.intColorInvertThresholdL = this->pDb->GetMod<unsigned int>("THRESHOLD_LOW");
    this->stRgnInfo.intColorInvertThresholdH = this->pDb->GetMod<unsigned int>("THRESHOLD_HIGH");

    unsigned int attachCnt      = this->pDb->GetMod<unsigned int>("ATTACH_COUNT");
    char         attachName[32] = "";
    for (unsigned int i = 0; i < attachCnt; ++i)
    {
        sprintf(attachName, "ATTACH_%u", i);
        RgnModAttachInfo stAttachInfo;
        stAttachInfo.strMod      = this->pDb->GetMod<std::string>(attachName, "MODULE");
        stAttachInfo.intPort     = this->pDb->GetMod<unsigned int>(attachName, "PORT");
        stAttachInfo.intIsInPort = this->pDb->GetMod<unsigned int>(attachName, "IS_INPORT");
        stAttachInfo.intTimingW  = this->pDb->GetMod<unsigned int>(attachName, "TIMING_W");
        stAttachInfo.intTimingH  = this->pDb->GetMod<unsigned int>(attachName, "TIMING_H");
        this->stRgnInfo.lstAttach.push_back(stAttachInfo);
    }

    for (auto iter = this->mapModInputInfo.begin(); iter != this->mapModInputInfo.end(); ++iter)
    {
        const char  *strParamKey = "MODE_PARAM";
        RgnInputInfo stRgnInputInfo;
        stRgnInputInfo.strMode = this->pDb->GetIn<std::string>(iter->first, "MODE");
        if (stRgnInputInfo.strMode == "canvas")
        {
            CanvasInfo &info   = stRgnInputInfo.stCanvasInfo;
            info.intShow       = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "SHOW");
            info.intLayer      = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "LAYER");
            info.intPosX       = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "POS_X");
            info.intPosY       = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "POS_Y");
            info.strAlphaType  = this->pDb->GetIn<std::string>(iter->first, strParamKey, "ALPHA_TYPE");
            info.intAlphaVal   = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "ALPHA_VAL");
            info.intAlpha0     = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "ALPHA_0");
            info.intAlpha1     = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "ALPHA_1");
            info.intPaletteIdx = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "PALETTE_IDX");
        }
        else if (stRgnInputInfo.strMode == "line" || stRgnInputInfo.strMode == "osd_frame")
        {
            LineInfo &info = stRgnInputInfo.stLineInfo;
            info.strPixelFmt  = this->pDb->GetIn<std::string>(iter->first, strParamKey, "PIXEL_FORMAT");
            info.intColor     = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "COLOR");
            info.strThickness = this->pDb->GetIn<std::string>(iter->first, strParamKey, "THICKNESS");
        }
        else if (stRgnInputInfo.strMode == "osd_dot_matrix")
        {
            DotMatrixInfo &info = stRgnInputInfo.stDotMatrixInfo;
            info.strPixelFmt  = this->pDb->GetIn<std::string>(iter->first, strParamKey, "PIXEL_FORMAT");
            info.intColor     = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "COLOR");
            info.strSize      = this->pDb->GetIn<std::string>(iter->first, strParamKey, "SIZE");
        }
        else if (stRgnInputInfo.strMode == "text")
        {
            TextInfo &info   = stRgnInputInfo.stTextInfo;
            info.strPixelFmt = this->pDb->GetIn<std::string>(iter->first, strParamKey, "PIXEL_FORMAT");
            info.intColor    = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "COLOR");
            info.strFontSize = this->pDb->GetIn<std::string>(iter->first, strParamKey, "FONT_SIZE");
            info.intPosX     = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "POS_X");
            info.intPosY     = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "POS_Y");
            info.intAreaW    = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "AREA_W");
            info.intAreaH    = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "AREA_H");
            info.strFontFile = this->pDb->GetIn<std::string>(iter->first, strParamKey, "FONT_FILE");
        }
        else if (stRgnInputInfo.strMode == "cover" || stRgnInputInfo.strMode == "poly")
        {
            CoverInfo &info   = stRgnInputInfo.stCoverInfo;
            info.strType      = this->pDb->GetIn<std::string>(iter->first, strParamKey, "TYPE");
            info.intColor     = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "COLOR");
            info.strBlockSize = this->pDb->GetIn<std::string>(iter->first, strParamKey, "BLOCK_SIZE");
        }
        else if (stRgnInputInfo.strMode == "frame")
        {
            FrameInfo &info   = stRgnInputInfo.stFrameInfo;
            info.intColor     = this->pDb->GetIn<unsigned int>(iter->first, strParamKey, "COLOR");
            info.strThickness = this->pDb->GetIn<std::string>(iter->first, strParamKey, "THICKNESS");
        }
        this->mapRgnInputInfo[iter->first] = stRgnInputInfo;
    }
}

void AmigosSurfaceRgn::_UnloadDb()
{
    mapRgnInputInfo.clear();
    stRgnInfo.Clear();
}

bool AmigosSurfaceRgn::GetRgnInfo(RgnInfo &stInfo) const
{
    stInfo = this->stRgnInfo;
    return true;
}
void AmigosSurfaceRgn::SetRgnInfo(const RgnInfo &stInfo)
{
    this->stRgnInfo = stInfo;
}
bool AmigosSurfaceRgn::GetRgnInInfo(unsigned int portId, RgnInputInfo &stIn) const
{
    auto it = this->mapRgnInputInfo.find(portId);
    if (it == this->mapRgnInputInfo.end())
    {
        return false;
    }
    stIn = it->second;
    return true;
}
void AmigosSurfaceRgn::SetRgnInInfo(unsigned int portId, const RgnInputInfo &stIn)
{
    auto it = this->mapRgnInputInfo.find(portId);
    if (it == this->mapRgnInputInfo.end())
    {
        return;
    }
    it->second = stIn;
}
