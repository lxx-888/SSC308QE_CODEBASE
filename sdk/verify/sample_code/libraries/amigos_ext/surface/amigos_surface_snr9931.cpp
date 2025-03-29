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
#include <sstream>
#include "amigos_surface_snr9931.h"

AmigosSurfaceSnr9931::AmigosSurfaceSnr9931(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false) {}

AmigosSurfaceSnr9931::~AmigosSurfaceSnr9931() {}

void AmigosSurfaceSnr9931::_LoadDb()
{
    int intSnrCnt = this->pDb->GetMod<int>("SNR_CNT");

    for (int i = 0; i < intSnrCnt; i++)
    {
        std::stringstream ss;
        ss << "SNR" << i;
        mapSnr9931Info[i].intHdrType   = this->pDb->GetMod<int>(ss.str().c_str(), "HDR_TYPE");
        mapSnr9931Info[i].intSensorId  = this->pDb->GetMod<int>(ss.str().c_str(), "SNR_ID");
        mapSnr9931Info[i].intSensorRes = this->pDb->GetMod<int>(ss.str().c_str(), "SNR_RES");
        mapSnr9931Info[i].intADIndex   = this->pDb->GetMod<int>(ss.str().c_str(), "AD_CNT");
    }
}

void AmigosSurfaceSnr9931::_UnloadDb()
{
    mapSnr9931Info.clear();
}

void AmigosSurfaceSnr9931::GetInfo(std::map<int, Snr9931Info> &info) const
{
    info = mapSnr9931Info;
}

void AmigosSurfaceSnr9931::SetInfo(const std::map<int, Snr9931Info> &info)
{
    mapSnr9931Info = info;
}
