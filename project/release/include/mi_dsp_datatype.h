/* SigmaStar trade secret */
/* Copyright (c) [2019~2021] SigmaStar Technology.
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

#ifndef _MI_DSP_DATATYPE_H_
#define _MI_DSP_DATATYPE_H_
#include "mi_common_datatype.h"

#define MAX_PARA_CNT 2
#define MAX_BUF_CNT  10

typedef enum
{
    E_MI_DSP_DEV_0 = 0,
    E_MI_DSP_DEV_1,
    E_MI_DSP_DEV_2,
    E_MI_DSP_DEV_3,
    E_MI_DSP_DEV_NUM
} MI_DSP_DEV_e;

typedef enum
{
    E_MI_DSP_PRI_0 = 0,
    E_MI_DSP_PRI_1,
    E_MI_DSP_PRI_2,
    E_MI_DSP_PRI_NUM
} MI_DSP_PRI_e;

typedef enum
{
    E_MI_DSP_FW_PACKING_ROMING = 0,
    E_MI_DSP_FW_PACKING_PLAIN,
    E_MI_DSP_FW_PACKING_BUTT
} MI_DSP_FW_PACKING_TYPE_e;

typedef struct MI_DSP_DevAttr_s
{
    MI_DSP_FW_PACKING_TYPE_e eFwPackingType;
    MI_U32                   u32ResetVecAddr;
    MI_U16                   u16Prid;
    MI_BOOL                  bRebootFw;
} MI_DSP_DevAttr_t;

typedef int (*CUST_READ_FUNC)(void *dst_buf, int offset, int size, char *ctx);

typedef struct MI_DSP_BufInfo_s
{
    MI_PHY phyAddr;
    MI_U32 u32Size;
    MI_U32 u32Flag;
} MI_DSP_BufInfo_t;

typedef struct MI_DSP_BufVector_s
{
    MI_U32           u32BufCount;
    MI_DSP_BufInfo_t astBufInfoVec[MAX_BUF_CNT];
} MI_DSP_BufVector_t;

typedef struct MI_DSP_PARAM_s
{
    MI_U64 u64Data[MAX_PARA_CNT];
} MI_DSP_PARAM_t;

typedef enum
{
    E_MI_DSP_ERR_READ_FILE = 128,
    E_MI_DSP_ERR_INVALID_DEVID,
    E_MI_DSP_ERR_DEV_ATTR,
    E_MI_DSP_ERR_DEV_EXIST,
    E_MI_DSP_ERR_NULL_PTR,
    E_MI_DSP_ERR_PRI_FULL,
    E_MI_DSP_ERR_SYS_NOTREADY,
    E_MI_DSP_ERR_SYS_TIMEOUT,
    E_MI_DSP_ERR_ILLEGAL_PARAM,
    E_MI_DSP_ERR_ILLEGAL_ADDR,
    E_MI_DSP_ERR_FAILED,
} MI_DSP_ErrCode_e;

#define MI_DSP_ERR_INVALID_DEVID \
    (MI_DEF_ERR(E_MI_MODULE_ID_DSP, E_MI_ERR_LEVEL_ERROR, E_MI_DSP_ERR_INVALID_DEVID)) //设备号错误或超出硬件支持范围
#define MI_DSP_ERR_INVALID_PARAM \
    (MI_DEF_ERR(E_MI_MODULE_ID_DSP, E_MI_ERR_LEVEL_ERROR, E_MI_DSP_ERR_ILLEGAL_PARAM)) //参数超出合法范围
#define MI_DSP_ERR_RE_CREATE \
    (MI_DEF_ERR(E_MI_MODULE_ID_DSP, E_MI_ERR_LEVEL_ERROR, E_MI_DSP_ERR_DEV_EXIST)) //重复创建已存在的设备
#define MI_DSP_ERR_READ_FILE \
    (MI_DEF_ERR(E_MI_MODULE_ID_DSP, E_MI_ERR_LEVEL_ERROR, E_MI_DSP_ERR_READ_FILE)) //读取文件错误
#define MI_DSP_ERR_NULL_PTR \
    (MI_DEF_ERR(E_MI_MODULE_ID_DSP, E_MI_ERR_LEVEL_ERROR, E_MI_DSP_ERR_NULL_PTR)) //函数参数中有不允许为空指针的空指针
#define MI_DSP_ERR_DEV_ATTR \
    (MI_DEF_ERR(E_MI_MODULE_ID_DSP, E_MI_ERR_LEVEL_ERROR, E_MI_DSP_ERR_DEV_ATTR)) // Device设备属性参数错误
#define MI_DSP_ERR_PRIQ_FULL \
    (MI_DEF_ERR(E_MI_MODULE_ID_DSP, E_MI_ERR_LEVEL_ERROR, E_MI_DSP_ERR_PRI_FULL)) // Priority Queue满
#define MI_DSP_ERR_NOT_READY                              \
    (MI_DEF_ERR(E_MI_MODULE_ID_DSP, E_MI_ERR_LEVEL_ERROR, \
                E_MI_DSP_ERR_SYS_NOTREADY)) //系统没有初始化或Firmware没有ready
#define MI_DSP_ERR_BAD_ADDRESS \
    (MI_DEF_ERR(E_MI_MODULE_ID_DSP, E_MI_ERR_LEVEL_ERROR, E_MI_DSP_ERR_ILLEGAL_ADDR)) //地址非法
#define MI_DSP_ERR_TIMEOUT \
    (MI_DEF_ERR(E_MI_MODULE_ID_DSP, E_MI_ERR_LEVEL_ERROR, E_MI_DSP_ERR_SYS_TIMEOUT)) //系统异常导致超时
#define MI_DSP_ERR_FAILED (MI_DEF_ERR(E_MI_MODULE_ID_DSP, E_MI_ERR_LEVEL_ERROR, E_MI_DSP_ERR_FAILED))

#endif
