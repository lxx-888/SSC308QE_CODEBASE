/*
 * drv_ive_io_st.h- Sigmastar
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
#ifndef _DRV_IVE_IO_ST_H_
#define _DRV_IVE_IO_ST_H_

#include "cam_os_wrapper.h"

//---------------------------------------------------------------------------
// Define.
//---------------------------------------------------------------------------

#define IVE_IOC_MAGIC   'I'
#define IVE_IOC_PROCESS _IOW(IVE_IOC_MAGIC, 1, ive_ioc_config*)

#define IVE_CCL_MAX_REGION_NUM (255)
#define IVE_CCL_REGION_LENGTH  (16)
#define IVE_BGBLUR_LUT_KEY_NUM (15)
#define IVE_BGBLUR_LUT_VAL_NUM (16)

//---------------------------------------------------------------------------
// Structure and enum.
//---------------------------------------------------------------------------

typedef enum
{
    IVE_IOC_ERROR_NONE                  = 0x0000,
    IVE_IOC_ERROR_IN_OUT_SIZE_DIFFERENT = 0x1000,
    IVE_IOC_ERROR_IMG_TOO_SMALL         = 0x1001,
    IVE_IOC_ERROR_PROC_CONFIG           = 0x1002,
    IVE_IOC_ERROR_BUSY                  = 0x1003,
    IVE_IOC_ERROR_MEMROY_FAILURE        = 0x1004,
    IVE_IOC_ERROR_CLK                   = 0x1005,
    IVE_IOC_ERROR_WRONG_SIZE            = 0x1006,
} IVE_IOC_ERROR;

typedef enum
{
    IVE_IOC_PADDING_MODE_MIRROR    = 0X1,
    IVE_IOC_PADDING_MODE_REPLICATE = 0X2
} IVE_IOC_PADDING_MODE;

typedef enum
{
    IVE_IOC_OP_TYPE_FILTER           = 0x00,
    IVE_IOC_OP_TYPE_CSC              = 0x01,
    IVE_IOC_OP_TYPE_FILTER_AND_CSC   = 0x02,
    IVE_IOC_OP_TYPE_SOBEL            = 0x03,
    IVE_IOC_OP_TYPE_MAG_AND_ANG      = 0x04,
    IVE_IOC_OP_TYPE_ORD_STA_FILTER   = 0x05,
    IVE_IOC_OP_TYPE_BERNSEN          = 0x06,
    IVE_IOC_OP_TYPE_DILATE           = 0x07,
    IVE_IOC_OP_TYPE_ERODE            = 0x08,
    IVE_IOC_OP_TYPE_THRESH           = 0x09,
    IVE_IOC_OP_TYPE_THRESH_S16       = 0x0A,
    IVE_IOC_OP_TYPE_THRESH_U16       = 0x0B,
    IVE_IOC_OP_TYPE_AND              = 0x0C,
    IVE_IOC_OP_TYPE_OR               = 0x0D,
    IVE_IOC_OP_TYPE_XOR              = 0x0E,
    IVE_IOC_OP_TYPE_ADD              = 0x0F,
    IVE_IOC_OP_TYPE_SUB              = 0x10,
    IVE_IOC_OP_TYPE_16BIT_TO_8BIT    = 0x11,
    IVE_IOC_OP_TYPE_MAP              = 0x12,
    IVE_IOC_OP_TYPE_HISTOGRAM        = 0x13,
    IVE_IOC_OP_TYPE_INTEGRAL         = 0x14,
    IVE_IOC_OP_TYPE_SAD              = 0x15,
    IVE_IOC_OP_TYPE_NCC              = 0x16,
    IVE_IOC_OP_TYPE_LBP              = 0x18,
    IVE_IOC_OP_TYPE_BAT              = 0x19,
    IVE_IOC_OP_TYPE_ADP_THRESH       = 0x1A,
    IVE_IOC_OP_TYPE_MATRIX_TRANSFORM = 0x1F,
    IVE_IOC_OP_TYPE_IMAGE_DOT        = 0x20,
    IVE_IOC_OP_TYPE_ALPHA_BLENDING   = 0x21,
    IVE_IOC_OP_TYPE_CCL              = 0x22,
    IVE_IOC_OP_TYPE_GMM              = 0x23,
    IVE_IOC_OP_TYPE_RESIZE           = 0x24,
    IVE_IOC_OP_TYPE_BGBLUR           = 0xFF,
} IVE_IOC_OP_TYPE;

typedef enum
{
    IVE_IOC_TASK_PRIORITY_DEFAULT = 0,
    IVE_IOC_TASK_PRIORITY_ONE     = 1,
    IVE_IOC_TASK_PRIORITY_MAX,
} IVE_IOC_TASK_PRIORITY_E;

typedef enum
{
    IVE_IOC_IMAGE_FORMAT_B8C1      = 0x00,
    IVE_IOC_IMAGE_FORMAT_B8C3_PLAN = 0x02,
    IVE_IOC_IMAGE_FORMAT_B16C1     = 0x03,
    IVE_IOC_IMAGE_FORMAT_B8C3_PACK = 0x04,
    IVE_IOC_IMAGE_FORMAT_420SP     = 0x05,
    IVE_IOC_IMAGE_FORMAT_422SP     = 0x06,
    IVE_IOC_IMAGE_FORMAT_422YUYV   = 0x20,
    IVE_IOC_IMAGE_FORMAT_B32C1     = 0x80, // dummy enum for destination buffer
    IVE_IOC_IMAGE_FORMAT_B64C1     = 0x81  // dummy enum for destination buffer
} IVE_IOC_IMAGE_FORMAT;

typedef enum
{
    IVE_IOC_MODE_SOBEL_BOTH       = 0x00,
    IVE_IOC_MODE_SOBEL_HORIZONTAL = 0x01,
    IVE_IOC_MODE_SOBEL_VERTICAL   = 0x02,
} IVE_IOC_MODE_SOBEL;

typedef enum
{
    IVE_IOC_MODE_MAG_AND_ANG_ONLY_MAG = 0x00,
    IVE_IOC_MODE_MAG_AND_ANG_BOTH     = 0x01
} IVE_IOC_MODE_MAG_AND_ANG;

typedef enum
{
    IVE_IOC_MODE_ORD_STAT_FILTER_MEDIAN = 0x00,
    IVE_IOC_MODE_ORD_STAT_FILTER_MAX    = 0x01,
    IVE_IOC_MODE_ORD_STAT_FILTER_MIN    = 0x02,
} IVE_IOC_MODE_ORD_STAT_FILTER;

typedef enum
{
    IVE_IOC_MODE_BERNSEN_NORMAL_3X3 = 0x00,
    IVE_IOC_MODE_BERNSEN_NORMAL_5X5 = 0x01,
    IVE_IOC_MODE_BERNSEN_THRESH_3X3 = 0x02,
    IVE_IOC_MODE_BERNSEN_THRESH_5X5 = 0x03
} IVE_IOC_MODE_BERNSEN;

typedef enum
{
    IVE_IOC_MODE_THRESH_BINARY      = 0x00,
    IVE_IOC_MODE_THRESH_TRUNC       = 0x01,
    IVE_IOC_MODE_THRESH_TO_MINVAL   = 0x02,
    IVE_IOC_MODE_THRESH_MIN_MID_MAX = 0x03,
    IVE_IOC_MODE_THRESH_ORI_MID_MAX = 0x04,
    IVE_IOC_MODE_THRESH_MIN_MID_ORI = 0x05,
    IVE_IOC_MODE_THRESH_MIN_ORI_MAX = 0x06,
    IVE_IOC_MODE_THRESH_ORI_MID_ORI = 0x07
} IVE_IOC_MODE_THRESH;

typedef enum
{
    IVE_IOC_MODE_THRESH_S16_TO_S8_MIN_MID_MAX = 0x00,
    IVE_IOC_MODE_THRESH_S16_TO_S8_MIN_ORI_MAX = 0x01,
    IVE_IOC_MODE_THRESH_S16_TO_U8_MIN_MID_MAX = 0x02,
    IVE_IOC_MODE_THRESH_S16_TO_U8_MIN_ORI_MAX = 0x03,
} IVE_IOC_MODE_THRESH_S16;

typedef enum
{
    IVE_IOC_MODE_THRESH_U16_TO_U8_MIN_MID_MAX = 0x00,
    IVE_IOC_MODE_THRESH_U16_TO_U8_MIN_ORI_MAX = 0x01,
} IVE_IOC_MODE_THRESH_U16;

typedef enum
{
    IVE_IOC_MODE_ADD_ROUNDING = 0x00,
    IVE_IOC_MODE_ADD_CLIPPING = 0x01,
} IVE_IOC_MODE_ADD;

typedef enum
{
    IVE_IOC_MODE_SUB_ABS   = 0x00,
    IVE_IOC_MODE_SUB_SHIFT = 0x01,
} IVE_IOC_MODE_SUB;

typedef enum
{
    IVE_IOC_MODE_S16_TO_S8      = 0x00,
    IVE_IOC_MODE_S16_TO_U8_ABS  = 0x01,
    IVE_IOC_MODE_S16_TO_U8_BIAS = 0x02,
    IVE_IOC_MODE_U16_TO_U8      = 0x03,
} IVE_IOC_MODE_16_TO_8;

typedef enum
{
    IVE_IOC_MODE_INTEGRAL_BOTH       = 0x00,
    IVE_IOC_MODE_INTEGRAL_SUM        = 0x01,
    IVE_IOC_MODE_INTEGRAL_SQUARE_SUM = 0x02
} IVE_IOC_MODE_INTEGRAL;

typedef enum
{
    IVE_IOC_MODE_SAD_BLOCK_4X4   = 0x00,
    IVE_IOC_MODE_SAD_BLOCK_8X8   = 0x01,
    IVE_IOC_MODE_SAD_BLOCK_16X16 = 0x02,
} IVE_IOC_MODE_SAD_BLOCK;

typedef enum
{
    IVE_IOC_MODE_SAD_OUT_CTRL_16BIT_BOTH = 0x00,
    IVE_IOC_MODE_SAD_OUT_CTRL_8BIT_BOTH  = 0x01,
    IVE_IOC_MODE_SAD_OUT_CTRL_16BIT_SAD  = 0x02,
    IVE_IOC_MODE_SAD_OUT_CTRL_8BIT_SAD   = 0x03,
    IVE_IOC_MODE_SAD_OUT_CTRL_THRESH     = 0x04
} IVE_IOC_MODE_SAD_OUT;

typedef enum
{
    IVE_IOC_MODE_LBP_COMP_NORMAL  = 0x00,
    IVE_IOC_MODE_LBP_COMP_ABS     = 0x01,
    IVE_IOC_MODE_LBP_COMP_ABS_MUL = 0x02,
} IVE_IOC_MODE_LBP;

typedef enum
{
    IVE_IOC_CHANNEL_MODE_LBP_C1 = 0x00,
    IVE_IOC_CHANNEL_MODE_LBP_C2 = 0x01,
} IVE_IOC_CHANNEL_MODE_LBP;

typedef enum
{
    IVE_IOC_MODE_MATRIX_TRANSFORM_C1 = 0x00,
    IVE_IOC_MODE_MATRIX_TRANSFORM_C2 = 0x01,
    IVE_IOC_MODE_MATRIX_TRANSFORM_C3 = 0x02
} IVE_IOC_CHANNEL_MODE_MATRIX_TRANSFORM;

typedef enum
{
    IVE_IOC_MODE_MATRIX_TRANSFORM_ROUNDING = 0x00,
    IVE_IOC_MODE_MATRIX_TRANSFORM_CLIPPING = 0x01
} IVE_IOC_CONTROL_MODE_MATRIX_TRANSFORM;

typedef enum
{
    IVE_IOC_MODE_IMAGE_DOT_ROUNDING = 0x00,
    IVE_IOC_MODE_IMAGE_DOT_CLIPPING = 0x01,
} IVE_IOC_MODE_IMAGE_DOT;

typedef enum
{
    IVE_IOC_MODE_CCL_C4 = 0x00,
    IVE_IOC_MODE_CCL_C8 = 0x01,
} IVE_IOC_MODE_CCL;

typedef enum
{
    IVE_IOC_MODE_GMM_ONE_FRAME   = 0x00,
    IVE_IOC_MODE_GMM_TWO_FRAME   = 0x01,
    IVE_IOC_MODE_GMM_THREE_FRAME = 0x02,
    IVE_IOC_MODE_GMM_FOUR_FRAME  = 0x03,
} IVE_IOC_MODE_GMM;

typedef enum
{
    IVE_IOC_MODE_RESIZE_BILINEAR = 0x00,
    IVE_IOC_MODE_RESIZE_AREA     = 0x01,
} IVE_IOC_MODE_RESIZE;

typedef enum
{
    IVE_IOC_MODE_BGBLUR_BLUR            = 0x00,
    IVE_IOC_MODE_BGBLUR_REPLACE         = 0x01,
    IVE_IOC_MODE_BGBLUR_MOSAIC          = 0x02,
    IVE_IOC_MODE_BGBLUR_BLUR_AND_MOSAIC = 0x03,
} IVE_IOC_MODE_BGBLUR;

typedef enum
{
    IVE_IOC_LUT_DISABLE = 0x00,
    IVE_IOC_LUT_ENABLE  = 0x01,
} IVE_IOC_LUT_EN_BGBLUR;

typedef struct
{
    IVE_IOC_IMAGE_FORMAT format;

    u16 width;
    u16 height;
    u64 address[3];
    u16 stride[3];
} ive_ioc_image;

typedef struct
{
    u8 mask[25];
    u8 shift;
} ive_ioc_coeff_filter;

typedef struct
{
    u8 clamp_low;
    u8 clamp_high;
} ive_ioc_coeff_csc_clamp;

typedef struct
{
    u16                     coeff[9];
    u16                     offset[3];
    ive_ioc_coeff_csc_clamp clamp[3];
} ive_ioc_coeff_csc;

typedef struct
{
    ive_ioc_coeff_filter filter;
    ive_ioc_coeff_csc    csc;
} ive_ioc_coeff_filter_csc;

typedef struct
{
    IVE_IOC_MODE_SOBEL mode;
    u8                 mask[25];
} ive_ioc_coeff_sobel;

typedef struct
{
    IVE_IOC_MODE_MAG_AND_ANG mode;
    u8                       mask[25];
    u16                      thresh;
} ive_ioc_coeff_mag_and_ang;

typedef struct
{
    IVE_IOC_MODE_ORD_STAT_FILTER mode;
} ive_ioc_coeff_ord_stat_filter;

typedef struct
{
    IVE_IOC_MODE_BERNSEN mode;
    u16                  thresh;
} ive_ioc_coeff_bernsen;

typedef struct
{
    u8 mask[25];
} ive_ioc_coeff_dilate;

typedef struct
{
    u8 mask[25];
} ive_ioc_coeff_erode;

typedef struct
{
    IVE_IOC_MODE_THRESH mode;
    u16                 low;
    u16                 high;
    u8                  min;
    u8                  mid;
    u8                  max;
} ive_ioc_coeff_thresh;

typedef struct
{
    IVE_IOC_MODE_THRESH_S16 mode;
    u16                     low;
    u16                     high;
    u8                      min;
    u8                      mid;
    u8                      max;
} ive_ioc_coeff_thresh_s16;

typedef struct
{
    IVE_IOC_MODE_THRESH_U16 mode;
    u16                     low;
    u16                     high;
    u8                      min;
    u8                      mid;
    u8                      max;
} ive_ioc_coeff_thresh_u16;

typedef struct
{
    IVE_IOC_MODE_ADD mode;
    u16              weight_x;
    u16              weight_y;
} ive_ioc_coeff_add;

typedef struct
{
    IVE_IOC_MODE_SUB mode;
} ive_ioc_coeff_sub;

typedef struct
{
    IVE_IOC_MODE_16_TO_8 mode;
    u16                  denominator;
    u8                   numerator;
    s8                   bias;
} ive_ioc_coeff_16to8;

typedef struct
{
    u64 map;
} ive_ioc_coeff_map;

typedef struct
{
    IVE_IOC_MODE_INTEGRAL mode;
} ive_ioc_coeff_integral;

typedef struct
{
    IVE_IOC_MODE_SAD_BLOCK block_mode;
    IVE_IOC_MODE_SAD_OUT   out_mode;
    u16                    thresh;
    u8                     min;
    u8                     max;
} ive_ioc_coeff_sad;

typedef struct
{
    u64 numerator;
    u64 sum1;
    u64 sum2;
} ive_ioc_coeff_ncc;

typedef struct
{
    IVE_IOC_MODE_LBP         mode;
    IVE_IOC_CHANNEL_MODE_LBP chlmode;
    u16                      thresh;
} ive_ioc_coeff_lbp;

typedef struct
{
    u16 h_times;
    u16 v_times;
} ive_ioc_coeff_bat;

typedef struct
{
    u8 u8RateThr;
    u8 u8HalfMaskx;
    u8 u8HalfMasky;
    s8 s8Offset;
    u8 u8ValueThr;
} ive_ioc_coeff_adp_thresh;

typedef struct
{
    IVE_IOC_CHANNEL_MODE_MATRIX_TRANSFORM chl_mode;
    IVE_IOC_CONTROL_MODE_MATRIX_TRANSFORM ctrl_mode;
    s32                                   s32MatrixArray[9];
} ive_ioc_coeff_matrix_transform;

typedef struct
{
    IVE_IOC_MODE_IMAGE_DOT mode;
} ive_ioc_coeff_image_dot;

typedef struct
{
    IVE_IOC_MODE_CCL eCclMode;
    u16              u16InitAreaThr;
    u16              u16Step;
} ive_ioc_coeff_ccl;

typedef struct
{
    IVE_IOC_MODE_GMM eFrameMode;
    u8               bRestartLearning; // indicate whether to reset model or not.
    u8               u8ModelNum;       // refer to nmixtures of OpenCV MOG2.
    u16              u16LearnRate;     // refer to learningRate of OpenCV MOG2.
    u16              u16BgRatio;       // background weight ratio threshold which is used to determine background.
    u64 u64MaxVariance; // refer to fVarMax of OpenCV MOG2, indicate the max model variance when updating variance.
    u64 u64MinVariance; // refer to fVarMin of OpenCV MOG2, indicate the min model variance when updating variance.
    u64 u64VarianceThr; // fit variance threshold which is used to detemine if a pixel hits the current model
    u64 u64ModelAddr;
    ive_ioc_image stSrc[4];
    ive_ioc_image stDstFg[4];
    ive_ioc_image stDstBg[4];
} ive_ioc_coeff_gmm;

typedef struct
{
    IVE_IOC_IMAGE_FORMAT eResizeFmt;
    IVE_IOC_MODE_RESIZE  eResizeMode;
} ive_ioc_coeff_resize;

typedef struct
{
    // common.
    IVE_IOC_MODE_BGBLUR eBgBlurMode;
    u8                  u8MaskThr;
    u8                  u8AlphaBlendingVal;

    ive_ioc_image stSrcYMask;
    ive_ioc_image stSrcUvMask;
    ive_ioc_image stSrcRepBg;
    ive_ioc_image stSrcOri;

    union
    {
        // For infinity6f.
        struct
        {
            /*
             * the following images are used in IVE_IOC_MODE_BGBLUR_BLUR mode.
             * stMarkFillImg:      used to store the output of G0 part, G0 part will do marker and fill process.
             * stSmallImg:         used to store the output of G1 part, G1 part will do scaling down.
             * stBlurImg:          used to store the output of G2 part, G2 part will do blur.
             * stAlphaBlendingImg: used to store the output of G3 part, G3 part will do scaling up and alpha
             *                     blending(with stSrcRepBg). do alpha blending with stSrcOri to generate the
             *                     final output.
             */
            ive_ioc_image stMarkFillImg;
            ive_ioc_image stSmallImg;
            ive_ioc_image stBlurImg;
            ive_ioc_image stAlphaBlendingImg;
        };

        // For infinity6d.
        struct
        {
            /*
             * stMidImge:           the multiplying image, used to store the output(of fill/scaling down/blur/scaling
             *                      up/mosaic/alpha blending) of internal Gn part of BGBlur.
             * u16DownWidth/Height: the size of the output of scaling down/blur of internal G1/G2 part.
             * u16FgFillSize:       used in G0 part, indicate the area between Fg and Bg which will be filled with
             *                      nearest Bg pixel.
             * eLutSwitch:          used in G5 part, remap the alpha image pixel into LutVal in alpha blending.
             */
            u8  u8SaturationLv;
            u8  u8MosaicSize;
            u16 u16DownWidth;
            u16 u16DownHeight;
            u16 u16FgFillSize;
            u8  u8LutKey[IVE_BGBLUR_LUT_KEY_NUM];
            u8  u8LutVal[IVE_BGBLUR_LUT_VAL_NUM];

            IVE_IOC_LUT_EN_BGBLUR eLutEn;
            ive_ioc_image         stMidImg;
        };
    };
} ive_ioc_coeff_bgblur;

typedef struct
{
    IVE_IOC_OP_TYPE         op_type;
    IVE_IOC_TASK_PRIORITY_E eTaskPriority;

    ive_ioc_image input;
    ive_ioc_image output;

    union
    {
        ive_ioc_coeff_filter           coeff_filter;
        ive_ioc_coeff_csc              coeff_csc;
        ive_ioc_coeff_filter_csc       coeff_filter_csc;
        ive_ioc_coeff_sobel            coeff_sobel;
        ive_ioc_coeff_mag_and_ang      coeff_mag_and_ang;
        ive_ioc_coeff_ord_stat_filter  coeff_ord_stat_filter;
        ive_ioc_coeff_bernsen          coeff_bernsen;
        ive_ioc_coeff_dilate           coeff_dilate;
        ive_ioc_coeff_erode            coeff_erode;
        ive_ioc_coeff_thresh           coeff_thresh;
        ive_ioc_coeff_thresh_s16       coeff_thresh_s16;
        ive_ioc_coeff_thresh_u16       coeff_thresh_u16;
        ive_ioc_coeff_add              coeff_add;
        ive_ioc_coeff_sub              coeff_sub;
        ive_ioc_coeff_16to8            coeff_16to8;
        ive_ioc_coeff_map              coeff_map;
        ive_ioc_coeff_integral         coeff_integral;
        ive_ioc_coeff_sad              coeff_sad;
        u64                            coeff_ncc;
        ive_ioc_coeff_lbp              coeff_lbp;
        ive_ioc_coeff_bat              coeff_bat;
        ive_ioc_coeff_adp_thresh       coeff_adp_thresh;
        ive_ioc_coeff_matrix_transform coeff_matrix_transform;
        ive_ioc_coeff_image_dot        coeff_image_dot;
        ive_ioc_coeff_ccl              coeff_ccl;
        ive_ioc_coeff_gmm              coeff_gmm;
        ive_ioc_coeff_resize           coeff_resize;
        ive_ioc_coeff_bgblur           coeff_bgblur;
    };
} ive_ioc_config;

#endif //_DRV_IVE_IO_ST_H_
