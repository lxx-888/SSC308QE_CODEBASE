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
#ifndef _MI_VDEC_DATATYPE_H_
#define _MI_VDEC_DATATYPE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "mi_common.h"
#include "mi_sys_datatype.h"

#define MI_DEF_VDEC_ERR(err) MI_DEF_ERR(E_MI_MODULE_ID_VDEC, E_MI_ERR_LEVEL_ERROR, err)

#define MI_ERR_VDEC_INVALID_DEVID  MI_DEF_VDEC_ERR(E_MI_ERR_INVALID_DEVID)
#define MI_ERR_VDEC_INVALID_CHNID  MI_DEF_VDEC_ERR(E_MI_ERR_INVALID_CHNID)
#define MI_ERR_VDEC_ILLEGAL_PARAM  MI_DEF_VDEC_ERR(E_MI_ERR_ILLEGAL_PARAM)
#define MI_ERR_VDEC_CHN_EXIST      MI_DEF_VDEC_ERR(E_MI_ERR_EXIST)
#define MI_ERR_VDEC_CHN_UNEXIST    MI_DEF_VDEC_ERR(E_MI_ERR_UNEXIST)
#define MI_ERR_VDEC_NULL_PTR       MI_DEF_VDEC_ERR(E_MI_ERR_NULL_PTR)
#define MI_ERR_VDEC_NOT_CONFIG     MI_DEF_VDEC_ERR(E_MI_ERR_NOT_CONFIG)
#define MI_ERR_VDEC_NOT_SUPPORT    MI_DEF_VDEC_ERR(E_MI_ERR_NOT_SUPPORT)
#define MI_ERR_VDEC_NOT_PERM       MI_DEF_VDEC_ERR(E_MI_ERR_NOT_PERM)
#define MI_ERR_VDEC_NOMEM          MI_DEF_VDEC_ERR(E_MI_ERR_NOMEM)
#define MI_ERR_VDEC_NOBUF          MI_DEF_VDEC_ERR(E_MI_ERR_NOBUF)
#define MI_ERR_VDEC_BUF_EMPTY      MI_DEF_VDEC_ERR(E_MI_ERR_BUF_EMPTY)
#define MI_ERR_VDEC_BUF_FULL       MI_DEF_VDEC_ERR(E_MI_ERR_BUF_FULL)
#define MI_ERR_VDEC_SYS_NOTREADY   MI_DEF_VDEC_ERR(E_MI_ERR_SYS_NOTREADY)
#define MI_ERR_VDEC_BADADDR        MI_DEF_VDEC_ERR(E_MI_ERR_BADADDR)
#define MI_ERR_VDEC_BUSY           MI_DEF_VDEC_ERR(E_MI_ERR_BUSY)
#define MI_ERR_VDEC_CHN_NOT_START  MI_DEF_VDEC_ERR(E_MI_ERR_CHN_NOT_STARTED)
#define MI_ERR_VDEC_CHN_NOT_STOP   MI_DEF_VDEC_ERR(E_MI_ERR_CHN_NOT_STOPED)
#define MI_ERR_VDEC_NOT_INIT       MI_DEF_VDEC_ERR(E_MI_ERR_NOT_INIT)
#define MI_ERR_VDEC_INITED         MI_DEF_VDEC_ERR(E_MI_ERR_INITED)
#define MI_ERR_VDEC_NOT_ENABLE     MI_DEF_VDEC_ERR(E_MI_ERR_NOT_ENABLE)
#define MI_ERR_VDEC_NOT_DISABLE    MI_DEF_VDEC_ERR(E_MI_ERR_NOT_DISABLE)
#define MI_ERR_VDEC_SYS_TIMEOUT    MI_DEF_VDEC_ERR(E_MI_ERR_SYS_TIMEOUT)
#define MI_ERR_VDEC_NOT_STARTED    MI_DEF_VDEC_ERR(E_MI_ERR_DEV_NOT_STARTED)
#define MI_ERR_VDEC_NOT_STOPED     MI_DEF_VDEC_ERR(E_MI_ERR_DEV_NOT_STOPED)
#define MI_ERR_VDEC_CHN_NO_CONTENT MI_DEF_VDEC_ERR(E_MI_ERR_CHN_NO_CONTENT)
#define MI_ERR_VDEC_FAILED         MI_DEF_VDEC_ERR(E_MI_ERR_FAILED)

    typedef enum
    {
        E_MI_VDEC_CODEC_TYPE_H264 = 0x0,
        E_MI_VDEC_CODEC_TYPE_H265,
        E_MI_VDEC_CODEC_TYPE_JPEG,
        E_MI_VDEC_CODEC_TYPE_MAX
    } MI_VDEC_CodecType_e;

    typedef enum
    {
        E_MI_VDEC_JPEG_FORMAT_YCBCR400 = 0x0,
        E_MI_VDEC_JPEG_FORMAT_YCBCR420,
        E_MI_VDEC_JPEG_FORMAT_YCBCR422,
        E_MI_VDEC_JPEG_FORMAT_YCBCR444,
        E_MI_VDEC_JPEG_FORMAT_MAX
    } MI_VDEC_JpegFormat_e;

    typedef enum
    {
        E_MI_VDEC_VIDEO_MODE_STREAM = 0x0,
        E_MI_VDEC_VIDEO_MODE_FRAME,
        E_MI_VDEC_VIDEO_MODE_MAX
    } MI_VDEC_VideoMode_e;

    typedef enum
    {
        E_MI_VDEC_ERR_CODE_NONE = 0x0,
        E_MI_VDEC_ERR_CODE_UNKNOWN,
        E_MI_VDEC_ERR_CODE_MB_ERROR,
        E_MI_VDEC_ERR_CODE_REF_FRAME_ERROR,
        E_MI_VDEC_ERR_CODE_REF_FRAME_BUFF_NOT_ENOUGH,
        E_MI_VDEC_ERR_CODE_VCL_NOT_FOUND,
        E_MI_VDEC_ERR_CODE_OVER_PROFILE,
        E_MI_VDEC_ERR_CODE_OVER_LEVEL,
        E_MI_VDEC_ERR_CODE_OVER_MULTISLICE_NUM,
        E_MI_VDEC_ERR_CODE_ILLEGAL_ACCESS,
        E_MI_VDEC_ERR_CODE_FRMRATE_UNSUPPORT,
        E_MI_VDEC_ERR_CODE_DEC_TIMEOUT,
        E_MI_VDEC_ERR_CODE_OUT_OF_MEMORY,
        E_MI_VDEC_ERR_CODE_CODEC_TYPE_UNSUPPORT,
        E_MI_VDEC_ERR_CODE_ERR_SPS_UNSUPPORT,
        E_MI_VDEC_ERR_CODE_ERR_PPS_UNSUPPORT,
        E_MI_VDEC_ERR_CODE_REF_LIST_ERR,
        E_MI_VDEC_ERR_CODE_MAX
    } MI_VDEC_ErrCode_e;

    typedef enum
    {
        E_MI_VDEC_DECODE_MODE_ALL = 0x0,
        E_MI_VDEC_DECODE_MODE_I,
        E_MI_VDEC_DECODE_MODE_IP,
        E_MI_VDEC_DECODE_MODE_MAX
    } MI_VDEC_DecodeMode_e;

    typedef enum
    {
        E_MI_VDEC_OUTPUT_ORDER_DISPLAY = 0x0,
        E_MI_VDEC_OUTPUT_ORDER_DECODE,
        E_MI_VDEC_OUTPUT_ORDER_MAX
    } MI_VDEC_OutputOrder_e;

    typedef enum
    {
        E_MI_VDEC_VIDEO_FORMAT_TILE = 0x0,
        E_MI_VDEC_VIDEO_FORMAT_REDUCE,
        E_MI_VDEC_VIDEO_FORMAT_MAX
    } MI_VDEC_VideoFormat_e;

    typedef enum
    {
        E_MI_VDEC_DISPLAY_MODE_PREVIEW = 0x0,
        E_MI_VDEC_DISPLAY_MODE_PLAYBACK,
        E_MI_VDEC_DISPLAY_MODE_MAX
    } MI_VDEC_DisplayMode_e;

    typedef enum
    {
        E_MI_VDEC_TILEMODE_OFF = 0x0,
        E_MI_VDEC_TILEMODE_ON,
        E_MI_VDEC_TILEMODE_MAX
    } MI_VDEC_TileMode_Status_e;

    typedef enum
    {
        E_MI_VDEC_DPB_MODE_NORMAL          = 0,
        E_MI_VDEC_DPB_MODE_INPLACE_ONE_BUF = 1,
        E_MI_VDEC_DPB_MODE_INPLACE_TWO_BUF = 2,
        E_MI_VDEC_DPB_MODE_MAX
    } MI_VDEC_DPB_BufMode_e;

    typedef enum
    {
        E_MI_VDEC_OUTBUF_LAYOUT_AUTO = 0x0,
        E_MI_VDEC_OUTBUF_LAYOUT_LINEAR,
        E_MI_VDEC_OUTBUF_LAYOUT_TILE,
        E_MI_VDEC_OUTBUF_LAYOUT_MAX
    } MI_VDEC_OutbufLayoutMode_e;

    typedef enum
    {
        E_MI_VDEC_EVENT_SEQCHANGE = 0,
        E_MI_VDEC_EVENT_EOS,
        E_MI_VDEC_EVENT_DECERR,
        E_MI_VDEC_EVENT_FRAME_DROPPED,
        E_MI_VDEC_EVENT_MAX = 64,
    } MI_VDEC_Event_e;

    typedef struct MI_VDEC_InitParam_s
    {
        MI_U16 u16MaxWidth;
        MI_U16 u16MaxHeight;
    } MI_VDEC_InitParam_t;

    typedef struct MI_VDEC_OutputPortAttr_s
    {
        MI_U16 u16Width;  // Width of target image
        MI_U16 u16Height; // Height of target image
    } MI_VDEC_OutputPortAttr_t;

    typedef struct MI_VDEC_JpegAttr_s
    {
        MI_VDEC_JpegFormat_e eJpegFormat;
    } MI_VDEC_JpegAttr_t;

    typedef struct MI_VDEC_ErrHandlePolicy_s
    {
        MI_BOOL bUseCusPolicy; // FALSE: use default; TRUE: use customized value
        MI_U8   u8ErrMBPercentThreshold;
    } MI_VDEC_ErrHandlePolicy_t;

    typedef struct MI_VDEC_VideoAttr_s
    {
        MI_U32                    u32RefFrameNum;
        MI_VDEC_ErrHandlePolicy_t stErrHandlePolicy;
        MI_BOOL                   bDisableLowLatency;
    } MI_VDEC_VideoAttr_t;

    typedef struct MI_VDEC_ChnAttr_s
    {
        MI_VDEC_CodecType_e   eCodecType;
        MI_U32                u32BufSize;
        MI_U32                u32Priority;
        MI_U32                u32PicWidth;
        MI_U32                u32PicHeight;
        MI_VDEC_VideoMode_e   eVideoMode;
        MI_VDEC_DPB_BufMode_e eDpbBufMode;
        union
        {
            MI_VDEC_JpegAttr_t  stVdecJpegAttr;
            MI_VDEC_VideoAttr_t stVdecVideoAttr;
        };
    } MI_VDEC_ChnAttr_t;

    typedef struct MI_VDEC_ChnStat_s
    {
        MI_VDEC_CodecType_e eCodecType;
        MI_U32              u32LeftStreamBytes;
        MI_U32              u32LeftStreamFrames;
        MI_U32              u32LeftPics;
        MI_U32              u32RecvStreamFrames;
        MI_U32              u32DecodeStreamFrames;
        MI_BOOL             bChnStart;
        MI_VDEC_ErrCode_e   eErrCode;
        MI_U16              u16DecodeWidth;
        MI_U16              u16DecodeHeight;
    } MI_VDEC_ChnStat_t;

    typedef struct MI_VDEC_ChnParam_s
    {
        MI_VDEC_DecodeMode_e  eDecMode;
        MI_VDEC_OutputOrder_e eOutputOrder;
        MI_VDEC_VideoFormat_e eVideoFormat;
    } MI_VDEC_ChnParam_t;

    typedef struct MI_VDEC_VideoStream_s
    {
        union
        {
            MI_U8*   pu8Addr;
            MI_PTR64 u64Reserved;
        };
        MI_U32  u32Len;
        MI_U64  u64PTS;
        MI_BOOL bEndOfFrame;
        MI_BOOL bEndOfStream;
    } MI_VDEC_VideoStream_t;

    typedef struct MI_VDEC_UserData_s
    {
        union
        {
            MI_U8*   pu8Addr;
            MI_PTR64 u64Reserved;
        };
        MI_U32  u32Len;
        MI_BOOL bValid;
    } MI_VDEC_UserData_t;

    typedef struct MI_VDEC_CropCfg_s
    {
        MI_BOOL             bEnable; /* Crop region enable */
        MI_SYS_WindowRect_t stRect;  /* Crop region */
    } MI_VDEC_CropCfg_t;

    typedef struct MI_VDEC_SeqChgEvtInfo_s
    {
        MI_U8               u8RefFrameNum;
        MI_U16              u16PicWidth;
        MI_U16              u16PicHeight;
        MI_SYS_WindowRect_t stCropInfo;
    } MI_VDEC_SeqChgEvtInfo_t;

    typedef struct MI_VDEC_DecErrEvtInfo_s
    {
        MI_VDEC_ErrCode_e eErrCode;
    } MI_VDEC_DecErrEvtInfo_t;

    typedef struct MI_VDEC_DecEosEvtInfo_s
    {
        MI_BOOL bAbnormal;
    } MI_VDEC_DecEosEvtInfo_t;

    typedef struct MI_VDEC_FrameDroppedEvtInfo_s
    {
        MI_BOOL bRemainDroppedData;
        MI_U8   u8FrameDroppedCnt;
        MI_U64  au64FrameDroppedPts[2];
    } MI_VDEC_FrameDroppedEvtInfo_t;

    typedef struct MI_VDEC_EventData_s
    {
        union
        {
            MI_VDEC_SeqChgEvtInfo_t       stSeqChgEvtInfo;
            MI_VDEC_DecErrEvtInfo_t       stDecErrEvtInfo;
            MI_VDEC_DecEosEvtInfo_t       stDecEosEvtInfo;
            MI_VDEC_FrameDroppedEvtInfo_t stFrameDroppedEvtInfo;
        } value;
    } MI_VDEC_EventData_t;

#ifdef __cplusplus
}
#endif

#endif ///_MI_VDEC_DATATYPE_H_
