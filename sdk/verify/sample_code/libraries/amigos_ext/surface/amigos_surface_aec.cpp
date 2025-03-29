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

#include "amigos_surface_aec.h"

AmigosSurfaceAec::AmigosSurfaceAec(const std::string &strInSection) : AmigosSurfaceBase(strInSection, false, false) {}

AmigosSurfaceAec::~AmigosSurfaceAec() {}

void AmigosSurfaceAec::_LoadDb()
{
    this->stAecInfo.uintSampleRate = this->pDb->GetMod<unsigned int>("SAMPLE_RATE");
    this->stAecInfo.uintPeriodSize = this->pDb->GetMod<unsigned int>("PERIOD_SIZE");
    this->stAecInfo.uintSoundMode  = this->pDb->GetMod<unsigned int>("CHN_NUM");
    this->stAecInfo.intDelaySample = this->pDb->GetMod<int>("DELAY_SAMPLE");
    this->stAecInfo.NoiseEnable    = this->pDb->GetMod<unsigned int>("NOISE");

    std::string str;
    for (unsigned int i = 0; i < 6; i++)
    {
        str = "FREQ_" + std::to_string(i);
        this->stAecInfo.uintSupFreqArray[i] = this->pDb->GetMod<unsigned int>("SUP_FREQ_PARAM", str.c_str());
    }
    for (unsigned int i = 0; i < 7; i++)
    {
        str = "INTENSITY_" + std::to_string(i);
        this->stAecInfo.uintSupIntensityArray[i] = this->pDb->GetMod<unsigned int>("SUP_INTENSITY_PARAM", str.c_str());
    }
}

void AmigosSurfaceAec::_UnloadDb()
{
    stAecInfo.Clear();
}

void AmigosSurfaceAec::GetInfo(AecInfo &info) const
{
    info = this->stAecInfo;
}

void AmigosSurfaceAec::SetInfo(const AecInfo &info)
{
    this->stAecInfo = info;
}
