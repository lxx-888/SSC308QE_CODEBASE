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

#ifndef __PTREE_SUR_HSEG_H__
#define __PTREE_SUR_HSEG_H__

#include "ptree_sur_sys.h"

typedef struct PTREE_SUR_HSEG_Info_s
{
    PTREE_SUR_SYS_Info_t base;
    char                 chHsegMode[32]; //"blur", "replace"
    char                 chMaskOp[32];   // bgblur的预处理。NONE是不做，DILATE 做膨胀，ERODE 做腐蚀
    char                 chIpuPath[64];
    char                 chModelPath[64];
    unsigned int         uiMaskThr;      // 用于Mask源图像二值化的阈值参数，取值范围[0, 255]。
    unsigned int         uiBlurLv;       // 用于配置背景模糊等级，取值范围[0, 255]。
    unsigned int         uiScalingStage; // 用于配置背景模糊缩放挡位，取值范围[1, 15]。
} PTREE_SUR_HSEG_Info_t;

#endif //__PTREE_SUR_HSEG_H__
