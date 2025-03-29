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

#ifndef __PTREE_API_SNR__
#define __PTREE_API_SNR__

#include "mi_sensor_datatype.h"

enum PTREE_API_SNR_CmdTable_e
{
    E_PTREE_API_SNR_CMD_SET_CONFIG,
    E_PTREE_API_SNR_CMD_GET_CONFIG,
    E_PTREE_API_SNR_CMD_MAX
};

typedef struct PTREE_API_SNR_Config_s
{
    unsigned int     id;  /* Refer to mi_sensor's sensor id. */
    unsigned int     res; /* Refer to mi_sensor's resoltion id. */
    unsigned int     fps;
    unsigned char    isMirror;
    unsigned char    isFlip;
    MI_SNR_HDRType_e hdrType;
} PTREE_API_SNR_Config_t;

#endif //__PTREE_API_SNR__
