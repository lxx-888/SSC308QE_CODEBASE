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
#include "amigos_surface_pool.h"

AmigosSurfacePool::~AmigosSurfacePool() {}

AmigosSurfacePool::AmigosSurfacePool(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false) {}

void AmigosSurfacePool::_LoadDb()
{
    stPoolInfo.uintPoolDevMod = pDb->GetMod<unsigned int>("DEV_MOD");
    stPoolInfo.uintPoolDevId = pDb->GetMod<unsigned int>("DEV_ID");
    stPoolInfo.uintPoolVidWid = pDb->GetMod<unsigned int>("VID_W");
    stPoolInfo.uintPoolVidHei = pDb->GetMod<unsigned int>("VID_H");
    stPoolInfo.uintRingLine = pDb->GetMod<unsigned int>("RING_LINE");
}

void AmigosSurfacePool::_UnloadDb()
{
    stPoolInfo.Clear();
}

void AmigosSurfacePool::GetInfo(PoolInfo &info) const 
{
    info = stPoolInfo;
}
void AmigosSurfacePool::UpdateInfo(const PoolInfo &info)
{
    stPoolInfo = info;
}
