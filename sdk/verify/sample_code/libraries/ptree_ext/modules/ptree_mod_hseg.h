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

#ifndef __PTREE_MOD_HSEG_H__
#define __PTREE_MOD_HSEG_H__

#include "ptree_mod.h"
typedef enum PTREE_MOD_HSEG_ParamTypeE_e
{
    E_PTREE_MOD_HSEG_PARAM_TYPE_MODE,
    E_PTREE_MOD_HSEG_PARAM_TYPE_OP,
    E_PTREE_MOD_HSEG_PARAM_TYPE_THR,
    E_PTREE_MOD_HSEG_PARAM_TYPE_LV,
    E_PTREE_MOD_HSEG_PARAM_TYPE_STAGE
} PTREE_MOD_HSEG_ParamTypeE_t;

void PTREE_MOD_HSEG_SetCtrlParam(PTREE_MOD_Obj_t* mod, const char* pchParam, PTREE_MOD_HSEG_ParamTypeE_t type);

#endif //__PTREE_MOD_HSEG_H__
