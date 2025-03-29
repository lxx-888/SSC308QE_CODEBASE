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

#include "amigos_surface_uac.h"

AmigosSurfaceUac::AmigosSurfaceUac(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false) {}

AmigosSurfaceUac::~AmigosSurfaceUac() {}

void AmigosSurfaceUac::_LoadDb()
{
    this->stUacInfo.inMode = this->pDb->GetMod<int>("IN_MODE");
    if (this->stUacInfo.inMode)
    {
        this->stUacInfo.inSndMode    = this->pDb->GetMod<int>("IN_MODE_PARAM", "SND_MOD");
        this->stUacInfo.inSampleRate = this->pDb->GetMod<int>("IN_MODE_PARAM", "SAMPLE_RATE");
    }
    this->stUacInfo.outMode = this->pDb->GetMod<int>("OUT_MODE");
    if (this->stUacInfo.outMode)
    {
        this->stUacInfo.outSndMode    = this->pDb->GetMod<int>("OUT_MODE_PARAM", "SND_MOD");
        this->stUacInfo.outSampleRate = this->pDb->GetMod<int>("OUT_MODE_PARAM", "SAMPLE_RATE");
    }
}

void AmigosSurfaceUac::_UnloadDb()
{
}
