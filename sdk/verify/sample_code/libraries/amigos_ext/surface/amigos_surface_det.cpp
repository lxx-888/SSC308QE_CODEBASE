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

#include "amigos_surface_det.h"
#include "amigos_surface_base.h"

AmigosSurfaceDet::AmigosSurfaceDet(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false)
{
}

AmigosSurfaceDet::~AmigosSurfaceDet()
{
}

void AmigosSurfaceDet::_LoadDb()
{
    this->stDetInfo.strFwPath    = this->pDb->GetMod<std::string>("FW_PATH");
    this->stDetInfo.strModelPath = this->pDb->GetMod<std::string>("MODEL_PATH");
    this->stDetInfo.strModelType = this->pDb->GetMod<std::string>("MODEL_TYPE");
    this->stDetInfo.intThresHold = this->pDb->GetMod<unsigned int>("THRESHOLD");
}

void AmigosSurfaceDet::_UnloadDb()
{
    this->stDetInfo.Clear();
}
