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
#include "amigos_surface_vdisp.h"

AmigosSurfaceVdisp::AmigosSurfaceVdisp(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, true, true) {}

AmigosSurfaceVdisp::~AmigosSurfaceVdisp() {}

void AmigosSurfaceVdisp::_LoadDb()
{
    VdispInputInfo  stVdispInputInfo;
    VdispOutputInfo stVdispOutputInfo;
    for (auto itMapVdispIn = this->mapModInputInfo.begin(); itMapVdispIn != this->mapModInputInfo.end(); itMapVdispIn++)
    {
        stVdispInputInfo.intChnId    = itMapVdispIn->second.curLoopId;
        stVdispInputInfo.intFreeRun  = this->pDb->GetIn<int>(itMapVdispIn->second.curLoopId, "FREE_RUN");
        stVdispInputInfo.intVdispInX = this->pDb->GetIn<int>(itMapVdispIn->second.curLoopId, "VDISP_X");
        stVdispInputInfo.intVdispInY = this->pDb->GetIn<int>(itMapVdispIn->second.curLoopId, "VDISP_Y");
        stVdispInputInfo.intVdispInWidth  = this->pDb->GetIn<int>(itMapVdispIn->second.curLoopId, "VDISP_W");
        stVdispInputInfo.intVdispInHeight = this->pDb->GetIn<int>(itMapVdispIn->second.curLoopId, "VDISP_H");
        vVdispInputInfo.push_back(stVdispInputInfo);
    }
    for (auto itMapVdispOut = this->mapModOutputInfo.begin(); itMapVdispOut != this->mapModOutputInfo.end();
            itMapVdispOut++)
    {
        stVdispOutputInfo.intPortId            = itMapVdispOut->second.curLoopId;
        stVdispOutputInfo.intVdispOutFrameRate = itMapVdispOut->second.curFrmRate;
        stVdispOutputInfo.intVdispOutWidth     = this->pDb->GetOut<int>(itMapVdispOut->second.curLoopId, "VID_W");
        stVdispOutputInfo.intVdispOutHeight    = this->pDb->GetOut<int>(itMapVdispOut->second.curLoopId, "VID_H");
        stVdispOutputInfo.intVdispOutPts       = this->pDb->GetOut<int>(itMapVdispOut->second.curLoopId, "PTS");
        stVdispOutputInfo.strVdispOutFormat    = this->pDb->GetOut<std::string>(itMapVdispOut->second.curLoopId, "OUT_FMT");
        stVdispOutputInfo.strVdispOutBkColor   = this->pDb->GetOut<std::string>(itMapVdispOut->second.curLoopId, "BK_COLOR");
        vVdispOutputInfo.push_back(stVdispOutputInfo);
    }
}

void AmigosSurfaceVdisp::_UnloadDb()
{
    vVdispInputInfo.clear();
    vVdispOutputInfo.clear();
}

void AmigosSurfaceVdisp::GetInfo(std::vector<VdispInputInfo> &in, std::vector<VdispOutputInfo> &out) const
{
    in  = vVdispInputInfo;
    out = vVdispOutputInfo;
}

void AmigosSurfaceVdisp::SetInfo(const std::vector<VdispInputInfo> &in, const std::vector<VdispOutputInfo> &out)
{
    vVdispInputInfo  = in;
    vVdispOutputInfo = out;
}
