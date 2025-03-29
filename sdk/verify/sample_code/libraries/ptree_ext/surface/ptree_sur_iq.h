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

#ifndef __PTREE_SUR_IQ_H__
#define __PTREE_SUR_IQ_H__

#include "mi_common_datatype.h"
#include "mi_isp_iq_datatype.h"
#include "ptree_sur_sys.h"

#define MAX_IQ_CALI_CFG E_SS_CALI_ITEM_MAX

typedef struct PTREE_SUR_IQ_CaliCfg_s
{
    unsigned char caliItem;
    char          caliFile[64];
} PTREE_SUR_IQ_CaliCfg_t;

typedef struct PTREE_SUR_IQ_Info_s
{
    PTREE_SUR_SYS_Info_t   base;
    unsigned char          u8OpenIqServer;
    unsigned char          u8InitCus3a;
    unsigned char          u8Cus3aAe;
    unsigned char          u8Cus3aAwb;
    unsigned char          u8Cus3aAf;
    unsigned char          u8Cus3aBlack;
    unsigned short         u16Key;
    char                   cus3aType[16];
    unsigned int           u32CaliCfgCnt;
    PTREE_SUR_IQ_CaliCfg_t arrCaliCfgs[MAX_IQ_CALI_CFG];
} PTREE_SUR_IQ_Info_t;

#endif //__PTREE_SUR_IQ_H__
