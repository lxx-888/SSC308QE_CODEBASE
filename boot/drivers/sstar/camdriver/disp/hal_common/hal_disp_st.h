/*
 * hal_disp_st.h- Sigmastar
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

#ifndef _HAL_DISP_ST_H_
#define _HAL_DISP_ST_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define HAL_DISP_INTF_HDMI  (0x00000001)
#define HAL_DISP_INTF_CVBS  (0x00000002)
#define HAL_DISP_INTF_VGA   (0x00000004)
#define HAL_DISP_INTF_YPBPR (0x00000008)
#define HAL_DISP_INTF_LCD   (0x00000010)
#define HAL_DISP_INTF_HVP   (0x00000040)
#define HAL_DISP_INTF_DMA   (0x00000020)

#define HAL_DISP_INTF_TTL              (0x00010000)
#define HAL_DISP_INTF_MIPIDSI          (0x00020000)
#define HAL_DISP_INTF_BT656            (0x00040000)
#define HAL_DISP_INTF_BT601            (0x00080000)
#define HAL_DISP_INTF_BT1120           (0x00100000)
#define HAL_DISP_INTF_MCU              (0x00200000)
#define HAL_DISP_INTF_SRGB             (0x00400000)
#define HAL_DISP_INTF_MCU_NOFLM        (0x00800000)
#define HAL_DISP_INTF_BT1120_DDR       (0x01000000)
#define HAL_DISP_INTF_LVDS             (0x02000000)
#define HAL_DISP_INTF_LVDS_1           (0x04000000)
#define HAL_DISP_INTF_DUAL_LVDS        (0x08000000)
#define HAL_DISP_INTF_MIPIDSI_1        (0x10000000)
#define HAL_DISP_INTF_PNL              (DISP_SUPPORT_PNL)
#define HAL_DISP_INTF_HW_LCD           (0x01FD0000)
#define DISP_DMA_IS_FRAME_MODE(eMode)  ((eMode) == E_HAL_DISP_ST_DMA_OUTPUT_MODE_FRAME)
#define DISP_DMA_IS_RING_MODE(eMode)   ((eMode) == E_HAL_DISP_ST_DMA_OUTPUT_MODE_RING)
#define DISP_OUTDEV_IS_CVBS(eMode)     ((eMode)&HAL_DISP_INTF_CVBS)
#define DISP_OUTDEV_IS_PNL(eMode)      ((eMode)&HAL_DISP_INTF_PNL)
#define DISP_OUTDEV_IS_HDMI(eMode)     ((eMode)&HAL_DISP_INTF_HDMI)
#define DISP_OUTDEV_IS_VGA(eMode)      ((eMode)&HAL_DISP_INTF_VGA)
#define DISP_OUTDEV_IS_HVP(eMode)      ((eMode)&HAL_DISP_INTF_HVP)
#define DISP_OUTDEV_IS_DMA(eMode)      ((eMode)&HAL_DISP_INTF_DMA)
#define DISP_FPLL_DEVICE_FLAG(Dev)     ((Dev < HAL_DISP_DEVICE_MAX) ? (1 << Dev) : 0)
#define DISP_IN_STR_STATUS(pstDispCtx) ((pstDispCtx->pstCtxContain->stStr.u8bSuspendEn))
#define HAL_DISP_DAC_RESET_OFF         0
#define HAL_DISP_DAC_RESET_ON          1
#define HAL_DISP_DAC_RESET_FORCE       2
#define DISP_ALIGN_4(x)                (((x + 0x3) >> 2) << 2)
#define DISP_ALIGN_8(x)                (((x + 0x7) >> 3) << 3)
#define DISP_ALIGN_16(x)               (((x + 0xF) >> 4) << 4)
#define DISP_ALIGN_32(x)               (((x + 0x1F) >> 5) << 5)
#define DISP_ALIGN_32_L(x)             (((x) >> 5) << 5)
#define DISP_ALIGN_32_REM(x)           ((x)&0x1F)
#define DISP_ALIGN_32_COMP(x)          (DISP_ALIGN_32(x) - (x))
#define DISP_ALIGN_64(x)               (((x + 0x3F) >> 6) << 6)
#define DISP_ALIGN_64_L(x)             (((x) >> 6) << 6)
#define DISP_ALIGN_512(x)              (((x + 0x1FF) >> 9) << 9)

#define PARSING_LINKTYPE_TO_PUSETYPE(x)                        \
    (x == HAL_DISP_INTF_TTL          ? MDRV_PUSE_TTL24_LCK     \
     : x == HAL_DISP_INTF_MIPIDSI    ? MDRV_PUSE_TX_MIPI_P_CH0 \
     : x == HAL_DISP_INTF_MIPIDSI_1  ? MDRV_PUSE_TX_MIPI_P_CH1 \
     : x == HAL_DISP_INTF_BT656      ? MDRV_PUSE_BT656_LCK     \
     : x == HAL_DISP_INTF_BT601      ? MDRV_PUSE_BT601_LCK     \
     : x == HAL_DISP_INTF_BT1120     ? MDRV_PUSE_BT1120_LCK    \
     : x == HAL_DISP_INTF_BT1120_DDR ? MDRV_PUSE_BT1120_LCK    \
     : x == HAL_DISP_INTF_MCU        ? MDRV_PUSE_MCU8_CS       \
     : x == HAL_DISP_INTF_SRGB       ? MDRV_PUSE_RGB8_LCK      \
                                     : MDRV_PUSE_BT1120_LCK)

//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_DISP_ST_QUERY_RET_OK = 0,
    E_HAL_DISP_ST_QUERY_RET_CFGERR,
    E_HAL_DISP_ST_QUERY_RET_NOTSUPPORT,
    E_HAL_DISP_ST_QUERY_RET_NONEED,
    E_HAL_DISP_ST_QUERY_RET_IMPLICIT_ERR,
} HAL_DISP_ST_QueryRet_e;

typedef enum
{
    E_HAL_DISP_ST_QUERY_DEVICE_INIT,
    E_HAL_DISP_ST_QUERY_DEVICE_DEINIT,
    E_HAL_DISP_ST_QUERY_DEVICE_ENABLE,
    E_HAL_DISP_ST_QUERY_DEVICE_ATTACH,
    E_HAL_DISP_ST_QUERY_DEVICE_DETACH,
    E_HAL_DISP_ST_QUERY_DEVICE_BACKGROUND_COLOR,
    E_HAL_DISP_ST_QUERY_DEVICE_INTERFACE,
    E_HAL_DISP_ST_QUERY_DEVICE_OUTPUTTIMING,
    E_HAL_DISP_ST_QUERY_DEVICE_PARAM,
    E_HAL_DISP_ST_QUERY_DEVICE_COLORTEMP,
    E_HAL_DISP_ST_QUERY_DEVICE_GAMMA_PARAM,
    E_HAL_DISP_ST_QUERY_DEVICE_TIME_ZONE,
    E_HAL_DISP_ST_QUERY_DEVICE_DISPLAY_INFO,
    E_HAL_DISP_ST_QUERY_DEVICE_TIME_ZONE_CFG,
    E_HAL_DISP_ST_QUERY_VIDEOLAYER_INIT,
    E_HAL_DISP_ST_QUERY_VIDEOLAYER_ENABLE,
    E_HAL_DISP_ST_QUERY_VIDEOLAYER_BIND,
    E_HAL_DISP_ST_QUERY_VIDEOLAYER_UNBIND,
    E_HAL_DISP_ST_QUERY_VIDEOLAYER_ATTR,
    E_HAL_DISP_ST_QUERY_VIDEOLAYER_COMPRESS,
    E_HAL_DISP_ST_QUERY_VIDEOLAYER_PRIORITY,
    E_HAL_DISP_ST_QUERY_VIDEOLAYER_BUFFER_FIRE,
    E_HAL_DISP_ST_QUERY_VIDEOLAYER_CHECK_FIRE,
    E_HAL_DISP_ST_QUERY_INPUTPORT_INIT,
    E_HAL_DISP_ST_QUERY_INPUTPORT_ENABLE,
    E_HAL_DISP_ST_QUERY_INPUTPORT_ATTR,
    E_HAL_DISP_ST_QUERY_INPUTPORT_SHOW,
    E_HAL_DISP_ST_QUERY_INPUTPORT_HIDE,
    E_HAL_DISP_ST_QUERY_INPUTPORT_BEGIN,
    E_HAL_DISP_ST_QUERY_INPUTPORT_END,
    E_HAL_DISP_ST_QUERY_INPUTPORT_FLIP,
    E_HAL_DISP_ST_QUERY_INPUTPORT_ROTATE,
    E_HAL_DISP_ST_QUERY_INPUTPORT_CROP,
    E_HAL_DISP_ST_QUERY_INPUTPORT_RING_BUFF_ATTR,
    E_HAL_DISP_ST_QUERY_INPUTPORT_IMIADDR,
    E_HAL_DISP_ST_QUERY_CLK_SET,
    E_HAL_DISP_ST_QUERY_CLK_GET,
    E_HAL_DISP_ST_QUERY_PQ_SET,
    E_HAL_DISP_ST_QUERY_PQ_GET,
    E_HAL_DISP_ST_QUERY_DRVTURNING_SET,
    E_HAL_DISP_ST_QUERY_DRVTURNING_GET,
    E_HAL_DISP_ST_QUERY_DBGMG_GET,
    E_HAL_DISP_ST_QUERY_REG_ACCESS,
    E_HAL_DISP_ST_QUERY_REG_FLIP,
    E_HAL_DISP_ST_QUERY_REG_WAITDONE,
    E_HAL_DISP_ST_QUERY_DMA_INIT,
    E_HAL_DISP_ST_QUERY_DMA_DEINIT,
    E_HAL_DISP_ST_QUERY_DMA_BIND,
    E_HAL_DISP_ST_QUERY_DMA_UNBIND,
    E_HAL_DISP_ST_QUERY_DMA_ATTR,
    E_HAL_DISP_ST_QUERY_DMA_BUFFERATTR,
    E_HAL_DISP_ST_QUERY_HW_INFO,
    E_HAL_DISP_ST_QUERY_CLK_INIT,
    E_HAL_DISP_ST_QUERY_CMDQINF,
    E_HAL_DISP_ST_QUERY_INTERCFG_SET,
    E_HAL_DISP_ST_QUERY_INTERCFG_GET,
    E_HAL_DISP_ST_QUERY_DEVICE_PARAM_GET,
    E_HAL_DISP_ST_QUERY_DEVICE_CAPABILITY_GET,
    E_HAL_DISP_ST_QUERY_VIDLAYER_CAPABILITY_GET,
    E_HAL_DISP_ST_QUERY_INTERFACE_CAPABILITY_GET,
    E_HAL_DISP_ST_QUERY_DMA_CAPABILITY_GET,
    E_HAL_DISP_ST_QUERY_SUSPEND,
    E_HAL_DISP_ST_QUERY_RESUME,
    E_HAL_DISP_ST_QUERY_PNL_UNIFIED_PARAM_SET,
    E_HAL_DISP_ST_QUERY_PNL_UNIFIED_PARAM_GET,
    E_HAL_DISP_ST_QUERY_PNL_POWER_SET,
    E_HAL_DISP_ST_QUERY_PNL_MIPIDSI_CMD_WRITE,
    E_HAL_DISP_ST_QUERY_PNL_MIPIDSI_CMD_READ,
    E_HAL_DISP_ST_QUERY_FPLL_LOCK_STATUS_GET,
    E_HAL_DISP_ST_QUERY_MAX,
} HAL_DISP_ST_QueryType_e;

typedef enum
{
    E_HAL_DISP_ST_PQ_INPUT_RES_NONE = 0x0000,
    E_HAL_DISP_ST_PQ_INPUT_RES_SD   = 0x0001,
    E_HAL_DISP_ST_PQ_INPUT_RES_HD   = 0x0002,
    E_HAL_DISP_ST_PQ_INPUT_RES_FHD  = 0x0003,
    E_HAL_DISP_ST_PQ_INPUT_RES_4K   = 0x0004,
} HAL_DISP_ST_PqInputRes_e;

typedef enum
{
    E_HAL_DISP_ST_GAMMA_R,
    E_HAL_DISP_ST_GAMMA_G,
    E_HAL_DISP_ST_GAMMA_B,
    E_HAL_DISP_ST_GAMMA_TYPE,
} HAL_DISP_ST_GammaType_e;

typedef enum
{
    E_HAL_DISP_TIMEZONE_NONE       = 0x00,
    E_HAL_DISP_TIMEZONE_SYNC       = 0x01,
    E_HAL_DISP_TIMEZONE_BACKPORCH  = 0x02,
    E_HAL_DISP_TIMEZONE_ACTIVE     = 0x03,
    E_HAL_DISP_TIMEZONE_FRONTPORCH = 0x04,
    E_HAL_DISP_TIMEZONE_NUM        = 0x05,
} HAL_DISP_ST_TimeZoneType_e;

typedef enum
{
    E_HAL_DISP_ST_HW_INFO_DEVICE,
    E_HAL_DISP_ST_HW_INFO_VIDEOLAYER,
    E_HAL_DISP_ST_HW_INFO_INPUTPORT,
    E_HAL_DISP_ST_HW_INFO_DMA,
    E_HAL_DISP_ST_HW_INFO_NUM,
} HAL_DISP_ST_HwInfoType_e;

typedef enum
{
    E_HAL_DISP_ST_COLOR_SPACE_YCC_BT601  = 0x1,     /* YCbCr */
    E_HAL_DISP_ST_COLOR_SPACE_YCC_BT709  = 0x2,     /* YCbCr */
    E_HAL_DISP_ST_COLOR_SPACE_YCC_BT2020 = 0x4,     /* YCbCr */
    E_HAL_DISP_ST_COLOR_SPACE_YCC_BT2100 = 0x8,     /* YCbCr */
    E_HAL_DISP_ST_COLOR_SPACE_YUV        = 0xFF,    /* YUV */
    E_HAL_DISP_ST_COLOR_SPACE_RGB_SDTV   = 0x100,   /* RGB */
    E_HAL_DISP_ST_COLOR_SPACE_RGB_HDTV   = 0x200,   /* RGB */
    E_HAL_DISP_ST_COLOR_SPACE_RGB_UHDTV  = 0x400,   /* RGB */
    E_HAL_DISP_ST_COLOR_SPACE_RGB_sRGB   = 0x800,   /* RGB */
    E_HAL_DISP_ST_COLOR_SPACE_RGB        = 0xFF00,  /* RGB */
    E_HAL_DISP_ST_COLOR_SPACE_HSI        = 0x10000, /* HSI */
} HAL_DISP_ST_ColorSpaceType_e;

typedef enum
{
    E_HAL_DISP_ST_MMAP_XC_MAIN = 0, /* XC Main buffer */
    E_HAL_DISP_ST_MMAP_XC_MENULOAD, /* XC Menuload buffer */
    E_HAL_DISP_ST_MMAP_MAX,
} HAL_DISP_ST_MmapType_e;

typedef enum
{
    E_HAL_DISP_ST_DRV_TURNING_RGB,
    E_HAL_DISP_ST_DRV_TURNING_CVBS,
    E_HAL_DISP_ST_DRV_TRUNING_NUM,
} HAL_DISP_ST_DrvTruningType_e;

typedef enum
{
    E_HAL_DISP_ST_PQ_LOAD_SETTING_AUTO,
    E_HAL_DISP_ST_PQ_LOAD_SETTING_MANUAL,
} HAL_DISP_ST_PqLoadSettingType_e;

typedef enum
{
    E_HAL_DISP_ST_DEV_PQ_MACE,
    E_HAL_DISP_ST_DEV_PQ_HPQ,
    E_HAL_DISP_ST_DEV_PQ_LITE,
} HAL_DISP_ST_DevicePqType_e;

typedef enum
{
    E_HAL_DISP_ST_PRESUSPEND_IDLE   = 0,
    E_HAL_DISP_ST_PRESUSPEND_GOING  = 1,
    E_HAL_DISP_ST_PRESUSPEND_REPENT = 2,
    E_HAL_DISP_ST_PRESUSPEND_DONE   = 3,
} HAL_DISP_ST_PreSuspendType_e;

typedef enum
{
    E_HAL_DISP_ST_CRC16_OFF,
    E_HAL_DISP_ST_CRC16_EXT,
    E_HAL_DISP_ST_CRC16_OVERWRITE,
} HAL_DISP_ST_Crc16Type_e;

typedef enum
{
    E_HAL_DISP_ST_DEV_SYNTH_HDMI,
    E_HAL_DISP_ST_DEV_SYNTH_VGA,
    E_HAL_DISP_ST_DEV_SYNTH_CVBS,
    E_HAL_DISP_ST_DEV_SYNTH_LCD,
    E_HAL_DISP_ST_DEV_SYNTH_MAX,
} HAL_DISP_ST_DeviceSynthType_e;

typedef enum
{
    E_HAL_DISP_ST_GOP_ORDER_MOPLOWERALL    = 0,
    E_HAL_DISP_ST_GOP_ORDER_MOPUPPERUI     = (0x1 << E_MI_DISP_VIDEOLAYER_UI),
    E_HAL_DISP_ST_GOP_ORDER_MOPUPPERCURSOR = (0x1 << E_MI_DISP_VIDEOLAYER_CURSOR),
    E_HAL_DISP_ST_GOP_ORDER_MOPUPPERALL = (E_HAL_DISP_ST_GOP_ORDER_MOPUPPERUI | E_HAL_DISP_ST_GOP_ORDER_MOPUPPERCURSOR),
    E_HAL_DISP_ST_GOP_ORDER_TYPE,
} HAL_DISP_ST_GopOrderState_e;

typedef enum
{
    E_HAL_DISP_ST_GAMMA_CURVE_POINT_NONE = 0,   ///< NO defined
    E_HAL_DISP_ST_GAMMA_CURVE_POINT_33   = 33,  ///< input 33 gamma curve points
    E_HAL_DISP_ST_GAMMA_CURVE_POINT_256  = 256, ///< input 256 gamma curve points
} HAL_DISP_ST_GammaCurvePointNum_e;

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

typedef struct
{
    HAL_DISP_ST_QueryType_e enQueryType;
    void *                  pInCfg;
    MI_U32                  u32CfgSize;
} HAL_DISP_ST_QueryInConfig_t;

typedef struct
{
    MI_U8  u8bEn[3];
    MI_U8  u8fifosize[3];
    MI_U8  u8fifostart[3];
    MI_U16 u16Width[3];
    MI_U16 u16Height[3];
    MI_U16 u16Pixel[3];
    MI_U16 u16Stride[3];
    MI_U16 u16BuffHeight[3];
} HAL_DISP_ST_FormatRela_t;

typedef struct
{
    HAL_DISP_ST_QueryRet_e enQueryRet;
    void (*pSetFunc)(void *, void *);
} HAL_DISP_ST_QueryOutConfig_t;

typedef struct
{
    HAL_DISP_ST_QueryInConfig_t  stInCfg;
    HAL_DISP_ST_QueryOutConfig_t stOutCfg;
} HAL_DISP_ST_QueryConfig_t;

typedef struct
{
    MI_U8  u8bTimegenStart;
    MI_U8  u8bBootInit;
    MI_U8  u8bInterfaceChange;
    MI_U8  u8bTimegenChange;
    MI_U16 u16Htotal;
    MI_U16 u16Vtotal;
    MI_U16 u16Fps;
    MI_U32 u32Dclk;
    MI_U32 u32Interface;
} HAL_DISP_ST_DeviceChangeTimingConfig_t;

typedef struct
{
    MI_U16 u16HsyncWidth;
    MI_U16 u16HsyncBackPorch;
    MI_U16 u16Hstart;
    MI_U16 u16Hactive;
    MI_U16 u16Htotal;

    MI_U16 u16VsyncWidth;
    MI_U16 u16VsyncBackPorch;
    MI_U16 u16Vstart;
    MI_U16 u16Vactive;
    MI_U16 u16Vtotal;
    MI_U16 u16Fps;

    MI_U16 u16SscStep;
    MI_U16 u16SscSpan;
    MI_U32 u32VSyncPeriod;
    MI_U32 u32VBackPorchPeriod;
    MI_U32 u32VActivePeriod;
    MI_U32 u32VFrontPorchPeriod;
    MI_U32 u32Dclk;
} HAL_DISP_ST_DeviceTimingConfig_t;

typedef struct
{
    MI_DISP_OutputTiming_e           eTimeType;
    HAL_DISP_ST_DeviceTimingConfig_t stDeviceTimingCfg;
} HAL_DISP_ST_DeviceTimingInfo_t;

typedef struct
{
    MI_U8         bEnable; /* color space */
    MI_DISP_Csc_t stCsc;   /* color space */
    MI_U32        u32Gain; /* current gain of VGA signals. [0, 64). default:0x30 */
    MI_U32        u32Sharpness;
} HAL_DISP_ST_DeviceParam_t;

typedef struct
{
    MI_SYS_PixelFormat_e enPixelFmt;
    MI_U16               u16Width;
    MI_U16               u16Height;
    MI_U8                bEn;
    u64                  au64PhyAddr[3];
    MI_U32               au32Stride[3];
} HAL_DISP_ST_HwDmaConfig_t;

typedef struct
{
    MI_U8  bEn[E_HAL_DISP_CLK_NUM];
    MI_U32 u32Rate[E_HAL_DISP_CLK_NUM];
    MI_U32 u32Num;
} HAL_DISP_ST_ClkConfig_t;

typedef struct
{
    HAL_DISP_ST_DrvTruningType_e enType;
    MI_U16                       u16Trim[4];
} HAL_DISP_ST_DrvTurningConfig_t;

typedef struct
{
    MI_U32 u32DbgmgFlags;
    void * pData;
} HAL_DISP_ST_DbgmgConfig_t;

typedef struct
{
    MI_U32 u32DevId;
    void * pCmdqInf;
} HAL_DISP_ST_CmdqInfConfig_t;

typedef struct
{
    HAL_DISP_ST_HwInfoType_e eType;
    MI_U32                   u32Id;
    MI_U32                   u32Count;
} HAL_DISP_ST_HwInfoConfig_t;

typedef struct
{
    MI_U8 bEn;
} HAL_DISP_ST_ClkInitConfig_t;

typedef struct
{
    MI_U32                                    u32DevId;
    MI_DISP_IMPL_MhalDeviceCapabilityConfig_t stDevCapCfg;
} HAL_DISP_ST_DeviceCapabilityConfig_t;

typedef struct
{
    MI_DISP_IMPL_MhalVideoLayerType_e             eVidLayerType;
    MI_DISP_IMPL_MhalVideoLayerCapabilityConfig_t stVidLayerCapCfg;
} HAL_DISP_ST_VideoLayerCapabilityConfig_t;

typedef struct
{
    MI_U32                                       u32Interface;
    MI_DISP_IMPL_MhalInterfaceCapabilityConfig_t stInterfaceCapCfg;
} HAL_DISP_ST_InterfaceCapabilityConfig_t;

typedef struct
{
    MI_U32                                 u32DmaId;
    MI_DISP_IMPL_MhalDmaCapabiliytConfig_t stDmaCapCfg;
} HAL_DISP_ST_DmaCapabilitytConfig_t;

typedef struct
{
    MI_U32 u32Addr;
    MI_U8  u8Val;
    MI_U8  u8Mask;
} HAL_DISP_ST_PqLoadEntry_t;

typedef struct
{
    MI_U8                bRotEn;
    MI_U32               u32InputPortId;
    MI_DISP_RotateMode_e eRotAng;
    MI_U16               u16RotSourceHeight;
    MI_U16               u16RotSourceWidth;
    MI_U8                bCropXEn;
    MI_U8                bCropYEn;
    MI_U16               u16ClipXNum;
    MI_U16               u16ClipXNumComp;
    MI_U16               u16ClipYNum;
    MI_U16               u16ClipYNumComp;
} HAL_DISP_ST_MopRotateConfig_t;

typedef struct
{
    MI_DISP_IMPL_MhalPreSuspendType_e enPreSuspendType;
    MI_U8                             u8bSuspendEn;
    void *                            pData;
} HAL_DISP_ST_StrConfig_t;

typedef struct
{
    MI_U32 address;
    MI_U16 value;
} HAL_DISP_ST_PnlLpllTbl_t;

#endif
