/*
 * mi_disp_impl_datatype.h- Sigmastar
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
#ifndef _MI_DISP_IMPL_DATATYPE_H_
#define _MI_DISP_IMPL_DATATYPE_H_

#include "mi_disp.h"
//#include "mi_sys_internal.h"
//#include "cam_os_wrapper.h"
//#include "cam_os_util.h"
//#include "cam_os_util_list.h"

#include "mhal_common.h"
#include "mhal_cmdq.h"
//#include "drv_miu.h"

#define MI_DISP_FRC_INVALID_VALUE (-1)
#define MI_DISP_DEV_MAX           (3)
#define MI_WBC_DEV_MAX            (1)
#define MI_DISP_LAYER_MAX         (16)
#define MI_DISP_DEV_CHN_MAX       (66)
#define STRING_LEN                (128)
#define UNSIGNED_DIFF(a, b)       (((a) >= (b)) ? ((a) - (b)) : ((b) - (a)))
#define TIME_OUT_US               (2000000UL)
#define TIME_OUT_MS               (10)
#define NOTIFYSYNC_TIME_OUT_MS    (2000)
#define INVALID_ID                (0xff)

//#define MI_DISP_GetDevCtxSem(_pstDevRes)         CamOsTsemDown(&_pstDevRes->stDevResSem)
//#define MI_DISP_ReleaseDevCtxSem(_pstDevRes)     CamOsTsemUp(&_pstDevRes->stDevResSem)
#define MI_DISP_GetLayerCtxSem(_pstLayerRes)     CamOsTsemDown(&_pstLayerRes->stLayerResSem)
#define MI_DISP_ReleaseLayerCtxSem(_pstLayerRes) CamOsTsemUp(&_pstLayerRes->stLayerResSem)
#define MI_DISP_GetChnCtxSem(_pstChnRes)         CamOsTsemDown(&_pstChnRes->stChnResSem)
#define MI_DISP_ReleaseChnCtxSem(_pstChnRes)     CamOsTsemUp(&_pstChnRes->stChnResSem)
#define MI_DISP_GetWbcResSem(_pstWbcRes)         CamOsTsemDown(&_pstWbcRes->stWbcResSem)
#define MI_DISP_ReleaseWbcResSem(_pstWbcRes)     CamOsTsemUp(&_pstWbcRes->stWbcResSem)

#define PARSING_DISP_INTERFACE(x)                    \
    (x == E_MI_DISP_INTF_CVBS         ? "CVBS"       \
     : x == E_MI_DISP_INTF_VGA        ? "VGA"        \
     : x == E_MI_DISP_INTF_BT656      ? "BT656"      \
     : x == E_MI_DISP_INTF_BT601      ? "BT601"      \
     : x == E_MI_DISP_INTF_BT1120     ? "BT1120"     \
     : x == E_MI_DISP_INTF_BT1120_DDR ? "BT1120_DDR" \
     : x == E_MI_DISP_INTF_MCU        ? "MCU"        \
     : x == E_MI_DISP_INTF_MCU_NOFLM  ? "MCU_NOFLM"  \
     : x == E_MI_DISP_INTF_HDMI       ? "HDMI"       \
     : x == E_MI_DISP_INTF_LCD        ? "LCD"        \
     : x == E_MI_DISP_INTF_TTL        ? "TTL"        \
     : x == E_MI_DISP_INTF_MIPIDSI    ? "MIPI_DSI"   \
     : x == E_MI_DISP_INTF_MIPIDSI1   ? "MIPI_DSI1"  \
     : x == E_MI_DISP_INTF_TTL_SPI_IF ? "TTL_SPI"    \
     : x == E_MI_DISP_INTF_SRGB       ? "SRGB"       \
     : x == E_MI_DISP_INTF_LVDS       ? "LVDS"       \
     : x == E_MI_DISP_INTF_LVDS1      ? "LVDS1"      \
     : x == E_MI_DISP_INTF_DUAL_LVDS  ? "DUAL_LVDS"  \
                                      : "UNKNOWN")

#define PARSING_DISP_OUTPUT_TIMING(x)                           \
    (x == E_MI_DISP_OUTPUT_PAL              ? "PAL"             \
     : x == E_MI_DISP_OUTPUT_NTSC           ? "NTSC"            \
     : x == E_MI_DISP_OUTPUT_480i60         ? "480I60"          \
     : x == E_MI_DISP_OUTPUT_576i50         ? "576I50"          \
     : x == E_MI_DISP_OUTPUT_480P60         ? "480P60"          \
     : x == E_MI_DISP_OUTPUT_576P50         ? "576P50"          \
     : x == E_MI_DISP_OUTPUT_720P50         ? "720P50"          \
     : x == E_MI_DISP_OUTPUT_720I50         ? "720I50"          \
     : x == E_MI_DISP_OUTPUT_720P60         ? "720P60"          \
     : x == E_MI_DISP_OUTPUT_720I60         ? "720I60"          \
     : x == E_MI_DISP_OUTPUT_1080P24        ? "1080P24"         \
     : x == E_MI_DISP_OUTPUT_1080P25        ? "1080P25"         \
     : x == E_MI_DISP_OUTPUT_1080P30        ? "1080P30"         \
     : x == E_MI_DISP_OUTPUT_1080I25        ? "1080I25"         \
     : x == E_MI_DISP_OUTPUT_1080I30        ? "1080I30"         \
     : x == E_MI_DISP_OUTPUT_1080I50        ? "1080I50"         \
     : x == E_MI_DISP_OUTPUT_1080I60        ? "1080I60"         \
     : x == E_MI_DISP_OUTPUT_1080P50        ? "1080P50"         \
     : x == E_MI_DISP_OUTPUT_1080P60        ? "1080P60"         \
     : x == E_MI_DISP_OUTPUT_640x480_60     ? "640x480P60"      \
     : x == E_MI_DISP_OUTPUT_800x600_60     ? "800x600P60"      \
     : x == E_MI_DISP_OUTPUT_1024x768_60    ? "1024x768P60"     \
     : x == E_MI_DISP_OUTPUT_1280x1024_60   ? "1280x1024P60"    \
     : x == E_MI_DISP_OUTPUT_1366x768_60    ? "1366x768P60"     \
     : x == E_MI_DISP_OUTPUT_1440x900_60    ? "1440x900P60"     \
     : x == E_MI_DISP_OUTPUT_1280x800_60    ? "1280x800P60"     \
     : x == E_MI_DISP_OUTPUT_1680x1050_60   ? "1680x1050P60"    \
     : x == E_MI_DISP_OUTPUT_1920x2160_30   ? "1920x2160P30"    \
     : x == E_MI_DISP_OUTPUT_1600x1200_60   ? "1600x1200P60"    \
     : x == E_MI_DISP_OUTPUT_1920x1200_60   ? "1920x1200P60"    \
     : x == E_MI_DISP_OUTPUT_2560x1440_30   ? "2560x1440P30"    \
     : x == E_MI_DISP_OUTPUT_2560x1600_60   ? "2560x1600P60"    \
     : x == E_MI_DISP_OUTPUT_3840x2160_30   ? "3840x2160P30"    \
     : x == E_MI_DISP_OUTPUT_3840x2160_60   ? "3840x2160P60"    \
     : x == E_MI_DISP_OUTPUT_1920x1080_5994 ? "1920x1080P59.94" \
     : x == E_MI_DISP_OUTPUT_1920x1080_2997 ? "1920x1080P29.97" \
     : x == E_MI_DISP_OUTPUT_1280x720_5994  ? "1280x720P59.94"  \
     : x == E_MI_DISP_OUTPUT_1280x720_2997  ? "1280x720P29.97"  \
     : x == E_MI_DISP_OUTPUT_720P24         ? "720P24"          \
     : x == E_MI_DISP_OUTPUT_720P25         ? "720P25"          \
     : x == E_MI_DISP_OUTPUT_720I25         ? "720I25"          \
     : x == E_MI_DISP_OUTPUT_720P30         ? "720P30"          \
     : x == E_MI_DISP_OUTPUT_720I30         ? "720I30"          \
     : x == E_MI_DISP_OUTPUT_1920x1080_2398 ? "1920x1080@23.98" \
     : x == E_MI_DISP_OUTPUT_2560x1440_50   ? "2560x1440@50"    \
     : x == E_MI_DISP_OUTPUT_2560x1440_60   ? "2560x1440@60"    \
     : x == E_MI_DISP_OUTPUt_3840x2160_24   ? "3840x2160@24"    \
     : x == E_MI_DISP_OUTPUT_3840x2160_25   ? "3840x2160@25"    \
     : x == E_MI_DISP_OUTPUT_3840x2160_2997 ? "3840x2160@29.97" \
     : x == E_MI_DISP_OUTPUT_848x480_60     ? "848x480@60"      \
     : x == E_MI_DISP_OUTPUT_1280x768_60    ? "1280x768@60"     \
     : x == E_MI_DISP_OUTPUT_1280x960_60    ? "1280x960@60"     \
     : x == E_MI_DISP_OUTPUT_1360x768_60    ? "1360x768@60"     \
     : x == E_MI_DISP_OUTPUT_1400x1050_60   ? "1400x1050@60"    \
     : x == E_MI_DISP_OUTPUT_1600x900_60    ? "1600x900@60"     \
     : x == E_MI_DISP_OUTPUT_1920x1440_60   ? "1920x1440@60"    \
     : x == E_MI_DISP_OUTPUT_USER           ? "user"            \
                                            : "UNKNOWN")

#define PARSING_DISP_PIXFORMAT(x)                                       \
    (x == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV               ? "yuv422yuyv" \
     : x == E_MI_SYS_PIXEL_FRAME_YUV422_YVYU             ? "yuv422yvyu" \
     : x == E_MI_SYS_PIXEL_FRAME_YUV422_UYVY             ? "yuv422uyvy" \
     : x == E_MI_SYS_PIXEL_FRAME_YUV422_VYUY             ? "yuv422vyuy" \
     : x == E_MI_SYS_PIXEL_FRAME_ARGB8888                ? "argb8888"   \
     : x == E_MI_SYS_PIXEL_FRAME_ABGR8888                ? "abgr8888"   \
     : x == E_MI_SYS_PIXEL_FRAME_BGRA8888                ? "bgra8888"   \
     : x == E_MI_SYS_PIXEL_FRAME_RGB565                  ? "rgb565"     \
     : x == E_MI_SYS_PIXEL_FRAME_ARGB1555                ? "argb1555"   \
     : x == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422      ? "yuv422sp"   \
     : x == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420      ? "yuv420sp"   \
     : x == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21 ? "yuv420nv21" \
     : x == E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR           ? "yuv422p"    \
     : x == E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR           ? "yuv420p"    \
                                                         : "UNKNOWN")

#define PARSING_DISP_STATUS(x)                              \
    (x == E_MI_LAYER_INPUTPORT_STATUS_INVALID   ? "invalid" \
     : x == E_MI_LAYER_INPUTPORT_STATUS_PAUSE   ? "pause"   \
     : x == E_MI_LAYER_INPUTPORT_STATUS_RESUME  ? "resume"  \
     : x == E_MI_LAYER_INPUTPORT_STATUS_STEP    ? "step"    \
     : x == E_MI_LAYER_INPUTPORT_STATUS_REFRESH ? "refresh" \
     : x == E_MI_LAYER_INPUTPORT_STATUS_SHOW    ? "show"    \
     : x == E_MI_LAYER_INPUTPORT_STATUS_HIDE    ? "hide"    \
                                                : "UNKNOWN")
#define PARSING_DISP_ROTATEMODE(x)       \
    (x == E_MI_DISP_ROTATE_NONE  ? "0"   \
     : x == E_MI_DISP_ROTATE_90  ? "90"  \
     : x == E_MI_DISP_ROTATE_180 ? "180" \
     : x == E_MI_DISP_ROTATE_270 ? "270" \
                                 : "UNKNOWN")

#define PARSING_DISP_SYNCMODE(x)                       \
    (x == E_MI_DISP_SYNC_MODE_INVALID     ? "Invalid"  \
     : x == E_MI_DISP_SYNC_MODE_CHECK_PTS ? "CheckPts" \
     : x == E_MI_DISP_SYNC_MODE_FREE_RUN  ? "FreeRun"  \
                                          : "UNKNOWN")

#define PARSING_DISP_COMPRESS_MODE(x) \
    (x == E_MI_SYS_COMPRESS_MODE_NONE ? "none" : x == E_MI_SYS_COMPRESS_MODE_TO_6BIT ? "to6bit" : "UNKNOWN")

#define MI_DISP_CHECK_IS_VALID_DEV(_s32DevId)                                                  \
    (                                                                                          \
        {                                                                                      \
            int __ret = 1;                                                                     \
            if ((_s32DevId) < 0 || (_s32DevId) >= g_stDispModRes.stCapabilityCfg.u32DeviceCnt) \
            {                                                                                  \
                DBG_ERR("Invalid Dev:%d\n", _s32DevId);                                        \
                __ret = 0;                                                                     \
            }                                                                                  \
            __ret;                                                                             \
        })

#define MI_DISP_CHECK_IS_VALID_CHN(_u32ChnId)           \
    (                                                   \
        {                                               \
            int __ret = 1;                              \
            if ((_u32ChnId) >= MI_DISP_DEV_CHN_MAX)     \
            {                                           \
                DBG_ERR("Invalid chn:%d\n", _u32ChnId); \
                __ret = 0;                              \
            }                                           \
            __ret;                                      \
        })

#define MI_DISP_CHECK_IS_VAILD_PORT(_u32LayerId, _s32Port)                                       \
    (                                                                                            \
        {                                                                                        \
            int __ret = MI_DISP_COMMON_IsValidLayerId(_u32LayerId) ? 1 : 0;                      \
            if (__ret)                                                                           \
            {                                                                                    \
                if ((_s32Port) < 0 || (_s32Port) >= MI_DISP_COMMON_GetLayerPortCnt(_u32LayerId)) \
                {                                                                                \
                    DBG_ERR("Invalid Port:%d\n", _s32Port);                                      \
                    __ret = 0;                                                                   \
                }                                                                                \
            }                                                                                    \
            __ret;                                                                               \
        })

#define MI_DISP_CHECK_IS_VAILD_WBC(_u32DevId)                                                     \
    (                                                                                             \
        {                                                                                         \
            int __ret = 1;                                                                        \
            if ((_u32DevId) < 0 || (_u32DevId) >= g_stDispModRes.stCapabilityCfg.u32WbcDeviceCnt) \
            {                                                                                     \
                DBG_ERR("Invalid Wbc:%d\n", _u32DevId);                                           \
                __ret = 0;                                                                        \
            }                                                                                     \
            __ret;                                                                                \
        })

#define MI_DISP_CHECK_DEV_IS_CREATED(__pstDevRes)      \
    do                                                 \
    {                                                  \
        if (!__pstDevRes || !__pstDevRes->pDispDevObj) \
        {                                              \
            DBG_ERR("DevRes %d is null\n", DispDev);   \
            s32Ret = MI_ERR_DISP_ILLEGAL_PARAM;        \
            goto EXIT;                                 \
        }                                              \
    } while (0);

#define MI_DISP_CHECK_DEV_IS_SETPUBATTR(_pstDevRes)       \
    do                                                    \
    {                                                     \
        MI_DISP_CHECK_DEV_IS_CREATED(_pstDevRes);         \
        if (!pstDevRes->u32Interface)                     \
        {                                                 \
            DBG_ERR("Dev %d PubAttr not set\n", DispDev); \
            s32Ret = MI_ERR_DISP_NOT_PERMIT;              \
            goto EXIT;                                    \
        }                                                 \
    } while (0);

#define MI_DISP_CHECK_DEV_IS_ENABLED(_pstDevRes)                  \
    do                                                            \
    {                                                             \
        MI_DISP_CHECK_DEV_IS_CREATED(_pstDevRes);                 \
        if (!_pstDevRes->s32EnableCnt)                            \
        {                                                         \
            DBG_ERR("Dev %d is disabled\n", _pstDevRes->u8DevId); \
            s32Ret = MI_ERR_DISP_DEV_NOT_ENABLE;                  \
            goto EXIT;                                            \
        }                                                         \
    } while (0);

#define MI_DISP_CHECK_LAYER_IS_CREATED(__pstLayerRes)         \
    do                                                        \
    {                                                         \
        if (!__pstLayerRes || !__pstLayerRes->pVideoLayerObj) \
        {                                                     \
            DBG_ERR("LayerRes is null\n", DispLayer);         \
            s32Ret = MI_ERR_DISP_ILLEGAL_PARAM;               \
            goto EXIT;                                        \
        }                                                     \
    } while (0);

#define MI_DISP_CHECK_LAYER_IS_BOUND(__pstLayerRes)                       \
    do                                                                    \
    {                                                                     \
        if (__pstLayerRes->pstDevRes == NULL)                             \
        {                                                                 \
            DBG_ERR("Layer %d no bind dev\n", __pstLayerRes->u32LayerId); \
            s32Ret = MI_ERR_DISP_LAYER_NOT_BIND;                          \
            goto EXIT;                                                    \
        }                                                                 \
    } while (0);

#define MI_DISP_CHECK_LAYER_IS_BOUND_AND_ENABLED(_pstLayerRes)           \
    do                                                                   \
    {                                                                    \
        MI_DISP_CHECK_LAYER_IS_CREATED(_pstLayerRes);                    \
        MI_DISP_CHECK_LAYER_IS_BOUND(_pstLayerRes);                      \
        if (!_pstLayerRes->s32EnableCnt)                                 \
        {                                                                \
            DBG_ERR("Layer %d is disabled\n", _pstLayerRes->u32LayerId); \
            s32Ret = MI_ERR_DISP_LAYER_NOT_ENABLE;                       \
            goto EXIT;                                                   \
        }                                                                \
    } while (0);

#define MI_DISP_CHECK_LAYER_ALL_PORT_IS_DISABLED(_pstLayerRes)                                               \
    do                                                                                                       \
    {                                                                                                        \
        MI_DISP_IMPL_InputChnRes_t *_pstChnResPos = NULL;                                                    \
        CAM_OS_LIST_FOR_EACH_ENTRY(_pstChnResPos, &_pstLayerRes->stLayerChnRefer, stLayerChnRefer)           \
        {                                                                                                    \
            MI_SYS_BUG_ON(_pstChnResPos->pstLayerRes != _pstLayerRes);                                       \
            if (MI_DISP_COMMON_CHECK_CHNSTATUS(_pstChnResPos->eChnStatus, E_MI_DISP_IMPL_CHNSTATUS_ENABLE))  \
            {                                                                                                \
                DBG_ERR("Layer %d Port %d is enabled\n", _pstLayerRes->u32LayerId, _pstChnResPos->u8PortId); \
                s32Ret = MI_ERR_DISP_INPUTPORT_NOT_DISABLE;                                                  \
                goto EXIT;                                                                                   \
            }                                                                                                \
        }                                                                                                    \
    } while (0);

#define MI_DISP_CHECK_INPUTCHN_IS_CREATED(__pstChnRes)                            \
    do                                                                            \
    {                                                                             \
        if (!__pstChnRes || !__pstChnRes->pInputPortObj)                          \
        {                                                                         \
            DBG_ERR("Layer %d Port %d Res is null\n", DispLayer, LayerInputPort); \
            s32Ret = MI_ERR_DISP_ILLEGAL_PARAM;                                   \
            goto EXIT;                                                            \
        }                                                                         \
    } while (0);

#define MI_DISP_CHECK_INPUTCHN_IS_SETPORTATTR(_pstChnRes)                                              \
    do                                                                                                 \
    {                                                                                                  \
        MI_DISP_CHECK_INPUTCHN_IS_CREATED(_pstChnRes);                                                 \
        if (!MI_DISP_COMMON_CHECK_CHNSTATUS(_pstChnRes->eChnStatus, E_MI_DISP_IMPL_CHNSTATUS_ATTRSET)) \
        {                                                                                              \
            DBG_ERR("Layer %d Port %d Attr not set\n", DispLayer, LayerInputPort);                     \
            s32Ret = MI_ERR_DISP_NOT_PERMIT;                                                           \
            goto EXIT;                                                                                 \
        }                                                                                              \
    } while (0);

#define MI_DISP_CHECK_INPUTCHN_IS_ENABLED(_pstChnRes)                                                 \
    do                                                                                                \
    {                                                                                                 \
        MI_DISP_CHECK_INPUTCHN_IS_CREATED(_pstChnRes);                                                \
        if (!MI_DISP_COMMON_CHECK_CHNSTATUS(_pstChnRes->eChnStatus, E_MI_DISP_IMPL_CHNSTATUS_ENABLE)) \
        {                                                                                             \
            DBG_ERR("Layer %d Port %d is disabled\n", DispLayer, LayerInputPort);                     \
            s32Ret = MI_ERR_DISP_INPUTPORT_NOT_ENABLE;                                                \
            goto EXIT;                                                                                \
        }                                                                                             \
    } while (0);

#define MI_DISP_CHECK_INTERFACE_IS_MATCH(__pstDevRes, _E_MI_DISP_INTF_SET) \
    do                                                                     \
    {                                                                      \
        if ((__pstDevRes->u32Interface & (_E_MI_DISP_INTF_SET)) == 0)      \
        {                                                                  \
            DBG_ERR("dev%d interface is't match\n", DispDev);              \
            s32Ret = MI_ERR_DISP_NOT_PERMIT;                               \
            goto EXIT;                                                     \
        }                                                                  \
    } while (0);

#define MI_DISP_CHECK_WBC_RES_IS_DISABLED(_pstDevRes)                        \
    do                                                                       \
    {                                                                        \
        if (_pstDevRes->pstWbcRes && _pstDevRes->pstWbcRes->s32EnableCnt)    \
        {                                                                    \
            DBG_ERR("Wbc %d is enabled\n", _pstDevRes->pstWbcRes->u32WbcId); \
            s32Ret = MI_ERR_DISP_NOT_PERMIT;                                 \
            goto EXIT;                                                       \
        }                                                                    \
    } while (0);

#define MI_DISP_CHECK_WBC_RES_IS_DISABLED_V2(_pstWbcRes)          \
    do                                                            \
    {                                                             \
        if (_pstWbcRes->s32EnableCnt)                             \
        {                                                         \
            DBG_ERR("Wbc %d is enabled\n", _pstWbcRes->u32WbcId); \
            s32Ret = MI_ERR_DISP_NOT_PERMIT;                      \
            goto EXIT;                                            \
        }                                                         \
    } while (0);

#define MI_DISP_CHECK_INTERFACE_LCD_IS_MATCH(_pstDevRes)             \
    do                                                               \
    {                                                                \
        MI_U32 _u32Interface = 0;                                    \
        _u32Interface |= (1 << E_MI_DISP_INTF_LCD);                  \
        _u32Interface |= (1 << E_MI_DISP_INTF_BT656);                \
        _u32Interface |= (1 << E_MI_DISP_INTF_BT601);                \
        _u32Interface |= (1 << E_MI_DISP_INTF_BT1120);               \
        _u32Interface |= (1 << E_MI_DISP_INTF_BT1120_DDR);           \
        _u32Interface |= (1 << E_MI_DISP_INTF_TTL);                  \
        _u32Interface |= (1 << E_MI_DISP_INTF_MIPIDSI);              \
        _u32Interface |= (1 << E_MI_DISP_INTF_MIPIDSI1);             \
        _u32Interface |= (1 << E_MI_DISP_INTF_TTL_SPI_IF);           \
        _u32Interface |= (1 << E_MI_DISP_INTF_SRGB);                 \
        _u32Interface |= (1 << E_MI_DISP_INTF_MCU);                  \
        _u32Interface |= (1 << E_MI_DISP_INTF_MCU_NOFLM);            \
        _u32Interface |= (1 << E_MI_DISP_INTF_LVDS);                 \
        _u32Interface |= (1 << E_MI_DISP_INTF_LVDS1);                \
        _u32Interface |= (1 << E_MI_DISP_INTF_DUAL_LVDS);            \
        MI_DISP_CHECK_INTERFACE_IS_MATCH(_pstDevRes, _u32Interface); \
    } while (0);

#define GET_DEVRES(_u32DevId)                                       \
    (                                                               \
        {                                                           \
            MI_DISP_IMPL_DevRes_t *__pstDevRes = NULL;              \
            if (MI_DISP_CHECK_IS_VALID_DEV(_u32DevId))              \
            {                                                       \
                __pstDevRes = g_stDispModRes.apstDevRes[_u32DevId]; \
            }                                                       \
            __pstDevRes;                                            \
        })

#define GET_LAYERRES(_u32LayerId)                                       \
    (                                                                   \
        {                                                               \
            MI_DISP_IMPL_LayerRes_t *_pstLayerRes = NULL;               \
            if (MI_DISP_COMMON_IsValidLayerId(_u32LayerId))             \
            {                                                           \
                _pstLayerRes = MI_DISP_COMMON_GetLayerRes(_u32LayerId); \
            }                                                           \
            _pstLayerRes;                                               \
        })

#define GET_CHNRES(_u32LayerId, _u8PortId)                                                                     \
    (                                                                                                          \
        {                                                                                                      \
            MI_DISP_IMPL_LayerRes_t *   _pstLayerRes  = NULL;                                                  \
            MI_DISP_IMPL_InputChnRes_t *_pstChnResPos = NULL;                                                  \
            MI_DISP_IMPL_InputChnRes_t *_pstGetChnRes = NULL;                                                  \
            if (MI_DISP_COMMON_IsValidLayerId(_u32LayerId))                                                    \
            {                                                                                                  \
                _pstLayerRes = MI_DISP_COMMON_GetLayerRes(_u32LayerId);                                        \
                if (_pstLayerRes)                                                                              \
                {                                                                                              \
                    CAM_OS_LIST_FOR_EACH_ENTRY(_pstChnResPos, &_pstLayerRes->stLayerChnRefer, stLayerChnRefer) \
                    {                                                                                          \
                        if (_u8PortId == _pstChnResPos->u8PortId)                                              \
                        {                                                                                      \
                            _pstGetChnRes = _pstChnResPos;                                                     \
                            break;                                                                             \
                        }                                                                                      \
                    }                                                                                          \
                }                                                                                              \
            }                                                                                                  \
            _pstGetChnRes;                                                                                     \
        })

#define GET_WBCRES(_u32WbcId)                                      \
    (                                                              \
        {                                                          \
            MI_DISP_IMPL_WbcRes_t *_pstWbcRes = NULL;              \
            if (MI_DISP_CHECK_IS_VAILD_WBC(_u32WbcId))             \
            {                                                      \
                _pstWbcRes = g_stDispModRes.apstWbcRes[_u32WbcId]; \
            }                                                      \
            _pstWbcRes;                                            \
        })

#define MI_DISP_IMPL_INTEGER(_pstPerfStatInfo)                                                                       \
    (                                                                                                                \
        {                                                                                                            \
            MI_U32 _u32Ret =                                                                                         \
                _pstPerfStatInfo->u64CntDiff * 1000 / MAX(CamOsJiffiesToMs(_pstPerfStatInfo->u64CntJiffiesDiff), 1); \
            _u32Ret;                                                                                                 \
        })

#define MI_DISP_IMPL_DECIMAL(_pstPerfStatInfo)                                                                         \
    (                                                                                                                  \
        {                                                                                                              \
            MI_U32 _u32Ret, _u32Tmp;                                                                                   \
            _u32Tmp =                                                                                                  \
                _pstPerfStatInfo->u64CntDiff * 100000 / MAX(CamOsJiffiesToMs(_pstPerfStatInfo->u64CntJiffiesDiff), 1); \
            _u32Ret = _u32Tmp % 100;                                                                                   \
            _u32Ret;                                                                                                   \
        })

#define MI_DISP_IMPL_RESET_PERF_STATINFO(_pstPerfStatInfo)                     \
    do                                                                         \
    {                                                                          \
        MI_U32 _u32StatDurationSec = _pstPerfStatInfo->u64StatDurationSec;     \
        CamOsMemset(_pstPerfStatInfo, 0, sizeof(MI_DISP_IMPL_PerfStatInfo_t)); \
        _pstPerfStatInfo->u64StatDurationSec = _u32StatDurationSec;            \
    } while (0);

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
typedef struct MI_DISP_IMPL_DispCheckBuffInfo_s
{
    MI_U32                u32InportRecvBufCnt;
    MI_U32                u32RecvBufWidth;
    MI_U32                u32RecvBufHeight;
    MI_U32                u32ContentWidth;
    MI_U32                u32ContentHeight;
    MI_U32                u32RecvBufStride;
    MI_SYS_PixelFormat_e  ePixFormat;
    MI_SYS_CompressMode_e eCompressMode;
} MI_DISP_IMPL_CheckBuffInfo_t;
#endif

typedef enum
{
    E_MI_DISP_IMPL_FUNC_TYPE_INPUTPORT_DISABLE,
    E_MI_DISP_IMPL_FUNC_TYPE_INPUTPORT_ENABLE,
    E_MI_DISP_IMPL_FUNC_TYPE_INPUTPORT_HIDE,
    E_MI_DISP_IMPL_FUNC_TYPE_INPUTPORT_SHOW,
    E_MI_DISP_IMPL_FUNC_TYPE_MAX,
} MI_DISP_IMPL_FuncType_e;

typedef struct MI_DISP_IMPL_FuncInfo_s
{
    MI_U32 u32LayerId;
    MI_U8  u8PortId;
    void * pstChnRes;
    MI_U8 *pu8Param;
    // struct CamOsListHead_t stFuncInfoNode;
} MI_DISP_IMPL_FuncInfo_t;

typedef enum
{
    E_MI_DISP_IMPL_CHNSTATUS_ENABLE  = 0x10000000, // 1: enable ;0: disable
    E_MI_DISP_IMPL_CHNSTATUS_STOP    = 0x1,
    E_MI_DISP_IMPL_CHNSTATUS_STEP    = 0x2,
    E_MI_DISP_IMPL_CHNSTATUS_CROP    = 0x4,
    E_MI_DISP_IMPL_CHNSTATUS_ATTRSET = 0x8,
    E_MI_DISP_IMPL_CHNSTATUS_HIDE    = 0x10,
    E_MI_DISP_IMPL_CHNSTATUS_SUSPEND = 0x20,
    E_MI_DISP_IMPL_CHNSTATUS_PAUSE   = 0x40,
} MI_DISP_IMPL_ChnStatus_e;

// for performance statistics
typedef struct MI_DISP_IMPL_PerfStatInfo_s
{
    MI_U64 u64StatDurationSec; // statistics duration
    MI_U64 u64TimeStampLast;
    MI_U32 u32IntervalUs;
    MI_U32 au32MaxIntervalUs[2]; //[0] for result [1] for realtime statistics
    MI_U32 au32MinIntervalUs[2]; //[0] for result [1] for realtime statistics
    MI_U64 u64Cnt;
    MI_U64 u64CntLast;
    MI_U64 u64CntDiff;
    MI_U64 u64CntJiffiesLast;
    MI_U64 u64CntJiffiesDiff;
} MI_DISP_IMPL_PerfStatInfo_t;

typedef struct MI_DISP_IMPL_LayerRes_s
{
    MI_U32  u32LayerId;
    MI_U32  u32BindDispId;
    MI_S32  s32BindCnt;
    MI_U32  u32LayerType;
    MI_BOOL bCreate;
    MI_S32  s32EnableCnt;
    MI_U32  u32Toleration;

    // struct CamOsListHead_t   stLayerNode;
    // struct CamOsListHead_t   stLayerChnRefer;
    MI_DISP_VideoLayerAttr_t stVideoLayerAttr;
    MI_DISP_RotateConfig_t   stRotateConfig;

    void *                        pVideoLayerObj;
    struct MI_DISP_IMPL_DevRes_s *pstDevRes;
    // CamOsTsem_t                   stLayerResSem;

    MI_S32 s32BatchProcBeginCnt;
    // struct CamOsListHead_t astFuncInfoHead[E_MI_DISP_IMPL_FUNC_TYPE_MAX];
    unsigned long *pBatchBitMap;
} MI_DISP_IMPL_LayerRes_t;

typedef struct MI_DISP_IMPL_InputChnRes_s
{
    void *pInputPortObj;
    // struct CamOsListHead_t   stLayerChnRefer;
    MI_DISP_IMPL_LayerRes_t *pstLayerRes;
    MI_U16                   u16ChnId;
    MI_U8                    u8PortId;
    MI_BOOL                  bFirstShowFrame;
    MI_DISP_IMPL_ChnStatus_e eChnStatus;
    MI_BOOL                  bFramePtsBefore;
    MI_U64                   u64LastFiredTimeStamp;
    MI_U32                   u32SramSize;
    MI_PHY                   phySramAddr;

    MI_DISP_SyncMode_e        eSyncMode;
    MI_DISP_InputPortStatus_e eStatus;
    MI_DISP_VidWinRect_t      stDispWin;
    MI_DISP_VidWinRect_t      stCropWin;

    MI_DISP_InputPortAttr_t stInputPortAttr;

    MI_BOOL bUnBindInput;
    MI_U16  u16BindMode;

    // mi_sys_ChnInputTaskInfo_t *pstPendingTask;
    // mi_sys_ChnInputTaskInfo_t *pstFiredTask;
    // mi_sys_ChnInputTaskInfo_t *pstOnScreenTask;
    // CamOsTsem_t                stChnResSem;

    // mi_sys_FrameRate_t stSrcFrmrate; /* Set Src Framerate inputport frc. */
    // mi_sys_FrameRate_t stDstFrmrate; /* Set Dst Framerate. */
    MI_BOOL bNeedFrcBySelf;
    MI_U32  u32SurplusTimeDiffMs;
    MI_U64  u64LastTime;

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
    MI_BOOL                      bStopGetInBuf;
    MI_U8                        pu8DumpFramePath[STRING_LEN];
    MI_U32                       u32DumpFrameSeq;
    MI_DISP_IMPL_CheckBuffInfo_t stDispCheckBuffInfo;
    MI_DISP_IMPL_PerfStatInfo_t  stPerfStatInfo;
#endif
} MI_DISP_IMPL_InputChnRes_t;

typedef struct MI_DISP_IMPL_DevPassRes_s
{
    MI_U8 u8PassId;
    // mi_sys_DevPassOpsInfo_t stDevPassOpsInfo;
} MI_DISP_IMPL_DevPassRes_t;

typedef struct MI_DISP_IMPL_DevIrqInfo_s
{
    MI_BOOL bEnabled;
    MI_U32  u32IrqNum;
    MI_U64  u64IrqCnt;
    MI_U8   au8IrqName[STRING_LEN];
} MI_DISP_IMPL_DevIrqInfo_t;

typedef struct MI_DISP_IMPL_SramInfo_s
{
    MI_BOOL bNeedSram;
    MI_U32  u32SramSize;
    MI_PHY  phySramAddr;
    MI_S32  s32SramOffset;
} MI_DISP_IMPL_SramInfo_t;

typedef struct MI_DISP_IMPL_DevRes_s
{
    void *pDispDevObj;
    // MI_SYS_DRV_HANDLE         hDevSysHandle;
    MI_BOOL                   bInit;
    MI_S32                    s32EnableCnt;
    MI_U8                     u8DevId;
    MI_DISP_IMPL_DevIrqInfo_t stDevIrqInfo;
    MI_DISP_IMPL_DevIrqInfo_t stLcdIrqInfo;
    void *                    pIrqHandleParamDevId;
    // CamOsCondition_t          stWaitIsrTcond;

    void *pIrqHandleParamLcd;
    void *pstPanelInfo;
    // sram alloc/free by dev
    MI_DISP_IMPL_SramInfo_t     stSramInfo;
    MI_U64                      u64LastIntTimeStamp;
    MI_U64                      u64CurrentIntTimeStamp;
    MI_U32                      u32AccumInterruptCnt;
    MI_U32                      u32AccumInterruptTimeStamp;
    MI_U32                      u32VsyncInterval;
    MI_DISP_PubAttr_t           stPubAttr;
    MI_U32                      u32Interface;
    MI_DISP_ColorTemperature_t  stColorTempInfo;
    MI_DISP_GammaParam_t        stGammaParam;
    MI_U32                      au32Gain[E_MI_DISP_INTF_MAX];
    MI_U32                      au32Sharpness[E_MI_DISP_INTF_MAX];
    MI_DISP_Csc_t               astCscParam[E_MI_DISP_INTF_MAX];
    MI_DISP_IMPL_InputChnRes_t *apstInputChnRes[MI_DISP_DEV_CHN_MAX];
    MI_DISP_IMPL_InputChnRes_t *pstSyncInputChnRes;
    // struct CamOsListHead_t      stBindedLayer;
    MI_DISP_IMPL_DevPassRes_t stDevPassRes;
    // CamOsTsem_t                   stDevResSem;
    struct MI_DISP_IMPL_WbcRes_s *pstWbcRes;
    MI_BOOL                       bCloseRgn;
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
    MI_DISP_IMPL_PerfStatInfo_t stPerfStatInfo;
#endif
} MI_DISP_IMPL_DevRes_t;

typedef struct MI_DISP_IMPL_WbcRes_s
{
    MI_U32  u32WbcId;
    MI_S32  s32EnableCnt;
    MI_BOOL bSourceSet;
    MI_BOOL bAttrSet;
    MI_BOOL bAttrChange;
    MI_BOOL bStopTask;

    void *pDmaObj;
    // MI_SYS_DRV_HANDLE         hDevSysHandle;
    MI_DISP_IMPL_DevIrqInfo_t stDmaIrqInfo;
    MI_S32                    s32CrcType;
    MI_U32                    u32InputWidth;
    MI_U32                    u32InputHeight;
    MI_DISP_WBC_Source_t      stWbcSource;
    MI_DISP_WBC_Attr_t        stWbcAttr;

    // mi_sys_ChnOutputTaskInfo_t *pstPendingOutputTask;
    // mi_sys_ChnOutputTaskInfo_t *pstFiredOutputTask;
    // mi_sys_ChnOutputTaskInfo_t *pstOnScreenOutputTask;

    // CamOsTsem_t stWbcResSem;
#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_DISP_PROCFS_DEBUG == 1)
    MI_U8                       pu8DumpFramePath[STRING_LEN];
    MI_U32                      u32DumpFrameSeq;
    MI_DISP_IMPL_PerfStatInfo_t stPerfStatInfo;
#endif
} MI_DISP_IMPL_WbcRes_t;

typedef struct MI_DISP_IMPL_InterfaceCapability_s
{
    MI_U32  u32HwCount;
    MI_BOOL abSupportTiming[E_MI_DISP_OUTPUT_MAX];
} MI_DISP_IMPL_InterfaceCapability_t;

typedef struct MI_DISP_IMPL_PortCapability_s
{
    MI_U8 u8PortId;
} MI_DISP_IMPL_PortCapability_t;

///< Define the VideoLayer ID of DISP window type.
typedef enum
{
    E_MI_DISP_VIDEOLAYER_SINGLEWIN,   ///< VideoLayer by MOPS/RDMA...
    E_MI_DISP_VIDEOLAYER_MULTIWIN,    ///< VideoLayer by MOPG
    E_MI_DISP_VIDEOLAYER_REALTIMEWIN, ///< VideoLayer by HVP...
    E_MI_DISP_VIDEOLAYER_UI,          ///< VideoLayer by UI
    E_MI_DISP_VIDEOLAYER_CURSOR,      ///< VideoLayer by CURSOR
    E_MI_DISP_VIDEOLAYER_TYPE,        ///< Total type of videolayer
} MI_DISP_IMPL_MhalVideoLayerType_e;

typedef struct MI_DISP_IMPL_LayerCapability_s
{
    MI_U32                         u32LayerId;
    MI_U32                         u32LayerType;
    MI_U32                         u32PortCnt;
    MI_DISP_IMPL_PortCapability_t *astPortCapability;
    MI_BOOL                        bRotateSupport;
    MI_BOOL                        bCompressFmtSupport;
    MI_BOOL                        abCompressModeSupport[E_MI_SYS_COMPRESS_MODE_BUTT];
    MI_U32                         u32InputPortPitchAlignment;
    MI_U32                         u32RotateHeightAlighment;
    MI_U32                         u32RotateStrideAlighment;
    MI_U32                         u32DefaultPriority;
} MI_DISP_IMPL_LayerCapability_t;

typedef struct MI_DISP_IMPL_DevCapability_s
{
    MI_U32  u32PqType;
    MI_BOOL bWdmaSupport;
    MI_U32  u32MaxSupportPortCnt;
    MI_U32  au32BindLayerTypeCnt[E_MI_DISP_VIDEOLAYER_TYPE];
} MI_DISP_IMPL_DevCapability_t;

typedef struct MI_DISP_IMPL_WbcCapabiliyt_s
{
    MI_BOOL bCompressSupport;
    MI_BOOL abCompressModeSupport[E_MI_SYS_COMPRESS_MODE_BUTT];
} MI_DISP_IMPL_WbcCapabiliyt_t;

typedef struct MI_DISP_IMPL_CapabilityConfig_s
{
    MI_U32                             u32DeviceCnt;
    MI_DISP_IMPL_DevCapability_t *     astDevCapability;
    MI_U32                             u32LayerCnt;
    MI_DISP_IMPL_LayerCapability_t *   astLayerCapability;
    MI_U32                             u32WbcDeviceCnt;
    MI_DISP_IMPL_WbcCapabiliyt_t *     astWbcCapabiliyt;
    MI_DISP_IMPL_InterfaceCapability_t astInterfaceCapability[E_MI_DISP_INTF_MAX];
} MI_DISP_IMPL_CapabilityConfig_t;

typedef struct MI_DISP_IMPL_ModRes_s
{
    MI_U32                          u32InitCnt;
    MI_U32                          u32EnableCnt;
    MI_DISP_IMPL_CapabilityConfig_t stCapabilityCfg;
    MI_DISP_IMPL_DevRes_t **        apstDevRes;
    MI_DISP_IMPL_LayerRes_t **      apstLayerRes;
    MI_DISP_IMPL_WbcRes_t **        apstWbcRes;
    MI_BOOL                         bRegisterStr;
} MI_DISP_IMPL_ModRes_t;

/////////
// origin mhal datatype param

///< Define CRC type
typedef enum
{
    E_MI_DISP_CRC16_OFF,       ///< CRC off
    E_MI_DISP_CRC16_EXT,       ///< extend CRC
    E_MI_DISP_CRC16_OVERWRITE, ///< Overwrite CRC
} MI_DISP_IMPL_MhalCrc16Type_e;

///< Define Device ID
typedef enum
{
    E_MI_DISP_DEVICE_NONE = 0x0000, ///< No ID
    E_MI_DISP_DEVICE_0    = 0x0001, ///< Device0 (1 << 0)
    E_MI_DISP_DEVICE_1    = 0x0002, ///< Device1 (1 << 1)
    E_MI_DISP_DEVICE_2    = 0x0004, ///< Device2 (1 << 2)
} MI_DISP_IMPL_MhalDeviceType_e;

///< Define time-zone type
typedef enum
{
    E_MI_DISP_TIMEZONE_UNKONWN    = 0x00, ///< unknown time zone
    E_MI_DISP_TIMEZONE_SYNC       = 0x01, ///< Sync area
    E_MI_DISP_TIMEZONE_BACKPORCH  = 0x02, ///< Back porch area
    E_MI_DISP_TIMEZONE_ACTIVE     = 0x03, ///< Data Eanble area
    E_MI_DISP_TIMEZONE_FRONTPORCH = 0x04, ///< Front porch area
    E_MI_DISP_TIMEZONE_NUM        = 0x05, ///< Total number of TimeZone type
} MI_DISP_IMPL_MhalTimeZoneType_e;

///< Define timezone query configuration
typedef struct
{
    MI_DISP_IMPL_MhalTimeZoneType_e enType; ///< timezone vlaue
} MI_DISP_IMPL_MhalTimeZone_t;

///< Define timezone configuration
typedef struct
{
    MI_U16 u16InvalidArea[E_MI_DISP_TIMEZONE_NUM]; ///< Invalid area of echo timezone
} MI_DISP_IMPL_MhalTimeZoneConfig_t;

///< Define the register assess mode
typedef enum
{
    E_MI_DISP_REG_ACCESS_CPU,  ///< RIU mode
    E_MI_DISP_REG_ACCESS_CMDQ, ///< CMDQ mode
    E_MI_DISP_REG_ACCESS_CMDQ_DIRECT,
    E_MI_DISP_REG_ACCESS_NUM, ///< Total number of register acess mode
} MI_DISP_IMPL_MhalRegAccessType_e;

///< Define register accessing configuration
typedef struct
{
    MI_DISP_IMPL_MhalRegAccessType_e enType;   ///< register accessing type
    MHAL_CMDQ_CmdqInterface_t *      pCmdqInf; ///< CMDQ interface hanlder
} MI_DISP_IMPL_MhalRegAccessConfig_t;

///< Define PQ configuration
typedef struct
{
    MI_U32 u32PqFlags;  ///< The flag of pq action
    MI_U32 u32DataSize; ///< size of PQ binary
    void * pData;       ///< Data of PQ binary
} MI_DISP_IMPL_MhalPqConfig_t;

///< Define the position of data fetching for DISP DMA
typedef enum
{
    E_MI_DISP_DMA_INPUT_DEVICE_FRONT, ///< The postion of data fetching is front
    E_MI_DISP_DMA_INPUT_DEVICE_BACK,  ///< The position of data fetching is back
    E_MI_DISP_DMA_INPUT_NUM,          ///< Number of type
} MI_DISP_IMPL_MhalDmaInputType_e;

///< Define the output memory type of DISP DMA
typedef enum
{
    E_MI_DISP_DMA_ACCESS_TYPE_IMI, ///< The output DRAM is interal memory
    E_MI_DISP_DMA_ACCESS_TYPE_EMI, ///< The output DRAM is external memory
    E_MI_DISP_DMA_ACCESS_TYPE_NUM, ///< Total number of tpe
} MI_DISP_IMPL_MhalDmaAccessType_e;

///< Define the output mode of DISP DMA
typedef enum
{
    E_MI_DISP_DMA_OUTPUT_MODE_FRAME, ///< The output is frame mode
    E_MI_DISP_DMA_OUTPUT_MODE_RING,  ///< The output is ring mode
    E_MI_DISP_DMA_OUTPUT_MODE_NUM,   ///< Total number of type
} MI_DISP_IMPL_MhalDmaOutputMode_e;

///< Define the port ID of DISP DMA
typedef enum
{
    E_MI_DISP_DMA_PORT0    = 0, ///< DMA port id = 0
    E_MI_DISP_DMA_PORT_NUM = 1, ///< Total number of port
} MI_DISP_IMPL_MhalDmaPortType_e;

///< Define  DMA binding configuration
typedef struct
{
    void *pSrcDevCtx;  ///< The context of source deivce
    void *pDestDevCtx; ///< The context of destination device
} MI_DISP_IMPL_MhalDmaBindConfig_t;

///< Define input configuraiton of Disp DMA
typedef struct
{
    MI_DISP_IMPL_MhalDmaInputType_e eType;         ///< The position of data fetching
    MI_SYS_PixelFormat_e            ePixelFormat;  ///< Pixel format of input data
    MI_SYS_CompressMode_e           eCompressMode; ///< Compress mode of input data
    MI_U16                          u16Width;      ///< Horizontal size
    MI_U16                          u16Height;     ///< Vertical size
} MI_DISP_IMPL_MhalDmaInputConfig_t;

///< Define ouput configuraiton of Disp DMA
typedef struct
{
    MI_DISP_IMPL_MhalDmaAccessType_e eAccessType;   ///< Output memory type
    MI_DISP_IMPL_MhalDmaOutputMode_e eMode;         ///< Output mode
    MI_SYS_PixelFormat_e             ePixelFormat;  ///< Pixel fomrat of output data
    MI_SYS_CompressMode_e            eCompressMode; ///< Compress mode type
    MI_U16                           u16Width;      ///< Horizontal size
    MI_U16                           u16Height;     ///< Vertical size
} MI_DISP_IMPL_MhalDmaOutputConfig_t;

///< Define attribue of Disp DMA
typedef struct
{
    MI_DISP_IMPL_MhalDmaInputConfig_t  stInputCfg;  ///< input data configuration
    MI_DISP_IMPL_MhalDmaOutputConfig_t stOutputCfg; ///< Output data configuration
} MI_DISP_IMPL_MhalDmaAttrConfig_t;

///< Define buffer attribute of Disp DMA
typedef struct
{
    MI_U8                        bEn;               ///< 1:enable, 0:disable
    MI_PHY                       paPhyAddr[3];      ///< Physical address
    MI_U32                       u32Stride[3];      ///< Memory stirde
    MI_U16                       u16RingBuffHeight; ///< height of Ring buffer
    MI_U16                       u16RingStartLine;  ///< Starting line of ring buffer
    MI_U16                       u16FrameIdx;       ///< frame idex
    MI_DISP_IMPL_MhalCrc16Type_e enCrcType;         ///< CRC type
} MI_DISP_IMPL_MhalDmaBufferAttrConfig_t;

///< Define ring buffer configuration of Disp DMA
typedef struct
{
    MI_U8                            bEn;              ///< 1: enable, 0:disable
    MI_U16                           u16BuffHeight;    ///< Ring buffer height
    MI_U16                           u16BuffStartLine; ///< Start line of rign buffer
    MI_DISP_IMPL_MhalDmaAccessType_e eAccessType;      ///< output memory type
} MI_DISP_IMPL_MhalRingBufferAttr_t;

///< Define DMA capability configuration
typedef struct
{
    MI_U8                         bCompressSupport;            ///< 1: supported, 0:non-supported
    MI_DISP_IMPL_MhalDeviceType_e eDeviceType;                 ///< Supported binding Device ID
    MI_U8 u8FormatWidthAlign[E_MI_SYS_PIXEL_FRAME_FORMAT_MAX]; ///<  FormatWidthAlign, 0:non-supported
    MI_U8 bSupportCompress[E_MI_SYS_COMPRESS_MODE_BUTT];       ///< 1: supported, 0:non-supported
} MI_DISP_IMPL_MhalDmaCapabiliytConfig_t;

///< Define the input type of PQ
typedef enum
{
    E_MI_DISP_PQ_INPUT_NONE         = 0x0000, ///< No action
    E_MI_DISP_PQ_INPUT_GENERAL      = 0x0001, ///< General input type
    E_MI_DISP_PQ_INPUT_HDMIRX       = 0x0002, ///< HDMIRX input type
    E_MI_DISP_PQ_INPUT_ONLINE_VIDEO = 0x0003, ///< Online video input type
    E_MI_DISP_PQ_INPUT_MULTI_MEDIA  = 0x0004, ///< Multi media input type
} MI_DISP_IMPL_MhalPqInputType_e;

///< Define the flag of PQ
typedef enum
{
    E_MI_DISP_PQ_FLAG_NONE                  = 0x0000, ///< No action
    E_MI_DISP_PQ_FLAG_LOAD_BIN              = 0x0001, ///< Load binary file
    E_MI_DISP_PQ_FLAG_SET_SRC_ID            = 0x0002, ///< Set source ID
    E_MI_DISP_PQ_FLAG_PROCESS               = 0x0003, ///< Aplly PQ Setting
    E_MI_DISP_PQ_FLAG_SET_LOAD_SETTING_TYPE = 0x0004, ///< Set load type
    E_MI_DISP_PQ_FLAG_FREE_BIN              = 0x0005, ///< Free binary file
    E_MI_DISP_PQ_FLAG_BYPASS                = 0x0006, ///< OFF device pq setting.
    E_MI_DISP_PQ_FLAG_SET_INPUT_CFG         = 0x0007, ///< Set PQ input configuration
} MI_DISP_IMPL_MhalPqFlagType_e;

///< Define PQ type
typedef enum
{
    E_MI_DISP_DEV_PQ_MACE, ///< PQ type is MACE
    E_MI_DISP_DEV_PQ_HPQ,  ///< PQ type is HPQ
    E_MI_DISP_DEV_PQ_LITE, ///< PQ type is NON
} MI_DISP_IMPL_MhalDevicePqType_e;

///< Define Device capability configuration
typedef struct
{
    MI_DISP_IMPL_MhalDevicePqType_e ePqType;                  ///< PQ type
    MI_U8                           bWdmaSupport;             ///< 1: supported, 0:non-supported
    MI_U32                          u32VideoLayerHwCnt;       ///< count of videolayer
    MI_U32                          u32VideoLayerStartOffset; ///< start number of videolayer,only for loop use.
} MI_DISP_IMPL_MhalDeviceCapabilityConfig_t;

///< Define the infomation of Device configuration
typedef enum
{
    E_MI_DISP_DEV_CFG_NONE       = 0x00000000, ///< No actition
    E_MI_DISP_DEV_CFG_BOOTLOGO   = 0x00000001, ///< bootlogo on/off
    E_MI_DISP_DEV_CFG_COLORID    = 0x00000002, ///< csc matrix ID
    E_MI_DISP_DEV_CFG_GOPBLENDID = 0x00000004, ///< GOP blending ID
    E_MI_DISP_DEV_CFG_DISP_PAT   = 0x00000008, ///< Test pattern on/off
    E_MI_DISP_DEV_CFG_RST_MOP    = 0x00000010, ///< MOP reset on/off
    E_MI_DISP_DEV_CFG_RST_DISP   = 0x00000020, ///< DISP reset on/off
    E_MI_DISP_DEV_CFG_RST_DAC    = 0x00000040, ///< HDMITX reset on/off
    E_MI_DISP_DEV_CFG_RST_HDMITX = 0x00000080, ///< LCD reset on/off
    E_MI_DISP_DEV_CFG_RST_LCD    = 0x00000100, ///< DAC_AFF reset on/off
    E_MI_DISP_DEV_CFG_RST_DACAFF = 0x00000200, ///< DAC Syn reset on/off
    E_MI_DISP_DEV_CFG_RST_DACSYN = 0x00000400, ///< FPLL reset on/off
    E_MI_DISP_DEV_CFG_RST_FPLL   = 0x00000800, ///< MOP reset on/off
    E_MI_DISP_DEV_CFG_CSC_EN     = 0x00001000, ///< CSC on/off
    E_MI_DISP_DEV_CFG_CSC_MD     = 0x00002000, ///< CSC mode
    E_MI_DISP_DEV_CFG_MOP_Y_THRD = 0x00004000, ///< MOP Y DMA Threshold
    E_MI_DISP_DEV_CFG_MOP_C_THRD = 0x00008000, ///< MOP C DMA Threshold
    E_MI_DISP_DEV_CFG_RST_WDMA   = 0x00010000,
    E_MI_DISP_DEV_CFG_DISP_MUTE  = 0x00020000,
    E_MI_DISP_DEV_CFG_WDMA_CSC   = 0x00040000,
    E_MI_DISP_DEV_CFG_WDMA_DV    = 0x00080000,
    E_MI_DISP_DEV_CFG_HVP_ST     = 0x00100000,
    E_MI_DISP_DEV_CFG_RST_DACALL = 0x00000640,
} MI_DISP_IMPL_MhalDeviceConfigType_e;

///< Define Device configuraiton
typedef struct
{
    MI_DISP_IMPL_MhalDeviceConfigType_e eType;        ///< device config type
    MI_U8                               bBootlogoEn;  ///< Value of Bootlogo
    MI_U8                               u8ColorId;    ///< Value of CSC matrix ID
    MI_U8                               u8GopBlendId; ///< Value of GOP blendig ID
    MI_U8                               bDispPat;     ///< Value of disp test pattern on/off
    MI_U8                               u8PatMd;      ///< Value of disp test pattern mode
    MI_U8                               bRstMop;      ///< Value of MOP reset
    MI_U8                               bRstDisp;     ///< Value of DISP reset
    MI_U8                               bRstDac;      ///< Value of DAC reset
    MI_U8                               bRstHdmitx;   ///< Value of HDMITX reset
    MI_U8                               bRstLcd;      ///< Value of LCD reset
    MI_U8                               bRstDacAff;   ///< Value of DAC AFF reset
    MI_U8                               bRstDacSyn;   ///< Value of DAC SYN reset
    MI_U8                               bRstFpll;     ///< Value of FPLL reset
    MI_U8                               bCscEn;       ///< Value of CSC on/off
    MI_U8                               u8CscMd;      ///< Value of CSC mode
    MI_U8                               bCtx;         ///< Value of CTX
    MI_U8                               u8MopYThrd;   ///< Value of Mop Y Dma Threshold
    MI_U8                               u8MopCThrd;   ///< Value of Mop C Dma Threshold
    MI_U8                               bMute;
    MI_U8                               bFrameColor;
    MI_U8                               u8DmaCsc;
    MI_U8                               u8HvpVstOffset;
} MI_DISP_IMPL_MhalDeviceConfig_t;

typedef MI_S32 (*P_DISP_MEM_ALLOC)(MI_U8 *pu8Name, MI_U32 size, MI_PHY *ppaPhyAddr);
typedef MI_S32 (*P_DISP_MEM_FREE)(MI_PHY paPhyAddr);

typedef struct
{
    P_DISP_MEM_ALLOC alloc;
    // Success return 0
    P_DISP_MEM_FREE free;
} MI_DISP_IMPL_MhalMemAllocConfig_t;

// mhal panel param
typedef enum
{
    E_MI_DISP_MHALPNL_SCAN_NONE            = 0, ///< Undefined, default mode
    E_MI_DISP_MHALPNL_SCAN_PROGRESSIVE_ORG = 1, ///< Progressive scanning mode by TTL top
    E_MI_DISP_MHALPNL_SCAN_PROGRESSIVE_EXT = 2, ///< Progressive scanning mode by EXT top
    E_MI_DISP_MHALPNL_SCAN_INTERLACE_EXT   = 3, ///< Interlace scanning mode by EXT top
} MI_DISP_IMPL_MhalPnlScanMode_e;

typedef enum
{
    // HVP
    E_MI_DISP_FRMSYNC_SRC_HVP = 0,    ///< HVP
                                      // MIPI CSI
    E_MI_DISP_FRMSYNC_SRC_CSI_0 = 10, ///< CSI_0
    E_MI_DISP_FRMSYNC_SRC_CSI_1,      ///< CSI_1
    E_MI_DISP_FRMSYNC_SRC_CSI_2,      ///< CSI_2
    E_MI_DISP_FRMSYNC_SRC_CSI_3,      ///< CSI_3
    E_MI_DISP_FRMSYNC_SRC_MAX,
} MI_DISP_IMPL_MhalFrmSyncSrc_e;

///< Define lane order of RGB
typedef enum
{
    E_MI_DISP_MHALPNL_RGB_SWAP_NONE = 0, ///< HW Default
    E_MI_DISP_MHALPNL_RGB_SWAP_B    = 1, ///< Swap to R channel
    E_MI_DISP_MHALPNL_RGB_SWAP_G    = 2, ///< Swap to G channel
    E_MI_DISP_MHALPNL_RGB_SWAP_R    = 3, ///< Swap to B channel
} MI_DISP_IMPL_MhalPnlRgbSwapType_e;

///< Define panel output format bit mode
typedef enum
{
    E_MI_DISP_MHALPNL_OUTPUT_10BIT_MODE  = 0, ///< default is 10bit, because 8bit panel can use 10bit CFG and 8bit CFG.
    E_MI_DISP_MHALPNL_OUTPUT_6BIT_MODE   = 1, ///< but 10bit panel(like PDP panel) can only use 10bit CFG.
    E_MI_DISP_MHALPNL_OUTPUT_8BIT_MODE   = 2, ///< and some PAD panel is 6bit.
    E_MI_DISP_MHALPNL_OUTPUT_565BIT_MODE = 3, ///< 565
} MI_DISP_IMPL_MhalPnlOutputFormatBitMode_e;
///< Define MCU I/F system
typedef enum
{
    E_MI_DISP_MHALPNL_MCU_68SYS, ///< MCU 68 system
    E_MI_DISP_MHALPNL_MCU_80SYS  ///< MCU 80 system
} MI_DISP_IMPL_MhalPnlMcuType_e;

///< Define MCU data bus configuration
typedef enum
{
    // 16bit bus
    E_MI_DISP_MHALPNL_MCU_RGB565_BUS16_CFG,       ///< RGB565_BUS16 0x0000
    E_MI_DISP_MHALPNL_MCU_RGB444_BUS16_CFG,       ///< RGB444_BUS16 0x0001
    E_MI_DISP_MHALPNL_MCU_RGB666_BUS16_CFG,       ///< RGB666_BUS16 0x0002
                                                  // 8 bit bus
    E_MI_DISP_MHALPNL_MCU_RGB888_BUS8_CFG,        ///< RGB888_BUS8 0x0003
    E_MI_DISP_MHALPNL_MCU_RGB332_BUS8_CFG,        ///< RGB332_BUS8 0x0004
    E_MI_DISP_MHALPNL_MCU_RGB444_BUS8_CFG,        ///< RGB444_BUS8 0x0005
    E_MI_DISP_MHALPNL_MCU_RGB666_BUS8_CFG,        ///< RGB666_BUS8 0x0006
    E_MI_DISP_MHALPNL_MCU_RGB565_BUS8_CFG,        ///< RGB565_BUS8 0x0007
                                                  // 18 bit bus
    E_MI_DISP_MHALPNL_MCU_RGB666_BUS18_CFG,       ///< RGB666_BUS18      0x0008
    E_MI_DISP_MHALPNL_MCU_RGB666_BUS18_9_9_CFG,   ///< RGB666_BUS18_9_9  0x0009
    E_MI_DISP_MHALPNL_MCU_RGB666_BUS18_2_16_CFG,  ///< RGB666_BUS18_2_16 0x000A
    E_MI_DISP_MHALPNL_MCU_RGB666_BUS18_16_2_CFG,  ///< RGB666_BUS18_16_2 0x000B
    E_MI_DISP_MHALPNL_MCU_RGB24_BUS18_16_8_CFG,   ///< RGB24_BUS18_16_8  0x000C
    E_MI_DISP_MHALPNL_MCU_RGB24_BUS18_8_16_CFG,   ///< RGB24_BUS18_8_16  0x000D
                                                  // 8 bit
    E_MI_DISP_MHALPNL_MCU_RGB18_BUS8_2_8_8_CFG,   ///< RGB18_BUS_8_2_8_8 0x000E
                                                  // TBD
    E_MI_DISP_MHALPNL_MCU_RGB666_BUS8_2_7_CFG,    ///< RGB666_BUS8_2_7 0x000F
    E_MI_DISP_MHALPNL_MCU_RGB444_B12_EXT_B16_CFG, ///< RGB444_B12_EXT_B16  0x0010
    E_MI_DISP_MHALPNL_MCU_RGB444_B15_4_CFG,       ///< RGB444_B15_4        0x0011
    E_MI_DISP_MHALPNL_MCU_RGBB9_9_17_CFG,         ///< RGBB9_17            0x0012
    E_MI_DISP_MHALPNL_MCU_CFG_NOT_SUPPORT         ///< NOTSUPPORT CFG
} MI_DISP_IMPL_MhalPnlMcuDataBusCfg_e;

///< Define RGB data type and LCD data bus
typedef enum
{
    E_MI_DISP_MHALPNL_RGB_DTYPE_RGB888        = 0x0,  ///< DTYPE is RGB888
    E_MI_DISP_MHALPNL_RGB_DTYPE_RGB666        = 0x1,  ///< DTYPE is RGB666
    E_MI_DISP_MHALPNL_RGB_DTYPE_RGB565        = 0x2,  ///< DTYPE is RGB565
    E_MI_DISP_MHALPNL_RGB_DTYPE_RGB444        = 0x3,  ///< DTYPE is RGB444
    E_MI_DISP_MHALPNL_RGB_DTYPE_RGB333        = 0x4,  ///< DTYPE is RGB333
    E_MI_DISP_MHALPNL_RGB_DTYPE_RGB332        = 0x5,  ///< DTYPE is RGB332
    E_MI_DISP_MHALPNL_RGB_DTYPE_BGR888        = 0x8,  ///< DTYPE is RGB888
    E_MI_DISP_MHALPNL_RGB_DTYPE_BGR666        = 0x9,  ///< DTYPE is RGB666
    E_MI_DISP_MHALPNL_RGB_DTYPE_BGR565        = 0xA,  ///< DTYPE is RGB565
    E_MI_DISP_MHALPNL_RGB_DTYPE_BGR444        = 0xB,  ///< DTYPE is RGB444
    E_MI_DISP_MHALPNL_RGB_DTYPE_BGR333        = 0xC,  ///< DTYPE is RGB333
    E_MI_DISP_MHALPNL_RGB_DTYPE_BGR332        = 0xD,  ///< DTYPE is RGB332
    E_MI_DISP_MHALPNL_RGB_DTYPE_YUV422_UY0VY1 = 0x0F, ///< DTYPE is YUV422(UY0VY1)
    E_MI_DISP_MHALPNL_RGB_DTYPE_YUV422_VY0UY1 = 0x1F, ///< DTYPE is YUV422(VY0UY1)
    E_MI_DISP_MHALPNL_RGB_DTYPE_YUV422_UY1VY0 = 0x2F, ///< DTYPE is YUV422(UY1VY0)
    E_MI_DISP_MHALPNL_RGB_DTYPE_YUV422_VY1UY0 = 0x3F, ///< DTYPE is YUV422(VY1UY0)
    E_MI_DISP_MHALPNL_RGB_DTYPE_YUV422_Y0UY1V = 0x4F, ///< DTYPE is YUV422(Y0UY1V)
    E_MI_DISP_MHALPNL_RGB_DTYPE_YUV422_Y1UY0V = 0x5F, ///< DTYPE is YUV422(Y1UY0V)
    E_MI_DISP_MHALPNL_RGB_DTYPE_YUV422_Y0VY1U = 0x6F, ///< DTYPE is YUV422(Y0VY1U)
    E_MI_DISP_MHALPNL_RGB_DTYPE_YUV422_Y1VY0U = 0x7F, ///< DTYPE is YUV422(Y1VY0U)
} MI_DISP_IMPL_MhalPnlRgbDataType_e;

///< Define RGB delta mode
typedef enum
{
    E_MI_DISP_MHALPNL_RGB_DELTA_RGB_MODE, ///< RGB mode
    E_MI_DISP_MHALPNL_RGB_DELTA_RBG_MODE, ///< RBG mode
    E_MI_DISP_MHALPNL_RGB_DELTA_GRB_MODE, ///< GRB mode
    E_MI_DISP_MHALPNL_RGB_DELTA_GBR_MODE, ///< GBR mode
    E_MI_DISP_MHALPNL_RGB_DELTA_BRG_MODE, ///< BRG mode
    E_MI_DISP_MHALPNL_RGB_DELTA_BGR_MODE, ///< BGR mode
} MI_DISP_IMPL_MhalPnlRgbDeltaMode_e;

///< Define number of lane of MIPIDSI
typedef enum
{
    E_MI_DISP_MHALPNL_MIPI_DSI_LANE_NONE = 0, ///< NO defined
    E_MI_DISP_MHALPNL_MIPI_DSI_LANE_1    = 1, ///< 1 lane mode
    E_MI_DISP_MHALPNL_MIPI_DSI_LANE_2    = 2, ///< 2 lane mode
    E_MI_DISP_MHALPNL_MIPI_DSI_LANE_3    = 3, ///< 3 lane mode
    E_MI_DISP_MHALPNL_MIPI_DSI_LANE_4    = 4, ///< 4 lane mode
} MI_DISP_IMPL_MhalPnlMipiDsiLaneMode_e;

///< Define output color format of MIPIDSI
typedef enum
{
    E_MI_DISP_MHALPNL_MIPI_DSI_RGB565         = 0, ///< MIPIDSI output RGB565
    E_MI_DISP_MHALPNL_MIPI_DSI_RGB666         = 1, ///< MIPIDSI output RGB666
    E_MI_DISP_MHALPNL_MIPI_DSI_LOOSELY_RGB666 = 2, ///< MIPIDSI output loosely RGB666
    E_MI_DISP_MHALPNL_MIPI_DSI_RGB888         = 3, ///< MIPIDSI output RGB888
} MI_DISP_IMPL_MhalPnlMipiDsiFormat_e;

///< Define control mode of MIPIDSI
typedef enum
{
    E_MI_DISP_MHALPNL_MIPI_DSI_CMD_MODE   = 0, ///< MIPIDSI command mode
    E_MI_DISP_MHALPNL_MIPI_DSI_SYNC_PULSE = 1, ///< MIPIDSI sync pulse mode
    E_MI_DISP_MHALPNL_MIPI_DSI_SYNC_EVENT = 2, ///< MIPIDSI sync event mode
    E_MI_DISP_MHALPNL_MIPI_DSI_BURST_MODE = 3, ///< MIPIDSI burst mode
} MI_DISP_IMPL_MhalPnlMipiDsiCtrlMode_e;

///< Define MIPIDSI lane order
typedef enum
{
    E_MI_DISP_MHALPNL_CH_SWAP_0, ///< MIPIDSI lane 0
    E_MI_DISP_MHALPNL_CH_SWAP_1, ///< MIPIDSI lane 1
    E_MI_DISP_MHALPNL_CH_SWAP_2, ///< MIPIDSI lane 2
    E_MI_DISP_MHALPNL_CH_SWAP_3, ///< MIPIDSI lane 3
    E_MI_DISP_MHALPNL_CH_SWAP_4, ///< MIPIDSI lane 4
} MI_DISP_IMPL_MhalPnlChannelSwapType_e;

///< Define packet type of MIPIDSI command
typedef enum
{
    E_MI_DISP_MHALPNL_MIPI_DSI_PACKET_TYPE_DCS     = 0, ///< DCS mode
    E_MI_DISP_MHALPNL_MIPI_DSI_PACKET_TYPE_GENERIC = 1, ///< generic mode
} MI_DISP_IMPL_MhalPnlMipiDsiPacketType_e;

///< Define LVDSTX format
typedef enum
{
    E_MI_DISP_MHALPNL_LVDSTX_VESA  = 0, ///< LVDSTX format VESA
    E_MI_DISP_MHALPNL_LVDSTX_JEIDA = 1, ///< LVDSTX format JEIDA
} MI_DISP_IMPL_MhalPnlLvdsTxFormat_e;

///< Define number of lane of LVDSTX
typedef enum
{
    E_MI_DISP_MHALPNL_LVDSTX_LANE_NONE = 0, ///< NO defined
    E_MI_DISP_MHALPNL_LVDSTX_LANE_3    = 3, ///< 3 lane mode
    E_MI_DISP_MHALPNL_LVDSTX_LANE_4    = 4, ///< 4 lane mode
} MI_DISP_IMPL_MhalPnlLvdsTxLaneMode_e;

///< Define unified timing generator configuration
typedef struct
{
    //==========================================
    // panel timing SPEC.
    //==========================================
    // sync related
    MI_U16 u16HSyncWidth;     ///<  HSYNC Width
    MI_U16 u16HSyncBackPorch; ///<  HSYNC back porch

    MI_U16 u16VSyncWidth;     ///<  VSYNC width
    MI_U16 u16VSyncBackPorch; ///<  VSYNC back porch

    // DE related
    MI_U16 u16HStart;  ///<  HDE start
    MI_U16 u16VStart;  ///<  VDE start
    MI_U16 u16HActive; ///<  H Active = Panel Width
    MI_U16 u16VActive; ///<  V Active = Panel Height

    // DClk related
    MI_U16 u16HTotal; ///<  H Total
    MI_U16 u16VTotal; ///<  V Total
    MI_U32 u32Dclk;   ///<  DCLK(HTT * VTT * FPS)

    // interlace related
    MI_U16 u16OddVfdeStart;  ///<  Odd field vfde start
    MI_U16 u16OddVfdeEnd;    ///<  Odd field vfde End
    MI_U16 u16EvenVfdeStart; ///<  Even field vfde start
    MI_U16 u16EvenVfdeEnd;   ///<  Even field vfde end
    MI_U16 u16Field0Start;   ///<  Field0 start
    MI_U16 u16Field0End;     ///<  Field0 end
} MI_DISP_IMPL_MhalPnlUnifiedTgnTimingConfig_t;

///< Define polarity of unified timing configuration
typedef struct
{
    //==========================================
    // TTL polarity
    //==========================================
    MI_U8 u8InvDCLK;  ///<  CLK Invert
    MI_U8 u8InvDE;    ///<  DE Invert
    MI_U8 u8InvHSync; ///<  HSync Invert
    MI_U8 u8InvVSync; ///<  VSync Invert
} MI_DISP_IMPL_MhalPnlUnifiedTgnPolarityConfig_t;

///< Define output RGB order of unified timing configuration
typedef struct
{
    //==========================================
    // TTL RGB output swap SPEC.
    //==========================================
    MI_U8 u8SwapChnR;  ///<  Swap Channel R
    MI_U8 u8SwapChnG;  ///<  Swap Channel G
    MI_U8 u8SwapChnB;  ///<  Swap Channel B
    MI_U8 u8SwapRgbML; ///<  Swap RGB MSB/LSB
} MI_DISP_IMPL_MhalPnlUnifiedTgnRgbOutputSwapConfig_t;

///< Define SSC of unified timing configuration
typedef struct
{
    //==========================================
    // TTL spread spectrum SPEC.
    //==========================================
    MI_U16 u16SpreadSpectrumStep; ///<  Swap Channel R
    MI_U16 u16SpreadSpectrumSpan; ///<  Swap Channel G
} MI_DISP_IMPL_MhalPnlUnifiedTgnSpreadSpectrumConfig_t;

///< Define MCU initialization command configuration
typedef struct
{
    MI_U32 *pu32CmdBuf;
    MI_U32  u32CmdBufSize;
} MI_DISP_IMPL_MhalPnlUnifiedMcuInitCmdConfig_t;

///< Define MCU auto command configuration
typedef struct
{
    MI_U32 *pu32CmdBuf;
    MI_U32  u32CmdBufSize;
} MI_DISP_IMPL_MhalPnlUnifiedMcuAutoCmdConfig_t;

///< Define MCU unified configuration
typedef struct
{
    MI_U32                                        u32HActive;         ///< Horizontal active size
    MI_U32                                        u32VActive;         ///< Vertical active size
    MI_U8                                         u8WRCycleCnt;       ///< W/R cycle count
    MI_U8                                         u8CSLeadWRCycleCnt; ///< CS lead W/R cycle count
    MI_U8                                         u8RSLeadCSCycleCnt; ///< RS Lead CS cycle count
    MI_DISP_IMPL_MhalPnlMcuType_e                 enMcuType;          ///< MCU type
    MI_DISP_IMPL_MhalPnlMcuDataBusCfg_e           enMcuDataBusCfg;    ///< MCU data bus CFG
    MI_DISP_IMPL_MhalPnlUnifiedMcuInitCmdConfig_t stMcuInitCmd;       ///< Initialization MCU command
    MI_DISP_IMPL_MhalPnlUnifiedMcuAutoCmdConfig_t stMcuAutoCmd;       ///< Auto MCU command
} MI_DISP_IMPL_MhalPnlUnifiedMcuConfig_t;

///< Define RGB data type  configuration
typedef struct
{
    MI_U8                             u8RgbDswap; ///< Dswap of RGB
    MI_DISP_IMPL_MhalPnlRgbDataType_e eRgbDtype;  ///< Dtype of RGB
} MI_DISP_IMPL_MhalPnlUnifiedRgbDataConfig_t;

///< Define RGB delta configuration
typedef struct
{
    MI_U8                              u8DummyMode;
    MI_DISP_IMPL_MhalPnlRgbDeltaMode_e eOddLine;  ///< Delta mode for Odd line
    MI_DISP_IMPL_MhalPnlRgbDeltaMode_e eEvenLine; ///< Delta mode for Even line
} MI_DISP_IMPL_MhalPnlUnifiedRgbDeltaConfig_t;

///< Define unified MIPIDSI configuration
typedef struct
{
    MI_U8 u8HsTrail;   ///< HsTrail
    MI_U8 u8HsPrpr;    ///< HsPrpr
    MI_U8 u8HsZero;    ///< HsZero
    MI_U8 u8ClkHsPrpr; ///< ClkHsPrpr
    MI_U8 u8ClkHsExit; ///< ClkHsexit
    MI_U8 u8ClkTrail;  ///< ClkTrail
    MI_U8 u8ClkZero;   ///< ClkZero
    MI_U8 u8ClkHsPost; ///< ClkHsPost
    MI_U8 u8DaHsExit;  ///< DaHsexit
    MI_U8 u8ContDet;   ///< ConDet

    MI_U8 u8Lpx;    ///< X
    MI_U8 u8TaGet;  ///< TaGet
    MI_U8 u8TaSure; ///< TaSure
    MI_U8 u8TaGo;   ///< TaGo

    MI_U16 u16Hactive; ///< Horizontal active size
    MI_U16 u16Hpw;     ///< HSYNC width
    MI_U16 u16Hbp;     ///< Horizontal back porch
    MI_U16 u16Hfp;     ///< vertical back porch

    MI_U16 u16Vactive; ///< vertical active size
    MI_U16 u16Vpw;     ///< VSYNC Width
    MI_U16 u16Vbp;     ///< Vertical back porch
    MI_U16 u16Vfp;     ///< Vertical front porch

    MI_U16 u16Bllp; ///< Bllp
    MI_U16 u16Fps;  ///< Frame Rate

    MI_DISP_IMPL_MhalPnlMipiDsiLaneMode_e enLaneNum; ///< Number of lane
    MI_DISP_IMPL_MhalPnlMipiDsiFormat_e   enFormat;  ///< Output color format
    MI_DISP_IMPL_MhalPnlMipiDsiCtrlMode_e enCtrl;    ///< Control mode

    MI_DISP_IMPL_MhalPnlChannelSwapType_e enCh[5];
    MI_U8 *                               pu8CmdBuf;     ///< command Data
    MI_U32                                u32CmdBufSize; ///< command size

    MI_U8  u8SyncCalibrate;
    MI_U16 u16VirHsyncSt;
    MI_U16 u16VirHsyncEnd;
    MI_U16 u16VsyncRef;
    MI_U16 u16DataClkSkew;

    MI_U8 u8PolCh0; ///< Channel 0 polarity, 0:HW default, 1:positive, 2:negative
    MI_U8 u8PolCh1; ///< Channel 1 polarity, 0:HW default, 1:positive, 2:negative
    MI_U8 u8PolCh2; ///< Channel 2 polarity, 0:HW default, 1:positive, 2:negative
    MI_U8 u8PolCh3; ///< Channel 3 polarity, 0:HW default, 1:positive, 2:negative
    MI_U8 u8PolCh4; ///< Channel 4 polarity, 0:HW default, 1:positive, 2:negative

    MI_DISP_IMPL_MhalPnlMipiDsiPacketType_e enPacketType; ///< MIPIDSI command type
} MI_DISP_IMPL_MhalPnlUnifiedMipiDsiConfig_t;

///< Define unified Pad Driving configuration
typedef struct
{
    MI_U32 u32PadMode;    ///< pad mode
    MI_U8  u8PadDrvngLvl; ///< pad driving level to set
} MI_DISP_IMPL_MhalPnlUnifiedPadDrvngConfig_t;

///< Define unified LVDSTX configuration
typedef struct
{
    MI_DISP_IMPL_MhalPnlLvdsTxFormat_e   enFormat; ///< LVDSTX format
    MI_DISP_IMPL_MhalPnlLvdsTxLaneMode_e eLaneNum; ///< Number of lane

    MI_U8 u8SwapOddEven;
    MI_U8 u8SwapML;
    MI_U8 u8PolHsync; ///< HSYNC polarity
    MI_U8 u8PolVsync; ///< VSYNC polarity

    MI_U8 u8ClkLane;

    MI_U8 u8PolLane0; ///< Lane0 polarity
    MI_U8 u8PolLane1; ///< Lane1 polarity
    MI_U8 u8PolLane2; ///< Lane2 polarity
    MI_U8 u8PolLane3; ///< Lane3 polarity
    MI_U8 u8PolLane4; ///< Lane4 polarity

    MI_U8 u8SwapLane0; ///< Lane0 selection
    MI_U8 u8SwapLane1; ///< Lane1 selection
    MI_U8 u8SwapLane2; ///< Lane2 selection
    MI_U8 u8SwapLane3; ///< Lane3 selection
    MI_U8 u8SwapLane4; ///< Lane4 selection
} MI_DISP_IMPL_MhalPnlUnifiedLvdsTxConfig_t;

///< Define unified parameter configuration
typedef struct
{
    const MI_U8 pPanelName[128]; ///<  PanelName

    MI_U8                  bPanelDither;
    MI_DISP_Interface_e    eIntfType;
    MI_DISP_OutputTiming_e eDisplayTiming;

    //==========================================
    // Flags
    //==========================================
    // TGEN related
    MI_U8 u8TgnTimingFlag;      ///<  1: stTgnTimingInfo is available;0 not available
    MI_U8 u8TgnPolarityFlag;    ///<  1: stTgnPolarityInfo is available;0 not available
    MI_U8 u8TgnRgbSwapFlag;     ///<  1: stTgnRgbSwapInfo is available;0 not available
    MI_U8 u8TgnOutputBitMdFlag; ///<  1: eOutputFormatBitMode is available;0 not available
    MI_U8 u8TgnFixedDclkFlag;   ///<  1: u16FDclk is available;0 not available
    MI_U8 u8TgnSscFlag;         ///<  1: stTgnSscInfo is available;0 not available
    MI_U8 u8TgnPadMuxFlag;      ///<  1: u16PadMux is available;0 not available

    // MCU related
    MI_U8 u8McuPhaseFlag;      ///<  1 u8McuPhase is available;0 not available
    MI_U8 u8McuPolarityFlag;   ///<  1 u8McuPolarity is available;0 not available
    MI_U8 u8McuRsPolarityFlag; ///<  1 u8RsPolarity is available;0 not available
    MI_U8 u8McuConfigFlag;     ///<  1 stMcuInfo is available;0 not available

    MI_U8 u8RgbDataFlag;    ///<  1: stRgbDataInfo is available;0 not available
    MI_U8 u8RgbDeltaMdFlag; ///<  1: stRgbDeltaInfo is available;0 not available

    // MIPI DSI related
    MI_U8 u8MpdFlag; ///<  1: stMpdInfo is available;0 not available

    // BT1120 related
    MI_U8 u8Bt1120PhaseFlag; ///<  1: u8Bt1120Phase is available;0 not available

    // PAD driving related
    MI_U8 u8PadDrvngFlag;      ///<  1: u8PadDrvngInfo is available;0 not available
    MI_U8 u8PadDrvngPadMdFlag; ///<  1: u8PadDrvngInfo.u32PadMode is available;0 not available

    // Frame sync related
    MI_U8 u8FrameSyncFlag; ///<  1: eFrmSyncSrc is available;0 not available

    // LVDSTX
    MI_U8 u8LvdsTxFlag; ///<  1: stLvdsTxInfo is available;0 not available

    //==========================================
    // Parameters
    //==========================================
    // TGEN related
    MI_DISP_IMPL_MhalPnlUnifiedTgnTimingConfig_t         stTgnTimingInfo;      ///< timing generator configuration
    MI_DISP_IMPL_MhalPnlUnifiedTgnPolarityConfig_t       stTgnPolarityInfo;    ///< polarity configuration
    MI_DISP_IMPL_MhalPnlUnifiedTgnRgbOutputSwapConfig_t  stTgnRgbSwapInfo;     ///< Output RGB order configuration
    MI_DISP_IMPL_MhalPnlOutputFormatBitMode_e            eOutputFormatBitMode; ///< output bits configuration
    MI_U32                                               u32FDclk;             ///< Fixed DClk
    MI_U16                                               u16FDclk;             ///< Fixed DClk
    MI_DISP_IMPL_MhalPnlUnifiedTgnSpreadSpectrumConfig_t stTgnSscInfo;         ///< SSC configuration
    MI_U16                                               u16PadMux;            ///< Pad MUX

    // MCU related
    MI_U8                                  u8McuPhase;      ///<  MCU Phase
    MI_U8                                  u8McuPolarity;   ///<  MCU Polarity
    MI_U8                                  u8McuRsPolarity; ///<  MCU RsPolarity
    MI_DISP_IMPL_MhalPnlUnifiedMcuConfig_t stMcuInfo;       ///<  MCU configuration

    MI_DISP_IMPL_MhalPnlUnifiedRgbDataConfig_t  stRgbDataInfo;  ///< RGB data type configuration
    MI_DISP_IMPL_MhalPnlUnifiedRgbDeltaConfig_t stRgbDeltaInfo; ///< RGB delta configuration

    // MIPI DSI related
    MI_DISP_IMPL_MhalPnlUnifiedMipiDsiConfig_t stMpdInfo; ///< MIPIDSI configuration

    // BT1120 related
    MI_U8 u8Bt1120Phase; ///<  BT1120 Phase, means phase DIFF between data & CLK. 0:0; 1:90; 2:180; 3:270

    // PAD driving related
    MI_DISP_IMPL_MhalPnlUnifiedPadDrvngConfig_t stPadDrvngInfo; ///<  Pad Driving

    // LVDSTX related
    MI_DISP_IMPL_MhalPnlUnifiedLvdsTxConfig_t stLvdsTxInfo; ///< LVDSTX configuration

    MI_DISP_IMPL_MhalPnlScanMode_e ePnlScanMode;
    // Frame sync related
    MI_DISP_IMPL_MhalFrmSyncSrc_e eFrmSyncSrc;
} MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t;

typedef struct
{
    MI_U8 bEn;
} MI_DISP_IMPL_MhalPnlPowerConfig_t;

typedef struct
{
    MI_DISP_IMPL_MhalPnlMipiDsiPacketType_e enPacketType;
    MI_U32                                  u32CmdBufSize;
    MI_U8 *                                 pu8CmdBuf;
} MI_DISP_IMPL_MhalPnlMipiDsiWriteCmdConfig_t;

typedef struct
{
    MI_U8  u8RegAddr;
    MI_U32 u32ReadBufSize;
    MI_U8 *pu8ReadBuf;
} MI_DISP_IMPL_MhalPnlMipiDsiReadCmdConfig_t;

// mhal
///< Define timing configuration for timing table
typedef struct
{
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t stPnlUniParamCfg; ///< Attribute of timing
    MI_U32                                   u32OutputDev;     ///< Output Device
    MI_U8                                    bValid;           ///< 1: valiad, 0: non-valid
} MI_DISP_IMPL_MhalPanelConfig_t;

///< Define timing inforamtion of device output resolution
typedef struct
{
    MI_DISP_OutputTiming_e eTimeType;   ///< output timing ID
    MI_DISP_SyncInfo_t *   pstSyncInfo; ///< timing sync information
} MI_DISP_IMPL_MhalDeviceTimingInfo_t;

///< Define video frame configuration
typedef struct
{
    MI_SYS_PixelFormat_e  ePixelFormat;  ///< Pixel format
    MI_SYS_CompressMode_e eCompressMode; ///< Compress mode

    MI_SYS_FrameTileMode_e eTileMode; ///< Tile mode

    MI_PHY aPhyAddr[3];   ///< Physical address
    MI_U32 au32Stride[3]; ///< Memory stride
} MI_DISP_IMPL_MhalVideoFrameData_t;

///< Define register flip configuration
typedef struct
{
    MI_U8                      bEnable;  ///< 1: enable, 0: disable
    MHAL_CMDQ_CmdqInterface_t *pCmdqInf; ///< CMDQ interface handler
} MI_DISP_IMPL_MhalRegFlipConfig_t;

///< Define register waiting done configuration
typedef struct
{
    MI_U32                     u32WaitType; ///< wait type
    MHAL_CMDQ_CmdqInterface_t *pCmdqInf;    ///< CMDQ interface handler
} MI_DISP_IMPL_MhalRegWaitDoneConfig_t;

///< Define CMDQ interface configuration
typedef struct
{
    MHAL_CMDQ_CmdqInterface_t *pCmdqInf; ///< CMDQ interface handler
} MI_DISP_IMPL_MhalCmdqInfConfig_t;

///< Define VideoLayer capability configuration
typedef struct
{
    MI_U8  u8DevId;                    ///< device id want to bind;
    MI_U8  bRotateSupport;             ///< 1: supported, 0:non-supported
    MI_U32 u32InputPortHwCnt;          ///< Count of Inputport
    MI_U8  bCompressFmtSupport;        ///< 1: supported, 0:non-supported
    MI_U32 u32InputPortPitchAlignment; ///< Pitch alignmnet
    MI_U32 u32RotateHeightAlighment;   ///< Height Alignment in ratate case
    MI_U32 u32RotateStrideAlighment;   ///< Stride Alignment in ratate case
    MI_U32 u32LayerTypeHwCnt;          ///< The layer can support count in device.
} MI_DISP_IMPL_MhalVideoLayerCapabilityConfig_t;

///< Define Inputport capability configuration
typedef struct
{
    MI_U32 u32HwCount;                           ///< Count of Inputport
    MI_U8  bSupportTiming[E_MI_DISP_OUTPUT_MAX]; ///< 1: supported, 0:non-supported
} MI_DISP_IMPL_MhalInterfaceCapabilityConfig_t;

///< Define IRQ flag type
typedef struct
{
    MI_U32 u32IrqMask; ///< Irq mask
    MI_U32 u32IrqFlag; ///< Irq flag
} MI_DISP_IMPL_MhalIRQFlag_t;

///< Define PreSuspend type
typedef enum
{
    E_MI_DISP_PRESUSPEND_IDLE   = 0,
    E_MI_DISP_PRESUSPEND_GOING  = 1,
    E_MI_DISP_PRESUSPEND_REPENT = 2,
    E_MI_DISP_PRESUSPEND_DONE   = 3,
} MI_DISP_IMPL_MhalPreSuspendType_e;

///< Define Pre Suspend configuration
typedef struct
{
    MI_DISP_IMPL_MhalPreSuspendType_e enPreSuspendType;
} MI_DISP_IMPL_MhalPreSuspendConfig_t;

///< Define PQ input configuration
typedef struct
{
    MI_DISP_IMPL_MhalPqInputType_e enType;
    MI_U16                         u16Width;
    MI_U16                         u16Height;
} MI_DISP_IMPL_MhalPqInputConfig_t;

#endif
