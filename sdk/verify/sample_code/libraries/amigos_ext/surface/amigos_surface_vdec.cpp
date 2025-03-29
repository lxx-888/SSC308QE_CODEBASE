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
#include "amigos_surface_vdec.h"

AmigosSurfaceVdec::AmigosSurfaceVdec(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, true)
{
}

AmigosSurfaceVdec::~AmigosSurfaceVdec() {}

void AmigosSurfaceVdec::_LoadDb()
{
    VdecOutInfo stVdecOutInfo;
    stVdecInfo.dpBufMode     = this->pDb->GetMod<int>("BUF_MODE");
    stVdecInfo.refFrameNum   = this->pDb->GetMod<int>("REF_FRAME");
    stVdecInfo.bitstreamSize = this->pDb->GetMod<int>("BIT_STREAM_BUFFER");
    stVdecInfo.uintBufWidth  = this->pDb->GetMod<int>("BUF_WIDTH");
    stVdecInfo.uintBufHeight = this->pDb->GetMod<int>("BUF_HEIGHT");
    //output info
    for (auto itVdecOut = this->mapModOutputInfo.begin(); itVdecOut != this->mapModOutputInfo.end(); itVdecOut++)
    {
        stVdecOutInfo.intPortId        = itVdecOut->second.curLoopId;
        stVdecOutInfo.uintDecOutWidth  = this->pDb->GetOut<int>(itVdecOut->second.curLoopId, "VID_W");
        stVdecOutInfo.uintDecOutHeight = this->pDb->GetOut<int>(itVdecOut->second.curLoopId, "VID_H");
        // crop
        stVdecOutInfo.bEnable = this->pDb->GetOut<int>(itVdecOut->second.curLoopId, "CROP_ENABLE");
        if (stVdecOutInfo.bEnable)
        {
            stVdecOutInfo.uintVdecCropX = this->pDb->GetOut<int>(itVdecOut->second.curLoopId, "CROP_X");
            stVdecOutInfo.uintVdecCropY = this->pDb->GetOut<int>(itVdecOut->second.curLoopId, "CROP_Y");
            stVdecOutInfo.uintVdecCropW = this->pDb->GetOut<int>(itVdecOut->second.curLoopId, "CROP_W");
            stVdecOutInfo.uintVdecCropH = this->pDb->GetOut<int>(itVdecOut->second.curLoopId, "CROP_H");
        }
        vVdecOutInfo.push_back(stVdecOutInfo);
    }
}

void AmigosSurfaceVdec::_UnloadDb()
{
    vVdecOutInfo.clear();
    stVdecInfo.Clear();
}

void AmigosSurfaceVdec::GetInfo(VdecInfo &info, std::vector<VdecOutInfo> &out) const
{
    info = stVdecInfo;
    out  = vVdecOutInfo;
}

void AmigosSurfaceVdec::SetInfo(const VdecInfo &info, const std::vector<VdecOutInfo> &out)
{
    stVdecInfo   = info;
    vVdecOutInfo = out;
}
