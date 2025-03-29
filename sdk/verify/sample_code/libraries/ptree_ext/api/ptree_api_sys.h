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

#ifndef __PTREE_API_SYS__
#define __PTREE_API_SYS__

#include "mi_sys_datatype.h"

#define PTREE_API_SYS_CMD_SHIFT (0x10000)

enum PTREE_API_SYS_CmdTable_e
{
    E_PTREE_API_SYS_CMD_SET_OUTPUT_DEPTH = PTREE_API_SYS_CMD_SHIFT,
    E_PTREE_API_SYS_CMD_GET_OUTPUT_DEPTH,
    E_PTREE_API_SYS_CMD_SET_OUTPUT_FPS,
    E_PTREE_API_SYS_CMD_GET_OUTPUT_FPS,
    E_PTREE_API_SYS_CMD_SET_INPUT_FPS,
    E_PTREE_API_SYS_CMD_GET_INPUT_FPS,
    E_PTREE_API_SYS_CMD_SET_INPUT_BINDTYPE,
    E_PTREE_API_SYS_CMD_GET_INPUT_BINDTYPE,
    E_PTREE_API_SYS_CMD_MAX
};

typedef struct PTREE_API_SYS_OutputFpsParam_s
{
    unsigned char userFrc;
    unsigned int  fps;
} PTREE_API_SYS_OutputFpsParam_t;

typedef struct PTREE_API_SYS_InputFpsParam_s
{
    unsigned int fps;
} PTREE_API_SYS_InputFpsParam_t;

typedef struct PTREE_API_SYS_OutputDepthParam_s
{
    unsigned char bEn;
    unsigned int  user;
    unsigned int  total;
} PTREE_API_SYS_OutputDepthParam_t;

#endif //__PTREE_API_SYS__
