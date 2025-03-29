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

#include "amigos_surface_ao.h"
#include <string>

AmigosSurfaceAo::AmigosSurfaceAo(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false) {}

AmigosSurfaceAo::~AmigosSurfaceAo() {}

void AmigosSurfaceAo::_LoadDb()
{
    //get ao attr
    this->stAoInfo.strFormat             = this->pDb->GetMod<std::string>("FORMAT");
    this->stAoInfo.intVolume             = this->pDb->GetMod<int>("VOLUME");
    this->stAoInfo.strSoundMode          = this->pDb->GetMod<std::string>("SND_MOD");
    this->stAoInfo.uintSampleRate        = this->pDb->GetMod<int>("SAMPLE_RATE");
    this->stAoInfo.uintPeriodSize        = this->pDb->GetMod<int>("PERIOD_SIZE");
    this->stAoInfo.strChannelMode        = this->pDb->GetMod<std::string>("CHN_MOD");

    // get interface
    for (auto &iter : this->stAoInfo.mapInterface)
    {
        iter.second  = this->pDb->GetMod<int>("INTERFACE_PARAM", iter.first.c_str());
    }

    //get i2s attr
    this->stAoInfo.stAoI2sInfo.Mode      = this->pDb->GetMod<std::string>("I2S_MOD");
    this->stAoInfo.stAoI2sInfo.Format    = this->pDb->GetMod<std::string>("I2S_FORMAT");
    this->stAoInfo.stAoI2sInfo.Mclk      = this->pDb->GetMod<std::string>("I2S_MCLK");
    this->stAoInfo.stAoI2sInfo.SyncClock = this->pDb->GetMod<int>("I2S_SYNCCLOCK");
    this->stAoInfo.stAoI2sInfo.TdmSlots  = this->pDb->GetMod<int>("I2S_TMDSLOTS");
    this->stAoInfo.stAoI2sInfo.BitWidth  = this->pDb->GetMod<std::string>("I2S_BIT_WIDTH");
    this->stAoInfo.bSyncMode             = this->pDb->GetMod<int>("SYNC");
}

void AmigosSurfaceAo::_UnloadDb()
{
    this->stAoInfo.Clear();
}
