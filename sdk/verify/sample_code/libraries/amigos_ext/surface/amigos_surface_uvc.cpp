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

#include "amigos_surface_uvc.h"

AmigosSurfaceUvc::AmigosSurfaceUvc(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false) {}

AmigosSurfaceUvc::~AmigosSurfaceUvc() {}

void AmigosSurfaceUvc::_LoadDb()
{
    this->stUvcInfo.ucMaxCnt      = this->pDb->GetMod<int>("MAX_CNT");
    this->stUvcInfo.strMode       = this->pDb->GetMod<std::string>("MODE");
    this->stUvcInfo.strType       = this->pDb->GetMod<std::string>("TYPE");
    this->stUvcInfo.uintMaxPacket = this->pDb->GetMod<int>("MAX_PACKET");
    this->stUvcInfo.ucMult        = this->pDb->GetMod<int>("MULT");
    this->stUvcInfo.ucBurst       = this->pDb->GetMod<int>("BURST");
    this->stUvcInfo.ucCIntf       = this->pDb->GetMod<int>("C_INTF");
    this->stUvcInfo.ucSIntf       = this->pDb->GetMod<int>("S_INTF");
}
void AmigosSurfaceUvc::_UnloadDb()
{
    stUvcInfo.Clear();
}

void AmigosSurfaceUvc::GetInfo(UvcInfo &info) const
{
    info = this->stUvcInfo;
}

void AmigosSurfaceUvc::SetInfo(const UvcInfo &info)
{
    this->stUvcInfo = info;
}
