/*
 * rgn_debug.h - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#ifndef _RGN_DEBUG_H_
#define _RGN_DEBUG_H_

//-----------------------------------------------------------------------------------------------------
// Variable Prototype
//-----------------------------------------------------------------------------------------------------
extern MS_U32 g_u32DebugLevel;

//-----------------------------------------------------------------------------------------------------
// Debug Level
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
// Debug Macro
//-----------------------------------------------------------------------------------------------------
#define PRINT_NONE   "\33[m"
#define PRINT_RED    "\33[1;31m"
#define PRINT_YELLOW "\33[1;33m"
#define PRINT_GREEN  "\33[1;32m"

#define RGN_DBG_EN 0

#if RGN_DBG_EN

#define RGN_ASSERT(_con)                                                         \
    do                                                                           \
    {                                                                            \
        if (!(_con))                                                             \
        {                                                                        \
            CamOsPrintf("BUG at %s:%d assert(%s)\n", __FILE__, __LINE__, #_con); \
            BUG();                                                               \
        }                                                                        \
    } while (0)

#define RGN_ERR(_fmt, _args...)                                                                            \
    do                                                                                                     \
    {                                                                                                      \
        CamOsPrintf(PRINT_RED "RGN_ERR[%s, %d] > " _fmt PRINT_NONE "\n", __FUNCTION__, __LINE__, ##_args); \
    } while (0)

#define RGN_WRN(_fmt, _args...)                                                                               \
    do                                                                                                        \
    {                                                                                                         \
        CamOsPrintf(PRINT_YELLOW "RGN_WRN[%s, %d] > " _fmt PRINT_NONE "\n", __FUNCTION__, __LINE__, ##_args); \
    } while (0)

#define RGN_INFO(_lv, _fmt, _args...)                                                                             \
    do                                                                                                            \
    {                                                                                                             \
        if (g_u32DebugLevel & _lv)                                                                                \
        {                                                                                                         \
            CamOsPrintf(PRINT_GREEN "RGN_INFO[%s, %d] > " _fmt PRINT_NONE "\n", __FUNCTION__, __LINE__, ##_args); \
        }                                                                                                         \
    } while (0)

#define RGN_DBG(_lv, _fmt, _args...)                                                         \
    do                                                                                       \
    {                                                                                        \
        if (g_u32DebugLevel & _lv)                                                           \
        {                                                                                    \
            CamOsPrintf("[RGN_DBG] [%s, %d] > " _fmt "\n", __FUNCTION__, __LINE__, ##_args); \
        }                                                                                    \
    } while (0)
#else

#define RGN_ASSERT(arg)
#define RGN_ERR(_fmt, ...)
#define RGN_WRN(_fmt, ...)
#define RGN_INFO(_fmt, ...)
#define RGN_DBG(_fmt, ...)

#endif

#define RGN_CHECK(_conf, _errcode, _errmsg, _args...) \
    (                                                 \
        {                                             \
            if (!(_conf))                             \
            {                                         \
                RGN_ERR(_errmsg, ##_args);            \
                {                                     \
                    _errcode;                         \
                }                                     \
            }                                         \
        })

//#define RGN_DBG_CHECK(dbglv) (g_u32RgnDbgLevel & dbglv)
//-----------------------------------------------------------------------------------------------------
// Parsing String
//-----------------------------------------------------------------------------------------------------
/*
#define PARSING_HAL_QUERY_TYPE(x)                                             \
    (x == E_HAL_RGN_QUERY_GOP_INFO                  ? "GOP_INFO"              \
     : x == E_HAL_RGN_QUERY_GOP_CREATE              ? "GOP_CREATE"            \
     : x == E_HAL_RGN_QUERY_GOP_DESTORY             ? "GOP_DESTORY"           \
     : x == E_HAL_RGN_QUERY_GOP_CMDQ                ? "GOP_CMDQ"              \
     : x == E_HAL_RGN_QUERY_GOP_ACTIVE              ? "GOP_ACTIVE"            \
     : x == E_HAL_RGN_QUERY_GOP_DEACTIVE            ? "GOP_DEACTIVE"          \
     : x == E_HAL_RGN_QUERY_GOP_PALETTE             ? "PALETTE"               \
     : x == E_HAL_RGN_QUERY_GOP_COLORKEY            ? "COLORKEY"              \
     : x == E_HAL_RGN_QUERY_GOP_STRETCHWINDOW       ? "STRETCHWINDOW"         \
     : x == E_HAL_RGN_QUERY_GOP_MIRRORFLIP          ? "MIRRORFLIP"            \
     : x == E_HAL_RGN_QUERY_GOP_GWIN_PIXELFORMAT    ? "GWIN_PIXELFORMAT"      \
     : x == E_HAL_RGN_QUERY_GOP_GWIN_WINDOW         ? "GWIN_WINDOW"           \
     : x == E_HAL_RGN_QUERY_GOP_GWIN_BUFFER         ? "GWIN_BUFFER"           \
     : x == E_HAL_RGN_QUERY_GOP_GWIN_ONOFF          ? "GWIN_ONOFF"            \
     : x == E_HAL_RGN_QUERY_GOP_GWIN_ALPHA          ? "GWIN_ALPHA"            \
     : x == E_HAL_RGN_QUERY_GOP_GWIN_ARGB1555_ALPHA ? "GOP_ARGB1555_ALPHA"    \
     : x == E_HAL_RGN_QUERY_GOP_ALPHA_ZERO_OPAQUE   ? "GOP_ALPHA_ZERO_OPAQUE" \
     : x == E_HAL_RGN_QUERY_GOP_CSC                 ? "GOP_CSC"               \
     : x == E_HAL_RGN_QUERY_GOP_OUTPUTFORMAT        ? "GOP_OUTPUTFORMAT"      \
     : x == E_HAL_RGN_QUERY_GOP_TIMINGINFO          ? "GOP_TIMINGINFO"        \
     : x == E_HAL_RGN_QUERY_GOP_CLK                 ? "GOP_CLK"               \
     : x == E_HAL_RGN_QUERY_GOP_PROCESS             ? "GOP_PORCESS"           \
     : x == E_HAL_RGN_QUERY_GOP_ENC_MODE            ? "GOP_ENC_MODE"          \
     : x == E_HAL_RGN_QUERY_GOP_PALETTE_TBL         ? "GOP_PALETTE_TBL"       \
     : x == E_HAL_RGN_QUERY_GOP_BRIGHTNESS          ? "GOP_BRIGHTNESS"        \
     : x == E_HAL_RGN_QUERY_GOP_MISC                ? "GOP_MISC"              \
     : x == E_HAL_RGN_QUERY_COVER_INIT              ? "COVER_INIT"            \
     : x == E_HAL_RGN_QUERY_COVER_INIT              ? "COVER_INIT"            \
     : x == E_HAL_RGN_QUERY_COVER_DEINIT            ? "COVER_DEINIT"          \
     : x == E_HAL_RGN_QUERY_COVER_CMDQ              ? "COVER_CMDQ"            \
     : x == E_HAL_RGN_QUERY_COVER_COLOR             ? "COVER_COLOR"           \
     : x == E_HAL_RGN_QUERY_COVER_WINDOW            ? "COVER_WINDOW"          \
     : x == E_HAL_RGN_QUERY_COVER_ONOFF             ? "COVER_ONOFF"           \
     : x == E_HAL_RGN_QUERY_COVER_PROCESS           ? "COVER_PROCESS"         \
     : x == E_HAL_RGN_QUERY_COVER_INFO              ? "COVER_INFO"            \
                                                    : "UNKNOWN")

#define PARSING_HAL_QUERY_RET(x)                                  \
    (x == E_HAL_RGN_QUERY_RET_OK             ? "RET_OK"           \
     : x == E_HAL_RGN_QUERY_RET_CFGERR       ? "RET_CFGERR"       \
     : x == E_HAL_RGN_QUERY_RET_NONEED       ? "RET_NO_NEED"      \
     : x == E_HAL_RGN_QUERY_RET_NOTSUPPORT   ? "RET_NOT_SUPPORT"  \
     : x == E_HAL_RGN_QUERY_RET_IMPLICIT_ERR ? "RET_IMPLICIT_ERR" \
                                             : "UNKNOWN")

#define PARSING_HAL_GOP_TYPE(x)                          \
    (x == E_HAL_RGN_GOP_TYPE_SCL          ? "SCL"        \
     : x == E_HAL_RGN_GOP_TYPE_DISP_UI    ? "DISP_UI"    \
     : x == E_HAL_RGN_GOP_TYPE_DISP_CUR_0 ? "DISP_CUR_0" \
     : x == E_HAL_RGN_GOP_TYPE_DISP_CUR_1 ? "DISP_CUR_1" \
     : x == E_HAL_RGN_GOP_TYPE_LDC        ? "LDC"        \
     : x == E_HAL_RGN_GOP_TYPE_VENC       ? "VENC"       \
     : x == E_HAL_RGN_GOP_TYPE_JPE        ? "JPE"        \
                                          : "UNKNOWN")

#define PARSING_HAL_GOP_CSC(x) \
    (x == E_HAL_GOP_CSC_OFF ? "OFF" : x == E_HAL_GOP_CSC_R2Y ? "R2Y" : x == E_HAL_GOP_CSC_Y2R ? "Y2R" : "UNKNOWN")

#define PARSING_HAL_GOP_ALPHA_TYPE(x) \
    (x == E_HAL_GOP_GWIN_ALPHA_CONSTANT ? "CONSTANT" : x == E_HAL_GOP_GWIN_ALPHA_PIXEL ? "PIXEL" : "UNKNOWN")

#define PARSING_HAL_GOP_ARGB1555_ALPHA_TYPE(x)                 \
    (x == E_HAL_GOP_GWIN_ARGB1555_ALPHA0   ? "ARGB1555_ALPHA0" \
     : x == E_HAL_GOP_GWIN_ARGB1555_ALPHA1 ? "ARGB1555_ALPHA1" \
                                           : "UNKNOWN")

#define PARSING_HAL_GOP_OUTPUT_FMT(x) \
    (x == E_HAL_GOP_OUT_FMT_RGB ? "RGB" : x == E_HAL_GOP_OUT_FMT_YUV ? "YUV" : "UNKNOWN")

#define PARSING_HAL_GOP_PIXEL_FMT(x)                     \
    (x == E_HAL_GOP_PIXEL_FORMAT_ARGB1555   ? "ARGB1555" \
     : x == E_HAL_GOP_PIXEL_FORMAT_ARGB1555 ? "ARGB4444" \
     : x == E_HAL_GOP_PIXEL_FORMAT_I2       ? "I2"       \
     : x == E_HAL_GOP_PIXEL_FORMAT_I4       ? "I4"       \
     : x == E_HAL_GOP_PIXEL_FORMAT_I8       ? "I8"       \
     : x == E_HAL_GOP_PIXEL_FORMAT_RGB565   ? "RGB565"   \
     : x == E_HAL_GOP_PIXEL_FORMAT_ARGB8888 ? "ARGB8888" \
     : x == E_HAL_GOP_PIXEL_FORMAT_UV8Y8    ? "UV8Y8"    \
                                            : "UNKNOWN")

#define PARSING_HAL_GOP_DISPLAY_MD(x) \
    (x == E_HAL_GOP_DISPLAY_MODE_INTERLACE ? "INTERFACE" : x == E_HAL_GOP_DISPLAY_MODE_PROGRESS ? "PROGESS" : "UNKNOWN")
#define PARSING_HAL_GOP_ENC_MODE(x)                                \
    (x == E_HAL_GOP_ENC_MD_VENC_H265_MODE_0   ? "VENC_H265_MODE_0" \
     : x == E_HAL_GOP_ENC_MD_VENC_H265_MODE_1 ? "VENC_H265_MODE_1" \
     : x == E_HAL_GOP_ENC_MD_VENC_H264        ? "VENC_H264"        \
     : x == E_HAL_GOP_ENC_MD_JPE_420          ? "JPE_420"          \
     : x == E_HAL_GOP_ENC_MD_JPE_422          ? "JPE_422"          \
                                              : "UNKNOWN")

#define PARSAING_HAL_CMDQ_MODE(x) \
    (x == E_HAL_RGN_CMDQ_MD_MEM ? "CMDQ_MEM" : x == E_HAL_RGN_CMDQ_MD_NORMAL ? "CMDQ_NORMAL" : "UNKNOWN")
*/

#endif // #ifndef
