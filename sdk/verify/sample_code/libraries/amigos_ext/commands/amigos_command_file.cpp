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
#include "amigos_module_file.h"
#include "ss_log.h"

static int SetLeftFrameWrite(vector<string> &in_strs)
{
    AmigosModuleFile *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleFile, AmigosModuleBase);
    unsigned int      port     = ss_cmd_atoi(in_strs[1].c_str());
    unsigned int      left     = ss_cmd_atoi(in_strs[2].c_str());
    pMyClass->SetLeftFrame(port, left);
    return 0;
}

static int WaitFrameWrite(vector<string> &in_strs)
{
    AmigosModuleFile *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleFile, AmigosModuleBase);
    unsigned int      port     = ss_cmd_atoi(in_strs[1].c_str());
    unsigned int      timeOut  = ss_cmd_atoi(in_strs[2].c_str());
    bool              ret      = pMyClass->WaitFrame(port, timeOut);
    if (!ret)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Wait timeout or error!" << std::endl << PRINT_COLOR_END;
        return -1;
    }
    return 0;
}

MOD_CMDS(AmiCmdFile)
{
    ADD_CMD("set_left_frame_write", SetLeftFrameWrite, 2);
    ADD_CMD_HELP("set_left_frame_write", "[input port] [left frame cnt]", "Set the left frames for file writing.",
                 "[input port]: File module's input port id.",
                 "[lef frame cnt]: the count for the left frame while will be writen into file.");
    ADD_CMD("wait_frame_write", WaitFrameWrite, 2);
    ADD_CMD_HELP("wait_frame_write", "[input port] [ms]",
                 "Wait for the frame counting down to the zero or return timeout or error.",
                 "[input port]: File module's input port id.", "[ms]: Wait timeout in microsecond.");
}
