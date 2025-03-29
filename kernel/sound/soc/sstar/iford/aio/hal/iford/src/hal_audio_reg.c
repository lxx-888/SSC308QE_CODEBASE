/*
 * hal_audio_reg.c - Sigmastar
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

//=============================================================================
// Include files
//=============================================================================
#include "ms_platform.h"
#include "hal_audio_common.h"
#include "hal_audio_sys.h"
#include "hal_audio_reg.h"
#include "hal_audio_config.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"
#include "hal_audio_os_api.h"
//=============================================================================
// Macro
//=============================================================================
#define AUDIO_REG_HW_MASK (1)

#if AUDIO_REG_HW_MASK
#ifndef OUTREGMSK16_BIT_OP
#define OUTREGMSK16_BIT_OP(x, y, z) OUTREG32(x, ((INREG16(x) & ~(z)) | ((y) & (z))) | (u32)(z) << 16)
#endif
#endif

//=============================================================================
// Variable definition
//=============================================================================
static U64 g_nBaseRegAddr = BACH_RIU_BASE_ADDR;

extern void CamOsPrintf(const char *szFmt, ...);

void HalBachSysSetBankBaseAddr(U64 nAddr)
{
#if AUDIO_REG_HW_MASK
    g_nBaseRegAddr = BACH_RIU_BASE_ADDR;
#else
    g_nBaseRegAddr = nAddr;
#endif
}

void HalBachWriteRegByte(U32 nAddr, U8 regMsk, U8 nValue)
{
#if AUDIO_REG_HW_MASK
    U32 a = 0, v = 0, m = 0;

    a = (g_nBaseRegAddr + ((nAddr >> 1) << 2)); // 8bits addr transfer to 0, 4, 8 ...
    v = nValue;
    m = regMsk;

    if (nAddr & 1)
    {
        v = v << 8;
        m = m << 8;
    }

    OUTREGMSK16_BIT_OP(a, v, m);
#else

    U8 nConfigValue;
    nConfigValue = READ_BYTE(g_nBaseRegAddr + ((nAddr) << 1) - ((nAddr)&1));
    nConfigValue &= ~regMsk;
    nConfigValue |= (nValue & regMsk);
    WRITE_BYTE(g_nBaseRegAddr + ((nAddr) << 1) - ((nAddr)&1), nConfigValue);
#endif
}

void HalBachWriteReg2Byte(U32 nAddr, U16 regMsk, U16 nValue)
{
#if AUDIO_REG_HW_MASK
    U32 a = 0, v = 0, m = 0;

    a = (g_nBaseRegAddr + ((nAddr >> 1) << 2)); // 8bits addr transfer to 0, 4, 8 ...
    v = nValue;
    m = regMsk;

    OUTREGMSK16_BIT_OP(a, v, m);
#else
    U16 nConfigValue;
    nConfigValue = READ_WORD(g_nBaseRegAddr + ((nAddr) << 1));
    nConfigValue &= ~regMsk;
    nConfigValue |= (nValue & regMsk);
    WRITE_WORD(g_nBaseRegAddr + ((nAddr) << 1), nConfigValue);
#endif
}

void HalBachWriteReg(BachRegBank_e nBank, U8 nAddr, U16 regMsk, U16 nValue)
{
#if AUDIO_REG_HW_MASK
    U32 a = 0, v = 0, m = 0;

    a = (g_nBaseRegAddr + ((nAddr >> 1) << 2)); // 8bits addr transfer to 0, 4, 8 ...
    v = nValue;
    m = regMsk;

    switch (nBank)
    {
        case E_BACH_REG_BANK1:
            a = (g_nBaseRegAddr + (((BACH_REG_BANK_1 + nAddr) >> 1) << 2)); // 8bits addr transfer to 0, 4, 8 ...
            v = nValue;
            m = regMsk;
            OUTREGMSK16_BIT_OP(a, v, m);
            break;
        case E_BACH_REG_BANK2:
            a = (g_nBaseRegAddr + (((BACH_REG_BANK_2 + nAddr) >> 1) << 2)); // 8bits addr transfer to 0, 4, 8 ...
            v = nValue;
            m = regMsk;
            OUTREGMSK16_BIT_OP(a, v, m);
            break;
        case E_BACH_REG_BANK3:
            a = (g_nBaseRegAddr + (((BACH_REG_BANK_3 + nAddr) >> 1) << 2)); // 8bits addr transfer to 0, 4, 8 ...
            v = nValue;
            m = regMsk;
            OUTREGMSK16_BIT_OP(a, v, m);
            break;
        case E_BACH_REG_BANK4:
            a = (g_nBaseRegAddr + (((BACH_REG_BANK_4 + nAddr) >> 1) << 2)); // 8bits addr transfer to 0, 4, 8 ...
            v = nValue;
            m = regMsk;
            OUTREGMSK16_BIT_OP(a, v, m);
            break;
        case E_BACH_REG_BANK5:
            a = (g_nBaseRegAddr + (((BACH_REG_BANK_5 + nAddr) >> 1) << 2)); // 8bits addr transfer to 0, 4, 8 ...
            v = nValue;
            m = regMsk;
            OUTREGMSK16_BIT_OP(a, v, m);
            break;
        case E_BACH_REG_BANK6:
            a = (g_nBaseRegAddr + (((BACH_REG_BANK_6 + nAddr) >> 1) << 2)); // 8bits addr transfer to 0, 4, 8 ...
            v = nValue;
            m = regMsk;
            OUTREGMSK16_BIT_OP(a, v, m);
            break;
        case E_BACH_REG_BANK7:
            a = (g_nBaseRegAddr + (((BACH_REG_BANK_7 + nAddr) >> 1) << 2)); // 8bits addr transfer to 0, 4, 8 ...
            v = nValue;
            m = regMsk;
            OUTREGMSK16_BIT_OP(a, v, m);
            break;
        case E_BACH_PLL_BANK:
            a = (g_nBaseRegAddr + (((BACH_PLL_BANK + nAddr) >> 1) << 2)); // 8bits addr transfer to 0, 4, 8 ...
            v = nValue;
            m = regMsk;
            OUTREGMSK16_BIT_OP(a, v, m);
            break;
        default:
            break;
    }
#else
    U16 nConfigValue;

    switch (nBank)
    {
        case E_BACH_REG_BANK1:
            nConfigValue = READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_1 + (nAddr)) << 1));
            nConfigValue &= ~regMsk;
            nConfigValue |= (nValue & regMsk);
            WRITE_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_1 + (nAddr)) << 1), nConfigValue);
            break;
        case E_BACH_REG_BANK2:
            nConfigValue = READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_2 + (nAddr)) << 1));
            nConfigValue &= ~regMsk;
            nConfigValue |= (nValue & regMsk);
            WRITE_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_2 + (nAddr)) << 1), nConfigValue);
            break;
        case E_BACH_REG_BANK3:
            nConfigValue = READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_3 + (nAddr)) << 1));
            nConfigValue &= ~regMsk;
            nConfigValue |= (nValue & regMsk);
            WRITE_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_3 + (nAddr)) << 1), nConfigValue);
            break;
        case E_BACH_REG_BANK4:
            nConfigValue = READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_4 + (nAddr)) << 1));
            nConfigValue &= ~regMsk;
            nConfigValue |= (nValue & regMsk);
            WRITE_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_4 + (nAddr)) << 1), nConfigValue);
            break;
        case E_BACH_REG_BANK5:
            nConfigValue = READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_5 + (nAddr)) << 1));
            nConfigValue &= ~regMsk;
            nConfigValue |= (nValue & regMsk);
            WRITE_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_5 + (nAddr)) << 1), nConfigValue);
            break;
        case E_BACH_REG_BANK6:
            nConfigValue = READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_6 + (nAddr)) << 1));
            nConfigValue &= ~regMsk;
            nConfigValue |= (nValue & regMsk);
            WRITE_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_6 + (nAddr)) << 1), nConfigValue);
            break;
        case E_BACH_REG_BANK7:
            nConfigValue = READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_7 + (nAddr)) << 1));
            nConfigValue &= ~regMsk;
            nConfigValue |= (nValue & regMsk);
            WRITE_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_7 + (nAddr)) << 1), nConfigValue);
            break;
        case E_BACH_PLL_BANK:
            nConfigValue = READ_WORD(g_nBaseRegAddr + ((BACH_PLL_BANK + (nAddr)) << 1));
            nConfigValue &= ~regMsk;
            nConfigValue |= (nValue & regMsk);
            WRITE_WORD(g_nBaseRegAddr + ((BACH_PLL_BANK + (nAddr)) << 1), nConfigValue);
            break;
        default:
            break;
    }
#endif
    {
        int bank = 0x00;
        switch (nBank)
        {
            case E_BACH_REG_BANK1:
                bank = BACH_REG_BANK_1;
                break;
            case E_BACH_REG_BANK2:
                bank = BACH_REG_BANK_2;
                break;
            case E_BACH_REG_BANK3:
                bank = BACH_REG_BANK_3;
                break;
            case E_BACH_REG_BANK4:
                bank = BACH_REG_BANK_4;
                break;
            case E_BACH_REG_BANK5:
                bank = BACH_REG_BANK_5;
                break;
            case E_BACH_REG_BANK6:
                bank = BACH_REG_BANK_6;
                break;
            case E_BACH_REG_BANK7:
                bank = BACH_REG_BANK_7;
                break;
            default:
                break;
        }
        if (nAddr == AUDIO_BANK1_REG_DMA1_CTRL_9_ADDR && regMsk == AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_TRIG_MASK)
        {
            DBGMSG(AUDIO_DBG_LEVEL_TEST, "trigger val [%d]\n", nValue);
        }
        DBGMSG(AUDIO_DBG_LEVEL_TEST, "wriu -w 0x%x 0x%04x", bank | nAddr, HalBachReadReg(nBank, nAddr));
    }
}

U16 HalBachReadReg2Byte(U32 nAddr)
{
#if AUDIO_REG_HW_MASK
    return INREG16(g_nBaseRegAddr + ((nAddr) << 1));
#else
    return READ_WORD(g_nBaseRegAddr + ((nAddr) << 1));
#endif
}

U8 HalBachReadRegByte(U32 nAddr)
{
#if AUDIO_REG_HW_MASK
    return INREG8(g_nBaseRegAddr + ((nAddr) << 1) - ((nAddr)&1));
#else
    return READ_BYTE(g_nBaseRegAddr + ((nAddr) << 1) - ((nAddr)&1));
#endif
}

U16 HalBachReadReg(BachRegBank_e nBank, U8 nAddr)
{
#if AUDIO_REG_HW_MASK
    switch (nBank)
    {
        case E_BACH_REG_BANK1:
            return INREG16(g_nBaseRegAddr + ((BACH_REG_BANK_1 + (nAddr)) << 1));
        case E_BACH_REG_BANK2:
            return INREG16(g_nBaseRegAddr + ((BACH_REG_BANK_2 + (nAddr)) << 1));
        case E_BACH_REG_BANK3:
            return INREG16(g_nBaseRegAddr + ((BACH_REG_BANK_3 + (nAddr)) << 1));
        case E_BACH_REG_BANK4:
            return INREG16(g_nBaseRegAddr + ((BACH_REG_BANK_4 + (nAddr)) << 1));
        case E_BACH_REG_BANK5:
            return INREG16(g_nBaseRegAddr + ((BACH_REG_BANK_5 + (nAddr)) << 1));
        case E_BACH_REG_BANK6:
            return INREG16(g_nBaseRegAddr + ((BACH_REG_BANK_6 + (nAddr)) << 1));
        case E_BACH_REG_BANK7:
            return INREG16(g_nBaseRegAddr + ((BACH_REG_BANK_7 + (nAddr)) << 1));
        case E_BACH_PLL_BANK:
            return INREG16(g_nBaseRegAddr + ((BACH_PLL_BANK + (nAddr)) << 1));
        default:
            return 0;
    }
#else
    switch (nBank)
    {
        case E_BACH_REG_BANK1:
            return READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_1 + (nAddr)) << 1));
        case E_BACH_REG_BANK2:
            return READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_2 + (nAddr)) << 1));
        case E_BACH_REG_BANK3:
            return READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_3 + (nAddr)) << 1));
        case E_BACH_REG_BANK4:
            return READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_4 + (nAddr)) << 1));
        case E_BACH_REG_BANK5:
            return READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_5 + (nAddr)) << 1));
        case E_BACH_REG_BANK6:
            return READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_6 + (nAddr)) << 1));
        case E_BACH_REG_BANK7:
            return READ_WORD(g_nBaseRegAddr + ((BACH_REG_BANK_7 + (nAddr)) << 1));
        case E_BACH_PLL_BANK:
            return READ_WORD(g_nBaseRegAddr + ((BACH_PLL_BANK + (nAddr)) << 1));
        default:
            return 0;
    }
#endif
}
