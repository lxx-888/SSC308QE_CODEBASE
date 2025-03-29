/*
 * mhal_pnl.h- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
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

/**
 * \defgroup HAL_PNL_group  HAL_PNL driver
 * @{
 */
#ifndef __MHAL_PNL_H__
#define __MHAL_PNL_H__

#include "mhal_pnl_datatype.h"
//=============================================================================
// API
//=============================================================================

#ifndef __MHAL_PNL_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif

/// @brief Create instance of panel with ouptut protocal
/// @param[out] pCtx: Context of created instance
/// @param[in]  enLinkType: panel ouptut protocal
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlCreateInstance(void **pCtx, MhalPnlLinkType_e enLinkType);

/// @brief Create instance of panel width panel ID and ouptut protocal
/// @param[out] pCtx: Context of created instance
/// @param[in]  enLinkType: panel ouptut protocall
/// @param[in]  u16Id: panel ID
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlCreateInstanceEx(void **pCtx, MhalPnlLinkType_e enLinkType, u16 u16Id);

INTERFACE bool MhalPnlDestroyInstance(void *pCtx);

/// @brief Set panel parameter configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[in] pParamCfg: panel configuration
/// @note This function is not supported in Infinity7 and latter chip
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlSetParamConfig(void *pCtx, MhalPnlParamConfig_t *pParamCfg);

/// @brief Get panel parameter configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[out] pParamCfg: panel configuration]
/// @note This function is not supported in Infinity7 and latter chip
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlGetParamConfig(void *pCtx, MhalPnlParamConfig_t *pParamCfg);

/// @brief Set MipiDsi parameter configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[in] pMipiDsiCfg: MipiDsi configuration
/// @note This function is not supported in Infinity7 and latter chip
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlSetMipiDsiConfig(void *pCtx, MhalPnlMipiDsiConfig_t *pMipiDsiCfg);

/// @brief Get MipiDsi parameter configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[out] pMipiDsiCfg: MipiDsi configuration
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlGetMipiDsiConfig(void *pCtx, MhalPnlMipiDsiConfig_t *pMipiDsiCfg);

/// @brief Set SSC configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[in] pSscCfg: SSC configuration
/// @note This function is not supported in Infinity7 and latter chip
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlSetSscConfig(void *pCtx, MhalPnlSscConfig_t *pSscCfg);

/// @brief Set timing configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[in] pTimingCfg: timing configuration
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlSetTimingConfig(void *pCtx, MhalPnlTimingConfig_t *pTimingCfg);

/// @brief Get timing configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[out] pTimingCfg: timing configuration
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlGetTimingConfig(void *pCtx, MhalPnlTimingConfig_t *pTimingCfg);

/// @brief Set power configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[in] pPowerCfg: power configuration
/// @note This function is not supported in Infinity7 and latter chip
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlSetPowerConfig(void *pCtx, MhalPnlPowerConfig_t *pPowerCfg);

/// @brief Get power configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[out] pPowerCfg: power configuration
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlGetPowerConfig(void *pCtx, MhalPnlPowerConfig_t *pPowerCfg);

/// @brief Set backlight onoff configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[in] pBackLightOnOffCfg: backlight configuration
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlSetBackLightOnOffConfig(void *pCtx, MhalPnlBackLightOnOffConfig_t *pBackLightCfg);

/// @brief Get backlight on/off configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[out] pBackLightOnOffCfg: backlight configuration
/// @note This function is not supported in Infinity7 and latter chip
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlGetBackLightOnOffConfig(void *pCtx, MhalPnlBackLightOnOffConfig_t *pBackLightCfg);

/// @brief Set backlight level configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[in] pBackLightLevelCfg: backlight level configuration
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlSetBackLightLevelConfig(void *pCtx, MhalPnlBackLightLevelConfig_t *pBackLightCfg);

/// @brief Get backlight level configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[out] pBackLightLevelCfg: backlight level configuration
/// @note This function is not supported in Infinity7 and latter chip
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlGetBackLightLevelConfig(void *pCtx, MhalPnlBackLightLevelConfig_t *pBackLightCfg);

/// @brief Set driving current configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[in] pDrvCurrentCfg: driving current configuration
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlSetDrvCurrentConfig(void *pCtx, MhalPnlDrvCurrentConfig_t *pDrvCurrentCfg);

/// @brief Set test pattern  configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[in] pTestPatternCfg: test pattern configuration
/// @note This function is not supported in Infinity7 and latter chip
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlSetTestPatternConfig(void *pCtx, MhalPnlTestPatternConfig_t *pTestPatternCfg);

/// @brief Set debug level
/// @param[in] pDbgLevel: debug level
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlSetDebugLevel(void *pDbgLevel);

/// @brief Set unified panel parameter configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[in] pUdParamCfg: unified parameter configuration
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlSetUnifiedParamConfig(void *pCtx, MhalPnlUnifiedParamConfig_t *pUdParamCfg);

/// @brief Get unified panel parameter configuration
/// @param[in] pCtx: Context of created panel instance
/// @param[out] pUdParamCfg: unified parameter configuration
/// @return retval = 1 success,  retval = 0 failure
INTERFACE bool MhalPnlGetUnifiedParamConfig(void *pCtx, MhalPnlUnifiedParamConfig_t *pUdParamCfg);

#undef INTERFACE

#endif    //
/** @} */ // end of HAL_PNL_group
