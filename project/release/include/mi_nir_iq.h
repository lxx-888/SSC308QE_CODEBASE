/* SigmaStar trade secret */
/* Copyright (c) [2021~2022] SigmaStar Technology.
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

#ifndef _MI_NIR_IQ_H_
#define _MI_NIR_IQ_H_

#include "mi_nir_iq_datatype.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MI_NIR_API_MAX_PARM_NUMBRE (6)

    typedef struct MI_NIR_IQ_VersionInfoParam_s
    {
        MI_U32 u32Vendor;
        MI_U32 u32Major;
        MI_U32 u32Minor;
    } MI_NIR_IQ_VersionInfoParam_t;

    typedef struct MI_NIR_IQ_VersionInfoType_s
    {
        MI_NIR_IQ_VersionInfoParam_t stParaAPI;
    } MI_NIR_IQ_VersionInfoType_t;

    typedef struct MI_NIR_IQ_ExcuteCmdArray_s
    {
        MI_U32 u32CmdType;
        MI_U16 u16APIID;
        MI_S32 (*callback)(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[]);
    } MI_NIR_IQ_ExcuteCmdArray_t;

    typedef enum __attribute__((aligned(1)))
    {
        E_MI_NIR_CAMERA_CMD_SET = 0,
        E_MI_NIR_CAMERA_CMD_GET,
        E_MI_NIR_CAMERA_CMD_SET_MODE,
        E_MI_NIR_CAMERA_CMD_GET_MODE,
        E_MI_NIR_CAMERA_CMD_GET_PIC,
        E_MI_NIR_CAMERA_CMD_SET_API,
        E_MI_NIR_CAMERA_CMD_GET_API,
        E_MI_NIR_CAMERA_CMD_UPLOAD_FILE,   /* client upload file to server */
        E_MI_NIR_CAMERA_CMD_DOWNLOAD_FILE, /* client download file from server*/
        E_MI_NIR_CAMERA_CMD_SET_NIR_API,
        E_MI_NIR_CAMERA_CMD_GET_NIR_API,
    } MI_NIR_IQ_CameraExtCmdType_e;

    typedef struct MI_NIR_IQ_CmdHeader_s
    {
        MI_NIR_IQ_CameraExtCmdType_e CmdType;
        MI_S32                       CmdLen;
    } MI_NIR_IQ_CmdHeader_t;

    typedef struct MI_NIR_IQ_ApiParam_s
    {
        MI_U32                u32MagicKey;
        MI_NIR_IQ_CmdHeader_t sCmdheader;
        MI_U16                u16APIID;
        MI_U16                u16ParamNum;
    } MI_NIR_IQ_ApiParam_t;

    typedef struct MI_NIR_IQ_ApiBinFile_s
    {
        MI_U32 u32FileID;
        MI_U32 u32NIRVer;
        MI_U32 u32DataLen;
        MI_U32 u32Checksum;
        MI_U32 u32MagicKey;
        MI_U32 u32Reserved[3];
    } MI_NIR_IQ_ApiBinFile_t;

    typedef struct MI_NIR_IQ_ApiCaliFile_s
    {
        MI_U32 u32CaliVer;
        MI_U32 u32DataSize;
        MI_U32 u32Checksum;
        MI_U32 u32Reserved[2];
    } MI_NIR_IQ_ApiCaliFile_t;

    MI_S32 MI_NIR_IQ_SetBlendingXnr(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_BlendingXnrAttr_t *data);
    MI_S32 MI_NIR_IQ_GetBlendingXnr(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_BlendingXnrAttr_t *data);
    MI_S32 MI_NIR_IQ_SetBlendingSaturation(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_BlendingSaturationAttr_t *data);
    MI_S32 MI_NIR_IQ_GetBlendingSaturation(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_BlendingSaturationAttr_t *data);
    MI_S32 MI_NIR_IQ_SetContrast(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_ContrastAttr_t *data);
    MI_S32 MI_NIR_IQ_GetContrast(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_ContrastAttr_t *data);
    MI_S32 MI_NIR_IQ_SetSaturation(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_SaturationAttr_t *data);
    MI_S32 MI_NIR_IQ_GetSaturation(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_SaturationAttr_t *data);
    MI_S32 MI_NIR_IQ_SetWeight(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightAttr_t *data);
    MI_S32 MI_NIR_IQ_GetWeight(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightAttr_t *data);
    MI_S32 MI_NIR_IQ_SetBaseWeightBySaturation(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightBySaturationAttr_t *data);
    MI_S32 MI_NIR_IQ_GetBaseWeightBySaturation(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightBySaturationAttr_t *data);
    MI_S32 MI_NIR_IQ_SetDetailWeightBySaturation(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightBySaturationAttr_t *data);
    MI_S32 MI_NIR_IQ_GetDetailWeightBySaturation(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightBySaturationAttr_t *data);
    MI_S32 MI_NIR_IQ_SetBaseWeightByVisY(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByVisYAttr_t *data);
    MI_S32 MI_NIR_IQ_GetBaseWeightByVisY(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByVisYAttr_t *data);
    MI_S32 MI_NIR_IQ_SetDetailWeightByVisY(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByVisYAttr_t *data);
    MI_S32 MI_NIR_IQ_GetDetailWeightByVisY(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByVisYAttr_t *data);
    MI_S32 MI_NIR_IQ_SetBaseWeightByNirY(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByNirYAttr_t *data);
    MI_S32 MI_NIR_IQ_GetBaseWeightByNirY(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByNirYAttr_t *data);
    MI_S32 MI_NIR_IQ_SetDetailWeightByNirY(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByNirYAttr_t *data);
    MI_S32 MI_NIR_IQ_GetDetailWeightByNirY(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByNirYAttr_t *data);
    MI_S32 MI_NIR_IQ_SetBaseWeightByGapContrast(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByGapContrastAttr_t *data);
    MI_S32 MI_NIR_IQ_GetBaseWeightByGapContrast(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByGapContrastAttr_t *data);
    MI_S32 MI_NIR_IQ_SetDetailWeightByGapContrast(MI_U32 DevId, MI_U32 ChnId,
                                                  MI_NIR_IQ_WeightByGapContrastAttr_t *data);
    MI_S32 MI_NIR_IQ_GetDetailWeightByGapContrast(MI_U32 DevId, MI_U32 ChnId,
                                                  MI_NIR_IQ_WeightByGapContrastAttr_t *data);
    MI_S32 MI_NIR_IQ_SetBaseWeightByGapLuma(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByGapLumaAttr_t *data);
    MI_S32 MI_NIR_IQ_GetBaseWeightByGapLuma(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByGapLumaAttr_t *data);
    MI_S32 MI_NIR_IQ_SetDetailWeightByGapLuma(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByGapLumaAttr_t *data);
    MI_S32 MI_NIR_IQ_GetDetailWeightByGapLuma(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_WeightByGapLumaAttr_t *data);
    MI_S32 MI_NIR_IQ_SetColorRecovery(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_ColorRecoveryAttr_t *data);
    MI_S32 MI_NIR_IQ_GetColorRecovery(MI_U32 DevId, MI_U32 ChnId, MI_NIR_IQ_ColorRecoveryAttr_t *data);

    MI_S32 MI_NIR_IQ_ApiCmdLoadBinFile(MI_U32 DevId, MI_U32 Channel, char *filepath, MI_U32 user_key);

#ifdef __cplusplus
} // end of extern C
#endif

#endif //_MI_NIR_IQ_H_
