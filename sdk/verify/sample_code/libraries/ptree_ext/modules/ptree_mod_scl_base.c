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

#include "mi_scl_datatype.h"
#include "mi_scl.h"
#include "ptree_log.h"
#include "ptree_mod_scl_base.h"

static unsigned int g_sclCreateDev[PTREE_MOD_SCL_DEV_NUM] = {0};

int PTREE_MOD_SCL_BASE_CreateDevice(unsigned int devId, MI_SCL_DevAttr_t *sclDevAttr)
{
    if (!g_sclCreateDev[devId])
    {
        if (MI_SUCCESS != MI_SCL_CreateDevice(devId, sclDevAttr))
        {
            PTREE_ERR("MI_SCL_CreateDevice failed");
            return SSOS_DEF_FAIL;
        }
    }
    ++g_sclCreateDev[devId];

    return SSOS_DEF_OK;
}

int PTREE_MOD_SCL_BASE_DestroyDevice(unsigned int devId)
{
    --g_sclCreateDev[devId];
    if (!g_sclCreateDev[devId])
    {
        MI_SCL_DestroyDevice(devId);
    }

    return SSOS_DEF_OK;
}
