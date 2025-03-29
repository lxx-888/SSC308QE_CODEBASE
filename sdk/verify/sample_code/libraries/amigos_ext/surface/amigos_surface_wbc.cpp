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
#include "amigos_surface_wbc.h"

AmigosSurfaceWbc::AmigosSurfaceWbc(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, true) {}

AmigosSurfaceWbc::~AmigosSurfaceWbc() {}

void AmigosSurfaceWbc::_LoadDb()
{
    for (auto it = this->mapModOutputInfo.begin(); it != this->mapModOutputInfo.end(); it++)
    {
        mapWbcInfo[it->first].uintDispDev      = this->pDb->GetOut<unsigned int>(it->second.curLoopId, "DISP_DEV");
        mapWbcInfo[it->first].uintSrcType      = this->pDb->GetOut<unsigned int>(it->second.curLoopId, "TYPE");
        mapWbcInfo[it->first].uintWid          = this->pDb->GetOut<unsigned int>(it->second.curLoopId, "VID_W");
        mapWbcInfo[it->first].uintHei          = this->pDb->GetOut<unsigned int>(it->second.curLoopId, "VID_H");
        mapWbcInfo[it->first].uintCompressMode = this->pDb->GetOut<unsigned int>(it->second.curLoopId, "COMPRESS_MODE");
        mapWbcInfo[it->first].strOutType       = this->pDb->GetOut<std::string>(it->second.curLoopId, "OUT_TYPE");
        mapWbcInfo[it->first].strOutFmt        = this->pDb->GetOut<std::string>(it->second.curLoopId, "OUT_FMT");
    }
}

void AmigosSurfaceWbc::_UnloadDb()
{
    mapWbcInfo.clear();
}

void AmigosSurfaceWbc::GetInfo(std::map<unsigned int, WbcOutInfo> &info) const
{
    info = mapWbcInfo;
}
void AmigosSurfaceWbc::SetInfo(const std::map<unsigned int, WbcOutInfo> &info)
{
    mapWbcInfo = info;
}
