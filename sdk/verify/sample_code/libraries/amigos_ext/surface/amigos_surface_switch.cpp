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

#include "amigos_surface_switch.h"
#include "amigos_surface_base.h"
AmigosSurfaceSwitch::AmigosSurfaceSwitch(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false)
{
}
AmigosSurfaceSwitch::~AmigosSurfaceSwitch() {}

void AmigosSurfaceSwitch::_LoadDb()
{
    for (auto it = this->mapModInputInfo.begin(); it != this->mapModInputInfo.end(); ++it)
    {
        this->mapSwitchInInfo[it->first] = this->pDb->GetIn<int>(it->second.curLoopId, "DST_PORT");
    }
}
void AmigosSurfaceSwitch::_UnloadDb()
{
    this->mapSwitchInInfo.clear();
}

