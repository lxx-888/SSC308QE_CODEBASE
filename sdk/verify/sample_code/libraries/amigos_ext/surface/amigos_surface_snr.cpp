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
#include "amigos_surface_snr.h"

AmigosSurfaceSnr::~AmigosSurfaceSnr() {}

AmigosSurfaceSnr::AmigosSurfaceSnr(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false) {}

void AmigosSurfaceSnr::_LoadDb()
{
    stSnrInfo.intHdrType   = this->pDb->GetMod<int>("HDR_TYPE");
    stSnrInfo.intSensorId  = this->pDb->GetMod<int>("SNR_ID");
    stSnrInfo.intSensorRes = this->pDb->GetMod<int>("SNR_RES");
    stSnrInfo.intSensorFps = this->pDb->GetMod<int>("SNR_FPS");
    stSnrInfo.intSensorMirror = this->pDb->GetMod<int>("SNR_MIRROR");
    stSnrInfo.intSensorFlip = this->pDb->GetMod<int>("SNR_FLIP");
}

void AmigosSurfaceSnr::_UnloadDb()
{
    stSnrInfo.Clear();
}

void AmigosSurfaceSnr::GetInfo(SnrInfo &info) const
{
    info = stSnrInfo;
}

void AmigosSurfaceSnr::SetInfo(const SnrInfo &info)
{
    stSnrInfo = info;
}
