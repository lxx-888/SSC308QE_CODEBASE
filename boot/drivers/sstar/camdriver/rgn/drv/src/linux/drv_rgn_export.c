/*
 * drv_rgn_export.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#include "drv_rgn_os_header.h"
#include "mhal_rgn.h"
#if ((defined ALKAID_MHAL_UT) && (ALKAID_MHAL_UT == 0))
#else
#endif
EXPORT_SYMBOL(MHAL_RGN_SetDebugLevel);
EXPORT_SYMBOL(MHAL_RGN_GetChipCapOps);
EXPORT_SYMBOL(MHAL_RGN_Create);
EXPORT_SYMBOL(MHAL_RGN_Destory);
EXPORT_SYMBOL(MHAL_RGN_Active);
EXPORT_SYMBOL(MHAL_RGN_Deactive);
EXPORT_SYMBOL(MHAL_RGN_SetCmdq);
EXPORT_SYMBOL(MHAL_RGN_Resume);
EXPORT_SYMBOL(MHAL_RGN_Suspend);
EXPORT_SYMBOL(MHAL_RGN_SetResolution);
EXPORT_SYMBOL(MHAL_RGN_Process);
EXPORT_SYMBOL(MHAL_RGN_GopSetPalette);
EXPORT_SYMBOL(MHAL_RGN_GopSetPaletteTbl);
EXPORT_SYMBOL(MHAL_RGN_GopSetBaseWindow);
EXPORT_SYMBOL(MHAL_RGN_GopSetColorKey);
EXPORT_SYMBOL(MHAL_RGN_GopSetAlphaZeroOpaque);
EXPORT_SYMBOL(MHAL_RGN_GopSetOutputFormat);
EXPORT_SYMBOL(MHAL_RGN_GopSetContrast);
EXPORT_SYMBOL(MHAL_RGN_GopSetBrightness);
EXPORT_SYMBOL(MHAL_RGN_GopSetEncMode);
EXPORT_SYMBOL(MHAL_RGN_GopSetDmaThreahold);
EXPORT_SYMBOL(MHAL_RGN_GopGwinSetPixelFormat);
EXPORT_SYMBOL(MHAL_RGN_GopGwinSetPaletteIndex);
EXPORT_SYMBOL(MHAL_RGN_GopGwinSetWindow);
EXPORT_SYMBOL(MHAL_RGN_GopGwinSetBuffer);
EXPORT_SYMBOL(MHAL_RGN_GopGwinSetOnOff);
EXPORT_SYMBOL(MHAL_RGN_GopGwinSetAlphaType);
EXPORT_SYMBOL(MHAL_RGN_GopSetArgb1555AlphaVal);
EXPORT_SYMBOL(MHAL_RGN_GopSetBlend);
EXPORT_SYMBOL(MHAL_RGN_CoverSetPalette);
EXPORT_SYMBOL(MHAL_RGN_CoverSetWindowBuffer);
EXPORT_SYMBOL(MHAL_RGN_CoverSetColor);
EXPORT_SYMBOL(MHAL_RGN_CoverSetBlock);
EXPORT_SYMBOL(MHAL_RGN_CoverSetMap);
EXPORT_SYMBOL(MHAL_RGN_CoverSetWindow);
EXPORT_SYMBOL(MHAL_RGN_CoverSetOnOff);
EXPORT_SYMBOL(MHAL_RGN_FrameSetPalette);
EXPORT_SYMBOL(MHAL_RGN_FrameSetWindowBuffer);
EXPORT_SYMBOL(MHAL_RGN_FrameSetBorder);
EXPORT_SYMBOL(MHAL_RGN_FrameSetColor);
EXPORT_SYMBOL(MHAL_RGN_FrameSetWindow);
EXPORT_SYMBOL(MHAL_RGN_FrameSetOnOff);
EXPORT_SYMBOL(MHAL_RGN_ColorInvertSetCalMode);
EXPORT_SYMBOL(MHAL_RGN_ColorInvertSetStroageMode);
EXPORT_SYMBOL(MHAL_RGN_ColorInvertSetBlock);
EXPORT_SYMBOL(MHAL_RGN_ColorInvertSetThreshold);
EXPORT_SYMBOL(MHAL_RGN_ColorInvertSetBuffer);
EXPORT_SYMBOL(MHAL_RGN_ColorInvertSetOnOff);
EXPORT_SYMBOL(MHAL_RGN_IfcdSetAttr);
EXPORT_SYMBOL(MHAL_RGN_IfcdSetBuffer);
EXPORT_SYMBOL(MHAL_RGN_IfcdSetOnOff);
