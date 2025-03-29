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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <string>

#include "amigos_surface_base.h"
#include "amigos_module_init.h"
#include "amigos_module_nir.h"
#include "mi_nir.h"
#include "mi_nir_iq.h"
#include "mi_sys.h"
#include "mi_sys_datatype.h"
#include "ss_enum_cast.hpp"

SS_ENUM_CAST_STR(MI_NIR_Mode_e, {
                                    {E_MI_NIR_MODE_NORMAL, "normal"},
                                    {E_MI_NIR_MODE_BYPASS_VIS, "vis"},
                                    {E_MI_NIR_MODE_BYPASS_NIR, "nir"},
                                    {E_MI_NIR_MODE_INVALID, "invalid"},
                                });
std::map<unsigned int, unsigned int> AmigosModuleNir::mapNirCreateDev;

AmigosModuleNir::AmigosModuleNir(const std::string &strInSection)
    : AmigosSurfaceNir(strInSection), AmigosModuleMiBase(this)
{
}
AmigosModuleNir::~AmigosModuleNir() {}
unsigned int AmigosModuleNir::GetModId() const
{
    return E_MI_MODULE_ID_NIR;
}
unsigned int AmigosModuleNir::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleNir::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}

void AmigosModuleNir::_Init()
{
    MI_NIR_DevAttr_t                               stNirDevAttr;
    MI_NIR_ChannelAttr_t                           stNirChnAttr;
    MI_NIR_ChnParam_t                              stNirChnParam;
    MI_SYS_ChnPort_t                               stChnPort;
    std::map<unsigned int, unsigned int>::iterator itMapNirCreateDev;

    memset(&stNirChnParam, 0x0, sizeof(MI_NIR_ChnParam_t));
    memset(&stNirChnAttr, 0x0, sizeof(MI_NIR_ChannelAttr_t));
    memset(&stNirDevAttr, 0x0, sizeof(MI_NIR_DevAttr_t));

    itMapNirCreateDev = mapNirCreateDev.find(stModInfo.devId);
    if (itMapNirCreateDev == mapNirCreateDev.end())
    {
        MI_NIR_CreateDevice(stModInfo.devId, &stNirDevAttr);
        mapNirCreateDev[stModInfo.devId] = 0;
    }
    mapNirCreateDev[stModInfo.devId]++;
    stNirChnAttr.eMode = ss_enum_cast<MI_NIR_Mode_e>::from_str(this->stNirInfo.strMode);
    MI_NIR_CreateChannel(stModInfo.devId, stModInfo.chnId, &stNirChnAttr);

    stNirChnParam.eMode = ss_enum_cast<MI_NIR_Mode_e>::from_str(this->stNirInfo.strMode);
    MI_NIR_SetChnParam(stModInfo.devId, stModInfo.chnId, &stNirChnParam);
    MI_NIR_StartChannel(stModInfo.devId, stModInfo.chnId);
    stChnPort.eModId    = E_MI_MODULE_ID_NIR;
    stChnPort.u32DevId  = stModInfo.devId;
    stChnPort.u32ChnId  = stModInfo.chnId;
    stChnPort.u32PortId = 0;
    MI_SYS_SetChnOutputPortDepth(0, &stChnPort, 2, 4);
    if(!this->stNirInfo.strIqFile.empty())
    {
        const char * filepath = this->stNirInfo.strIqFile.c_str();
        if (MI_SUCCESS != MI_NIR_IQ_ApiCmdLoadBinFile(stModInfo.devId, stModInfo.chnId, (char *)filepath, 0))
        {
            AMIGOS_ERR("Nir load iq bin fail, path:%s\n", filepath);
        }
    }

    AMIGOS_INFO("LDC ENABLE dev%d chn %d \n", stModInfo.devId, stModInfo.chnId);
}
void AmigosModuleNir::_Deinit()
{
    MI_NIR_StopChannel(stModInfo.devId, stModInfo.chnId);
    MI_NIR_DestroyChannel(stModInfo.devId, stModInfo.chnId);
    auto itMapNirCreateDev = mapNirCreateDev.find(stModInfo.devId);
    if (itMapNirCreateDev != mapNirCreateDev.end())
    {
        itMapNirCreateDev->second--;
        if (!itMapNirCreateDev->second)
        {
            MI_NIR_DestroyDevice(stModInfo.devId);
            mapNirCreateDev.erase(itMapNirCreateDev);
        }
    }
}
AMIGOS_MODULE_INIT("NIR", AmigosModuleNir);
