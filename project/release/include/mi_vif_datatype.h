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
#ifndef _MI_VIF_DATATYPE_H_
#define _MI_VIF_DATATYPE_H_

#include "mi_sys_datatype.h"

#pragma pack(push)
#pragma pack(4)

typedef enum
{
    E_MI_VIF_ERR_INVALID_PORTID = E_MI_ERR_MAX + 1,
    E_MI_VIF_ERR_FAILED_DEVNOTENABLE,   /* device not enable*/
    E_MI_VIF_ERR_FAILED_DEVNOTDISABLE,  /* device not disable*/
    E_MI_VIF_ERR_FAILED_PORTNOTENABLE,  /* port not enable*/
    E_MI_VIF_ERR_FAILED_PORTNOTDISABLE, /* port not disable*/
    E_MI_VIF_ERR_CFG_TIMEOUT,           /* config timeout*/
    E_MI_VIF_ERR_NORM_UNMATCH,          /* video norm of ADC and VIU is unmatch*/
    E_MI_VIF_ERR_INVALID_WAYID,         /* invlalid way ID     */
    E_MI_VIF_ERR_INVALID_PHYCHNID,      /* invalid phychn id*/
    E_MI_VIF_ERR_FAILED_NOTBIND,        /* device or channel not bind */
    E_MI_VIF_ERR_FAILED_BINDED,         /* device or channel not unbind */
} MI_VIF_ErrCode_e;

#define MI_VIF_SUCCESS (0)
#define MI_VIF_FAIL    (1)

#define MI_ERR_VIF_INVALID_DEVID    MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID)
#define MI_ERR_VIF_INVALID_CHNID    MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_CHNID)
#define MI_ERR_VIF_INVALID_PARA     MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM)
#define MI_ERR_VIF_INVALID_NULL_PTR MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NULL_PTR)
#define MI_ERR_VIF_FAILED_NOTCONFIG MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_CONFIG)
#define MI_ERR_VIF_NOT_SUPPORT      MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT)
#define MI_ERR_VIF_NOT_PERM         MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_PERM)
#define MI_ERR_VIF_NOMEM            MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOMEM)
#define MI_ERR_VIF_BUF_EMPTY        MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_EMPTY)
#define MI_ERR_VIF_BUF_FULL         MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_FULL)
#define MI_ERR_VIF_SYS_NOTREADY     MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SYS_NOTREADY)

#define MI_ERR_VIF_BUSY MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUSY)
#define MI_ERR_VIF_INVALID_PORTID \
    MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_VIF_ERR_INVALID_PORTID) /* 0xA0108040*/
#define MI_ERR_VIF_FAILED_DEVNOTENABLE \
    MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_VIF_ERR_FAILED_DEVNOTENABLE) /* 0xA0108040*/
#define MI_ERR_VIF_FAILED_DEVNOTDISABLE \
    MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_VIF_ERR_FAILED_DEVNOTDISABLE) /* 0xA0108041*/
#define MI_ERR_VIF_FAILED_PORTNOTENABLE \
    MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_VIF_ERR_FAILED_PORTNOTENABLE) /* 0xA0108042*/
#define MI_ERR_VIF_FAILED_PORTNOTDISABLE \
    MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_VIF_ERR_FAILED_PORTNOTDISABLE) /* 0xA0108042*/
#define MI_ERR_VIF_CFG_TIMEOUT \
    MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_VIF_ERR_CFG_TIMEOUT) /* 0xA0108043*/
#define MI_ERR_VIF_NORM_UNMATCH \
    MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_VIF_ERR_NORM_UNMATCH) /* 0xA0108044*/
#define MI_ERR_VIF_INVALID_WAYID \
    MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_VIF_ERR_INVALID_WAYID) /* 0xA0108045*/
#define MI_ERR_VIF_INVALID_PHYCHNID \
    MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_VIF_ERR_INVALID_PHYCHNID) /* 0xA0108046*/
#define MI_ERR_VIF_FAILED_NOTBIND \
    MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_VIF_ERR_FAILED_NOTBIND) /* 0xA0108047*/
#define MI_ERR_VIF_FAILED_BINDED \
    MI_DEF_ERR(E_MI_MODULE_ID_VIF, E_MI_ERR_LEVEL_ERROR, E_MI_VIF_ERR_FAILED_BINDED) /* 0xA0108048*/

typedef MI_U32 MI_VIF_GROUP;
typedef MI_U32 MI_VIF_DEV;
typedef MI_U32 MI_VIF_PORT;

#define MI_VIF_MAX_GROUP_DEV_CNT 4

typedef enum
{
    E_MI_VIF_MODE_BT656,
    E_MI_VIF_MODE_DIGITAL_CAMERA,
    E_MI_VIF_MODE_BT1120_STANDARD,
    E_MI_VIF_MODE_BT1120_INTERLEAVED,
    E_MI_VIF_MODE_MIPI,
    E_MI_VIF_MODE_LVDS,
    E_MI_VIF_MODE_MAX
} MI_VIF_IntfMode_e;

typedef enum
{
    E_MI_VIF_WORK_MODE_1MULTIPLEX,
    E_MI_VIF_WORK_MODE_2MULTIPLEX,
    E_MI_VIF_WORK_MODE_4MULTIPLEX,

    E_MI_VIF_WORK_MODE_MAX
} MI_VIF_WorkMode_e;

typedef enum
{
    E_MI_VIF_FRAMERATE_FULL,
    E_MI_VIF_FRAMERATE_HALF,
    E_MI_VIF_FRAMERATE_QUARTER,
    E_MI_VIF_FRAMERATE_OCTANT,
    E_MI_VIF_FRAMERATE_THREE_QUARTERS,
    E_MI_VIF_FRAMERATE_MAX
} MI_VIF_FrameRate_e;

typedef enum
{
    E_MI_VIF_CLK_EDGE_SINGLE_UP,
    E_MI_VIF_CLK_EDGE_SINGLE_DOWN,
    E_MI_VIF_CLK_EDGE_DOUBLE,
    E_MI_VIF_CLK_EDGE_MAX
} MI_VIF_ClkEdge_e;

typedef enum
{
    E_MI_VIF_HDR_TYPE_OFF,
    E_MI_VIF_HDR_TYPE_VC, // virtual channel mode HDR,vc0->long, vc1->short
    E_MI_VIF_HDR_TYPE_DOL,
    E_MI_VIF_HDR_TYPE_COMP, // compressed HDR mode
    E_MI_VIF_HDR_TYPE_LI,   // Line interlace HDR
    E_MI_VIF_HDR_TYPE_COMPVS,
    E_MI_VIF_HDR_TYPE_DCG, // Dual conversion gain HDR
    E_MI_VIF_HDR_TYPE_MAX
} MI_VIF_HDRType_e;

typedef enum
{
    E_MI_VIF_HDR_FUSION_TYPE_NONE,
    E_MI_VIF_HDR_FUSION_TYPE_2T1,
    E_MI_VIF_HDR_FUSION_TYPE_3T1,
    E_MI_VIF_HDR_FUSION_TYPE_MAX
} MI_VIF_HDRFusionType_e;

typedef enum
{
    E_MI_VIF_HDR_EXPOSURE_TYPE_NONE,
    E_MI_VIF_HDR_EXPOSURE_TYPE_SHORT  = 0x1,
    E_MI_VIF_HDR_EXPOSURE_TYPE_MEDIUM = 0x2,
    E_MI_VIF_HDR_EXPOSURE_TYPE_LONG   = 0x4,
    E_MI_VIF_HDR_EXPOSURE_TYPE_MAX
} MI_VIF_HDRExposureType_e;

typedef enum
{
    E_MI_VIF_MCLK_12MHZ,
    E_MI_VIF_MCLK_18MHZ,
    E_MI_VIF_MCLK_27MHZ,
    E_MI_VIF_MCLK_36MHZ,
    E_MI_VIF_MCLK_54MHZ,
    E_MI_VIF_MCLK_108MHZ,
    E_MI_VIF_MCLK_MAX
} MI_VIF_MclkSource_e;

typedef enum
{
    E_MI_VIF_GROUPMASK_ID0    = 0x0001,
    E_MI_VIF_GROUPMASK_ID1    = 0x0002,
    E_MI_VIF_GROUPMASK_ID2    = 0x0004,
    E_MI_VIF_GROUPMASK_ID3    = 0x0008,
    E_MI_VIF_GROUPMASK_ID4    = 0x0010,
    E_MI_VIF_GROUPMASK_ID5    = 0x0020,
    E_MI_VIF_GROUPMASK_ID6    = 0x0040,
    E_MI_VIF_GROUPMASK_ID7    = 0x0080,
    E_MI_VIF_GROUPMASK_ID_MAX = 0xffff
} MI_VIF_GroupIdMask_e;

typedef enum
{
    E_MI_VIF_SNRPAD_ID_0 = 0,
    E_MI_VIF_SNRPAD_ID_1 = 1,
    E_MI_VIF_SNRPAD_ID_2 = 2,
    E_MI_VIF_SNRPAD_ID_3 = 3,
    E_MI_VIF_SNRPAD_ID_4 = 4,
    E_MI_VIF_SNRPAD_ID_5 = 5,
    E_MI_VIF_SNRPAD_ID_6 = 6,
    E_MI_VIF_SNRPAD_ID_7 = 7,
    E_MI_VIF_SNRPAD_ID_MAX,
    E_MI_VIF_SNRPAD_ID_NA = 0xFF,
} MI_VIF_SNRPad_e;

typedef enum
{
    E_MI_VIF_METADATA_NONE,
    E_MI_VIF_METADATA_PDAF     = 0x01,
    E_MI_VIF_METADATA_HEADER   = 0x02,
    E_MI_VIF_METADATA_AE_STAT  = 0x04,
    E_MI_VIF_METADATA_AWB_STAT = 0x08,
    E_MI_VIF_METADATA_MAX
} MI_VIF_MetaDataType_e;

typedef enum
{
    E_MI_VIF_CUSTCMD_PUTNUCDATA,
    E_MI_VIF_CUSTCMD_SETIRSTATUS,
    E_MI_VIF_CUSTCMD_FRAMEEND_NOTIFY,
    E_MI_VIF_CUSTCMD_SLEEPPARAM_SET,
    E_MI_VIF_CUSTCMD_SHUTTER_GAIN_SET,
    E_MI_VIF_CUSTCMD_FORCEFRAMEPARAM_SET,
    E_MI_VIF_CUSTCMD_FASTAE_STATUS,
    E_MI_VIF_CUSTCMD_MAX
} MI_VIF_CustCmd_e;

typedef struct MI_VIF_PutDataAttr_s
{
    MI_U32 u32Stride;
    MI_U32 u32Buffsize;
    MI_PHY phyAddr;
} MI_VIF_PutDataAttr_t;

typedef struct MI_VIF_MetaDataAttr_s
{
    MI_U32              u32MetaDataTypeMask;
    MI_SYS_WindowRect_t stCropRect; // For header usage only
} MI_VIF_MetaDataAttr_t;

typedef struct MI_VIF_GroupAttr_s
{
    MI_VIF_IntfMode_e      eIntfMode;
    MI_VIF_WorkMode_e      eWorkMode;
    MI_VIF_HDRType_e       eHDRType;
    MI_VIF_HDRFusionType_e eHDRFusionTpye;
    MI_U8                  u8HDRExposureMask;
    MI_VIF_ClkEdge_e       eClkEdge; // BT656
    MI_VIF_MclkSource_e    eMclk;
    MI_SYS_FrameScanMode_e eScanMode;
    MI_U32                 u32GroupStitchMask; // multi vif dev bitmask by MI_VIF_GroupIdMask_e

    MI_VIF_MetaDataAttr_t stMetaDataAttr;
} MI_VIF_GroupAttr_t;

typedef struct MI_VIF_DevAttr_s
{
    MI_SYS_PixelFormat_e   eInputPixel;
    MI_SYS_DataPrecision_e eDataPrecision;
    MI_SYS_WindowRect_t    stInputRect;
    MI_SYS_FieldType_e     eField;
    MI_BOOL                bEnH2T1PMode;
    MI_U16                 u16MaxFps;
} MI_VIF_DevAttr_t;

typedef struct MI_VIF_OutputPortAttr_s
{
    MI_SYS_WindowRect_t stCapRect;

    MI_SYS_WindowSize_t stDestSize;

    MI_SYS_PixelFormat_e ePixFormat;

    MI_VIF_FrameRate_e eFrameRate;

    MI_SYS_CompressMode_e eCompressMode;
} MI_VIF_OutputPortAttr_t;

typedef struct MI_VIF_DevPortStat_s
{
    MI_BOOL bEnable;
    MI_U32  u32IntCnt;
    MI_U32  u32FrameRate;
    MI_U32  u32LostInt;
    MI_U32  u32VbFail;
    MI_U32  u32PicWidth;
    MI_U32  u32PicHeight;
} MI_VIF_DevPortStat_t;

typedef struct MI_VIF_VIFDevStatus_s
{
    MI_VIF_GROUP    GroupId;
    MI_BOOL         bGroupCreated;
    MI_U32          bDevEn;
    MI_VIF_SNRPad_e eSensorPadID;
    MI_U32          u32PlaneID;
} MI_VIF_DevStatus_t;

typedef MI_S32 (*MI_VIF_CALLBK_FUNC)(MI_U64 u64Data);

typedef enum
{
    E_MI_VIF_CALLBACK_ISR,
    E_MI_VIF_CALLBACK_MAX,
} MI_VIF_CallBackMode_e;

typedef enum
{
    E_MI_VIF_IRQ_FRAMESTART, // frame start irq
    E_MI_VIF_IRQ_FRAMEEND,   // frame end irq
    E_MI_VIF_IRQ_LINEHIT,    // frame line hit irq
    E_MI_VIF_IRQ_MAX,
} MI_VIF_IrqType_e;

typedef struct MI_VIF_CallBackParam_s
{
    MI_VIF_CallBackMode_e eCallBackMode;
    MI_VIF_IrqType_e      eIrqType;
    MI_VIF_CALLBK_FUNC    pfnCallBackFunc;
    MI_U64                u64Data;
} MI_VIF_CallBackParam_t;

typedef struct MI_VIF_SleepModeParams_s
{
    MI_BOOL bSleepEnable;
    MI_U32  u32FrameCntBeforeSleep;
} MI_VIF_SleepModeParams_t;

typedef struct MI_VIF_ForceFrameParams_s
{
    MI_U32  u32ForceFrameCnt; //force getting frame when sleep enable already
    MI_U32  u32DropCnt;
} MI_VIF_ForceFrameParams_t;

typedef struct MI_VIF_FrameEndInfo_s
{
    MI_BOOL bDoneFlag;
    MI_U32  u32FrameCnt;
    MI_VIF_DEV vifDevId;
} MI_VIF_FrameEndInfo_t;

typedef struct MI_VIF_ShutterGainParams_s
{
    MI_U32 u32ShutterTimeUs;
    MI_U32 u32AeGain;
} MI_VIF_ShutterGainParams_t;

typedef struct MI_VIF_FastAEStatus_s
{
    MI_BOOL bFastAERunning;
} MI_VIF_FastAEStatus_t;

#pragma pack(pop)

#endif //_MI_VIF_DATATYPE_H_
