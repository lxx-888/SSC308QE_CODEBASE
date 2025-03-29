/*
 * hal_ive_common.h- Sigmastar
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

#ifndef _HAL_IVE_COMMON_H_
#define _HAL_IVE_COMMON_H_
//---------------------------------------------------------------------------
// INCLUDE
//---------------------------------------------------------------------------
#include "ms_platform.h"
#include "cam_os_wrapper.h"

//---------------------------------------------------------------------------
// VARIABLE
//---------------------------------------------------------------------------
// ive sys axi node.
extern u32 g_u32BurstOutstanding;

//---------------------------------------------------------------------------
// MACRO
//---------------------------------------------------------------------------
#define MIN_WIDTH  (16)
#define MIN_HEIGHT (4)

#define ADDR_LOW(addr)      (((u32)(addr)) & 0x0000FFFF)
#define ADDR_HIGH(addr)     ((((u32)(addr)) & 0xFFFF0000) >> 16)
#define ADDR_HIGH_EXT(addr) ((((u64)(addr) >> 32) & 0xF))

#define REGR(base, idx)      ms_readw(((uint)base + (idx)*4))
#define REGW(base, idx, val) ms_writew(val, ((uint)base + (idx)*4))

#define IVE_MSG_ERR    (3)
#define IVE_MSG_WRN    (4)
#define IVE_MSG_INF    (4)
#define IVE_MSG_DBG    (5)
#define IVE_MSG_LEVEL  (IVE_MSG_WRN)
#define IVE_MSG_ENABLE (1)

#if defined(IVE_MSG_ENABLE)
#if IVE_MSG_LEVEL >= IVE_MSG_ERR
#define LOG_IVE_MSG_ERR(dbglv, _fmt, _args...) CamOsPrintf("[%s, %s] " _fmt, #dbglv, __func__, ##_args)
#else
#define LOG_IVE_MSG_ERR(dbglv, _fmt, _args...)
#endif
#if IVE_MSG_LEVEL >= IVE_MSG_WRN
#define LOG_IVE_MSG_WRN(dbglv, _fmt, _args...) CamOsPrintf("[%s, %s] " _fmt, #dbglv, __func__, ##_args)
#else
#define LOG_IVE_MSG_WRN(dbglv, _fmt, _args...)
#endif
#if IVE_MSG_LEVEL >= IVE_MSG_INF
#define LOG_IVE_MSG_INF(dbglv, _fmt, _args...) CamOsPrintf("[%s, %s] " _fmt, #dbglv, __func__, ##_args)
#else
#define LOG_IVE_MSG_INF(dbglv, _fmt, _args...)
#endif
#if IVE_MSG_LEVEL >= IVE_MSG_DBG
#define LOG_IVE_MSG_DBG(dbglv, _fmt, _args...) CamOsPrintf("[%s, %s] " _fmt, #dbglv, __func__, ##_args)
#else
#define LOG_IVE_MSG_DBG(dbglv, _fmt, _args...)
#endif

#define IVE_MSG(dbglv, _fmt, _args...) LOG_##dbglv(dbglv, _fmt, ##_args)
#else
#define IVE_MSG(dbglv, _fmt, _args...)
#endif

#define IVE_GET_8BIT_BY_OFFSET(Var, Offset)      ((Var & (0xFF << Offset)) >> Offset)
#define IVE_SET_8BIT_BY_OFFSET(Var, Offset, Val) ((Var & ~(0xFF << Offset)) | ((Val & 0xFF) << Offset))

#define ALIGN_UP(AlignBit, Val) (((Val) + (1 << (AlignBit)) - 1) & (~((1 << (AlignBit)) - 1)))

#define RETURN_IF_CHECK_MAX_WH(OP, CheckWidth, CheckHeight, MaxWidth, MaxHeight)                                  \
    {                                                                                                             \
        if (CheckWidth > MaxWidth || CheckHeight > MaxHeight)                                                     \
        {                                                                                                         \
            IVE_MSG(IVE_MSG_ERR, "op:%d invalid w/h (%d/%d), max %d/%d\n", OP, CheckWidth, CheckHeight, MaxWidth, \
                    MaxHeight);                                                                                   \
            return IVE_IOC_ERROR_WRONG_SIZE;                                                                      \
        }                                                                                                         \
    }

#define RETURN_IF_CHECK_MIN_WH(OP, CheckWidth, CheckHeight, MinWidth, MinHeight)                                  \
    {                                                                                                             \
        if (CheckWidth < MinWidth || CheckHeight < MinHeight)                                                     \
        {                                                                                                         \
            IVE_MSG(IVE_MSG_ERR, "op:%d invalid w/h (%d/%d), min %d/%d\n", OP, CheckWidth, CheckHeight, MinWidth, \
                    MinHeight);                                                                                   \
            return IVE_IOC_ERROR_WRONG_SIZE;                                                                      \
        }                                                                                                         \
    }

#define RETURN_IF_CHECK_MAX_AND_MIN_WH(OP, CheckWidth, CheckHeight, MaxWidth, MaxHeight, MinWidth, MinHeight) \
    {                                                                                                         \
        if (MaxWidth && MaxHeight)                                                                            \
        {                                                                                                     \
            RETURN_IF_CHECK_MAX_WH(OP, CheckWidth, CheckHeight, MaxWidth, MaxHeight)                          \
        }                                                                                                     \
        if (MinWidth && MinHeight)                                                                            \
        {                                                                                                     \
            RETURN_IF_CHECK_MIN_WH(OP, CheckWidth, CheckHeight, MinWidth, MinHeight)                          \
        }                                                                                                     \
    }

#define RETURN_IF_CHECK_WH_MATCH(OP, CheckInputWidth, CheckInputHeight, CheckOutputWidth, CheckOutputHeight) \
    {                                                                                                        \
        if (CheckInputWidth != CheckOutputWidth || CheckInputHeight != CheckOutputHeight)                    \
        {                                                                                                    \
            IVE_MSG(IVE_MSG_ERR, "op:%d, w/h mismatch, in(%d/%d) out(%d/%d).\n", OP, CheckInputWidth,        \
                    CheckInputHeight, CheckOutputWidth, CheckOutputHeight);                                  \
            return IVE_IOC_ERROR_IN_OUT_SIZE_DIFFERENT;                                                      \
        }                                                                                                    \
    }

//---------------------------------------------------------------------------
// STRUCT
//---------------------------------------------------------------------------
typedef enum
{
    IVE_CONFIG_WH_CHECK_NONE   = 0,
    IVE_CONFIG_WH_CHECK_MATCH  = 1 << 0,
    IVE_CONFIG_WH_CHECK_INPUT  = 1 << 1,
    IVE_CONFIG_WH_CHECK_OUTPUT = 1 << 2,
} IVE_CONFIG_WH_CHECK_E;

typedef enum
{
    IVE_HAL_SUCCESS = 0,
    IVE_HAL_FAIL    = 1,
} IVE_HAL_RET_E;
#endif // _HAL_IVE_COMMON_H_
