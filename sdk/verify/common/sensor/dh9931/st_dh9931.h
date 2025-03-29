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


#ifndef _ST_DH9931_H_
#define _ST_DH9931_H_


#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

#include "mi_common_datatype.h"
#include "mi_sys_datatype.h"
#include "mi_vif_datatype.h"
#include "mi_sensor_datatype.h"
#include "dhc_dh9931_api.h"
typedef enum {
    CUS_SENIF_BUS_PARL = 0, /**< sensor data bus is parallel bus */
    CUS_SENIF_BUS_MIPI = 1,  /**<  sensor data bus is mipi */
    CUS_SENIF_BUS_BT601 = 2,
    CUS_SENIF_BUS_BT656 = 3,
    CUS_SENIF_BUS_BT1120 = 4,
    CUS_SENIF_BUS_MAX
} CUS_SENIF_BUS;

/*! @brief Sensor input raw data precision */
typedef enum {
    CUS_DATAPRECISION_8 = 0,    /**< raw data precision is 8bits */
    CUS_DATAPRECISION_10 = 1,   /**< raw data precision is 10bits */
    CUS_DATAPRECISION_16 = 2,    /**< raw data precision is 16bits */
    CUS_DATAPRECISION_12 = 3,   /**< raw data precision is 12bits */
    CUS_DATAPRECISION_14 = 4,   /**< raw data precision is 14bits */
} CUS_DATAPRECISION;

/*! @brief Sensor bayer raw pixel order */
typedef enum {
    CUS_BAYER_RG = 0,       /**< bayer data start with R channel */
    CUS_BAYER_GR,             /**<  bayer data start with Gr channel */
    CUS_BAYER_BG,             /**<  bayer data start with B channel */
    CUS_BAYER_GB,              /**<  bayer data start with Gb channel */
    CUS_BAYER_MAX              /**<  bayer data start with Gb channel */
} CUS_SEN_BAYER;

typedef enum
{
    CUS_BT656_CLK_EDGE_SINGLE_UP,
    CUS_BT656_CLK_EDGE_SINGLE_DOWN,
    CUS_BT656_CLK_EDGE_DOUBLE,
    CUS_BT656_CLK_EDGE_MAX
}CUS_BT656_ClkEdge_e;

typedef enum
{
    CUS_BT656_WORK_MODE_1MULTIPLEX,
    CUS_BT656_WORK_MODE_2MULTIPLEX,
    CUS_BT656_WORK_MODE_4MULTIPLEX,

    CUS_BT656_WORK_MODE_MAX
} CUS_BT656_MULTIPLEX_e;

typedef struct _cus_camsensor_res{
    MI_SYS_WindowRect_t  stCropRect;
    MI_SYS_WindowSize_t  stOutputSize;  /**< Sensor actual output size */

    MI_U32 u32MaxFps;    /**< Max fps in this resolution */
    MI_U32 u32MinFps;    /**< Min fps in this resolution*/
    char strResDesc[32];	// Need to put “HDR” here if the resolution is for HDR
} __attribute__((packed, aligned(4))) cus_camsensor_res;

/*! @brief Resolution list*/
typedef struct _cus_camsensor_res_list
{
    MI_U32 num_res;                        /**< number of sensor resolution in list */
    MI_U32 ulcur_res;                        /**< current sensor resolution*/
    cus_camsensor_res res[12];      /**< resolution list */
} __attribute__((packed, aligned(4))) cus_camsensor_res_list;

typedef struct ss_cus_sensor_s{
    // sensor data enum list*/
    CUS_SEN_BAYER           bayer_id;    /** < Sensor bayer raw pixel order */
    CUS_SENIF_BUS           sif_bus;     /** < Select sensor interface */
    CUS_DATAPRECISION       data_prec;   /** < Raw data bits */

    cus_camsensor_res_list  video_res_supported; /** < Resolution list */

   //sensor calibration
    MI_U32                     mclk;        /** < Sensor master clock frequency */

    MI_U32 multiplex;
    MI_U32 lib_res;
    MI_U8 I2cId;

    CUS_BT656_ClkEdge_e bt656_clkedge;

    DHC_DH9931_ATTR_S stDh9931Attr;
    DHC_U8 u8ADIndex;              /** < record Index for dh9931_sdk_driver api usage */
    MI_BOOL bUsed;
}ss_cus_sensor_t;

void _dump_9931_reg(int I2cBus, unsigned char u8ChipAddr);
int DHC_DH9931_Init(MI_U8 u8ChipIndex);
int DHC_DH9931_DeInit(MI_U8 u8ChipIndex);
int Cus_SnrEnable(void);
int Cus_SnrDisable(MI_U8 u8ChipIndex);
int Cus_SetVideoRes(MI_U8 u8ChipIndex, MI_U32 res_idx);
int Cus_SetHalfMode(MI_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_BOOL bHalfEn, DHC_DH9931_HALFMODE_E enHalfMode);
int Cus_GetVideoStatus(MI_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_DH9931_VIDEO_STATUS_S *pstVideoStatus);
int Cus_GetCurMultiplex(MI_U8 u8ChipIndex, CUS_BT656_MULTIPLEX_e *eMultiplex);
int Cus_GetCurVideoRes(MI_U8 u8ChipIndex, MI_U32 *cur_idx, cus_camsensor_res *res);
int Cus_GetVideoRes(MI_U8 u8ChipIndex, MI_U32 res_idx, cus_camsensor_res *res);
int Cus_GetVideoResNum(MI_U8 u8ChipIndex, MI_U32 *ulres_num);
int Cus_GetSnrPadInfo(MI_U8 u8ChipIndex, MI_SNR_PADInfo_t  *pstPadInfo);
int Cus_GetSnrPlaneInfo(MI_U8 u8ChipIndex, MI_U32  u32PlaneID, MI_SNR_PlaneInfo_t *pstPlaneInfo);
int Cus_SetDH9931Data(MI_U8 u8ChipIndex, MI_U16 u16RegAddr, MI_U8 u8RegValue);
int Cus_GetDH9931Data(MI_U8 u8ChipIndex, MI_U16 u16RegAddr, MI_U8* pu8RegValue);
int Cus_SetInterface(MI_U8 u8ChipIndex, CUS_SENIF_BUS VoMode);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif

