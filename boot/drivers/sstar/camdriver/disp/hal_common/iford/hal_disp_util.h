/*
 * hal_disp_util.h- Sigmastar
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
#ifndef __HAL_DISP_UTIL_H__
#define __HAL_DISP_UTIL_H__
#include "drv_disp_os_header.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define RIU_OFFSET           0x1F000000
#define RIU_READ_BYTE(addr)  (READ_BYTE(RIU_OFFSET + (addr)))
#define RIU_READ_2BYTE(addr) (READ_WORD(RIU_OFFSET + (addr)))
#define RIU_READ_4BYTE(addr) (READ_LONG(RIU_OFFSET + (addr)))

#define RIU_WRITE_BYTE(addr, val)  WRITE_BYTE((RIU_OFFSET + (addr)), val)
#define RIU_WRITE_2BYTE(addr, val) WRITE_WORD(RIU_OFFSET + (addr), val)
#define RIU_WRITE_4BYTE(addr, val) WRITE_LONG(RIU_OFFSET + (addr), val)

#define RBYTE(u32Reg) RIU_READ_BYTE((u32Reg) << 1)

#define R2BYTE(u32Reg) RIU_READ_2BYTE((u32Reg) << 1)

#define R2BYTEMSK(u32Reg, u16mask) ((RIU_READ_2BYTE((u32Reg) << 1) & u16mask))

#define R4BYTE(u32Reg) ({ (RIU_READ_4BYTE((u32Reg) << 1)); })

#define R4BYTEMSK(u32Reg, u32Mask) (RIU_READ_4BYTE((u32Reg) << 1) & u32Mask)

#define WBYTE(u32Reg, u8Val) RIU_WRITE_BYTE(((u32Reg) << 1), u8Val)

#define WBYTEMSK(u32Reg, u8Val, u8Mask)              \
    RIU_WRITE_BYTE((((u32Reg) << 1) - ((u32Reg)&1)), \
                   (RIU_READ_BYTE((((u32Reg) << 1) - ((u32Reg)&1))) & ~(u8Mask)) | ((u8Val) & (u8Mask)))

#define W2BYTE(u32Reg, u16Val) RIU_WRITE_2BYTE((u32Reg) << 1, u16Val)

#define W2BYTEMSK(u32Reg, u16Val, u16Mask) \
    RIU_WRITE_2BYTE((u32Reg) << 1, (RIU_READ_2BYTE((u32Reg) << 1) & ~(u16Mask)) | ((u16Val) & (u16Mask)))

#if 0
#define W4BYTE(u32Reg, u32Val)                                                     \
    (                                                                              \
        {                                                                          \
            RIU_WRITE_2BYTE((u32Reg) << 1, ((u32Val)&0x0000FFFF));                 \
            RIU_WRITE_2BYTE(((u32Reg) + 2) << 1, (((u32Val) >> 16) & 0x0000FFFF)); \
        })
#else
#define W4BYTE(u32Reg, u32Val) ({ RIU_WRITE_4BYTE((u32Reg) << 1, u32Val); })
#endif

#define W4BYTEMSK(u32Reg, u32Val, u32Mask) \
    RIU_WRITE_4BYTE((u32Reg) << 1, (RIU_READ_4BYTE((u32Reg) << 1) & ~(u32Mask)) | ((u32Val) & (u32Mask)))

#define W3BYTE(u32Reg, u32Val)                                   \
    (                                                            \
        {                                                        \
            RIU_WRITE_2BYTE((u32Reg) << 1, u32Val);              \
            RIU_WRITE_BYTE((u32Reg + 2) << 1, ((u32Val) >> 16)); \
        })

#define DISP_BIT0  0x00000001
#define DISP_BIT1  0x00000002
#define DISP_BIT2  0x00000004
#define DISP_BIT3  0x00000008
#define DISP_BIT4  0x00000010
#define DISP_BIT5  0x00000020
#define DISP_BIT6  0x00000040
#define DISP_BIT7  0x00000080
#define DISP_BIT8  0x00000100
#define DISP_BIT9  0x00000200
#define DISP_BIT10 0x00000400
#define DISP_BIT11 0x00000800
#define DISP_BIT12 0x00001000
#define DISP_BIT13 0x00002000
#define DISP_BIT14 0x00004000
#define DISP_BIT15 0x00008000
#define DISP_BIT16 0x00010000
#define DISP_BIT17 0x00020000
#define DISP_BIT18 0x00040000
#define DISP_BIT19 0x00080000
#define DISP_BIT20 0x00100000
#define DISP_BIT21 0x00200000
#define DISP_BIT22 0x00400000
#define DISP_BIT23 0x00800000
#define DISP_BIT24 0x01000000
#define DISP_BIT25 0x02000000
#define DISP_BIT26 0x04000000
#define DISP_BIT27 0x08000000
#define DISP_BIT28 0x10000000
#define DISP_BIT29 0x20000000
#define DISP_BIT30 0x40000000
#define DISP_BIT31 0x80000000

#define HAL_DISP_STR_TEST_BANK                                                                       \
    {                                                                                                \
        REG_DISP_LPLL_BASE, REG_CLKGEN_BASE, REG_PADTOP_BASE, REG_CKG_BLOCK_BASE, REG_SC_TOP_0_BASE, \
            REG_SC_TOP_1_BASE, REG_SC_RDMA1_BASE, REG_DISP_TTL_BASE, REG_SC_SGRDMA1_BASE,            \
    }

#define HAL_DISP_STR_WHITE_LIST_ADDR                                                                   \
    {                                                                                                  \
        REG_DISP_TTL_66_L, REG_DISP_TTL_67_L, REG_DISP_TTL_6B_L, REG_DISP_TTL_6C_L, REG_SC_RDMA1_40_L, \
            REG_SC_RDMA1_41_L, REG_SC_RDMA1_42_L,                                                      \
    }
#define HAL_DISP_UTILITY_CMDQ_ID_DEVICE0 (0)
#define HAL_DISP_UTILITY_CMDQ_NUM        (1)

#define HAL_DISP_UTILIYT_REG_NUM (0x80)
#define HAL_DISP_UTILITY_REG_BANK_SIZE \
    (HAL_DISP_UTILIYT_REG_NUM * 2) // Each bank has 128 register and each 16bits for value
#define HAL_DISP_UTILITY_DIRECT_CMDQ_CNT   (384)
#define HAL_DISP_UTILITY_CMDQ_CMD_CNT      (32)
#define HAL_DISP_UTILITY_CMDQ_DELAY_CMD    (0xFFFF02)
#define HAL_DISP_UTILITY_CMDQ_WAITDONE_CMD (0xFFFF04)

#define HAL_DISP_UTILITY_CMDQDEV_WAIT_DONE_EVENT(id) E_MHAL_CMDQEVE_DISP_0_INT

#define HAL_DISP_UTILITY_SC_TO_CMDQDEV_FLAG REG_DISP_TTL_66_L

#define REG_HAL_DISP_UTILIYT_CMDQDEV_IN_PROCESS(id)    REG_DISP_TTL_28_L
#define REG_HAL_DISP_UTILIYT_CMDQDEV_TIMEGEN_START(id) REG_DISP_TTL_28_L

#define REG_CMDQ_PROCESS_FENCE_MSK     (0x0FFE)
#define REG_CMDQ_IN_PROCESS_MSK        (0x8000)
#define REG_CMDQ_IN_PROCESS_ON         (0x8000)
#define REG_CMDQ_IN_PROCESS_OFF        (0x0000)
#define REG_CMDQ_DEV_TIMEGEN_START_MSK (0x4000)

#define REG_HAL_DISP_UTILITY_CMDQ_WAIT_CNT(id) REG_DISP_TTL_58_L

#define REG_CMDQ_WAIT_CNT_MSK   (0xFFFF)
#define REG_CMDQ_WAIT_CNT_SHIFT (0)

#define HAL_DISP_UTILITY_CMDQ_CNT_MAX  (0xFFFF)
#define HAL_DISP_UTILITY_CNT_ADD(x, y) (x = (x + y) >= HAL_DISP_UTILITY_CMDQ_CNT_MAX ? 1 : (x + y))

//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    MI_U32 u32Addr; // 16bit Bank addr + 8bit/16bit-regaddr
    MI_U16 u16Data; // 16bit data
    MI_U16 u16Mask; // inverse normal case
} HAL_DISP_UTILITY_CmdReg_t;

typedef struct
{
    MI_U32 u32CmdqBufIdx;
    MI_U32 u32Addr;
    MI_U16 u16Data;
    MI_U16 u16Mask;
    MI_U32 u32Time;
    MI_U8  bPollEq;
} HAL_DISP_UTILITY_CmdqCmd_t;

typedef struct
{
    MI_S32 s32UtilityId;

    void * pvDirectCmdqBuffer;
    MI_U32 u32DirectCmdqCnt;

    void * pvCmdqCmdBuffer;
    MI_U32 u32CmdqCmdCnt;

    void * pvCmdqInf;
    MI_U16 u16RegFlipCnt;
    MI_U16 u16WaitDoneCnt;

    DRV_DISP_OS_MutexConfig_t stMutxCfg;
} HAL_DISP_UTILITY_CmdqContext_t;

//-------------------------------------------------------------------------------------------------
// Prototype
//-------------------------------------------------------------------------------------------------

#ifndef __HAL_DISP_UTILITY_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE MI_U8 HAL_DISP_UTILITY_Init(MI_U32 u32CtxId);
INTERFACE MI_U8 HAL_DISP_UTILITY_DeInit(MI_U32 u32CtxId);
INTERFACE MI_U8 HAL_DISP_UTILITY_GetCmdqContext(void **pvCtx, MI_U32 u32CtxId);
INTERFACE void  HAL_DISP_UTILITY_SetCmdqInf(void *pCmdqInf, MI_U32 u32CtxId);

INTERFACE MI_U16 HAL_DISP_UTILITY_R2BYTEDirect(MI_U32 u32Reg);
INTERFACE MI_U16 HAL_DISP_UTILITY_R2BYTEMaskDirect(MI_U32 u32Reg, MI_U16 u16Mask);
INTERFACE void   HAL_DISP_UTILITY_W2BYTEDirect(MI_U32 u32Reg, MI_U16 u16Val);
INTERFACE void   HAL_DISP_UTILITY_W2BYTE(MI_U32 u32Reg, MI_U16 u16Val, void *pvCtxIn);
INTERFACE void   HAL_DISP_UTILITY_W2BYTEMSKDirect(MI_U32 u32Reg, MI_U16 u16Val, MI_U16 u16Mask);
INTERFACE void   HAL_DISP_UTILITY_W2BYTEMSK(MI_U32 u32Reg, MI_U16 u16Val, MI_U16 u16Mask, void *pvCtxIn);
INTERFACE void   HAL_DISP_UTILITY_W2BYTEMSKDirectCmdq(void *pvCtxIn, MI_U32 u32Reg, MI_U16 u16Val, MI_U16 u16Mask);
INTERFACE void   HAL_DISP_UTILITY_W2BYTEMSKDirectCmdqWrite(void *pvCtxIn);
INTERFACE void   HAL_DISP_UTILITY_KickoffCmdq(void *pvCtxIn);
INTERFACE void   HAL_DISP_UTILITY_AddW2BYTEMSK(MI_U32 u32Reg, MI_U16 u16Val, MI_U16 u16Mask, void *pvCtxIn);
INTERFACE MI_U8  HAL_DISP_UTILITY_AddWaitCmd(void *pvCtxIn);
INTERFACE MI_U8  HAL_DISP_UTILITY_AddDelayCmd(void *pvCtxIn, MI_U32 u32PollingTime);
INTERFACE void   HAL_DISP_UTILITY_SetRegAccessMode(MI_U32 u32Id, MI_U32 u32Mode);
INTERFACE MI_U32 HAL_DISP_UTILITY_GetRegAccessMode(MI_U32 u32Id);
INTERFACE MI_U8  HAL_DISP_UTILITY_PollWait(void *pvCmdqInf, MI_U32 u32Reg, MI_U16 u16Val, MI_U16 u16Mask,
                                           MI_U32 u32PollTime, MI_U8 bPollEq);
INTERFACE void   HAL_DISP_UTILITY_WriteDelayCmd(MI_U32 u32PollingTime, void *pvCtxIn);
INTERFACE void   HalDispUtilityReadBankCpyToBuffer(u32 u32Bank, void *pBuf);

#undef INTERFACE
#endif /* __HAL_DISP_UTIL_H__ */
