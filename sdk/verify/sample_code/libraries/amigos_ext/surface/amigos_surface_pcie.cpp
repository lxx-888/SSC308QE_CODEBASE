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
#include "amigos_surface_pcie.h"

AmigosSurfacePcie::AmigosSurfacePcie(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, true, true) {}

AmigosSurfacePcie::~AmigosSurfacePcie() {}

void AmigosSurfacePcie::_LoadDb()
{
    stPcieInfo.uintDevType   = this->pDb->GetMod<unsigned int>("DEV_MODE");
    stPcieInfo.uintTgtId     = this->pDb->GetMod<unsigned int>("TGT_ID");
    stPcieInfo.uintTgtChnId  = this->pDb->GetMod<unsigned int>("TGT_CHN_ID");
    stPcieInfo.uintTgtDevId  = this->pDb->GetMod<unsigned int>("TGT_DEV_ID");
    stPcieInfo.uintEsRingBuf = this->pDb->GetMod<unsigned int>("ES_RING_BUF");
}

void AmigosSurfacePcie::_UnloadDb()
{
   stPcieInfo.Clear();
}

void AmigosSurfacePcie::GetInfo(PcieInfo &info) const
{
    info = stPcieInfo;
}
void AmigosSurfacePcie::SetInfo(const PcieInfo &info)
{
    stPcieInfo = info;
}
