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

#include "amigos_surface_ai.h"
#include <cstdio>

AmigosSurfaceAi::AmigosSurfaceAi(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, true) {}

AmigosSurfaceAi::~AmigosSurfaceAi() {}

void AmigosSurfaceAi::_LoadDb()
{
    //get ai attr
    this->stAiInfo.uintSampleRate  = this->pDb->GetMod<int>("SAMPLE_RATE");
    this->stAiInfo.strSoundMode    = this->pDb->GetMod<std::string>("SND_MOD");
    this->stAiInfo.intVolume       = this->pDb->GetMod<int>("VOLUME");
    this->stAiInfo.strFormat       = this->pDb->GetMod<std::string>("FORMAT");
    this->stAiInfo.uintPeriodSize  = this->pDb->GetMod<int>("PERIOD_SIZE");
    this->stAiInfo.intInterleaved  = this->pDb->GetMod<int>("INTERLEAVED");

    //get i2s attr
    this->stAiInfo.stAiI2sInfo.Mode      = this->pDb->GetMod<std::string>("I2S_MOD");
    this->stAiInfo.stAiI2sInfo.Format    = this->pDb->GetMod<std::string>("I2S_FORMAT");
    this->stAiInfo.stAiI2sInfo.Mclk      = this->pDb->GetMod<std::string>("I2S_MCLK");
    this->stAiInfo.stAiI2sInfo.SyncClock = this->pDb->GetMod<int>("I2S_SYNCCLOCK");
    this->stAiInfo.stAiI2sInfo.TdmSlots  = this->pDb->GetMod<int>("I2S_TMDSLOTS");
    this->stAiInfo.stAiI2sInfo.BitWidth  = this->pDb->GetMod<std::string>("I2S_BIT_WIDTH");

    // get interface
    for (auto &iter : this->stAiInfo.mapInterface)
    {
        iter.second  = this->pDb->GetMod<int>("INTERFACE_PARAM", iter.first.c_str());
    }
}

void AmigosSurfaceAi::_UnloadDb()
{
    stAiInfo.Clear();
}

void AmigosSurfaceAi::GetInfo(AiInfo &info) const
{
    info = this->stAiInfo;
}

void AmigosSurfaceAi::SetInfo(const AiInfo &info)
{
    this->stAiInfo = info;
}

