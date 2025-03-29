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
#include "mi_sys.h"
#include "ss_cmd_base.h"
#include "amigos_module_mi_base.h"

static int SetDepth(vector<string> &in_strs)
{
    MI_SYS_ChnPort_t stChnPort;
    MI_U8 u8UserDepth  = 0;
    MI_U8 u8TotalDepth = 0;
    MI_S32 s32Ret      = 0;

    AmigosModuleMiBase *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleMiBase, AmigosModuleBase);
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetSurface()->GetModInfo();
    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId = (MI_ModuleId_e)pMyClass->GetModId();
    stChnPort.u32DevId = (MI_U32)stInfo.devId;
    stChnPort.u32ChnId = (MI_U32)stInfo.chnId;
    stChnPort.u32PortId = (MI_U32)ss_cmd_atoi(in_strs[1].c_str());;

    u8UserDepth = (MI_U8)ss_cmd_atoi(in_strs[2].c_str());
    u8TotalDepth =(MI_U8)ss_cmd_atoi(in_strs[3].c_str());

    sslog.lv_set(PRINT_LV_TRACE) << "mod " << stChnPort.eModId << ", dev " << stChnPort.u32DevId << ", chn "
                                 << stChnPort.u32ChnId << ", port " << stChnPort.u32PortId
                                 << ",depth(" << u8UserDepth << ", " << u8TotalDepth << ")\n" << COLOR_ENDL;
    EXPECT_OK(MI_SYS_SetChnOutputPortDepth(0, &stChnPort, u8UserDepth, u8TotalDepth), s32Ret, MI_SUCCESS);

    return s32Ret;
}

MOD_CMDS(AmiCmdMiBase)
{
    ADD_CMD("sys_set_depth", SetDepth, 3);
    ADD_CMD_HELP("sys_set_depth", "[output port] [user depth] [total depth]",
                 "Set module output depth.");
}
