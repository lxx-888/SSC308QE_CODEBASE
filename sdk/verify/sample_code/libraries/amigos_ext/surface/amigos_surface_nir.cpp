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
#include <cstring>
#include <string>
#include "amigos_surface_nir.h"

AmigosSurfaceNir::AmigosSurfaceNir(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, true, true) {}

AmigosSurfaceNir::~AmigosSurfaceNir() {}

void AmigosSurfaceNir::_LoadDb()
{
    this->stNirInfo.strMode   = this->pDb->GetMod<std::string>("MODE");
    this->stNirInfo.strIqFile = this->pDb->GetMod<std::string>("IQ_FILE");
}

void AmigosSurfaceNir::_UnloadDb()
{
    stNirInfo.Clear();
}

void AmigosSurfaceNir::GetInfo(NirInfo &info) const
{
    info = this->stNirInfo;
}

void AmigosSurfaceNir::SetInfo(const NirInfo &info)
{
    this->stNirInfo = info;
}
