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

#include "amigos_surface_pares.h"
#include <string>
#include "amigos_surface_base.h"

AmigosSurfacePares::AmigosSurfacePares(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false)
{
}
AmigosSurfacePares::~AmigosSurfacePares() {}
void AmigosSurfacePares::_LoadDb()
{
    for (auto it = this->mapModInputInfo.begin(); it != this->mapModInputInfo.end(); ++it)
    {
        this->mapParesIn[it->first].strMode = this->pDb->GetIn<std::string>(it->second.curLoopId, "MODE");
        this->mapParesIn[it->first].uiPacketDepth = this->pDb->GetIn<unsigned int>(it->second.curLoopId, "PACKET_DEPTH");
        this->mapParesIn[it->first].uiDropPacketMsg = this->pDb->GetIn<unsigned int>(it->second.curLoopId, "DROP_MSG");
    }
}
void AmigosSurfacePares::_UnloadDb()
{
    this->mapParesIn.clear();
}
