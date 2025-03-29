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
#ifndef _MI_NIR_IQ_API_ID_H_
#define _MI_NIR_IQ_API_ID_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define NIR_ID_API_COMMON_BASE (0x0100)
#define NIR_ID_API_COMMON_END  (0x01FF)

#define NIR_ID_API_IQ_BASE (0x1000)
#define NIR_ID_API_IQ_END  (0x10FF)

//================================================================
//  ID Defined : NIR COMMON  API
//================================================================
#define NIR_ID_API_COMMON_DEVICE_ID  (NIR_ID_API_COMMON_BASE + 1)
#define NIR_ID_API_COMMON_CHANNEL_ID (NIR_ID_API_COMMON_BASE + 2)

//================================================================
//  ID Defined : NIR IQ  API
//================================================================
#define NIR_ID_API_BLENDING_XNR            (NIR_ID_API_IQ_BASE + 1)
#define NIR_ID_API_BLENDING_SATURATION     (NIR_ID_API_IQ_BASE + 2)
#define NIR_ID_API_CONTRAST                (NIR_ID_API_IQ_BASE + 3)
#define NIR_ID_API_SATURATION              (NIR_ID_API_IQ_BASE + 4)
#define NIR_ID_API_WEIGHT                  (NIR_ID_API_IQ_BASE + 5)
#define NIR_ID_API_BWEIGHT_BY_SAT          (NIR_ID_API_IQ_BASE + 6)
#define NIR_ID_API_DWEIGHT_BY_SAT          (NIR_ID_API_IQ_BASE + 7)
#define NIR_ID_API_BWEIGHT_BY_VISY         (NIR_ID_API_IQ_BASE + 8)
#define NIR_ID_API_DWEIGHT_BY_VISY         (NIR_ID_API_IQ_BASE + 9)
#define NIR_ID_API_BWEIGHT_BY_NIRY         (NIR_ID_API_IQ_BASE + 10)
#define NIR_ID_API_DWEIGHT_BY_NIRY         (NIR_ID_API_IQ_BASE + 11)
#define NIR_ID_API_BWEIGHT_BY_GAP_CONTRAST (NIR_ID_API_IQ_BASE + 12)
#define NIR_ID_API_DWEIGHT_BY_GAP_CONTRAST (NIR_ID_API_IQ_BASE + 13)
#define NIR_ID_API_BWEIGHT_BY_GAP_LUMA     (NIR_ID_API_IQ_BASE + 14)
#define NIR_ID_API_DWEIGHT_BY_GAP_LUMA     (NIR_ID_API_IQ_BASE + 15)
#define NIR_ID_API_BYPASS_MODE             (NIR_ID_API_IQ_BASE + 16)
#define NIR_ID_API_COLOR_RECOVERY          (NIR_ID_API_IQ_BASE + 17)

#ifdef __cplusplus
} // end of extern C
#endif

#endif
