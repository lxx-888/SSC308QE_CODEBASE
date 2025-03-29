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

#ifndef __MI_HVP_DATATYPE_H__
#define __MI_HVP_DATATYPE_H__

#include "mi_common_datatype.h"
#include "mi_sys_datatype.h"

typedef enum
{
    E_MI_HVP_SRC_TYPE_HDMI,
    E_MI_HVP_SRC_TYPE_DMA
} MI_HVP_SourceType_e;

typedef enum
{
    E_MI_HVP_COLOR_FORMAT_RGB444,
    E_MI_HVP_COLOR_FORMAT_YUV444,
    E_MI_HVP_COLOR_FORMAT_YUV422,
    E_MI_HVP_COLOR_FORMAT_YUV420,
    E_MI_HVP_COLOR_FORMAT_MAX
} MI_HVP_ColorFormat_e;

typedef enum
{
    E_MI_HVP_COLOR_DEPTH_8,
    E_MI_HVP_COLOR_DEPTH_10,
    E_MI_HVP_COLOR_DEPTH_12,
    E_MI_HVP_COLOR_DEPTH_MAX
} MI_HVP_ColorDepth_e;

typedef enum
{
    E_MI_HVP_SIGNAL_STABLE,
    E_MI_HVP_SIGNAL_UNSTABLE
} MI_HVP_SignalStatus_e;

typedef enum
{
    E_MI_HVP_PIX_REP_TYPE_1X,
    E_MI_HVP_PIX_REP_TYPE_2X,
    E_MI_HVP_PIX_REP_TYPE_3X,
    E_MI_HVP_PIX_REP_TYPE_4X,
    E_MI_HVP_PIX_REP_TYPE_MAX
} MI_HVP_PixelRepetitionType_e;

typedef struct MI_HVP_DeviceParam_s
{
    MI_HVP_SourceType_e enSrcType;
} MI_HVP_DeviceAttr_t;

typedef struct MI_HVP_SourceParam_s
{
    MI_HVP_PixelRepetitionType_e enPixRepType;
    MI_HVP_ColorFormat_e         enInputColor;
    MI_HVP_ColorDepth_e          enColorDepth;
    MI_SYS_WindowRect_t          stCropWin;
} MI_HVP_SrcParam_t;

typedef enum
{
    E_MI_HVP_FRC_MODE_FBL,
    E_MI_HVP_FRC_MODE_RATIO,
    E_MI_HVP_FRC_MODE_LOCK_OUT
} MI_HVP_FrcMode_e;

typedef struct MI_HVP_FrcFblModeConfig_s
{
    MI_U32 u32Reserve;
} MI_HVP_FrcFblModeConfig_t;

typedef struct MI_HVP_PqBufModeConfig_s
{
    MI_HVP_ColorFormat_e  eDmaColor;
    MI_U16                u16BufMaxCount;
    MI_U16                u16BufMaxWidth;
    MI_U16                u16BufMaxHeight;
    MI_SYS_CompressMode_e eBufCompressMode;
    MI_SYS_FieldType_e    eFieldType;
} MI_HVP_PqBufModeConfig_t;

typedef struct MI_HVP_ChannelAttr_s
{
    MI_HVP_FrcMode_e enFrcMode;
    union
    {
        MI_HVP_FrcFblModeConfig_t stFrcFblModeConfig;
        MI_HVP_PqBufModeConfig_t  stPqBufModeConfig;
    };
} MI_HVP_ChannelAttr_t;

typedef struct MI_HVP_DstParam_s
{
    MI_HVP_ColorFormat_e enColor;
    MI_BOOL              bMirror;
    MI_BOOL              bFlip;
    MI_U16               u16Fpsx100;
    MI_U16               u16Width;
    MI_U16               u16Height;
    MI_SYS_WindowRect_t  stDispWin;
    MI_SYS_WindowRect_t  stCropWin; // dipsWin --> Crop
} MI_HVP_DstParam_t;

typedef struct MI_HVP_ChannelParam_s
{
    MI_HVP_SrcParam_t stChnSrcParam;
    MI_HVP_DstParam_t stChnDstParam;
} MI_HVP_ChannelParam_t;

typedef struct MI_HVP_SignalTiming_s
{
    MI_BOOL bInterlace;
    MI_U16  u16Fpsx100;
    MI_U16  u16Width;
    MI_U16  u16Height;
} MI_HVP_SignalTiming_t;

#define MI_DEF_HVP_ERR(err) MI_DEF_ERR(E_MI_MODULE_ID_HVP, E_MI_ERR_LEVEL_ERROR, err)

/* invalid device ID */
#define MI_ERR_HVP_INVALID_DEVID MI_DEF_HVP_ERR(E_MI_ERR_INVALID_DEVID)
/* invalid channel ID */
#define MI_ERR_HVP_INVALID_CHNID MI_DEF_HVP_ERR(E_MI_ERR_INVALID_CHNID)
/* at lease one parameter is illegal, e.g, an illegal enumeration value  */
#define MI_ERR_HVP_ILLEGAL_PARAM MI_DEF_HVP_ERR(E_MI_ERR_ILLEGAL_PARAM)
/* channel exists */
#define MI_ERR_HVP_EXIST MI_DEF_HVP_ERR(E_MI_ERR_EXIST)
/*UN exist*/
#define MI_ERR_HVP_UNEXIST MI_DEF_HVP_ERR(E_MI_ERR_UNEXIST)
/* using a NULL point */
#define MI_ERR_HVP_NULL_PTR MI_DEF_HVP_ERR(E_MI_ERR_NULL_PTR)
/* try to enable or initialize system,device or channel, before configuring attribute */
#define MI_ERR_HVP_NOT_CONFIG MI_DEF_HVP_ERR(E_MI_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define MI_ERR_HVP_NOT_SUPPORT MI_DEF_HVP_ERR(E_MI_ERR_NOT_SUPPORT)
/* operation is not permitted, e.g, try to change static attribute */
#define MI_ERR_HVP_NOT_PERM MI_DEF_HVP_ERR(E_MI_ERR_NOT_PERM)
/* failure caused by malloc memory */
#define MI_ERR_HVP_NOMEM MI_DEF_HVP_ERR(E_MI_ERR_NOMEM)
/* failure caused by malloc buffer */
#define MI_ERR_HVP_NOBUF MI_DEF_HVP_ERR(E_MI_ERR_NOBUF)
/* no data in buffer */
#define MI_ERR_HVP_BUF_EMPTY MI_DEF_HVP_ERR(E_MI_ERR_BUF_EMPTY)
/* no buffer for new data */
#define MI_ERR_HVP_BUF_FULL MI_DEF_HVP_ERR(E_MI_ERR_BUF_FULL)
/* System is not ready,maybe not initialed or loaded.
 * Returning the error code when opening a device file failed.
 */
#define MI_ERR_HVP_NOTREADY MI_DEF_HVP_ERR(E_MI_ERR_SYS_NOTREADY)

/* bad address, e.g. used for copy_from_user & copy_to_user */
#define MI_ERR_HVP_BADADDR MI_DEF_HVP_ERR(E_MI_ERR_BADADDR)
/* resource is busy, e.g. destroy a HVP channel without unregistering it */
#define MI_ERR_HVP_BUSY MI_DEF_HVP_ERR(E_MI_ERR_BUSY)

/* channel not start*/
#define MI_ERR_HVP_CHN_NOT_STARTED MI_DEF_HVP_ERR(E_MI_ERR_CHN_NOT_STARTED)
/* channel not stop*/
#define MI_ERR_HVP_CHN_NOT_STOPPED MI_DEF_HVP_ERR(E_MI_ERR_CHN_NOT_STOPED)
/* to be removed later */
#define MI_ERR_HVP_UNDEFINED MI_DEF_HVP_ERR(E_MI_ERR_FAILED)

#endif
