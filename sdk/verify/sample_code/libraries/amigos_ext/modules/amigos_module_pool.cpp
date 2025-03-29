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
#include "mi_sys.h"
#include "amigos_module_init.h"
#include "amigos_module_pool.h"

AmigosModulePool::AmigosModulePool(const std::string &strInSection)
    : AmigosSurfacePool(strInSection), AmigosModuleMiBase(this)
{
}
unsigned int AmigosModulePool::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModulePool::GetInputType(unsigned int port) const
{
    return 0;
}
unsigned int AmigosModulePool::GetOutputType(unsigned int port) const
{
    return 0;
}
AmigosModulePool::~AmigosModulePool()
{
}
void AmigosModulePool::_Init()
{
    MI_SYS_GlobalPrivPoolConfig_t stPrivPoolCfg;
    memset(&stPrivPoolCfg, 0, sizeof(MI_SYS_GlobalPrivPoolConfig_t));
    stPrivPoolCfg.bCreate  = TRUE;
    stPrivPoolCfg.eConfigType  = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.eModule = (MI_ModuleId_e)stPoolInfo.uintPoolDevMod;
    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u32Devid = stPoolInfo.uintPoolDevId;
    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth = stPoolInfo.uintPoolVidWid;
    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = stPoolInfo.uintPoolVidHei;
    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16RingLine = stPoolInfo.uintRingLine;
    MI_SYS_ConfigPrivateMMAPool(0, &stPrivPoolCfg);
}
void AmigosModulePool::_Deinit()
{
    MI_SYS_GlobalPrivPoolConfig_t stPrivPoolCfg;
    memset(&stPrivPoolCfg, 0, sizeof(MI_SYS_GlobalPrivPoolConfig_t));
    stPrivPoolCfg.bCreate  = FALSE;
    stPrivPoolCfg.eConfigType  = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.eModule = (MI_ModuleId_e)stPoolInfo.uintPoolDevMod;
    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u32Devid = stPoolInfo.uintPoolDevId;
    MI_SYS_ConfigPrivateMMAPool(0, &stPrivPoolCfg);
}
AMIGOS_MODULE_INIT("POOL", AmigosModulePool);
