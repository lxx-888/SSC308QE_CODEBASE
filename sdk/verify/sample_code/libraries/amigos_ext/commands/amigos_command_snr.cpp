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
#include "amigos_module_snr.h"

static int SensorRes(vector<string> &in_strs)
{
    AmigosModuleSnr *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleSnr, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    AmigosSurfaceSnr::SnrInfo stInfo;
    pMyClass->GetInfo(stInfo);
    int pad = stInfo.intSensorId;
    int res = ss_cmd_atoi(in_strs[1].c_str());
    MI_SNR_SetRes((MI_SNR_PADID)pad, res);
    ss_print(PRINT_LV_TRACE,"set pad: %d res: %d \n",pad,res);
    return 0;
}

static int SensorFps(vector<string> &in_strs)
{
    AmigosModuleSnr *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleSnr, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    AmigosSurfaceSnr::SnrInfo stInfo;
    pMyClass->GetInfo(stInfo);
    int pad = stInfo.intSensorId;
    int fps = ss_cmd_atoi(in_strs[1].c_str());
    MI_SNR_SetFps((MI_SNR_PADID)pad, fps);
    ss_print(PRINT_LV_TRACE,"set pad: %d fps: %d \n",pad,fps);
    return 0;
}

static int SensorMirrorFlip(vector<string> &in_strs)
{
    AmigosModuleSnr *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleSnr, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    AmigosSurfaceSnr::SnrInfo stInfo;
    pMyClass->GetInfo(stInfo);
    int pad = stInfo.intSensorId;
    bool mirror = ss_cmd_atoi(in_strs[1].c_str());
    bool flip = ss_cmd_atoi(in_strs[2].c_str());
    MI_SNR_SetOrien(pad, mirror,flip);
    ss_print(PRINT_LV_TRACE,"set pad: %d, mirror: %d, flip: %d \n",pad,mirror,flip);
    return 0;
}

static int SensorTriggerDoubleVsync(vector<string> &in_strs)
{
    AmigosModuleSnr *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleSnr, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    AmigosSurfaceSnr::SnrInfo stInfo;
    pMyClass->GetInfo(stInfo);
    int pad = stInfo.intSensorId;
    MI_U32 u32Unused = 0;
    MI_SNR_CustFunction(pad, 0, sizeof(MI_U32), (void *)(&u32Unused), E_MI_SNR_CUSTDATA_TO_DRIVER);
    return 0;
}

MOD_CMDS(AmiCmdSnr)
{
    ADD_CMD("snr_res", SensorRes, 1);
    ADD_CMD_HELP("snr_res", "[res]", "example: snr_res 0");
    ADD_CMD("snr_fps", SensorFps, 1);
    ADD_CMD_HELP("snr_fps", "[fps]", "example: snr_fps 20");
    ADD_CMD("snr_mirror_flip", SensorMirrorFlip, 2);
    ADD_CMD_HELP("snr_mirror_flip", "[mirror] [flip]", "0:close,1:open, example: snr_mirror_flip 1 1");
    ADD_CMD("snr_trigger_dv", SensorTriggerDoubleVsync, 0);
    ADD_CMD_HELP("snr_trigger_dv", "", "Trigger double vsync for imx226");
}
