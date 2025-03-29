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
#include "mi_common.h"
#include "mi_disp.h"
#include "amigos_module_init.h"
#include "amigos_module_wbc.h"
#include "mi_sys_datatype.h"
#include "ss_enum_cast.hpp"

AmigosModuleWbc::AmigosModuleWbc(const std::string &strInSection)
    : AmigosSurfaceWbc(strInSection), AmigosModuleMiBase(this)
{
}
AmigosModuleWbc::~AmigosModuleWbc()
{
}
unsigned int AmigosModuleWbc::GetModId() const
{
    return E_MI_MODULE_ID_WBC;
}
unsigned int AmigosModuleWbc::GetInputType(unsigned int port) const
{
    return 0;
}
unsigned int AmigosModuleWbc::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
void AmigosModuleWbc::_Init()
{
    for (auto it = mapWbcInfo.begin(); it != mapWbcInfo.end(); it++)
    {
        MI_DISP_WBC_Source_t  stWbcSource;
        MI_DISP_WBC_Attr_t stWbcAttr;
        memset(&stWbcSource, 0, sizeof(MI_DISP_WBC_Source_t));
        memset(&stWbcAttr, 0, sizeof(MI_DISP_WBC_Attr_t));
        stWbcSource.eSourceType = (MI_DISP_WBC_SourceType_e)it->second.uintSrcType;
        stWbcSource.u32SourceId = (MI_U32)it->second.uintDispDev;
        MI_DISP_SetWBCSource(stModInfo.devId, &stWbcSource);
        stWbcAttr.ePixFormat = ss_enum_cast<MI_SYS_PixelFormat_e>::from_str(it->second.strOutFmt);
        stWbcAttr.stTargetSize.u32Width = it->second.uintWid;
        stWbcAttr.stTargetSize.u32Height = it->second.uintHei;
        stWbcAttr.eCompressMode = (MI_SYS_CompressMode_e)it->second.uintCompressMode;
        MI_DISP_SetWBCAttr(stModInfo.devId, &stWbcAttr);
        MI_DISP_EnableWBC(stModInfo.devId);
    }
}
void AmigosModuleWbc::_Deinit()
{
    for (auto it = mapWbcInfo.begin(); it != mapWbcInfo.end(); it++)
    {
        MI_DISP_DisableWBC(stModInfo.devId);
    }
}
AMIGOS_MODULE_INIT("WBC", AmigosModuleWbc);
