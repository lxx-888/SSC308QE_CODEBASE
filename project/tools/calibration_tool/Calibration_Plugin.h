////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2012 SigmaStar Technology Corp.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// SigmaStar Technology Corp. and be kept in strict confidence
// (SigmaStar Confidential Information) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of SigmaStar Confidential
// Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

/*
*    Calibration_Plugin.h
*
*    Created on: Oct 02, 2019
*        Author: Jeffrey Chou
*/

#ifndef __CALIBRATION_PLUGIN_H__
#define __CALIBRATION_PLUGIN_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "Calibration_API.h"
 
    // ********************************** Calibration for Analyzer API - S ***********************************
    DLLIMPORT calibration_handle* DoCalibration_CreateHandle();
    DLLIMPORT s32 DoCalibrationGetDataSize(calibration_handle *handle, u8 item_id, s32 *cali_data_size);
    DLLIMPORT s32 DoCalibrationGetDataBuffer(calibration_handle *handle, u8 item_id, s32 cali_data_size, void *pData);
    DLLIMPORT void DoCalibration_SettingParam(calibration_handle *handle, bool isPluginMode);
    DLLIMPORT void DoCalibration_ReleaseHandle(calibration_handle *handle);

    // ***************************************** Not implemented - S *****************************************
    DLLIMPORT s32 DoCalibrationAE(calibration_handle *handle, void *pData, char *pStrFileName);
    DLLIMPORT s32 DoCalibrationMinGain(calibration_handle *handle, void *pData, char *pStrFileName);
    DLLIMPORT s32 DoCalibrationShutterLinearity(calibration_handle *handle, void *pData, char *pStrFileName);
    DLLIMPORT s32 DoCalibrationGainLinearity(calibration_handle *handle, void *pData, char *pStrFileName);
    // ***************************************** Not implemented - E *****************************************

    // ********************************** Calibration for Analyzer API - E ***********************************

#ifdef __cplusplus
}    //end of extern C
#endif

#endif