/*
 * mhal_rgn.h - Sigmastar
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

#ifndef _MHAL_RGN_H_
#define _MHAL_RGN_H_

#include "mhal_cmdq.h"
#include "mhal_rgn_datatype.h"

///
/// @brief Set debug level
/// @param[in] u32DbgLv: debug level mask
///
void MHAL_RGN_SetDebugLevel(MS_U32 u32DbgLv);

///
/// @brief Get Chip capability ops
/// @param[out] pstChipCapOps: Chip capability ops
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GetChipCapOps(MHAL_RGN_ChipCapOps_t *pstChipCapOps);

///
/// @brief Create instance of Rgn Dev
/// @param[out] pCtx: Context of created instance
/// @param[in]  pstDevAttr: Rgn Dev attr
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_Create(const MHAL_RGN_DevAttr_t *pstDevAttr, void **pCtx);

///
/// @brief Destroy instance of Rgn Dev
/// @param[in] pCtx: Context of created instance
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_Destory(const void *pCtx);

///
/// @brief Active Rgn Dev
/// @param[in] pCtx: Context of created instance
/// @note if one Rgn hw (GOP/MFF) is shared by several port of SCL/DISP/JPE/VENC, only one port can use it at one time.
///       Use this function to block the other port to use the dev, which is used.
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_Active(const void *pCtx);

///
/// @brief Deactive Rgn Dev
/// @param[in] pCtx: Context of created instance
/// @note if one Rgn hw is shared by several port of SCL/DISP/JPE/VENC, only one port can use it at one time.
///       Use this function to release the block of the dev.
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_Deactive(const void *pCtx);

///
/// @brief Update CMDQ interface handler to Rgn Dev
/// @param[in] pCtx: Context of created instance
/// @param[in] pstCmdqCfg: Configurationof CMDQ interface handler
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_SetCmdq(const void *pCtx, const MHAL_RGN_CmdqConfig_t *pstCmdqCfg);

///
/// @brief Resume reg
/// @param[in] pCtx: Context of created instance
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_Resume(const void *pCtx);

///
/// @brief Suspend reg
/// @param[in] pCtx: Context of created instance
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_Suspend(const void *pCtx);

///
/// @brief Set Resolution info for Rgn Dev
/// @param[in] pCtx: Context of created instance
/// @param[in] pstResolutionCfg: Resolution info
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_SetResolution(const char *pCtx, const MHAL_RGN_ResolutionConfig_t *pstResolutionCfg);

///
/// @brief update register setting from SW buffer to CMDQ buffer or HW.
/// @param[in] pCtx: Context of created instance
/// @note if MHAL_RGN_SetCmdq is called and successful, this function, MHAL_RGN_Process, will copy register
/// setting to CMDQ.
///        Otherwise, this function will update register setting to HW by RIU moode.
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_Process(const void *pCtx);

///
/// @brief Set Rgn GOP palette
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstPaletteCfg: Configuration of palette
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopSetPalette(const void *pCtx, const MHAL_RGN_GopPaletteConfig_t *pstPaletteCfg);

///
/// @brief Set palette table of Rgn GOP
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstPaletteTblCfg: Palette table configuraiton
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopSetPaletteTbl(const void *pCtx, const MHAL_RGN_GopPaletteTblConfig_t *pstPaletteTblCfg);

///
/// @brief Set Stretch window and scaling up ratio of Rgn GOP
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstStretchWinCfg: Configuration of stretch window
/// @note This function will setup strech windows and scaling up ratio
///       pstStretchWinCfg->tSrcWinCfg is for stretch window, pstStretchWinCfg->tDestWinCfg is for Gwin windows
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopSetBaseWindow(const void *pCtx, const MHAL_RGN_GopStretchWinConfig_t *pstStretchWinCfg);

///
/// @brief Set colorkey configuration of Rgn GOP
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstColorKeyCfg: Configuration of GOP colorkey
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopSetColorKey(const void *pCtx, const MHAL_RGN_GopColorKeyConfig_t *pstColorKeyCfg);

///
/// @brief Set  Invert alpha of Rgn GOP
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstAlphaZeroOpaqueCfg: Configurationof invert alpha value for GOP
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopSetAlphaZeroOpaque(const void *                               pCtx,
                                      const MHAL_RGN_GopAlphaZeroOpaqueConfig_t *pstAlphaZeroOpaqueCfg);

///
/// @brief Set output color fomrat of Rgn GOP
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstFmtCfg: Output color format
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopSetOutputFormat(const void *pCtx, const MHAL_RGN_GopOutputFmtConfig_t *pstFmtCfg);

///
/// @brief Adjust contrast value of Rgn GOP
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstContrastCfg: Contrast value
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopSetContrast(const void *pCtx, const MHAL_RGN_GopContrastConfig_t *pstContrastCfg);

///
/// @brief Adjust brightness value of Rgn GOP
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstBirghtnessCfg: Brightness value
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopSetBrightness(const void *pCtx, const MHAL_RGN_GopBrightnessConfig_t *pstBirghtnessCfg);

///
/// @brief Set trigger mode of Rgn VENC_GOP or JPE_GOP
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstEncFmtCfg: Trigger mode of VENC_GOP or JPE_GOP
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopSetEncMode(const void *pCtx, const MHAL_RGN_GopEncFormatConfig_t *pstEncFmtCfg);

///
/// @brief Set dma threahold value of Rgn GOP
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstMiscCfg: GOP dma threahold configuation value
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopSetDmaThreahold(const void *pCtx, const MHAL_RGN_GopDmaThreaholdConfig_t *pstDmaThrdCfg);

///
/// @brief Set pixel format of source image for Rgn GOP Gwin
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstPixelFmtCfg: Configuration of pixel format for source image
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopGwinSetPixelFormat(const void *pCtx, const MHAL_RGN_GopGwinPixelFmtConfig_t *pstPixelFmtCfg);

///
/// @brief Set palette index of I4/I2 for Rgn GOP Gwin
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstPixelFmtCfg: Configuration of palette index for I4 or I2
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopGwinSetPaletteIndex(const void *pCtx, const MHAL_RGN_GopGwinPaletteIdxConfig_t *pstPltIdxCfg);

///
/// @brief Set Rgn GOP Gwin positon and size
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstWinCfg: GWIN window configuration
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopGwinSetWindow(const void *pCtx, const MHAL_RGN_GopGwinWinConfig_t *pstWinCfg);

///
/// @brief Set GWIN buffer configuration of Rgn GOP
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstWptBufferCfginCfg: GWIN buffer configuration
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopGwinSetBuffer(const void *pCtx, const MHAL_RGN_GopGwinBufferConfig_t *pstBufferCfg);

///
/// @brief Set On/Off of Rgn GOP Gwin
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstOnOffCfg: GWIN on/off configuration
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopGwinSetOnOff(const void *pCtx, const MHAL_RGN_GopGwinOnOffConfig_t *pstOnOffCfg);

///
/// @brief Set alpha blending configuration of  Rgn GOP Gwin
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstAlphaCfg: Configurationof gwin alpha blending
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopGwinSetAlphaType(const void *pCtx, const MHAL_RGN_GopGwinAlphaConfig_t *pstAlphaCfg);

///
/// @brief Set  alpha0/alpha1 value of Rgn GOP Gwin ARGB1555
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pptAlphaCfgtAlphaCfg: Value of alph0 or alpha1 for ARGB1555
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopSetArgb1555AlphaVal(const void *pCtx, const MHAL_RGN_GopGwinArgb1555AlphaConfig_t *pstAlphaCfg);

///
/// @brief Set  GOP blend type
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pptAlphaCfgtAlphaCfg: blend type
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_GopSetBlend(const void *pCtx, const MHAL_RGN_GopBlendConfig_t *pstBlendCfg);
///
/// @brief Set palette of Rgn Cover cover mode
/// @param[in] pCtx: Context of created COVER instance
/// @param[in] pstPaletteCfg: Configuration of palette
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_CoverSetPalette(const void *pCtx, const MHAL_RGN_CoverPaletteConfig_t *pstPaletteCfg);

///
/// @brief Set Window Buffer of Rgn COVER
/// @param[in] pCtx: Context of created COVER instance
/// @param[in] pstBufferCfg: Configuration of window buffer (Hardware will read window position info from this address)
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_CoverSetWindowBuffer(const void *pCtx, const MHAL_RGN_BufferConfig_t *pstBufferCfg);

///
/// @brief Set color mode or color index of Rgn Cover
/// @param[in] pCtx: Context of created COVER instance
/// @param[in] pstColorCfg: Configuration of color
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_CoverSetColor(const void *pCtx, const MHAL_RGN_CoverColorConfig_t *pstColorCfg);

///
/// @brief Set window block size of Rgn COVER
/// @param[in] pCtx: Context of created COVER instance
/// @param[in] pstBlockCfg: Configuration of window block size
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_CoverSetBlock(const void *pCtx, const MHAL_RGN_CoverBlockConfig_t *pstBlockCfg);

///
/// @brief Set config of Rgn COVER Map
/// @param[in] pCtx: Context of created COVER instance
/// @param[in] pstMapCfg: Configuration of COVER MAP
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_CoverSetMap(const void *pCtx, const MHAL_RGN_CoverMapConfig_t *pstMapCfg);

///
/// @brief Set window of Rgn COVER
/// @param[in] pCtx: Context of created COVER instance
/// @param[in] pstWinCfg: Configuration of window
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_CoverSetWindow(const void *pCtx, const MHAL_RGN_CoverWindowConfig_t *pstWinCfg);

///
/// @brief Set window OnOff of Rgn COVER
/// @param[in] pCtx: Context of created COVER instance
/// @param[in] pstOnOffCfg: Configuration of window OnOff
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_CoverSetOnOff(const void *pCtx, const MHAL_RGN_CoverOnOffConfig_t *pstOnOffCfg);

///
/// @brief Set window block size of Rgn FRAME
/// @param[in] pCtx: Context of created FRAME instance
/// @param[in] pstPaletteCfg: Configuration of FRAME palette
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_FrameSetPalette(const void *pCtx, const MHAL_RGN_FramePaletteConfig_t *pstPaletteCfg);

///
/// @brief Set Window Buffer of Rgn FRAME
/// @param[in] pCtx: Context of created FRAME instance
/// @param[in] pstBufferCfg: Configuration of window buffer (Hardware will read window position info from this address)
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_FrameSetWindowBuffer(const void *pCtx, const MHAL_RGN_BufferConfig_t *pstBufferCfg);

///
/// @brief Set window block size of Rgn FRAME
/// @param[in] pCtx: Context of created FRAME instance
/// @param[in] pstBorderCfg: Configuration of window border
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_FrameSetBorder(const void *pCtx, const MHAL_RGN_FrameBorderConfig_t *pstBorderCfg);

///
/// @brief Set window color index of Rgn FRAME
/// @param[in] pCtx: Context of created FRAME instance
/// @param[in] pstColorCfg: Configuration of window color
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_FrameSetColor(const void *pCtx, const MHAL_RGN_FrameColorConfig_t *pstColorCfg);

///
/// @brief Set window block size of Rgn FRAME
/// @param[in] pCtx: Context of created FRAME instance
/// @param[in] pstOnOffCfg: Configuration of window OnOff
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_FrameSetWindow(const void *pCtx, const MHAL_RGN_FrameWindowConfig_t *pstWinCfg);

///
/// @brief Set window block size of Rgn FRAME
/// @param[in] pCtx: Context of created FRAME instance
/// @param[in] pstOnOffCfg: Configuration of window OnOff
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_FrameSetOnOff(const void *pCtx, const MHAL_RGN_FrameOnOffConfig_t *pstOnOffCfg);

///
/// @brief Set window block size of Rgn COLOR INVERT
/// @param[in] pCtx: Context of created COLOR INVERT instance
/// @param[in] pstCalModeCfg: Configuration of cal mode
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_ColorInvertSetCalMode(const void *pCtx, const MHAL_RGN_ColorInvertCalModeConfig_t *pstCalModeCfg);

///
/// @brief Set window block size of Rgn COLOR INVERT
/// @param[in] pCtx: Context of created COLOR INVERT instance
/// @param[in] pstStorageModeCfg: Configuration of storage mode
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_ColorInvertSetStroageMode(const void *                                   pCtx,
                                          const MHAL_RGN_ColorInvertStorageModeConfig_t *pstStorageModeCfg);

///
/// @brief Set window block size of Rgn COLOR INVERT
/// @param[in] pCtx: Context of created COLOR INVERT instance
/// @param[in] pstBlockCfg: Configuration of block H/V
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_ColorInvertSetBlock(const void *pCtx, const MHAL_RGN_ColorInvertBlockSizeConfig_t *pstBlockCfg);

///
/// @brief Set window block size of Rgn COLOR INVERT
/// @param[in] pCtx: Context of created COLOR INVERT instance
/// @param[in] pstThresholdCfg: Configuration of threahold L/H
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_ColorInvertSetThreshold(const void *pCtx, const MHAL_RGN_ColorInvertThresholdConfig_t *pstThresholdCfg);

///
/// @brief Set window block size of Rgn COLOR INVERT
/// @param[in] pCtx: Context of created COLOR INVERT instance
/// @param[in] pstBufferCfg: Configuration of buffer R/W
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_ColorInvertSetBuffer(const void *pCtx, const MHAL_RGN_ColorInvertBufferConfig_t *pstBufferCfg);

///
/// @brief Set window block size of Rgn COLOR INVERT
/// @param[in] pCtx: Context of created COLOR INVERT instance
/// @param[in] pstOnOffCfg: Configuration of window OnOff
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_ColorInvertSetOnOff(const void *pCtx, const MHAL_RGN_ColorInvertOnOffConfig_t *pstOnOffCfg);

///
/// @brief Update IFCD atribute of Rgn GOP
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstAttrCfg: GOP IFCD attrbute configuation
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_IfcdSetAttr(const void *pCtx, const MHAL_RGN_IfcdAttrConfig_t *pstAttrCfg);

///
/// @brief Update IFCD buffer of Rgn GOP
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstBufferCfg: GOP IFCD buffer configuation
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_IfcdSetBuffer(const void *pCtx, const MHAL_RGN_IfcdBufferConfig_t *pstBufferCfg);

///
/// @brief Update IFCD on/off of Rgn GOP
/// @param[in] pCtx: Context of created GOP instance
/// @param[in] pstBufferCfg: GOP IFCD on/off configuation
/// @return retval >=0 success,  retval < 0 failure
///
MS_S32 MHAL_RGN_IfcdSetOnOff(const void *pCtx, const MHAL_RGN_IfcdOnOffConfig_t *pstOnOffCfg);

#endif //_MHAL_RGN_
