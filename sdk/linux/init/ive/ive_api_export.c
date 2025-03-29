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
#include <linux/module.h>
#include "mi_ive.h"

#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(MI_IVE_Create_Handle);
EXPORT_SYMBOL(MI_IVE_Create);
EXPORT_SYMBOL(MI_IVE_Destroy);
EXPORT_SYMBOL(MI_IVE_Filter);
EXPORT_SYMBOL(MI_IVE_Csc);
EXPORT_SYMBOL(MI_IVE_FilterAndCsc);
EXPORT_SYMBOL(MI_IVE_Sobel);
EXPORT_SYMBOL(MI_IVE_MagAndAng);
EXPORT_SYMBOL(MI_IVE_Dilate);
EXPORT_SYMBOL(MI_IVE_Erode);
EXPORT_SYMBOL(MI_IVE_Thresh);
EXPORT_SYMBOL(MI_IVE_And);
EXPORT_SYMBOL(MI_IVE_Sub);
EXPORT_SYMBOL(MI_IVE_Or);
EXPORT_SYMBOL(MI_IVE_Integ);
EXPORT_SYMBOL(MI_IVE_Hist);
EXPORT_SYMBOL(MI_IVE_ThreshS16);
EXPORT_SYMBOL(MI_IVE_ThreshU16);
EXPORT_SYMBOL(MI_IVE_16BitTo8Bit);
EXPORT_SYMBOL(MI_IVE_OrdStatFilter);
EXPORT_SYMBOL(MI_IVE_Map);
EXPORT_SYMBOL(MI_IVE_EqualizeHist);
EXPORT_SYMBOL(MI_IVE_Add);
EXPORT_SYMBOL(MI_IVE_Xor);
EXPORT_SYMBOL(MI_IVE_Ncc);
EXPORT_SYMBOL(MI_IVE_Ccl);
EXPORT_SYMBOL(MI_IVE_Gmm);
EXPORT_SYMBOL(MI_IVE_CannyHysEdge);
EXPORT_SYMBOL(MI_IVE_CannyEdge);
EXPORT_SYMBOL(MI_IVE_Lbp);
EXPORT_SYMBOL(MI_IVE_NormGrad);
EXPORT_SYMBOL(MI_IVE_LkOpticalFlow);
EXPORT_SYMBOL(MI_IVE_Sad);
EXPORT_SYMBOL(MI_IVE_Resize);
EXPORT_SYMBOL(MI_IVE_Bernsen);
EXPORT_SYMBOL(MI_IVE_AdpThresh);
EXPORT_SYMBOL(MI_IVE_LineFilterHor);
EXPORT_SYMBOL(MI_IVE_LineFilterVer);
EXPORT_SYMBOL(MI_IVE_NoiseRemoveHor);
EXPORT_SYMBOL(MI_IVE_NoiseRemoveVer);
EXPORT_SYMBOL(MI_IVE_Acc);
EXPORT_SYMBOL(MI_IVE_BAT);
EXPORT_SYMBOL(MI_IVE_Matrix_Transform);
EXPORT_SYMBOL(MI_IVE_Image_Dot);
EXPORT_SYMBOL(MI_IVE_AlphaBlending);
EXPORT_SYMBOL(MI_IVE_Shift_Detector);
EXPORT_SYMBOL(MI_IVE_BGBlur);
#endif
