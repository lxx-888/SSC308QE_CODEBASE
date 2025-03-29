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

#include "amigos_surface_base.h"
#include "mi_common_datatype.h"
#include "mi_scl_datatype.h"
#include "mi_scl.h"

#include "amigos_module_mi_base.h"
#include "amigos_module_scl_base.h"

std::map<unsigned int, unsigned int> AmigosModuleSclBase::mapSclCreateDev;

AmigosModuleSclBase::AmigosModuleSclBase(AmigosSurfaceBase *pSurface) :
    AmigosModuleMiBase(pSurface)
{}
AmigosModuleSclBase::~AmigosModuleSclBase() {}

void AmigosModuleSclBase::_CreateDeviceResource(unsigned int devId)
{
    auto it = mapSclCreateDev.find(devId);
    if (it != mapSclCreateDev.end())
    {
        ++it->second;
        return;
    }
    mapSclCreateDev[devId] = 1;
}
void AmigosModuleSclBase::_DestroyDeviceResource(unsigned int devId)
{
    auto it = mapSclCreateDev.find(devId);
    if (it == mapSclCreateDev.end())
    {
        return;
    }
    --it->second;
    if (0 == it->second)
    {
        mapSclCreateDev.erase(it);
    }
}
bool AmigosModuleSclBase::_CreateDevice(unsigned int devId, MI_SCL_DevAttr_t &stDevAttr)
{
    auto it = mapSclCreateDev.find(devId);
    if (it != mapSclCreateDev.end())
    {
        ++it->second;
        return true;
    }
    MI_S32 s32Ret = MI_SCL_CreateDevice((MI_SCL_DEV)devId, &stDevAttr);
    if (s32Ret != MI_SUCCESS)
    {
        return false;
    }
    mapSclCreateDev[devId] = 1;
    return true;
}
void AmigosModuleSclBase::_DestroyDevice(unsigned int devId)
{
    auto it = mapSclCreateDev.find(devId);
    if (it == mapSclCreateDev.end())
    {
        return;
    }
    --it->second;
    if (0 == it->second)
    {
        MI_SCL_DestroyDevice((MI_SCL_DEV)devId);
        mapSclCreateDev.erase(it);
    }
}

