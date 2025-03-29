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
#include "ss_cmd_base.h"
#include "amigos_module_nir.h"
#include "mi_nir_iq.h"
#include "mi_nir.h"

static int NirAddIqbin(vector<string> &in_strs)
{
    AmigosModuleNir *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleNir, AmigosModuleBase);
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    const char * filepath = in_strs[1].c_str();
    ss_print(PRINT_LV_TRACE,"%s:%d pIqBinPath:%s\n", __func__, __LINE__, filepath);
    if (MI_SUCCESS != MI_NIR_IQ_ApiCmdLoadBinFile(dev, chn, (char *)filepath, 0))
    {
        AMIGOS_ERR("Nir load iq bin fail, path:%s\n", filepath);
        return -1;
    }
    return 0;
}

static int NirSetMode(vector<string> &in_strs)
{
    AmigosModuleNir *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleNir, AmigosModuleBase);
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    MI_NIR_ChnParam_t stNirChnParam;

    memset(&stNirChnParam, 0x0, sizeof(MI_NIR_ChnParam_t));
    stNirChnParam.eMode = (MI_NIR_Mode_e)ss_cmd_atoi(in_strs[1].c_str());
    if (stNirChnParam.eMode >= E_MI_NIR_MODE_INVALID)
    {
        AMIGOS_ERR("Nir param should be 0:vir+nir; 1:nir, 2,vis\n");
        return -1;
    }
    if (MI_SUCCESS != MI_NIR_SetChnParam(dev, chn, &stNirChnParam))
    {
        AMIGOS_ERR("Nir set mode %s fail", in_strs[1].c_str());
        return -1;
    }
    return 0;
}

MOD_CMDS(AmiCmdNir)
{
    ADD_CMD("nir_add_iqbin", NirAddIqbin, 1);
    ADD_CMD_HELP("nir_add_iqbin", "[file_path]", "example: nir_add_iqbin ./iq_base.bin");
    ADD_CMD("nir_set_mode", NirSetMode, 1);
    ADD_CMD_HELP("nir_set_mode", "[mode]: 0:vir+nir; 1:nir, 2,vis,", "example: nir_set_mode 0");
}
