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

#include "amigos_surface_sync.h"
#include <sstream>
#include <iostream>

AmigosSurfaceSync::AmigosSurfaceSync(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false)
{
}

AmigosSurfaceSync::~AmigosSurfaceSync()
{
}

void AmigosSurfaceSync::_LoadDb()
{
    for (auto itMapSync = this->mapModInputInfo.begin(); itMapSync != this->mapModInputInfo.end(); itMapSync++)
    {
        mapSyncInInfo[itMapSync->first] = pDb->GetIn<unsigned int>(itMapSync->second.curLoopId, "OUTPUT_PORT_ID");
    }
}

void AmigosSurfaceSync::_UnloadDb()
{
    mapSyncInInfo.clear();
}
