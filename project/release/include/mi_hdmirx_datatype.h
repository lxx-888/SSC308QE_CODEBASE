/* SigmaStar trade secret */
/* Copyright (c) [2022~2023] SigmaStar Technology.
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
#ifndef _MI_HDMI_RX_DATATYPE_H_
#define _MI_HDMI_RX_DATATYPE_H_

#include "mi_sys_datatype.h"
//=============================================================================
// Macro definition
//=============================================================================
#define MI_HDMIRX_RET_SUCCESS     0
#define MI_ERR_HDMIRX_NOT_INIT    MI_DEF_ERR(E_MI_MODULE_ID_HDMIRX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_HDMIRX_NOT_INIT)
#define MI_ERR_HDMIRX_DEV_INVALID MI_DEF_ERR(E_MI_MODULE_ID_HDMIRX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_HDMIRX_INVALID_DEVID)
#define MI_ERR_HDMIRX_PORT_INVALID \
    MI_DEF_ERR(E_MI_MODULE_ID_HDMIRX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_HDMIRX_INVALID_PORTID)
#define MI_ERR_HDMIRX_PARAM_INVALID \
    MI_DEF_ERR(E_MI_MODULE_ID_HDMIRX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_HDMIRX_ILLEGAL_PARAM)
#define MI_ERR_HDMIRX_NOT_SUPPORT MI_DEF_ERR(E_MI_MODULE_ID_HDMIRX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_HDMIRX_NOT_SUPPORT)
#define MI_ERR_HDMIRX_TIMEOUT     MI_DEF_ERR(E_MI_MODULE_ID_HDMIRX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_HDMIRX_TIMEOUT)
#define MI_ERR_HDMIRX_NULL_PTR    MI_DEF_ERR(E_MI_MODULE_ID_HDMIRX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_HDMIRX_NULL_PTR)
#define MI_ERR_HDMIRX_HAS_CONNECTED \
    MI_DEF_ERR(E_MI_MODULE_ID_HDMIRX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_HDMIRX_HAS_CONNECTED)
#define MI_ERR_HDMIRX_HDCPKEY_INVALID \
    MI_DEF_ERR(E_MI_MODULE_ID_HDMIRX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_HDMIRX_HDCPKEY_INVALID)
#define MI_ERR_HDMIRX_FAILED MI_DEF_ERR(E_MI_MODULE_ID_HDMIRX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_HDMIRX_FAILED)
//=============================================================================
// Data type definition
//=============================================================================

typedef enum
{
    E_MI_ERR_HDMIRX_NOT_INIT        = 1,
    E_MI_ERR_HDMIRX_INVALID_DEVID   = 2,
    E_MI_ERR_HDMIRX_INVALID_PORTID  = 3,
    E_MI_ERR_HDMIRX_ILLEGAL_PARAM   = 4,
    E_MI_ERR_HDMIRX_TIMEOUT         = 5,
    E_MI_ERR_HDMIRX_NULL_PTR        = 6,
    E_MI_ERR_HDMIRX_HAS_CONNECTED   = 7,
    E_MI_ERR_HDMIRX_HDCPKEY_INVALID = 8,
    E_MI_ERR_HDMIRX_FAILED          = 9,
    E_MI_ERR_HDMIRX_NOT_SUPPORT     = 10,
    E_MI_ERR_HDMIRX_MAX
} MI_HDMIRX_ErrCode_e;

typedef enum
{
    E_MI_HDMIRX_PORT0 = 0,
    E_MI_HDMIRX_PORT_MAX
} MI_HDMIRX_PortId_e;

typedef struct MI_HDMIRX_Edid_s
{
    MI_U32 u32EdidLength;
    MI_U64 u64EdidDataVirAddr;
} MI_HDMIRX_Edid_t;

typedef struct MI_HDMIRX_Hdcp_s
{
    MI_U32 u32HdcpLength;
    MI_U64 u64HdcpDataVirAddr;
} MI_HDMIRX_Hdcp_t;

typedef enum
{
    E_MI_HDMIRX_EQ_AUTO,
    E_MI_HDMIRX_EQ_MANUAL,
    E_MI_HDMIRX_EQ_MAX
} MI_HDMIRX_EQModeType_e;

typedef struct MI_HDMIRX_EQAttr_s
{
    MI_HDMIRX_EQModeType_e eEQMode;
    MI_U8                  u8ChnEqStrength[3];
} MI_HDMIRX_EQAttr_t;

/**Defines color space enum*/
typedef enum
{
    E_MI_HDMIRX_COLOR_SPACE_UNKNOWN = 0,
    E_MI_HDMIRX_COLOR_SPACE_BT601, /* ::::Current Used:::: BT.601  */
    E_MI_HDMIRX_COLOR_SPACE_BT709, /* ::::Current Used:::: BT.709  */
    E_MI_HDMIRX_COLOR_SPACE_MAX
} MI_HDMIRX_ColorSpace_e;

typedef enum
{
    E_MI_HDMIRX_PIXEL_BITWIDTH_8BIT = 0,
    E_MI_HDMIRX_PIXEL_BITWIDTH_10BIT,
    E_MI_HDMIRX_PIXEL_BITWIDTH_12BIT,
    E_MI_HDMIRX_PIXEL_BITWIDTH_16BIT,
    E_MI_HDMIRX_PIXEL_BITWIDTH_MAX,
} MI_HDMIRX_PixelBitWidth_e;

typedef enum
{
    E_MI_HDMIRX_OVERSAMPLE_1X = 0,
    E_MI_HDMIRX_OVERSAMPLE_2X,
    E_MI_HDMIRX_OVERSAMPLE_3X,
    E_MI_HDMIRX_OVERSAMPLE_4X,
    E_MI_HDMIRX_OVERSAMPLE_MAX,
} MI_HDMIRX_OverSampleMode_e;

typedef enum
{
    E_MI_HDMIRX_PIXEL_FORMAT_RGB = 0,
    E_MI_HDMIRX_PIXEL_FORMAT_YUV444,
    E_MI_HDMIRX_PIXEL_FORMAT_YUV422,
    E_MI_HDMIRX_PIXEL_FORMAT_YUV420,
    E_MI_HDMIRX_PIXEL_FORMAT_MAX,
} MI_HDMIRX_PixelFormat_e;

/**Defines 3D frame packing type*/
typedef enum
{
    E_MI_HDMIRX_FRAME_PACKING_TYPE_NONE,            /**< Normal frame, not a 3D frame */
    E_MI_HDMIRX_FRAME_PACKING_TYPE_SIDE_BY_SIDE,    /**< Side by side */
    E_MI_HDMIRX_FRAME_PACKING_TYPE_TOP_AND_BOTTOM,  /**< Top and bottom */
    E_MI_HDMIRX_FRAME_PACKING_TYPE_TIME_INTERLACED, /**< Time interlaced: one frame for left eye, the next frame for
                                                       right eye */
    E_MI_HDMIRX_FRAME_PACKING_TYPE_FRAME_PACKING,   /**< frame packing */
    E_MI_HDMIRX_FRAME_PACKING_TYPE_3D_TILE,         /**< Tile 3D */
    E_MI_HDMIRX_FRAME_PACKING_TYPE_MAX
} MI_HDMIRX_FramePackingType_e;

typedef enum
{
    E_MI_HDMIRX_QUANTIZATION_DEFAULT,
    E_MI_HDMIRX_QUANTIZATION_LIMIT_RANGE,
    E_MI_HDMIRX_QUANTIZATION_FULL_RANGE,
    E_MI_HDMIRX_QUANTIZATION_RANGE_MAX
} MI_HDMIRX_QuantRange_e;

typedef struct MI_HDMIRX_TimingInfo_s
{
    MI_U32                       u32Width;     /**<HDMIRX Hactive*/
    MI_U32                       u32Height;    /**<HDMIRX Vactive*/
    MI_U32                       u32FrameRate; /**<HDMIRX frequency of vsync*/
    MI_HDMIRX_ColorSpace_e       eColorSpace;  /**<HDMIRX color space*/
    MI_HDMIRX_PixelFormat_e      ePixelFmt;    /**<HDMIRX video formate*/
    MI_HDMIRX_PixelBitWidth_e    eBitWidth;    /**<HDMIRX video BitWidth*/
    MI_BOOL                      bInterlace;   /**<HDMIRX video interlace or progressive*/
    MI_HDMIRX_OverSampleMode_e   eOverSample;  /**<HDMIRX video Oversample*/
    MI_HDMIRX_FramePackingType_e e3dFmt;       /**<HDMIRX video 3D formate*/
    MI_BOOL                      bHdmiMode;    /**<HDMIRX video HDMI or DVI*/
    MI_U32                 u32Vblank; /**<HDMIRX video Vblank the value must set when the 3D formate is Frame PACKING*/
    MI_BOOL                bPcMode;   /**<HDMIRX PC Timing Mode flag*/
    MI_HDMIRX_QuantRange_e eQuantRange;  /**<HDMIRX  Quantization Range*/
    MI_U32                 u32TimingIdx; /**<HDMIRX Timing Index Table*/
    MI_BOOL                bMHL;         /**<HDMIRX MHL flag*/
} MI_HDMIRX_TimingInfo_t;

typedef enum
{
    E_MI_HDMIRX_SIG_SUPPORT = 0, /**<Stable signal*/
    E_MI_HDMIRX_SIG_NO_SIGNAL,   /**<No signal*/
    E_MI_HDMIRX_SIG_NOT_SUPPORT, /**<Not support the signal*/
    E_MI_HDMIRX_SIG_UNSTABLE,    /**<Unstable signal*/
    E_MI_HDMIRX_SIG_MAX          /**<Invalid value*/
} MI_HDMIRX_SigStatus_e;

//*************************************************************CEC
#define CEC_LOGIC_ADDR_DEVICE_TV          0
#define CEC_LOGIC_ADDR_RECORDING_DEVICE_1 1
#define CEC_LOGIC_ADDR_RECORDING_DEVICE_2 2
#define CEC_LOGIC_ADDR_TUNER_1            3
#define CEC_LOGIC_ADDR_PLAYBACK_DEVICE_1  4
#define CEC_LOGIC_ADDR_AUDIO_SYSTEM       5
#define CEC_LOGIC_ADDR_TUNER_2            6
#define CEC_LOGIC_ADDR_TUNER_3            7
#define CEC_LOGIC_ADDR_PLAYBACK_DEVICE_2  8
#define CEC_LOGIC_ADDR_RECORDING_DEVICE_3 9
#define CEC_LOGIC_ADDR_TUNER_4            10
#define CEC_LOGIC_ADDR_PLAYBACK_DEVICE_4  11
#define CEC_LOGIC_ADDR_RESERVED_1         12
#define CEC_LOGIC_ADDR_RESERVED_2         13
#define CEC_LOGIC_ADDR_FREE_USE           14
#define CEC_LOGIC_ADDR_UNREGISTER         15

typedef struct MI_HDMIRX_CecMessage_s
{
    MI_U8 u8Opdata[16];
    MI_U8 u8Count;
} MI_HDMIRX_CecMessage_t;

typedef MI_S32 (*MI_HDMIRX_CEC_CALL_BACK)(MI_HDMIRX_PortId_e ePortId, MI_HDMIRX_CecMessage_t *pMessageData,
                                          void *pUsrParam);

//*************************************************************CEC

#endif ///_MI_HDMI_RX_DATATYPE_H_
