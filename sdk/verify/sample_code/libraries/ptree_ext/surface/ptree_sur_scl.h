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

#ifndef __PTREE_SUR_SCL_H__
#define __PTREE_SUR_SCL_H__
#include "ptree_sur_sys.h"
#include "ptree_packet.h"

typedef struct PTREE_SUR_SCL_Info_s
{
    PTREE_SUR_SYS_Info_t base;
    unsigned int         u32HwPortMode;
    unsigned int         u32Rotation;
} PTREE_SUR_SCL_Info_t;

typedef struct PTREE_SUR_SCL_InInfo_s
{
    PTREE_SUR_SYS_InInfo_t base;
    unsigned short         u16CropX;
    unsigned short         u16CropY;
    unsigned short         u16CropW;
    unsigned short         u16CropH;
} PTREE_SUR_SCL_InInfo_t;

typedef struct PTREE_SUR_SCL_OutInfo_s
{
    PTREE_SUR_SYS_OutInfo_t base;
    unsigned char           bMirror;
    unsigned char           bFlip;
    unsigned short          u16CropX;
    unsigned short          u16CropY;
    unsigned short          u16CropW;
    unsigned short          u16CropH;
    unsigned short          u16Width;
    unsigned short          u16Height;
    unsigned int            u32CompressMode;
    unsigned int            u32VideoFormat;
} PTREE_SUR_SCL_OutInfo_t;

#endif //__PTREE_MOD_SCL_H__
