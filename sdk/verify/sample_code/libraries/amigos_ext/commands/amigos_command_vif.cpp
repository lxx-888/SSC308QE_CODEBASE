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
#include "amigos_module_vif.h"
#include "mi_vif.h"

static int VifCompress(vector<string> &in_strs)
{
    AmigosModuleVif *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVif, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    MI_SYS_CompressMode_e mode = ss_enum_cast<MI_SYS_CompressMode_e>::from_str(in_strs[1].c_str());
    MI_VIF_OutputPortAttr_t stVifPortInfo;
    MI_VIF_GetOutputPortAttr((MI_VIF_DEV)dev, 0, &stVifPortInfo);
    stVifPortInfo.eCompressMode = (MI_SYS_CompressMode_e)mode;
    MI_VIF_SetOutputPortAttr((MI_VIF_DEV)dev, 0, &stVifPortInfo);

    ss_print(PRINT_LV_TRACE,"set dev: %d, compress: %d \n",dev,mode);
    return 0;
}

MOD_CMDS(AmiCmdVif)
{
    ADD_CMD("vif_compress", VifCompress, 1);
    ADD_CMD_HELP("vif_compress", "[mode]", "[mode]:na,8bit,6bit...","example :vif_compress na");
}
