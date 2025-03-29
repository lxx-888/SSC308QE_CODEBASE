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

#ifndef _DISP_INIT_H_
#define _DISP_INIT_H_

void DRV_DISP_ModuleDeviceInit(void);
void DRV_DISP_ModuleDeviceDeInit(void);
MI_U32* DRV_DISP_OS_GpioPadModeToPadIndex(MI_U32 u32PadMode);
MI_U8 DRV_DISP_OS_GpioDrvSet(MI_U8 u8GpioIdx, MI_U8 u8Level);
MI_U8 DRV_DISP_OS_GpioDrvGet(MI_U8 u8GpioIdx, MI_U8 *pu8Level);
MI_U8 DRV_DISP_OS_SetLpllClkConfig(MI_U64 u64Dclk, MI_U8 bDsi);

#endif
