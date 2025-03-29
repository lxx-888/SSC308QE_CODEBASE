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
#include "amigos_surface_tick.h"

AmigosSurfaceTick::AmigosSurfaceTick(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false) {}

void AmigosSurfaceTick::_LoadDb()
{
    stTickInfo.longTimerSec  = this->pDb->GetMod<long>("SEC");
    stTickInfo.longTimerNSec = this->pDb->GetMod<long>("NSEC");
    stTickInfo.strFrcType    = this->pDb->GetMod<std::string>("FRC_TYPE");
    stTickInfo.strMode       = this->pDb->GetMod<std::string>("MODE");
}

void AmigosSurfaceTick::_UnloadDb()
{
    stTickInfo.Clear();
}

void AmigosSurfaceTick::GetInfo(TickInfo &info) const
{
    info = stTickInfo;
}

void AmigosSurfaceTick::SetInfo(const TickInfo &info)
{
    stTickInfo = info;
}
