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

/*
 * The full name of 'ptree' is 'Pipeline tree', which use the idea of 'Amigos'
 * for reference and auther is 'pedro.peng' from Sigmastar.
 */

#ifndef __PTREE_API_VIF_H__
#define __PTREE_API_VIF_H__

enum PTREE_API_VIF_CmdTable_e
{
    E_PTREE_API_VIF_CMD_SET_SHUTTER_GAIN,
    E_PTREE_API_VIF_CMD_MAX
};

typedef struct PTREE_API_VIF_SetShutterGain_s
{
    unsigned int shutterTimeUs;
    unsigned int aeGain;
} PTREE_API_VIF_SetShutterGain_t;

#endif //__PTREE_API_VIF_H__
