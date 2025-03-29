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
*    Calibration_API.h
*
*    Created on: Mar 07, 2018
*        Author: Jeffrey Chou
*/

#ifndef __CALIBRATION_API_H__
#define __CALIBRATION_API_H__

#ifdef __cplusplus
extern "C"
{
#endif
    #include "Calibration_Types.h"

    #define DLLIMPORT                               _declspec (dllexport)
    #define CALIBRATION_INI_PATH                    ("CalibrationInitialParameter.ini")
    #define CALI_OUTPUT_FIRIST_LAYER_FOLDER_PATH    (".\\image")
    #define CALI_OUTPUT_SECOND_LAYER_FOLDER_PATH    (".\\image\\output")
    #define CALI_DB_DUMP_FIRIST_LAYER_FOLDER_PATH   (".\\data")
    #define CALI_DB_DUMP_SECOND_LAYER_FOLDER_PATH   (".\\data\\cfg")
    #define CALI_DB_DUMP_ALSC_FILENAME              (".\\data\\cfg\\alsc_cali.data")
    #define CALI_DB_DUMP_LSC_FILENAME               (".\\data\\cfg\\lsc_cali.data")
    #define CALI_DB_DUMP_SDC_FILENAME               (".\\data\\cfg\\sdc_cali.data")
    #define CALI_DB_DUMP_AWB_FILENAME               (".\\data\\cfg\\awb_cali.data")
    #define CALI_DB_DUMP_OBC_FILENAME               (".\\data\\cfg\\obc_cali.data")
    #define CALI_DB_DUMP_ALSC_REL_FILENAME          (".\\data\\cfg\\alsc_rel_cali.data")
    #define CALI_DB_DUMP_NE_FILENAME                (".\\data\\cfg\\ne_cali.data")
    #define CALI_DB_DUMP_AIBNR_FILENAME             (".\\data\\cfg\\aibnr_cali.data")

    #define OBC_WEIGHT_X_NUM                (3)
    #define OBC_WEIGHT_Y_NUM                (3)
    #define OBC_WEIGHT_NUM                  (OBC_WEIGHT_X_NUM * OBC_WEIGHT_Y_NUM)

    #define AWB_WIN_MAX_WIDTH               (128)
    #define AWB_WIN_MAX_HEIGHT              (90)

    #define CALI_ALSC_DELTA_LUT_MAX         (9)
    #define CALI_ALSC_RGB_NUM               (3)

    #define HDR_LOG_NUM                     (16)

    #define ISP_GAIN_BASE                   (1024)
    #define IPNR_NPF_NODE_NUM            (12)      // The number of nodes in the noise profile.
    #define IPNR_NE_SEG_NUM_MAX          (12)      // The maximum number of intensity segmentations of noise estimation.
    #define IPNR_BN_LOG_SEG_NUM          (8)       // The number of segments of the log curve for binning.
    #define ISP_DECOMP_MAX_NODE             (15)
    #define ISP_AIBNR_ROI_MAX_NUM           (24)
    typedef enum
    {
        eCALI_ITEM_FPN        = 0x00,
        eCALI_ITEM_OBC        = 0x01,
        eCALI_ITEM_AWB        = 0x02,
        eCALI_ITEM_LSC        = 0x03,
        eCALI_ITEM_ALSC       = 0x04,
        eCALI_ITEM_SDC        = 0x05,
        eCALI_ITEM_ALSC_REL   = 0x06,
        eCALI_ITEM_NE         = 0x07,
        eCALI_ITEM_AINR      = 0x08,
        eCALI_ITEM_ALSC_REL_EX= 0x09,
        eCONV_ITEM_RAW_STREAM = 0x20,
        eCALI_ITEM_MAX        = 0xFF
    } CALI_ITEM;

    typedef enum
    {
        CALIB_SOURCE_BAYER              = 0, // Bayer data from sensor, no any IQ processed
        CALIB_SOURCE_STATISTISC         = 1,
        CALIB_SOURCE_AFTER_WB_GAIN      = 2, // raw data output after ob, lsc, gamma, wb
        CALIB_SOURCE_YUV                = 3,
        CALIB_SOURCE_12BITSMIPI_BAYER   = 4,
        CALIB_SOURCE_TYPE_NUM           = 5
    } CALIB_SOURCE_TYPE;

    typedef enum
    {
        eCHIP_INFO_ID_TWINKIE  = 0x02,
        eCHIP_INFO_ID_PRETZEL  = 0x03,
        eCHIP_INFO_ID_MACARON  = 0x04,
        eCHIP_INFO_ID_PUDDING  = 0x05,
        eCHIP_INFO_ID_ISPAHAN  = 0x06,
        eCHIP_INFO_ID_IKAYAKI  = 0x07,
        eCHIP_INFO_ID_MUFFIN   = 0x08,
        eCHIP_INFO_ID_MARUKO   = 0x09,
        eCHIP_INFO_ID_SOUFFLE  = 0x0A,
        eCHIP_INFO_ID_IFORD    = 0x0B,
        eCHIP_INFO_ID_IFADO    = 0x0C,
        eCHIP_INFO_ID_IFACKEL  = 0x0D,
        eCHIP_INFO_ID_PCUPID   = 0x0E,
        eCHIP_INFO_ID_JAGUAR1  = 0x0F,
        eCHIP_INFO_ID_MERCURY5 = 0x10,
        eCHIP_INFO_ID_TIRAMISU = 0x11,
        eCHIP_INFO_ID_MOCHI    = 0x13,
        eCHIP_INFO_ID_OPERA    = 0x14,
        eCHIP_INFO_ID_MAX      = 0xFF
    } CHIP_INFO_ID;

    typedef enum
    {
        eCALI_POINT_X = 0,
        eCALI_POINT_Y = 1,
        eCALI_POINT_NUM = 2
    } CALI_POINT;
    // ********************************** Information for Calibration(Not User API) - S **********************************
    typedef struct
    {
        u32 u4Version;        // | 0~7 | 8~15 | 16~23 | 24~31 | --> 0 : Calibration version number, 1 : Checksum version number, 2 : Reserved buffer, 3 : Reserved buffer
        u32 u4DataSize;
        u32 u4CheckSum;
        u32 u4Rsvd[2];
    } ISP_CALI_HEADER;

    typedef struct
    {
        CALI_ITEM item;
        char filepath[FILENAME_PATH_MAX];
    } CALI_DATA_PATH;

    typedef struct
    {
        u8   dump_cali_data;
        char *dump_path;
        char *fpn_path;
        char *obc_path;
        char *awb_path;
        char *lsc_path;
        char *alsc_path;
        char *sdc_path;
        char *alsc_rel_path;
        char *ne_path;
        char *ainr_path;
    }CALI_DB_INFO;

    typedef struct
    {
        u16 x;
        u16 y;
        u16 width;
        u16 height;
        u16 **image;
    } IMAGE_RECT;

    typedef struct
    {
        u16 decomp_f0;
        u16 decomp_f1;
        u16 decomp_f2;
    } IMAGE_DECOMP;

    typedef struct
    {
        u8  decomp_en;
        u8  input_bits;
        u8  output_bits;
        u8  node_number;
        u16 decomp_range[ISP_DECOMP_MAX_NODE-1];
    } DECOMP_CONFIG;

    typedef struct
    {
        char                *filename_path;
        char                *foldername_path;
        char                *cali_output_path;
        u8                  cfa_type;
        u8                  in_data_precision;
        u8                  out_data_precision;
        u8                  dump_clip_image;
        IMAGE_RECT          full_image;
        IMAGE_RECT          clip_image;
        u32                 frame_start_index;
        u32                 frame_numbers;
        CALIB_SOURCE_TYPE   source_type;
        IMAGE_DECOMP        decomp_range[ISP_DECOMP_MAX_NODE];
        DECOMP_CONFIG       decomp_config;
    } CALI_ISP_RAW_INFO;

    typedef struct
    {
        u32 *pStatistics;
        u16 grid_x;
        u16 grid_y;
    } OBC_CALI_STATIS_INFO;

    typedef enum
    {
        AWB_CALI_DEFAULT_H      = 0,
        AWB_CALI_CT_H           = 1,
        AWB_CALI_DEFAULT_L      = 2,
        AWB_CALI_CT_L           = 3,
        AWB_CALI_NUM            = 4
    } AWB_CALI_TYPE;

	typedef enum
	{
		AWB_CALI_DEFAULT = 0,
		AWB_CALI_CT = 1,
		AWB_CALI_NUM_V2 = 2
	} AWB_CALI_TYPE_V2;

    typedef struct
    {
        u16 Rgain;
        u16 Bgain;
    } AWB_RBGain;

    typedef struct
    {
        u16 Ravg;
		u16 Gavg;
        u16 Bavg;
    } AWB_RGBAVG;

    typedef struct
    {
        u16 u2AvgR;
        u16 u2AvgG;
        u16 u2AvgB;
    } ISP_AWB_AVGS;

    typedef struct
    {
        u8 uHighLowCTMode;
        u16 u2LumMin;
        u16 u2LumMax;
		AWB_RGBAVG asCaliAvg[AWB_CALI_NUM_V2];
		AWB_RBGain asCaliGain[AWB_CALI_NUM];
        ISP_AWB_AVGS AwbIspStatis[AWB_WIN_MAX_HEIGHT*AWB_WIN_MAX_WIDTH];
    } AWB_CALI_STATIS_INFO;
 
    typedef enum
    {
        ALSC_HL_8XCAP_10XDARKCORNER_MODE = 0,
        ALSC_HL_8XCAP_MODE = 1, 
        ALSC_HL_8XRGBGAINRATIOKEEP_MODE = 2, 
        ALSC_HL_PASS_MODE = 3, 
        ALSC_HL_MODE_NUM = 4
    } ALSC_HARDWARE_LIMIT_MODE;

    typedef enum
    {
        ALSC_REL_EX_BIN_FILE_MODE = 0,
        ALSC_REL_EX_RAW_MODE = 1,
        ALSC_REL_EX_NUM = 2
    } ALSC_REL_EX_INPUT_MODE;

    typedef struct
    {
        u8  bSegDeltaAdvancedEnable;                            // 0 : False, 1 : True;
        u8  bPortraitEnable;                                    // 0 : False, 1 : True;
        u8  bUnitGainResult;                                    // 0 : False, 1 : True;
        u8  uSegDeltaStrMode;                                   // 0 ~ 15
        u8  uSelectCaliDomain;                                  // 0 --> Color domain, 1 --> Bayer domain [Debug mode]
        s16  sSplitOverlap;                                     // negative		--> split mode off,
		s16  sConcatMode;										// 0: 2 file, 1: 1 file
		ALSC_HARDWARE_LIMIT_MODE uMode;
        u8  uDumpALSC;                                          // 0 ~ 1 --> 0 : Disable, 1 : Enable
																// non-negative --> split mode on
        u16 u2BlockSize[eCALI_POINT_NUM];                       // Index : 0、1    --> X、Y
        u16 u2TableSizeMax;
        u32 u4DeltaLUT[eCALI_POINT_NUM][CALI_ALSC_DELTA_LUT_MAX];
        u16 *pRGBGain[CALI_ALSC_RGB_NUM];                       // Index : 0、1、2 --> R、G、B
        u16 *pRGBData[CALI_ALSC_RGB_NUM];                       // Index : 0、1、2 --> R、G、B
        u16 *pRGBData_Grid[CALI_ALSC_RGB_NUM];                  // Index : 0、1、2 --> R、G、B
    } ALSC_CALI_BUFFER_INFO;

    typedef struct
    {
        u8  bDbgFlag;                                           // 0 : False, 1 : True;
        u8  bRationAdvancedEnable;                              // 0 : False, 1 : True;
        u16 u2SlopRatioThreshold;
    } LSC_CALI_BUFFER_INFO;

    typedef struct
    {
        u8 bDetectUnqualifiedRaw;
        u16 u2DuTolerance;
        u16 u2DuObcR;
        u16 u2DuObcB;
        u16 u2DuObcGr;
        u16 u2DuObcGb;
        u32 u4DebugFlag;
    } SDC_CALI_BUFFER_INFO;

    typedef enum  
    {
        GCH_G_MODE,
        GCH_GRAY_MODE,
        GCH_NUM
    } NE_GCH_MODE;

    typedef struct
    {
        s32 diff_blur_en;
        s32 sad_h_sel;

        s32 bn_ratio_h;
        s32 bn_ratio_v;
        s32 bn_ob_r;
        s32 bn_ob_gr;
        s32 bn_ob_gb;
        s32 bn_ob_b;
        u32 bn_isp_gain;
        int bn_log_in[IPNR_BN_LOG_SEG_NUM];
        int bn_log_out[IPNR_BN_LOG_SEG_NUM];
        s32 npf_node_dist[IPNR_NPF_NODE_NUM - 1];

        s32 ne_blk_sample_step;       //1~31    (calculated by driver)
        s32 ne_blk_mean_lb;           //0~31    (16)
        s32 ne_blk_mean_ub;           //0~255 (240)
        s32 ne_blk_sad_dc_ub;         //0~31    (8)
        s32 ne_blk_sad_ub;            //0~4095  (40*64)
        s32 ne_noise_mean_th_gain;    //0~15    (6)
        s32 ne_noise_std_th_gain;     //0~15    (2)
        s32 ne_inten_blknum_lb;       //0~63    (16)
        s32 ne_blk_sad_min_lb;        //0~63    (64/2)
        s32 ne_noise_mean_lb;         //0~63    (64/2)
        s32 ne_noise_std_lb;          //0~63    (64/2)
        f32 ne_learn_rate_lb;       //0~1.  Noise profile learning rate lower bound. Larger values induce noise profile being closer to current status.
        f32 ne_learn_rate_ub;       //0~1.  Noise profile learning rate upper bound. Larger values induce noise profile being closer to current status.
        s32 ne_seg_num;               //1~12. Number of NE s32ensity segments.
        s32 ne_seg[IPNR_NE_SEG_NUM_MAX - 1];          //0~255. NE s32ensity segmentation pos32s.
        s32 ne_blk_sad_min_g[IPNR_NE_SEG_NUM_MAX];    //0~16383
        s32 ne_blk_sad_min_b[IPNR_NE_SEG_NUM_MAX];    //0~16383
        s32 ne_blk_sad_min_r[IPNR_NE_SEG_NUM_MAX];    //0~16383
        s32 ne_noise_mean_g[IPNR_NE_SEG_NUM_MAX];     //0~16383
        s32 ne_noise_mean_b[IPNR_NE_SEG_NUM_MAX];     //0~16383
        s32 ne_noise_mean_r[IPNR_NE_SEG_NUM_MAX];     //0~16383
        s32 ne_noise_std_g[IPNR_NE_SEG_NUM_MAX];      //0~16383
        s32 ne_noise_std_b[IPNR_NE_SEG_NUM_MAX];      //0~16383
        s32 ne_noise_std_r[IPNR_NE_SEG_NUM_MAX];      //0~16383
        s32 ne_blk_sample_num_target; //0~65535 (32768, virtual register for NE driver)
        s32 gray_w_r;
        s32 gray_w_g;
        s32 gray_w_b;
        s32 gray_w_sft;

        s32 ne_dummy_mode;
        u8  ne_auto_assign;
        u32 ne_seg_cnt[3][IPNR_NE_SEG_NUM_MAX];
        u8  ne_debug_engineer;
        u8  hdr_mode;
        u16 hdr_awb_gain[3];
        u32 hdr_log_in[HDR_LOG_NUM];
        u32 hdr_log_out[HDR_LOG_NUM];
        u32 hdr_log_out_ub;
        u32 hdr_log_in_ub;
        s8  hdr_shift_bits;

        u8  manual_npf_mode;
        s32 manual_npf_node_dist[IPNR_NPF_NODE_NUM - 1];
        u8  data_to_txt_mode;
        u8  gch_mode_sel;
    } NE_CALI_BUFFER_INFO;

    typedef struct
    {
        char  *darkfoldername_path;
        char  *colorcheckerfoldername_path;
        u8  uReGenerate;
        u8 IsoNum;
        u8 ROINum;
        u16 IsoThd[2];
        u16 ROITopLeftX[ISP_AIBNR_ROI_MAX_NUM];
        u16 ROITopLeftY[ISP_AIBNR_ROI_MAX_NUM];
        u16 ROIWidth[ISP_AIBNR_ROI_MAX_NUM];
        u16 ROIHeight[ISP_AIBNR_ROI_MAX_NUM];
        float MeanThd0[3];
        float MeanThd1[3];
        float BlackIsoThd;
        float ColorIsoThd;
        float Output_BlackISO[16];
        float Output_BlackS[16];
        float Output_ColorISO[16];
        float Output_ColorK[16];
        float Output_KA;
        float Output_KB;
        float Output_SA;
        float Output_SB;
    } AINR_CALI_BUFFER_INFO;

    typedef struct
    {
        char  *factory_shading_path;
        ALSC_REL_EX_INPUT_MODE input_mode;
        u8    input_precision;
        u8    output_precision;
    } ALSCRELEX_CALI_BUFFER_INFO;

    // ********************************** Buffer for Calibration(Not User API) - E **********************************

    typedef struct
    {
        u8  uChipID;
        u8  uCfaType;
        u8  uCfaPrecision;
        u8  uDumpCaliData;
        u8  uKeepCaliData;
        u8  uGainIndex;
        u8  uAutoAssignValue;
        u8  uWeightTable[OBC_WEIGHT_NUM];
        u8  uDecomp_enable;
        u8  uDecomp_input_bits;
        u8  uDecomp_output_bits;
        u16 u2ImageWidth;
        u16 u2ImageHeight;
        u16 u2ImageClipX;
        u16 u2ImageClipY;
        u16 u2ImageClipWidth;
        u16 u2ImageClipHeight;
        u16 u2TargetValue;
        u16 u2Decomp_range[ISP_DECOMP_MAX_NODE-1];
        u16 u2Decomp_range_f0[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f1[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f2[ISP_DECOMP_MAX_NODE];
    } OBC_PLUGIN_CTRL;

    typedef struct
    {
        u8  uChipID;
        u8  uCfaType;
        u8  uCfaPrecision;
        u8  uDumpCaliData;
        u8  uKeepCaliData;
        u8  uCTNumber;
        u8  uTargetId;
        u8  uTableSize;
        u8  uSegmentLength;
        u8  uIntervalShift;
        u8  uInOrientation;
        u8  uAutoSearchCenter;
        u8  uDumpLSCResult;
        u8  uShowDebugInfo;
        u8  uRatioTableAdvEn;
        u8  uDecomp_enable;
        u8  uDecomp_input_bits;
        u8  uDecomp_output_bits;
        u16 u2ImageWidth;
        u16 u2ImageHeight;
        u16 u2ImageClipX;
        u16 u2ImageClipY;
        u16 u2ImageClipWidth;
        u16 u2ImageClipHeight;
        u16 u2ColorTemperature;
        u16 u2InputCenterX;
        u16 u2InputCenterY;
        u16 u2RatioThreshold;
        u16 u2Decomp_range[ISP_DECOMP_MAX_NODE-1];
        u16 u2Decomp_range_f0[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f1[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f2[ISP_DECOMP_MAX_NODE];
    } LSC_PLUGIN_CTRL;

    typedef struct
    {
        u8  uChipID;
        u8  uCfaType;
        u8  uCfaPrecision;
        u8  uDumpCaliData;
        u8  uKeepCaliData;
        u8  uCTNumber;
        u8  uTargetId;
        u8  uGridX;
        u8  uGridY;
        u8  uGridXMax;
        u8  uGridYMax;
		u8  uDumpALSCResult;
        u8  uDeltaMode;
        u8  uDeltaModeAdvEnable;
        u8  uDecomp_enable;
        u8  uDecomp_input_bits;
        u8  uDecomp_output_bits;
        u16 u2ImageWidth;
        u16 u2ImageHeight;
        u16 u2ImageClipX;
        u16 u2ImageClipY;
        u16 u2ImageClipWidth;
        u16 u2ImageClipHeight;
        u16 u2ColorTemperature;
        u16 u2Decomp_range[ISP_DECOMP_MAX_NODE-1];
        u16 u2Decomp_range_f0[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f1[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f2[ISP_DECOMP_MAX_NODE];
    } ALSC_PLUGIN_CTRL;

    typedef struct
    {
        u8  uChipID;
        u8  uCfaType;
        u8  uCfaPrecision;
        u8  uDumpCaliData;
        u8  uKeepCaliData;
        u8  uDecomp_enable;
        u8  uDecomp_input_bits;
        u8  uDecomp_output_bits;
        u8  uHighLowCTMode;
        u8  uCaliState;
        u8  uCaliNum;
        u8  uBrightnessCaliMode;
        u16 u2ImageWidth;
        u16 u2ImageHeight;
        u16 u2ImageClipX;
        u16 u2ImageClipY;
        u16 u2ImageClipWidth;
        u16 u2Decomp_range[ISP_DECOMP_MAX_NODE-1];
        u16 u2Decomp_range_f0[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f1[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f2[ISP_DECOMP_MAX_NODE];
        u16 u2ImageClipHeight;
        u16 u2OBGain_R;
        u16 u2OBGain_GR;
        u16 u2OBGain_GB;
        u16 u2OBGain_B;
        u16 u2HighCT;
        u16 u2LowCT;
        u16 u2CT;
    } AWB_PLUGIN_CTRL;

    typedef struct
    {
        u8  uChipID;
        u8  uCfaType;
        u8  uCfaPrecision;
        u8  uDumpCaliData;
        u8  uKeepCaliData;
        u8  uDecomp_enable;
        u8  uDecomp_input_bits;
        u8  uDecomp_output_bits;
        u16 u2ImageWidth;
        u16 u2ImageHeight;
        u16 u2ImageClipX;
        u16 u2ImageClipY;
        u16 u2ImageClipWidth;
        u16 u2ImageClipHeight;
        u16 u2Decomp_range[ISP_DECOMP_MAX_NODE-1];
        u16 u2Decomp_range_f0[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f1[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f2[ISP_DECOMP_MAX_NODE];
    } SDC_PLUGIN_CTRL;

    typedef struct
    {
        u8  uChipID;
        u8  uCfaType;
        u8  uCfaPrecision;
        u8  uDumpCaliData;
        u8  uKeepCaliData;
        u8  uAutoAssign;
        u8  uGainIndex;
        u8  uDecomp_enable;
        u8  uDecomp_input_bits;
        u8  uDecomp_output_bits;
        u8  uHDR_mode;
        u16 u2ImageWidth;
        u16 u2ImageHeight;
        u16 u2ImageClipX;
        u16 u2ImageClipY;
        u16 u2ImageClipWidth;
        u16 u2ImageClipHeight;
        u16 u2Decomp_range[ISP_DECOMP_MAX_NODE-1];
        u16 u2Decomp_range_f0[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f1[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f2[ISP_DECOMP_MAX_NODE];
        u16 u2HDR_WB_R_gain;
        u16 u2HDR_WB_G_gain;
        u16 u2HDR_WB_B_gain;
        s32 s4DummyMode;
    } NE_PLUGIN_CTRL;

    typedef struct
    {
        u8  uChipID;
        u8  uCfaType;
        u8  uCfaPrecision;
        u8  uDumpCaliData;
        u8  uKeepCaliData;
        u8  uIsoNum;
        u8  uReGenerate;
        u8  uDecomp_enable;
        u8  uDecomp_input_bits;
        u8  uDecomp_output_bits;
        u8  uROINum;
        u16 u2ImageWidth;
        u16 u2ImageHeight;
        u16 u2ImageClipX;
        u16 u2ImageClipY;
        u16 u2ImageClipWidth;
        u16 u2ImageClipHeight;
        u16 u2Decomp_range[ISP_DECOMP_MAX_NODE-1];
        u16 u2Decomp_range_f0[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f1[ISP_DECOMP_MAX_NODE];
        u16 u2Decomp_range_f2[ISP_DECOMP_MAX_NODE];
        u16 u2ROITopLeftX[ISP_AIBNR_ROI_MAX_NUM];
        u16 u2ROITopLeftY[ISP_AIBNR_ROI_MAX_NUM];
        u16 u2ROIWidth[ISP_AIBNR_ROI_MAX_NUM];
        u16 u2ROIHeight[ISP_AIBNR_ROI_MAX_NUM];
        f32 IsoThd[2];
        f32 MeanThd0[3];
        f32 MeanThd1[3];
        f32 BlackIsoThd;
        f32 ColorIsoThd;
    } AINR_PLUGIN_CTRL;

    typedef struct
    {
        OBC_CALI_STATIS_INFO    *pObcStatisInfo;    // OBC parameters
        AWB_CALI_STATIS_INFO    *pAwbStatisInfo;    // AWB UNIT parameters
        ALSC_CALI_BUFFER_INFO   *pAlscBufferInfo;   // ALSC parameters
        LSC_CALI_BUFFER_INFO    *pLscBufferInfo;    // LSC parameters
        NE_CALI_BUFFER_INFO     *pNeBufferInfo;
        AINR_CALI_BUFFER_INFO   *pAInrBufferInfo;
        ALSCRELEX_CALI_BUFFER_INFO *pAlscRelExBufferInfo;
        SDC_CALI_BUFFER_INFO    *pSdcBufferInfo;

        // General parameters
        u8                      bEnableCommandLine; // 0 : False, 1 : True;
        u8                      bEnablePluginMode;  // 0 : False, 1 : True;
        u8                      bLoadCalibData;     // 0 : False, 1 : True;
        u8                      uChipInfoID;
        u8                      uAlignmentMode;     // 0 : LSB  , 1 : MSB
        u8                      uDebug;

        // handle 
        CALI_DATA_PATH          CaliData;
        CALI_ISP_RAW_INFO       *pIspRawInfo;
        CALI_DB_INFO            *pCaliDB;
        void                    *pCtrl;
        void                    *pData;
        void                    *pReadDataBuf;

        char                    *pInputFilename;
        char                    *pOutputFilename;
    } calibration_handle;
    /**
     * @brief create folder according to given path.
     *
     * @param path              : directory path to create, you should make sould that parent directory exists.
     */
    DLLIMPORT s32 CreatFolderDirectory(const char* path);
    /**
     * @brief parse calibration item from ini file.
     *
     * @param initial_path              : path of ini file.
     * @param item                      : pointer of calibration item. (output)
     */
    DLLIMPORT s32 GetCalibrationInformation(char* initial_path, CALI_ITEM *item);

    /**
     * @brief Function to release calibration_handle.
     *
     * @param handle              : pointer of calibration handle.
     */
    DLLIMPORT s32 ReleaseCalibrationHandle(void* handle);
    // ***************************************** Calibration XXX - S *****************************************

    /**
     * @brief Function to do FPN calibration, plugin mode not supported so far.
     *
     * @param handle              : handle to pass parameters of calibration.
     **
     * The following parameters works only when PLUGIN MODE(handle->bEnablePluginMode == 1)
     * @param pData               : pointer to pass parameters of calibration for PLUGIN MODE.
     * @param pStrFileName        : file path of input raw file for PLUGIN MODE.
     */
    DLLIMPORT s32 DoCalibrationFPN(calibration_handle *handle, void *pData, char *pStrFileName);

    /**
     * @brief Function to do OBC calibration.
     *
     * @param handle              : handle to pass parameters of calibration.
     **
     * The following parameters works only when PLUGIN MODE(handle->bEnablePluginMode == 1)
     * @param pData               : pointer to pass parameters of calibration.
     * @param pStrFileName        : file path of input raw file.
     * @param pOutput             : OB result table whose size should be 16*4. (Output)
     */
    DLLIMPORT s32 DoCalibrationOBC(calibration_handle *handle, void *pData, char *pStrFileName, u16 *pOutput);

    /**
     * @brief Function to do LSC calibration.
     *
     * @param handle              : handle to pass parameters of calibration.
     **
     * The following parameters works only when PLUGIN MODE(handle->bEnablePluginMode == 1)
     * @param pData               : pointer to pass parameters of calibration.
     * @param pStrFileName        : file path of input raw file.
     * @param pRatioTable         : The ratio of compensation intensity from the center of the picture to the corners.Parameter range: 0 ~ 255.
     * @param pOutIntervalShift   : The amount of shifting distance. Parameter range: 0 ~ 31. The distance to the center point will be shifted by this value, which serves as the index for the query of Gain Table.X
     * @param pOutCenterX         : X position of the brightest center point in the picture.
     * @param pOutCenterY         : Y position of the brightest center point in the picture.
     * @param pOBC                : OB value of current sensor in 16-bit. Parameter range: 0 ~ 65535.
     * @param pOutput             : LSC result table whose size should be 32*3. (Output)
     */
    DLLIMPORT s32 DoCalibrationLSC(calibration_handle *handle, void *pData, char *pStrFileName, u8 *pRatioTable, u8* pOutIntervalShift, u16* pOutCenterX, u16* pOutCenterY, u16 *pOBC, u16 *pOutput);

    /**
     * @brief Function to do ALSC calibration.
     *
     * @param handle              : handle to pass parameters of calibration.
     **
     * The following parameters works only when PLUGIN MODE(handle->bEnablePluginMode == 1)
     * @param pData               : pointer to pass parameters of calibration.
     * @param pStrFileName        : file path of input raw file.
     * @param pRatioTable         : The ratio of compensation intensity from the center of the picture to the corners.Parameter range: 0 ~ 255.
     * @param pDeltaLUT_X         : The spacing of X-axis. Every 4 bits represent a set of index [index = (block size / 16) – 1], so the actual number of pixels is (index + 1) × 16. The index value is limited to 1, 2, 4, 8, and 16. If no block is used, set it to 0. Up to 72 sets of blocks are supported. In this platform, the spacing is 26
     * @param pDeltaLUT_Y         : The spacing of Y-axis. Every 4 bits represent a set of index [index = (block size / 16) – 1], so the actual number of pixels is (index + 1) × 16. The index value is limited to 1, 2, 4, 8, and 16. If no block is used, set it to 0. Up to 72 sets of blocks are supported. In this platform, the spacing is 16-grid.
     * @param pOBC                : OB value of current sensor in 16-bit. Parameter range: 0 ~ 65535.X
     * @param pOutput             : ALSC result table whose size should be 27*17*3.
     */
    DLLIMPORT s32 DoCalibrationALSC(calibration_handle *handle, void *pData, char *pStrFileName, u8 *pRatioTable, u8 *pDeltaLUT_X, u8 *pDeltaLUT_Y, u16 *pOBC, u16 *pOutput);

    /**
     * @brief Function to do AWB calibration.
     *
     * @param handle              : handle to pass parameters of calibration.
     **
     * The following parameters works only when PLUGIN MODE(handle->bEnablePluginMode == 1)
     * @param pData               : pointer to pass parameters of calibration.
     * @param pStrFileName        : file path of input raw file.
     */

    DLLIMPORT s32 DoCalibrationAWB(calibration_handle *handle, void *pData, char *pStrFileName);      //TBD

    /**
     * @brief Function to do SDC calibration.
     *
     * @param handle              : handle to pass parameters of calibration.
     **
     * The following parameters works only when PLUGIN MODE(handle->bEnablePluginMode == 1)
     * @param pData               : pointer to pass parameters of calibration.
     * @param pStrFileName        : file path of input raw file.
     * @param pValidValue         : Number of pixels need to be calibrate, should be allocated with size 2, index 0/1 represents black/white defect pixel, respectively.
     * @param pClusterAmount      : Number of pixels be classified as cluster, should be allocated with size 2, index 0/1 represents black/white defect pixel, respectively.
     * @param pTotalAmount        : Number of pixels, should be allocated with size 2, index 0/1 represents black/white defect pixel, respectively.
     */
    DLLIMPORT s32 DoCalibrationSDC_OutputParams(calibration_handle *handle, void *pData, char *pStrFileName, unsigned short *pValidValue, unsigned int *pClusterAmount, unsigned int *pTotalAmount);      //TBD

    /**
     * @brief Function to do SDC calibration.
     *
     * @param handle              : handle to pass parameters of calibration.
     **
     * The following parameters works only when PLUGIN MODE(handle->bEnablePluginMode == 1)
     * @param pData               : pointer to pass parameters of calibration.
     * @param pStrFileName        : file path of input raw file.
     */
    DLLIMPORT s32 DoCalibrationSDC(calibration_handle *handle, void *pData, char *pStrFileName);      //TBD

    /**
     * @brief Function to do raw stream conversion, plugin mode not supported so far.
     *
     * @param handle              : handle to pass parameters of calibration.
     **
     * The following parameters works only when PLUGIN MODE(handle->bEnablePluginMode == 1)
     * @param pData               : pointer to pass parameters of calibration.
     * @param pStrFileName        : file path of input raw file.
     */
    DLLIMPORT s32 DoConversionRawStream(calibration_handle *handle, void *pData, char *pStrFileName); //TBD

     /**
     * @brief Function to do ALSC_REL calibration, plugin mode not supported so far.
     *
     * @param handle              : handle to pass parameters of calibration.
     **
     * The following parameters works only when PLUGIN MODE(handle->bEnablePluginMode == 1)
     * @param pData               : pointer to pass parameters of calibration.
     * @param pStrFileName        : file path of input raw file.
     * @param pRatioTable         : The ratio of compensation intensity from the center of the picture to the corners.Parameter range: 0 ~ 255.
     * @param pOutIntervalShift   : The amount of shifting distance. Parameter range: 0 ~ 31. The distance to the center point will be shifted by this value, which serves as the index for the query of Gain Table.X
     * @param pOutCenterX         : X position of the brightest center point in the picture.
     * @param pOutCenterY         : Y position of the brightest center point in the picture.
     * @param pOBC                : OB value of current sensor in 16-bit. Parameter range: 0 ~ 65535.
     * @param pOutput             : ALSC_REL result table whose size should be 32*4.
     */
    DLLIMPORT s32 DoCalibrationALSCRelationship(calibration_handle *handle, void *pData, char *pStrFileName, u8 *pRatioTable, u8* pOutIntervalShift, u16* pOutCenterX, u16* pOutCenterY, u16 *pOBC, u16 *pOutput);

     /**
     * @brief Function to do NE calibration.
     *
     * @param handle              : handle to pass parameters of calibration.
     **
     * The following parameters works only when PLUGIN MODE(handle->bEnablePluginMode == 1)
     * @param pData               : pointer to pass parameters of calibration.
     * @param pStrFileName        : file path of input raw file.
     * @param uFrameNumber        : frame number to calibration.
     * @param pOBC                : OB value of current sensor in 16-bit. Parameter range: 0 ~ 65535.
     * @param pOutput             : NE result table whose size should be 32*4.
     */
    DLLIMPORT s32 DoCalibrationNE(calibration_handle *handle, void *pData, char *pStrFolderName, u8 uFrameNumber, u16 *pOBC, u16 *pOutput, u32 *pSegCount);
     /**
     * @brief Function to do AINR calibration.
     *
     * @param handle                   : handle to pass parameters of calibration.
     **
     * The following parameters works only when PLUGIN MODE(handle->bEnablePluginMode == 1)
     * @param pData                    : pointer to pass parameters of calibration.
     * @param pStrDarkFileName         : file path of input dark raw file.
     * @param pStrColorCheckerFileName : file path of input color checker raw file.
     * @param uFrameNumber             : frame number to calibration.
     * @param pOutput                  : AIBNR result table whose size should be 32*4.
     */
    DLLIMPORT s32 DoCalibrationAINR(calibration_handle *handle, void *pData, char *pStrDarkFolderName, char *pStrColorCheckerFolderName, float BlackISO[], float BlackS[], float ColorISO[], float ColorK[], float* As, float* Bs, float* Ak, float* Bk);


    DLLIMPORT s32 DoCalibrationALSCRelationshipEX(calibration_handle *handle, void *pData, char *pStrFileName, u16 *pOBC, u16 *pOutput[CALI_ALSC_RGB_NUM]);
    // ***************************************** Calibration XXX - E *****************************************
    
    // **************************************** Wrapped API - S *****************************************
     /**
     * @brief RGBIR pattern to bayer pattern.
     */
    DLLIMPORT s32 RGBIR_to_Bayer(u16 *pSrc, u16 *pDst, s32 w, s32 h, s32 irfmt, s32 bayerfmt);
    // **************************************** Wrapped API - E *****************************************
#ifdef __cplusplus
}    //end of extern C
#endif

#endif
