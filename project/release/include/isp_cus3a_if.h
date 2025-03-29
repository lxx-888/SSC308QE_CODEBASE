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

#ifndef _ISP_CUS3A_IF_H_
#define _ISP_CUS3A_IF_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define CUS3A_VER_STR "CUS3A_V1.5"
#define CUS3A_VER_MAJOR 1
#define CUS3A_VER_MINOR 1

#include "cam_os_wrapper.h"
#include "mi_isp_hw_dep_datatype.h"

#if 0
typedef unsigned char   u8;
//typedef signed char   s8;
typedef unsigned short  u16;
//typedef signed short  s16;
typedef unsigned int   u32;
//typedef signed int   s32;
typedef unsigned long long u64;
//typedef signed long long s64;
#endif

#define MV_WIN_NUM                 25
#define CAM20_AUTO_NUM             16
#define CAM20_SDC_CTRL_ITEM_NUMBER 2
#define AWB_LV_CT_TBL_NUM          18
#define AWB_CT_TBL_NUM             10

#define MS_GAMMA_TBL               256
#define MS_ALSC_TBL_W              61
#define MS_ALSC_TBL_H              69
#define MS_ALSC_TBL                4209
#define MS_SDC_TBL                 1024
#define MS_FPN_TBL                 4000 //infinity5
#define MS_YUVGAMA_Y_TBL           256
#define MS_YUVGAMA_VU_TBL          128
#define MS_WDR_LOC_TBL             88

#ifdef ENABLE_ISP_PDAF
#define MS_CAM_PDAF_MAX_UNIT_NUM (1)
#endif

#define MS_CAM_AF_MAX_WIN_NUM 16 //

#ifdef _ENABLE_ISP_CH_NUM_MAX_4_
#define MAX_CUST_3A_CHINFO_NUM (4)
#else
#define MAX_CUST_3A_CHINFO_NUM (16)
#endif

#ifdef ENABLE_DUAL_CUS3A_ISP
#define MAX_CUST_3A_DEV_NUM (2)
#else
#define MAX_CUST_3A_DEV_NUM (1)
#endif

/*------AE/AWB interface--------*/
/*! @brief API error code*/
typedef enum CUS_3A_ERR_CODE
{
    CUS_3A_SUCCESS = 0, /**< operation successful */
    CUS_3A_ERROR = -1, /**< unspecified failure */
}CUS3A_ERR_CODE;

#define _3A_ROW             (128)   /**< number of 3A statistic blocks in a row */
#define _3A_COL             (90)    /**< number of 3A statistic blocks in a column */
#define _3A_HIST_BIN0    (40)    /**< histogram type0 resolution*/
#define _3A_HIST_BINX    (128)   /**< histogram type1 resolution*/
#define _3A_IR_HIST_BIN  (256)   /**< histogram type2 resolution*/

typedef enum
{
    E_ISP_CH_0 = 0,
    E_ISP_CH_1,
    E_ISP_CH_2,
    E_ISP_CH_3,
    E_ISP_CH_4,
    E_ISP_CH_5,
    E_ISP_CH_6,
    E_ISP_CH_7,
    E_ISP_CH_8,
    E_ISP_CH_9,
    E_ISP_CH_10,
    E_ISP_CH_11,
    E_ISP_CH_12,
    E_ISP_CH_13,
    E_ISP_CH_14,
    E_ISP_CH_15,
    E_ISP_CH_MAX
}CUS3A_ISP_CH_e;

typedef enum
{
    E_ISP_DEV_0 = 0,
    E_ISP_DEV_1,
    E_ISP_DEV_MAX
}CUS3A_ISP_DEV_e;

typedef enum
{
    E_ALGO_TYPE_AE = 0x0,
    E_ALGO_TYPE_AWB,
    E_ALGO_TYPE_AF,
    E_ALGO_TYPE_MAX
}CUS3A_ALGO_TYPE_e;

typedef enum
{
    E_ALGO_ADAPTOR_NATIVE = 0x0,
    E_ALGO_ADAPTOR_1,
    E_ALGO_ADAPTOR_MAX
}CUS3A_ALGO_ADAPTOR_e;

typedef enum
{
    CUS3A_CMD_ACK,
    CUS3A_CMD_MAX
}CUS3A_CMD_ID_e;

typedef struct
{
    CUS3A_ISP_DEV_e eDev;
    CUS3A_ISP_CH_e eCh;
    CUS3A_ALGO_TYPE_e eType;
    u8 bEn;
}ISP_ALGO_ACK_t;

typedef struct
{
    u32 nCmdID;
    union
    {
        ISP_ALGO_ACK_t tAlgoAck;
    };
}CUS3A_CMD_t;

// AWB statistic , one sample
typedef struct
{
    u8 r;
    u8 g;
    u8 b;
} __attribute__((packed, aligned(1))) ISP_AWB_SAMPLE;

/*! @brief ISP initial status*/
typedef struct _isp_awb_init_param
{
    u32 Size; /**< struct size*/
    u32          InitRgain;        /**< Init Rgain from Wifi */
    u32          InitGgain;        /**< Init Ggain from Wifi */
    u32          InitBgain;        /**< Init Bgain from Wifi */
} ISP_AWB_INIT_PARAM;

/*! @brief AWB HW statistics data*/
typedef struct
{
    u32 Size;           /**< struct size*/
    u32 AvgBlkX;
    u32 AvgBlkY;
    u32 CurRGain;
    u32 CurGGain;
    u32 CurBGain;
    ISP_AWB_SAMPLE *avgs;   /*awb statis for linear frame or HDR long frame*/
    /*CUS3A V1.1*/
    u8  HDRMode;             /**< Noramal or HDR mode*/
    ISP_AWB_SAMPLE*  pAwbStatisShort; /**<awb statis for HDR short Shutter AWB statistic data */
    s32 i4BVx16384;      /**< From AE output, Bv * 16384 in APEX system, EV = Av + Tv = Sv + Bv */
    u32 WeightY;                /**< frame brightness with ROI weight*/
    u32 nTotalStatsNum; /**< Total statistic histogram frame number*/
    u32 nNextStatsOffset; /**< statistic frame offset, by byte unit*/
} ISP_AWB_INFO;


/*! @brief AWB algorithm result*/
typedef struct isp_awb_result
{
    u32 Size; /**< struct size*/
    u32 Change; /**< if true, apply this result to hw register*/
    u32 R_gain; /**< AWB gain for R channel*/
    u32 G_gain; /**< AWB gain for G channel*/
    u32 B_gain; /**< AWB gain for B channel*/
    u32 ColorTmp; /**< Return color temperature*/
} ISP_AWB_RESULT;

// AE statistics data
typedef struct
{
    u8 r;
    u8 g;
    u8 b;
    u8 y;
} __attribute__((packed, aligned(1))) ISP_AE_SAMPLE;

typedef struct
{
    u16 u2HistY[_3A_HIST_BINX];
} __attribute__((packed, aligned(1))) ISP_HISTX;

typedef struct
{
    u16 u2IRHist[_3A_IR_HIST_BIN];
} __attribute__((packed, aligned(1))) ISP_IR_HISTX;

/*! @brief ISP initial status*/
typedef struct _isp_ae_init_param
{
    u32 Size;                       /**< struct size*/
    char sensor_id[32];             /**< sensor module id*/
    u32 FNx10;                      /**< F number * 10*/
    u32 shutter_min;                /**< shutter Shutter min us*/
    u32 sensor_gain_min;            /**< sensor_gain_min Minimum Sensor gain, 1X = 1024*/
    u32 shutterHDRShort_min;        /**< shutter Shutter min us*/
    u32 sensor_gainHDRShort_min;    /**< sensor_gain_min Minimum Sensor gain, 1X = 1024*/
    u32 shutterHDRMedium_min;       /**< shutter Shutter min us*/
    u32 sensor_gainHDRMedium_min;   /**< sensor_gain_min Minimum Sensor gain, 1X = 1024*/
    u32 fps;                        /**< initial frame per second*/
    u32 shutter;                    /**< shutter Shutter in us*/
    u32 shutter_step;               /**< shutter Shutter step ns*/
    u32 shutterHDRShort_step;       /**< shutter Shutter step ns*/
    u32 shutterHDRMedium_step;      /**< shutter Shutter step ns*/
    u32 sensor_gain;                /**< sensor_gain Sensor gain, 1X = 1024*/
    u32 isp_gain;                   /**< isp_gain Isp digital gain , 1X = 1024 */
    u32 isp_gain_max;               /**< isp_gain Maximum Isp digital gain , 1X = 1024 */
    u32 shutter_max;                /**< shutter Shutter max us*/
    u32 sensor_gain_max;            /**< sensor_gain_max Maximum Sensor gain, 1X = 1024*/
    u32 shutterHDRShort_max;        /**< shutter Shutter max us*/
    u32 sensor_gainHDRShort_max;    /**< sensor_gain_max Maximum Sensor gain, 1X = 1024*/
    u32 shutterHDRMedium_max;       /**< shutter Shutter max us*/
    u32 sensor_gainHDRMedium_max;   /**< sensor_gain_max Maximum Sensor gain, 1X = 1024*/
    u32 AvgBlkX;                    /**< HW statistics average block number*/
    u32 AvgBlkY;                    /**< HW statistics average block number*/
} ISP_AE_INIT_PARAM;

/*! @brief ISP report to AE, hardware statistic */
typedef struct
{
    u32            Size;                /**< CUS3A V1.0 - struct size*/
    ISP_HISTX     *hist1;               /**< CUS3A V1.0 - HW statistic histogram 1 - long*/
    ISP_HISTX     *hist2;               /**< CUS3A V1.0 - HW statistic histogram 2 - long*/
    ISP_IR_HISTX  *histIR;              /**< CUS3A V1.2 - HW statistic histogram IR*/
    u32            AvgBlkX;             /**< CUS3A V1.0 - HW statistics average block number*/
    u32            AvgBlkY;             /**< CUS3A V1.0 - HW statistics average block number*/
    ISP_AE_SAMPLE *avgs;                /**< CUS3A V1.0 - HW statistics average block data*/
    u32            FNx10;               /**< CUS3A V1.1 - Aperture in FNx10*/
    u32            Shutter;             /**< CUS3A V1.0 - Current shutter in us*/
    u32            SensorGain;          /**< CUS3A V1.0 - Current Sensor gain, 1X = 1024 */
    u32            IspGain;             /**< CUS3A V1.0 - Current ISP gain, 1X = 1024*/
    u32            ShutterHDRShort;     /**< CUS3A V1.0 - Current shutter in us*/
    u32            SensorGainHDRShort;  /**< CUS3A V1.0 - Current Sensor gain, 1X = 1024 */
    u32            IspGainHDRShort;     /**< CUS3A V1.0 - Current ISP gain, 1X = 1024*/
    u32            ShutterHDRMedium;    /**< CUS3A V1.5 - Shutter in us */
    u32            SensorGainHDRMedium; /**< CUS3A V1.5 - Sensor gain, 1X = 1024 */
    u32            IspGainHDRMedium;    /**< CUS3A V1.5 - ISP gain, 1X = 1024 */
    u32            PreWeightY;          /**< CUS3A V1.1 - Previous frame brightness with ROI weight*/
    u32            PreAvgY;             /**< CUS3A V1.1 - Previous frame brightness*/
    u8             HDRCtlMode;          /**< CUS3A V1.1 - 0 = HDR off; */
                                        /**<              1 = Separate shutter & Separate sensor gain settings */
                                        /**<              2 = Separate shutter & Share sensor gain settings */
                                        /**<              3 = Share shutter & Separate sensor gain settings */
    u32            CurFPS;              /**< CUS3A V1.1 - Current sensor FPS */
    ISP_HISTX     *hist1_short;         /**< CUS3A V1.2 - HW statistic histogram 1 - short*/
    ISP_HISTX     *hist2_short;         /**< CUS3A V1.2 - HW statistic histogram 2 - short*/
    u32            nTotalStatsNum;      /**<              Total statistic histogram frame number*/
    u32            nNextStatsOffset;    /**<              statistic frame offset, by byte unit*/
    u32 u32SensorDelayValidStatsFrame;
} ISP_AE_INFO;

//AE algorithm result
/*! @brief ISP ae algorithm result*/
typedef struct
{
    u32 Size;                       /**< CUS3A V1.0 - struct size*/
    u32 Change;                     /**< CUS3A V1.0 - if true, apply this result to hw register*/
    u32 FNx10;                      /**< CUS3A V1.1 - F number * 10*/
    u32 Shutter;                    /**< CUS3A V1.0 - Shutter in us */
    u32 SensorGain;                 /**< CUS3A V1.0 - Sensor gain, 1X = 1024 */
    u32 IspGain;                    /**< CUS3A V1.0 - ISP gain, 1X = 1024 */
    u32 ShutterHdrShort;            /**< CUS3A V1.0 - Shutter in us */
    u32 SensorGainHdrShort;         /**< CUS3A V1.0 - Sensor gain, 1X = 1024 */
    u32 IspGainHdrShort;            /**< CUS3A V1.0 - ISP gain, 1X = 1024 */
    u32 ShutterHdrMedium;           /**< CUS3A V1.5 - Shutter in us */
    u32 SensorGainHdrMedium;        /**< CUS3A V1.5 - Sensor gain, 1X = 1024 */
    u32 IspGainHdrMedium;           /**< CUS3A V1.5 - ISP gain, 1X = 1024 */
    s32 i4BVx16384;                 /**< CUS3A V1.0 - Bv * 16384 in APEX system, EV = Av + Tv = Sv + Bv */
    u32 WeightY;                    /**< CUS3A V1.1 - frame brightness with ROI weight*/
    u32 AvgY;                       /**< CUS3A V1.0 - frame brightness */
    u32 DebandFPS;                  /**< CUS3A V1.1 - Target fps when running auto debanding**/
    u16 GMBlendRatio;               /**< CUS3A V1.3 - Adaptive Gamma Blending Ratio from AE**/
    u32 HdrRatio;                   /**< CUS3A V1.0 - hdr ratio, 1X = 1024 compatible 2F */
    u32 HdrRatio1;                  /**< CUS3A V1.5 - hdr ratio, 1X = 1024 for 3F */
} ISP_AE_RESULT;

typedef enum
{
    ISP_AE_CMD_MAX
} ISP_AE_CTRL_CMD;

typedef struct
{
    u32 start_x; /*range : 0~1023*/
    u32 start_y; /*range : 0~1023*/
    u32 end_x;   /*range : 0~1023*/
    u32 end_y;   /*range : 0~1023*/
} ISP_AF_RECT;

/*! @brief ISP initial status*/
typedef struct _isp_af_init_param
{
    u32 Size; /**< struct size*/
    ISP_AF_RECT af_stats_win[16];
    /*CUS3A v1.3*/
    u32 CurPos; //motor current position
    u32 MinPos; //motor minimum position
    u32 MaxPos; //motor maximum position
    u32 MinStep;//motor minimum step
    u32 MaxStep;//motor maximum step
} ISP_AF_INIT_PARAM;

typedef enum
{
    ISP_AF_CMD_MAX,
} ISP_AF_CTRL_CMD;

typedef struct
{
    u8 high_iir[AF_STATS_IIR_1_SIZE * AF_HW_WIN_NUM];
    u8 low_iir[AF_STATS_IIR_2_SIZE * AF_HW_WIN_NUM];
    u8 luma[AF_STATS_LUMA_SIZE * AF_HW_WIN_NUM];
    u8 sobel_v[AF_STATS_FIR_V_SIZE * AF_HW_WIN_NUM];
    u8 sobel_h[AF_STATS_FIR_H_SIZE * AF_HW_WIN_NUM];
    u8 ysat[AF_STATS_YSAT_SIZE * AF_HW_WIN_NUM];
}ISP_AF_INFO_STATS_PARAM_t;

typedef struct
{
    ISP_AF_INFO_STATS_PARAM_t stParaAPI[MS_CAM_AF_MAX_WIN_NUM];
}ISP_AF_INFO_STATS;

// --------------------------------------------
#ifdef ENABLE_ISP_PDAF
typedef struct
{
    u8 u8Pdaf[4680*2];
} ISP_PDAF_INFO_STATS_PARAM_t;

typedef struct
{
    ISP_PDAF_INFO_STATS_PARAM_t stParaAPI[MS_CAM_PDAF_MAX_UNIT_NUM];
} ISP_PDAF_INFO_STATS;
#endif
// --------------------------------------------

/*! @brief ISP report to AF, hardware statistic */
typedef struct
{
    u32 Size; /**< struct size*/
    ISP_AF_INFO_STATS *pStats; /**< AF statistic*/
#ifdef ENABLE_ISP_PDAF
    ISP_PDAF_INFO_STATS *pPdafStats; /**< AF statistic*/
#endif
    /*CUS3A v1.3*/
    u32 CurPos; //motor current position
    u32 MinPos; //motor minimum position
    u32 MaxPos; //motor maximum position
} ISP_AF_INFO;

typedef struct
{
    u32 Size;           /**< struct size*/
    u32 Change; /**< if true, apply this result to hw*/
    u32 NextPos; /**< Next position*/
#if 0
    /*CUS3A v1.3*/
    u32                   ChangeParam;      /** if true, apply the following 4 results to hw register **/
    CusAFRoiMode_t        ROIMode;          /** roi mode configuration**/
    CusAFWin_t            Window[16];       /** AF statistics window position **/
    CusAFFilter_t         Filter;           /** AF filter paramater**/
    CusAFFilterSq_t       FilterSq;         /** AF filter square parameter**/
#endif
} ISP_AF_RESULT;

/**@brief ISP AE interface*/
typedef struct isp_ae_interface
{
    void *pdata; /**< Private data for AE algorithm.*/

    /** @brief AE algorithm init callback
     @param[in] pdata AE algorithm private data
     @param[in] init_state ISP initial status.
     @retval CUS_3A_SUCCESS or CUS_3A_ERROR if error occurs.
     @remark ISP call this function when AE algorithm initialize.
     */
    int (*init)(void* pdata, ISP_AE_INIT_PARAM *init_state);

    /** @brief AE algorithm close
     @param[in] pdata AE algorithm private data
     @remark ISP call this function when AE close.
     */
    void (*release)(void* pdata);

    /** @brief AE algorithm run
     @param[in] pdata AE algorithm private data
     @param[in] info ISP HW statistics
     @param[out] result AE algorithm return calculated result.
     @remark ISP call this function when AE close.
     */
    void (*run)(void* pdata, const ISP_AE_INFO *info, ISP_AE_RESULT *result);

    /** @brief AE algorithm control
     @param[in] pdata AE algorithm private data
     @param[in] cmd Control ID
     @param[in out] param Control parameter.
     @retval CUS_3A_SUCCESS or CUS_3A_ERROR if error occurs.
     @remark ISP call this function to change parameter
     */
    int (*ctrl)(void* pdata, ISP_AE_CTRL_CMD cmd, void* param);
} ISP_AE_INTERFACE;

typedef enum
{
    ISP_AWB_CMD_MAX,
} ISP_AWB_CTRL_CMD;
/**@brief ISP AWB interface*/

typedef struct isp_awb_interface
{
    void *pdata; /**< Private data for AE algorithm.*/

    /** @brief AWB algorithm init callback
     @param[in] pdata Algorithm private data
     @retval CUS_3A_SUCCESS or CUS_3A_ERROR if error occurs.
     @remark ISP call this function when AE algorithm initialize.
     */
    int (*init)(void *pdata, ISP_AWB_INIT_PARAM *param);

    /** @brief AWB algorithm close
     @param[in] pdata Algorithm private data
     @remark ISP call this function when AE close.
     */
    void (*release)(void *pdata);

    /** @brief AWB algorithm run
     @param[in] pdata Algorithm private data
     @param[in] info ISP HW statistics
     @param[out] result AWB algorithm return calculated result.
     @remark ISP call this function when AE close.
     */
    void (*run)(void *pdata, const ISP_AWB_INFO *awb_info, ISP_AWB_RESULT *result);

    /** @brief AWB algorithm control
     @param[in] pdata Algorithm private data
     @param[in] cmd Control ID
     @param[in out] param Control parameter.
     @retval CUS_3A_SUCCESS or CUS_3A_ERROR if error occurs.
     @remark ISP call this function to change parameter
     */
    int (*ctrl)(void *pdata, ISP_AWB_CTRL_CMD cmd, void* param);
} ISP_AWB_INTERFACE;

/**@brief ISP AF interface*/
typedef struct isp_af_interface
{
    void *pdata; /**< Private data for AF algorithm.*/

    /** @brief AF algorithm init callback
     @param[in] pdata Algorithm private data
     @retval CUS_3A_SUCCESS or CUS_3A_ERROR if error occurs.
     @remark ISP call this function when AF algorithm initialize.
     */
    int (*init)(void *pdata, ISP_AF_INIT_PARAM *param);

    /** @brief AF algorithm close
     @param[in] pdata Algorithm private data
     @remark ISP call this function when AF close.
     */
    void (*release)(void *pdata);

    /** @brief AF algorithm run
     @param[in] pdata Algorithm private data
     @param[in] info ISP HW statistics
     @param[out] result AF algorithm return calculated result.
     @remark ISP call this function when AF close.
     */
    void (*run)(void *pdata, const ISP_AF_INFO *af_info, ISP_AF_RESULT *result);

    /** @brief AF algorithm control
     @param[in] pdata Algorithm private data
     @param[in] cmd Control ID
     @param[in out] param Control parameter.
     @retval CUS_3A_SUCCESS or CUS_3A_ERROR if error occurs.
     @remark ISP call this function to change parameter
     */
    int (*ctrl)(void *pdata, ISP_AF_CTRL_CMD cmd, void* param);
} ISP_AF_INTERFACE;

typedef enum
{
    E_ALGO_STATUS_UNINIT,
    E_ALGO_STATUS_RUNNING
}CUS3A_ALGO_STATUS_e;

typedef struct
{
    CUS3A_ALGO_STATUS_e Ae;
    CUS3A_ALGO_STATUS_e Awb;
    CUS3A_ALGO_STATUS_e Af;
} CUS3A_ALGO_STATUS_t;

typedef enum
{
    E_CUS3A_MODE_NORMAL = 0,
    E_CUS3A_MODE_OFF,
    E_CUS3A_MODE_INJECT,
    E_CUS3A_MODE_MAX
}CUS3A_RUN_MODE_e;

unsigned int CUS3A_GetVersion(char* pVerStr);
int CUS3A_Init(CUS3A_ISP_DEV_e eDev);
void CUS3A_Release(CUS3A_ISP_DEV_e eDev);
int CUS3A_RegInterface(CUS3A_ISP_DEV_e eDev, CUS3A_ISP_CH_e eCh,ISP_AE_INTERFACE *pAE,ISP_AWB_INTERFACE *pAWB,ISP_AF_INTERFACE *pAF); /*This function is deprecated, use CUS3A_RegInterfaceEX instead*/
int CUS3A_AERegInterface(CUS3A_ISP_DEV_e eDev, CUS3A_ISP_CH_e eCh, ISP_AE_INTERFACE *pAE); /*This function is deprecated, use CUS3A_RegInterfaceEX instead*/
int CUS3A_AWBRegInterface(CUS3A_ISP_DEV_e eDev, CUS3A_ISP_CH_e eCh, ISP_AWB_INTERFACE *pAWB); /*This function is deprecated, use CUS3A_RegInterfaceEX instead*/
int CUS3A_AFRegInterface(CUS3A_ISP_DEV_e eDev, CUS3A_ISP_CH_e eCh,ISP_AF_INTERFACE *pAF); /*This function is deprecated, use CUS3A_RegInterfaceEX instead*/
int CUS3A_RegInterfaceEX(CUS3A_ISP_DEV_e eDev, CUS3A_ISP_CH_e eCh, CUS3A_ALGO_ADAPTOR_e eAdaptor, CUS3A_ALGO_TYPE_e eType, void* pAlgo);
int CUS3A_SetAlgoAdaptor(CUS3A_ISP_DEV_e eDev, CUS3A_ISP_CH_e nCh, CUS3A_ALGO_ADAPTOR_e eAdaptor, CUS3A_ALGO_TYPE_e eType);
int CUS3A_GetAlgoAdaptor(CUS3A_ISP_DEV_e eDev, CUS3A_ISP_CH_e nCh, CUS3A_ALGO_TYPE_e eType);
int CUS3A_GetAlgoStatus(CUS3A_ISP_DEV_e eDev, CUS3A_ISP_CH_e eCh, CUS3A_ALGO_STATUS_t *pStatus);
int CUS3A_SetRunMode(CUS3A_ISP_DEV_e eDev, CUS3A_ISP_CH_e eCh, CUS3A_RUN_MODE_e eMode);
int CUS3A_RunOnce(CUS3A_ISP_DEV_e eDev, CUS3A_ISP_CH_e eCh);
int CUS3A_RunOnceEn(CUS3A_ISP_DEV_e eDev, CUS3A_ISP_CH_e eCh, u8 bAeEn, u8 bAwbEn, u8 bAfEn);
int CUS3A_CreateChannel(CUS3A_ISP_DEV_e eDev, CUS3A_ISP_CH_e eIspCh);
int CUS3A_DestroyChannel(CUS3A_ISP_DEV_e eDev, CUS3A_ISP_CH_e eIspCh);

typedef u32 CUS3A_EventHandle;
CUS3A_EventHandle CUS3A_OpenIspEvent(void);
void CUS3A_CloseIspEvent(CUS3A_EventHandle Handle);
u32 CUS3A_WaitIspEvent(CUS3A_EventHandle Handle);

void* pAllocDmaBuffer(const char* pName, u32 nReqSize, u32 *pPhysAddr, u32 *pMiuAddr, u8 bCache); /*Do not use, This function is for SStar internal use only*/
int FreeDmaBuffer(const char* pName, u32 u32MiuAddr, void *pVirtAddr, u32 u32FreeSize); /*Do not use, This function is for SStar internal use only*/
int Cus3AOpenIspFrameSync(int *fd);
int Cus3ACloseIspFrameSync(int fd);
unsigned int Cus3AWaitIspFrameSync(int fd, int timeout);
int CUS3A_AeAvgDownSample(const ISP_AE_SAMPLE *pInBuf, ISP_AE_SAMPLE *pOutBuf, unsigned int nInBlkX, unsigned int nInBlkY, unsigned int nOutBlkX, unsigned int nOutBlkY);

#ifdef __cplusplus
}
#endif

#endif
