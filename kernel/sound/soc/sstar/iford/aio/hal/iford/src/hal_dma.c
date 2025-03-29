/*
 * hal_dma.c - Sigmastar
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

#include "hal_audio_common.h"
#include "hal_audio_sys.h"
#include "hal_audio_reg.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"
#include "hal_audio_os_api.h"

#include <drv_padmux.h>
#include <drv_puse.h>
#include <drv_gpio.h>
#include "cam_os_wrapper.h"

// -------------------------------------------------------------------------------
PcmRuntimeData_t g_aRuntimeData[E_CHIP_AIO_DMA_TOTAL];
DmaPar_t         g_AudDmaParam[E_CHIP_AIO_DMA_TOTAL];

// -------------------------------------------------------------------------------
static int _HalAudIntEnable(void);
static int _HalAudDmaFirstInit(void);
static int _HalAudDmaWrMchInit(void);
static int _HalAudDmaReInit(CHIP_AIO_DMA_e eAioDma);
static int _HalAudDmaRdIntEnable(CHIP_AIO_DMA_e eAioDma, BOOL bUnderrun, BOOL bEmpty, BOOL bDataTranslate);
static int _HalAudDmaWrIntEnable(CHIP_AIO_DMA_e eAioDma, BOOL bOverrun, BOOL bFull, BOOL bDataTranslate);
static int _HalAudDmaGetRdInt(CHIP_AIO_DMA_e eAioDma, BOOL *bUnderrun, BOOL *bEmpty, BOOL *bLocalEmpty);
static int _HalAudDmaGetWrInt(CHIP_AIO_DMA_e eAioDma, BOOL *bOverrun, BOOL *bFull, BOOL *bLocalFull);
static int _HalAudDmaRdGetFlags(CHIP_AIO_DMA_e eAioDma, BOOL *pbUnderrun, BOOL *pbEmtpy, BOOL *pbTransmit,
                                BOOL *pbLocalEmpty);
static int _HalAudDmaWrGetFlags(CHIP_AIO_DMA_e eAioDma, BOOL *pbOverrun, BOOL *pbFull, BOOL *pbLocalFull,
                                BOOL *pbTransmit);
/**
 * bytes_to_frames - Unit conversion of the size from bytes to frames
 * @runtime: PCM runtime instance
 * @size: size in bytes
 */
static inline PcmFrames_t _BytesToHwFrames(PcmRuntimeData_t *pRtd, u32 size)
{
    return size / (pRtd->nHwFrameBits >> 3);
}

/**
 * frames_to_bytes - Unit conversion of the size from frames to bytes
 * @runtime: PCM runtime instance
 * @size: size in frames
 */
static inline u32 _HwFramesToBytes(PcmRuntimeData_t *pRtd, PcmFrames_t size)
{
    return size * (pRtd->nHwFrameBits >> 3);
}

static int _HalAudDmaFirstInit(void)
{
    // reset DMA1 interal register
    // reset DMA 1 read size
    HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_4_ADDR,
                    AUDIO_BANK1_REG_DMA1_CTRL_4_REG_DMA1_RD_SIZE_MASK, 0);

    // reset DMA 1 write size
    HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_12_ADDR,
                    AUDIO_BANK1_REG_DMA1_CTRL_12_REG_DMA1_WR_SIZE_MASK, 0);

    HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR,
                    AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_SW_RST_DMA_MASK,
                    AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_SW_RST_DMA_MASK); // DMA 1 software reset
    HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR,
                    AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_SW_RST_DMA_MASK, 0);

    HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR,
                    (AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_PRIORITY_KEEP_HIGH_MASK
                     | AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_LEVEL_CNT_LIVE_MASK_MASK),
                    (AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_PRIORITY_KEEP_HIGH_MASK
                     | AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_LEVEL_CNT_LIVE_MASK_MASK));

    return AIO_OK;
}

static int _HalAudDmaWrMchInit(void)
{
    // E_CHIP_AI_MCH_A
    HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_ADDR,
                    AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_DMA_MCH_32B_DBG_1_MASK, 0);
    HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_ADDR,
                    AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_DMA_W1_BIT_MODE_MASK, 0);
    HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_ADDR,
                    AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_DMA_MCH_32B_ENABLE_1_MASK,
                    AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_DMA_MCH_32B_ENABLE_1_MASK);

    return AIO_OK;
}

static int _HalAudIntEnable(void)
{
    // dma1 & dma2 interrupt mask
    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_INT_EN_ADDR, AUDIO_BANK4_REG_INT_EN_REG_INT_EN0_MASK,
                    AUDIO_BANK4_REG_INT_EN_REG_INT_EN0_MASK);

    // period size interrupt mask
    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_INT_EN_ADDR, AUDIO_BANK4_REG_INT_EN_REG_INT_EN5_MASK,
                    AUDIO_BANK4_REG_INT_EN_REG_INT_EN5_MASK);

    return AIO_OK;
}

static int _HalAudDmaReInit(CHIP_AIO_DMA_e eAioDma)
{
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_1_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_TRIG_MASK,
                            0); // prevent from triggering levelcount at toggling init step
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_1_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_INIT_MASK,
                            AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_INIT_MASK);
            _UDelay(10);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_1_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_INIT_MASK, 0);
            break;

        case E_CHIP_AIO_DMA_AI_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_9_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_TRIG_MASK, 0);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_9_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_INIT_MASK,
                            AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_INIT_MASK);
            _UDelay(10);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_9_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_INIT_MASK, 0);
            break;

        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }

    DBGMSG(AUDIO_DBG_LEVEL_DMA, "eAioDma[%d]", eAioDma);

    return AIO_OK;
FAIL:
    return AIO_NG;
}

static U32 _HalAudDmaGetRawMiuUnitLevelCnt(CHIP_AIO_DMA_e eAioDma)
{
    U16 nConfigValue = 0;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_9_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_LEVEL_CNT_MASK_MASK,
                            AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_LEVEL_CNT_MASK_MASK);
            nConfigValue = HalBachReadReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_15_ADDR);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_9_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_LEVEL_CNT_MASK_MASK, 0);
            break;

        case E_CHIP_AIO_DMA_AO_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_1_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_LEVEL_CNT_MASK_MASK,
                            AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_LEVEL_CNT_MASK_MASK);
            nConfigValue = HalBachReadReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_7_ADDR);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_1_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_LEVEL_CNT_MASK_MASK, 0);
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            return 0;
    }

    return (U32)nConfigValue;
}

static int _HalAudDmaRdIntEnable(CHIP_AIO_DMA_e eAioDma, BOOL bUnderrun, BOOL bEmpty, BOOL bDataTranslate)
{
    U8            nAddr;
    BachRegBank_e eBank;
    U16           nConfigValue;
    U16           nMask;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR;
            break;
        case E_CHIP_AIO_DMA_AO_B:
        case E_CHIP_AIO_DMA_AO_C:
        case E_CHIP_AIO_DMA_AO_D:
        case E_CHIP_AIO_DMA_AO_E:
        case E_CHIP_AIO_DMA_AO_DIRECT_A:
        case E_CHIP_AIO_DMA_AO_DIRECT_B:
            return AIO_OK;
        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue = HalBachReadReg(eBank, nAddr);

    /* Underrun interrupt */
    if (bUnderrun)
    {
        nConfigValue |= AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_UNDERRUN_INT_EN_MASK;
    }
    else
    {
        nConfigValue &= ~AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_UNDERRUN_INT_EN_MASK;
    }

    /* Empty interrupt */
    if (bEmpty)
    {
        nConfigValue |= AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_EMPTY_INT_EN_MASK;
    }
    else
    {
        nConfigValue &= ~AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_EMPTY_INT_EN_MASK;
    }

    HalBachWriteReg(eBank, nAddr,
                    (AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_UNDERRUN_INT_EN_MASK
                     | AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_EMPTY_INT_EN_MASK),
                    nConfigValue);

    // translate interrupt
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA_WR_AFTER_RD_ADDR;
            nMask = AUDIO_BANK1_REG_DMA_WR_AFTER_RD_REG_DMA_1_RD_TH_EN_MASK;
            break;
        case E_CHIP_AIO_DMA_AO_B:
        case E_CHIP_AIO_DMA_AO_C:
        case E_CHIP_AIO_DMA_AO_D:
        case E_CHIP_AIO_DMA_AO_E:
        case E_CHIP_AIO_DMA_AO_DIRECT_A:
        case E_CHIP_AIO_DMA_AO_DIRECT_B:
            return AIO_OK;
        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }
    if (bDataTranslate)
    {
        HalBachWriteReg(eBank, nAddr, nMask, nMask);
    }
    else
    {
        HalBachWriteReg(eBank, nAddr, nMask, 0);
    }

    // DBGMSG(AUDIO_DBG_LEVEL_DMA, "eAioDma[%d], bUnderrun[%d], bEmpty[%d], bDataTranslate[%d]", eAioDma, bUnderrun,
    //        bEmpty, bDataTranslate);
    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudDmaWrIntEnable(CHIP_AIO_DMA_e eAioDma, BOOL bOverrun, BOOL bFull, BOOL bDataTranslate)
{
    U8            nAddr;
    BachRegBank_e eBank;
    U16           nConfigValue;
    U16           nMask;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR;
            break;
        case E_CHIP_AIO_DMA_AI_B:
        case E_CHIP_AIO_DMA_AI_C:
        case E_CHIP_AIO_DMA_AI_D:
        case E_CHIP_AIO_DMA_AI_E:
        case E_CHIP_AIO_DMA_AI_DIRECT_A:
        case E_CHIP_AIO_DMA_AI_DIRECT_B:
            return AIO_OK;

        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue = HalBachReadReg(eBank, nAddr);

    /* Overrun interrupt */
    if (bOverrun)
    {
        nConfigValue |= AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_WR_OVERRUN_INT_EN_MASK;
    }
    else
    {
        nConfigValue &= ~AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_WR_OVERRUN_INT_EN_MASK;
    }

    /* Full interrupt */
    if (bFull)
    {
        nConfigValue |= AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_WR_FULL_INT_EN_MASK;
    }
    else
    {
        nConfigValue &= ~AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_WR_FULL_INT_EN_MASK;
    }

    HalBachWriteReg(eBank, nAddr,
                    (AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_WR_OVERRUN_INT_EN_MASK
                     | AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_WR_FULL_INT_EN_MASK),
                    nConfigValue);

    // translate interrupt
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA_WR_AFTER_RD_ADDR;
            nMask = AUDIO_BANK1_REG_DMA_WR_AFTER_RD_REG_DMA_1_WR_TH_EN_MASK;
            break;

        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }

    if (bDataTranslate)
    {
        HalBachWriteReg(eBank, nAddr, nMask, nMask);
    }
    else
    {
        HalBachWriteReg(eBank, nAddr, nMask, 0);
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

static int _HalAudDmaGetRdInt(CHIP_AIO_DMA_e eAioDma, BOOL *bUnderrun, BOOL *bEmpty, BOOL *bLocalEmpty)
{
    BachRegBank_e eBank;
    U8            nAddr;
    U16           nConfigValue = 0;
    U16           nMask;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR;
            break;
        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue = HalBachReadReg(eBank, nAddr);

    *bEmpty    = ((nConfigValue & AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_EMPTY_INT_EN_MASK) != 0);
    *bUnderrun = ((nConfigValue & AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_UNDERRUN_INT_EN_MASK) != 0);
    // Local empty
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA_RD_LOCAL_BUFF_CTRL_ADDR;
            nMask = AUDIO_BANK1_REG_DMA_RD_LOCAL_BUFF_CTRL_REG_DMA1_RD_LOCAL_BUFF_EMPTY_INT_EN_MASK;
            break;
        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }
    nConfigValue = HalBachReadReg(eBank, nAddr);
    *bLocalEmpty = (nConfigValue & nMask) ? TRUE : FALSE;

    return AIO_OK;
FAIL:
    return AIO_NG;
}

static int _HalAudDmaGetWrInt(CHIP_AIO_DMA_e eAioDma, BOOL *bOverrun, BOOL *bFull, BOOL *bLocalFull)
{
    U8            nAddr;
    BachRegBank_e eBank;
    U16           nConfigValue = 0;
    U16           nMask;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR;
            break;
        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue = HalBachReadReg(eBank, nAddr);

    *bFull    = ((nConfigValue & AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_WR_FULL_INT_EN_MASK) != 0);
    *bOverrun = ((nConfigValue & AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_WR_OVERRUN_INT_EN_MASK) != 0);

    // Local Full
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA_RD_LOCAL_BUFF_CTRL_ADDR;
            nMask = AUDIO_BANK1_REG_DMA_RD_LOCAL_BUFF_CTRL_REG_DMA1_WR_LOCAL_BUFF_FULL_INT_EN_MASK;
            break;
        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
    }
    nConfigValue = HalBachReadReg(eBank, nAddr);
    *bLocalFull  = ((nConfigValue & nMask) != 0);

    return AIO_OK;
FAIL:
    return AIO_NG;
}

static int _HalAudDmaRdGetFlags(CHIP_AIO_DMA_e eAioDma, BOOL *pbUnderrun, BOOL *pbEmtpy, BOOL *pbLocalEmpty,
                                BOOL *pbTransmit)
{
    BachRegBank_e eBank;
    U8            nAddr;
    U16           nConfigValue;
    U16           nMask;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA1_CTRL_8_ADDR;
            break;
        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue  = HalBachReadReg(eBank, nAddr);
    *pbUnderrun   = (nConfigValue & AUDIO_BANK1_REG_DMA1_CTRL_8_REG_DMA1_RD_UNDERRUN_FLAG_MASK) ? TRUE : FALSE;
    *pbEmtpy      = (nConfigValue & AUDIO_BANK1_REG_DMA1_CTRL_8_REG_DMA1_RD_EMPTY_FLAG_MASK) ? TRUE : FALSE;
    *pbLocalEmpty = (nConfigValue & AUDIO_BANK1_REG_DMA1_CTRL_8_REG_DMA1_RD_LOCALBUF_EMPTY_MASK) ? TRUE : FALSE;

    // period size
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA12_INT_STATUS_ADDR;
            nMask = AUDIO_BANK1_REG_DMA12_INT_STATUS_DMA_1_RD_TH_INT_STATUS_MASK;
            break;
        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue = HalBachReadReg(eBank, nAddr);
    *pbTransmit  = (nConfigValue & nMask) ? TRUE : FALSE;

    DBGMSG(AUDIO_DBG_LEVEL_IRQ | AUDIO_DBG_LEVEL_DMA,
           "DMA[%d] pbUnderrun[%d] pbEmtpy[%d] pbLocalEmpty[%d] pbTransmit[%d]\n", eAioDma, *pbUnderrun, *pbEmtpy,
           *pbLocalEmpty, *pbTransmit);

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudDmaWrGetFlags(CHIP_AIO_DMA_e eAioDma, BOOL *pbOverrun, BOOL *pbFull, BOOL *pbLocalFull,
                                BOOL *pbTransmit)
{
    BachRegBank_e eBank;
    U8            nAddr;
    U16           nConfigValue;
    U16           nMask;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA1_CTRL_8_ADDR;
            break;
        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue = HalBachReadReg(eBank, nAddr);
    *pbOverrun   = (nConfigValue & AUDIO_BANK1_REG_DMA1_CTRL_8_REG_DMA1_WR_OVERRUN_FLAG_MASK) ? TRUE : FALSE;
    *pbFull      = (nConfigValue & AUDIO_BANK1_REG_DMA1_CTRL_8_REG_DMA1_WR_FULL_FLAG_MASK) ? TRUE : FALSE;
    *pbLocalFull = (nConfigValue & AUDIO_BANK1_REG_DMA1_CTRL_8_REG_DMA1_WR_LOCALBUF_FULL_MASK) ? TRUE : FALSE;

    // period size
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA12_INT_STATUS_ADDR;
            nMask = AUDIO_BANK1_REG_DMA12_INT_STATUS_DMA_1_WR_TH_INT_STATUS_MASK;
            break;
        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue = HalBachReadReg(eBank, nAddr);
    *pbTransmit  = (nConfigValue & nMask) ? TRUE : FALSE;

    // DBGMSG(AUDIO_DBG_LEVEL_IRQ | AUDIO_DBG_LEVEL_DMA,
    //        "DMA is %d, pbOverrun is %d, pbFull is %d, pbLocalFull is %d, pbTransmit is %d \n", eAioDma, *pbOverrun,
    //        *pbFull, *pbLocalFull, *pbTransmit);

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudDmaGetMchSelConfigValue(AudMchSel_e eMchSel, U16 *nChSel, EN_CHIP_AI_MCH eMchSet)
{
    if (eMchSet == E_CHIP_AI_MCH_A)
    {
        switch (eMchSel)
        {
            case E_AUD_MCH_SEL_DMIC01:
                *nChSel = 0; // b'0000
                break;
            case E_AUD_MCH_SEL_AMIC01:
                *nChSel = 3; // b'0011
                break;
            case E_AUD_MCH_SEL_I2S_A_RX01:
                *nChSel = 5; // b'0101
                break;
            case E_AUD_MCH_SEL_I2S_A_RX23:
                *nChSel = 6; // b'0110
                break;
            case E_AUD_MCH_SEL_I2S_A_RX45:
                *nChSel = 7; // b'0111
                break;
            case E_AUD_MCH_SEL_I2S_A_RX67:
                *nChSel = 8; // b'1000
                break;
            case E_AUD_MCH_SEL_SRC01:
                *nChSel = 13; // b'1101
                break;
            case E_AUD_MCH_SEL_FIXED_ZERO:
            case E_AUD_MCH_SEL_NULL:
                *nChSel = 15; // b'1111, fix zero
                break;
            default:
                goto FAIL;
                break;
        }
    }
    else
    {
        goto FAIL;
    }

    return AIO_OK;
FAIL:
    ERRMSG("eMchSet[%d] eMchSel[%d] Fail !\n", eMchSet, eMchSel);
    return AIO_NG;
}

static int _HalAudDmaSetChannel(CHIP_AIO_DMA_e eAioDma, U16 channel)
{
    u16 nChannelMode = 0;

    switch (channel)
    {
        case E_AUD_CHANNEL_NUM_1:
        case E_AUD_CHANNEL_NUM_2:
            nChannelMode = 1;
            break;
        case E_AUD_CHANNEL_NUM_4:
            nChannelMode = 2;
            break;
        case E_AUD_CHANNEL_NUM_8:
            nChannelMode = 3;
            break;
        case E_AUD_CHANNEL_NUM_16:
            nChannelMode = 4;
            break;
        default:
            break;
    }

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_ADDR,
                            AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_REG_DMA_W1_CH_MODE_MASK,
                            (nChannelMode << AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_REG_DMA_W1_CH_MODE_SHIFT));
            break;
        default:
            break;
    }

    return AIO_OK;
}

static int _HalAudDmaSetBitMode(CHIP_AIO_DMA_e eAioDma, U16 bitWidth)
{
    return AIO_OK;
}

static int _HalAudDmaSelChannelMono(CHIP_AIO_DMA_e eAioDma, AudAoChMode_e enChMode)
{
    int aoChL, aoChR;

    switch (enChMode)
    {
        case E_AUD_AO_CH_MODE_STEREO:
        case E_AUD_AO_CH_MODE_DOUBLE_LEFT:
        case E_AUD_AO_CH_MODE_DOUBLE_RIGHT:
        case E_AUD_AO_CH_MODE_EXCHANGE:
            goto FAIL;
            break;
        case E_AUD_AO_CH_MODE_DOUBLE_MONO:
            aoChL = E_AUD_AO_CH_SEL_L;
            aoChR = E_AUD_AO_CH_SEL_L;
            break;
        case E_AUD_AO_CH_MODE_ONLY_LEFT:
            aoChL = E_AUD_AO_CH_SEL_L;
            aoChR = E_AUD_AO_CH_MASK_TO_ZERO;
            break;
        case E_AUD_AO_CH_MODE_ONLY_RIGHT: // Mono enable just DMA L has data
            aoChL = E_AUD_AO_CH_MASK_TO_ZERO;
            aoChR = E_AUD_AO_CH_SEL_L;
            break;
        default:
            goto FAIL;
            break;
    }

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL9_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL9_REG_DMA1_RD_L_SEL_MASK,
                            (aoChL << AUDIO_BANK1_REG_DMA_TEST_CTRL9_REG_DMA1_RD_L_SEL_SHIFT));
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL9_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL9_REG_DMA1_RD_R_SEL_MASK,
                            (aoChR << AUDIO_BANK1_REG_DMA_TEST_CTRL9_REG_DMA1_RD_R_SEL_SHIFT));
            break;
        default:
            goto FAIL;
            break;
    }

    return AIO_OK;
FAIL:
    ERRMSG("eAioDma[%d] channel[%d] Fail !", eAioDma, enChMode);
    return AIO_NG;
}

static int _HalAudDmaSelChannelStereo(CHIP_AIO_DMA_e eAioDma, AudAoChMode_e enChMode)
{
    int aoChL, aoChR;

    switch (enChMode)
    {
        case E_AUD_AO_CH_MODE_STEREO:
            aoChL = E_AUD_AO_CH_SEL_L;
            aoChR = E_AUD_AO_CH_SEL_R;
            break;
        case E_AUD_AO_CH_MODE_DOUBLE_LEFT:
            aoChL = E_AUD_AO_CH_SEL_L;
            aoChR = E_AUD_AO_CH_SEL_L;
            break;
        case E_AUD_AO_CH_MODE_DOUBLE_RIGHT:
            aoChL = E_AUD_AO_CH_SEL_R;
            aoChR = E_AUD_AO_CH_SEL_R;
            break;
        case E_AUD_AO_CH_MODE_EXCHANGE:
            aoChL = E_AUD_AO_CH_SEL_R;
            aoChR = E_AUD_AO_CH_SEL_L;
            break;
        case E_AUD_AO_CH_MODE_DOUBLE_MONO:
            aoChL = E_AUD_AO_CH_SEL_RL_DIV2;
            aoChR = E_AUD_AO_CH_SEL_RL_DIV2;
            break;
        case E_AUD_AO_CH_MODE_ONLY_LEFT:
            aoChL = E_AUD_AO_CH_SEL_L;
            aoChR = E_AUD_AO_CH_MASK_TO_ZERO;
            break;
        case E_AUD_AO_CH_MODE_ONLY_RIGHT:
            aoChL = E_AUD_AO_CH_MASK_TO_ZERO;
            aoChR = E_AUD_AO_CH_SEL_R;
            break;
        case E_AUD_AO_CH_MODE_DOUBLE_NONE:
            aoChL = E_AUD_AO_CH_MASK_TO_ZERO;
            aoChR = E_AUD_AO_CH_MASK_TO_ZERO;
            break;
        default:
            goto FAIL;
            break;
    }

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL9_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL9_REG_DMA1_RD_L_SEL_MASK,
                            (aoChL << AUDIO_BANK1_REG_DMA_TEST_CTRL9_REG_DMA1_RD_L_SEL_SHIFT));
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL9_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL9_REG_DMA1_RD_R_SEL_MASK,
                            (aoChR << AUDIO_BANK1_REG_DMA_TEST_CTRL9_REG_DMA1_RD_R_SEL_SHIFT));
            break;
        default:
            goto FAIL;
            break;
    }

    return AIO_OK;
FAIL:
    ERRMSG("aoDma sel eAioDma[%d] channel[%d] Fail !", eAioDma, enChMode);
    return AIO_NG;
}

// ------------------------------Extern Function-------------------------------------------------
int HalAudDmaInit(void)
{
    int ret = AIO_OK;
    u16 i;

    _HalAudDmaFirstInit();
    _HalAudDmaWrMchInit();
    _HalAudIntEnable();
    // Software init
    for (i = 0; i < E_CHIP_AIO_DMA_TOTAL; i++)
    {
        memset(&g_aRuntimeData[i], 0, sizeof(PcmRuntimeData_t));
        g_aRuntimeData[i].nStatus = E_DMA_STATUS_INIT;
        CamOsSpinInit(&g_aRuntimeData[i].tPcmLock);
        ret |= HalAudDmaIntEnable(i, FALSE, FALSE, FALSE);
        if (ret != AIO_OK)
        {
            goto FAIL;
        }
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

void HalAudDmaDeInit(void)
{
    int i = 0;
    for (i = 0; i < E_CHIP_AIO_DMA_TOTAL; i++)
    {
        CamOsSpinDeinit(&g_aRuntimeData[i].tPcmLock);
    }
}

int HalAudDmaConfig(CHIP_AIO_DMA_e eAioDma, DmaPar_t *ptParam)
{
    int ret         = AIO_OK;
    int line        = 0;
    u16 nThdValue   = 0;
    u32 nPeriodSize = 0, nBufferSize = 0;

    DBGMSG(AUDIO_DBG_LEVEL_DMA, "eAioDma[%d] status[%u]\n", eAioDma, g_aRuntimeData[eAioDma].nStatus);

    if (ptParam->nPeriodSize >= ptParam->nBufferSize)
    {
        line = __LINE__;
        goto FAIL;
    }

    nBufferSize = ptParam->nBufferSize;

// FIXME
#if 0
    if (eAioDma >= E_CHIP_AIO_DMA_AO_A) // Reader
    {
        nPeriodSize = ptParam->nPeriodSize;
    }
    else // Writer "overrun threshold" should add dma local buffer size !
    {
        nPeriodSize = ptParam->nPeriodSize;//((ptParam->nPeriodSize) ? (ptParam->nPeriodSize + DMA_LOCALBUF_SIZE) : (ptParam->nPeriodSize));
    }
#else
    nPeriodSize = ptParam->nPeriodSize;
#endif

    //
    ret |= HalAudDmaSetPhyAddr(eAioDma, ptParam->nPhysDmaAddr, ptParam->nBufferSize);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    ptParam->nPeriodCnt = round_down(ptParam->nPeriodCnt, MIU_WORD_BYTE_SIZE);
    nThdValue           = ptParam->nPeriodCnt / MIU_WORD_BYTE_SIZE;

    //
    if (eAioDma >= E_CHIP_AIO_DMA_AO_A) // Reader
    {
        ret |= HalAudDmaAoSetThreshold(eAioDma, (nBufferSize - nPeriodSize));
        // When written to the TH value of the register, the hardware will count from 0, so it will subtract one from
        // the expected value
        ret |= HalAudDmaAoSetTransThreshold(eAioDma, nThdValue);
    }
    else
    {
        ret |= HalAudDmaAiSetThreshold(eAioDma, (nPeriodSize));
        // ret |= HalAudDmaAiSetThreshold(eAioDma, (nBufferSize - 16));
        //  because WDMA sends requests only once every four period counts, so the period size needs to be divisible by
        //  four
        nThdValue = round_down(nThdValue, 8);
        if (nThdValue == 0)
        {
            goto FAIL;
        }
        ret |= HalAudDmaAiSetTransThreshold(eAioDma, nThdValue);
    }

    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    g_AudDmaParam[eAioDma].pDmaBuf       = ptParam->pDmaBuf;
    g_AudDmaParam[eAioDma].bIsOnlyEvenCh = ptParam->bIsOnlyEvenCh;
    g_AudDmaParam[eAioDma].nPhysDmaAddr  = ptParam->nPhysDmaAddr;
    g_AudDmaParam[eAioDma].nBufferSize   = ptParam->nBufferSize;
    g_AudDmaParam[eAioDma].nChannels     = ptParam->nChannels;
    g_AudDmaParam[eAioDma].nPeriodSize   = ptParam->nPeriodSize;
    g_AudDmaParam[eAioDma].nPeriodCnt    = ptParam->nPeriodCnt;
    g_AudDmaParam[eAioDma].nBitWidth     = ptParam->nBitWidth;
    g_AudDmaParam[eAioDma].nSampleRate   = ptParam->nSampleRate;
    g_AudDmaParam[eAioDma].nInterleaved  = ptParam->nInterleaved;
    g_aRuntimeData[eAioDma].nFrameBits   = ptParam->nChannels * ptParam->nBitWidth;
    g_aRuntimeData[eAioDma].nHwFrameBits = g_aRuntimeData[eAioDma].nFrameBits;
    g_aRuntimeData[eAioDma].nStatus      = E_DMA_STATUS_SETUP;

    _HalAudDmaSetChannel(eAioDma, g_AudDmaParam[eAioDma].nChannels);

    _HalAudDmaSetBitMode(eAioDma, g_AudDmaParam[eAioDma].nBitWidth);

    return AIO_OK;

FAIL:
    ERRMSG("[%d] eAioDma %d error\n", line, eAioDma);
    return AIO_NG;
}

int HalAudDmaOpen(CHIP_AIO_DMA_e eAioDma)
{
    int           ret = AIO_OK;
    PcmDmaData_t *ptPcmData;

    if (g_aRuntimeData[eAioDma].nStatus != E_DMA_STATUS_SETUP)
    {
        ERRMSG("status wrong\n");
        goto FAIL;
    }

    ptPcmData = CamOsMemCalloc(1, sizeof(PcmDmaData_t));
    if (ptPcmData == NULL)
    {
        ERRMSG("insufficient resource\n");
        goto FAIL;
    }

    ptPcmData->nChnId      = eAioDma;
    ptPcmData->tDmaAddr    = g_AudDmaParam[eAioDma].nPhysDmaAddr;
    ptPcmData->nBufBytes   = g_AudDmaParam[eAioDma].nBufferSize;
    ptPcmData->tBufSize    = _BytesToHwFrames(&g_aRuntimeData[eAioDma], g_AudDmaParam[eAioDma].nBufferSize);
    ptPcmData->tPeriodSize = _BytesToHwFrames(&g_aRuntimeData[eAioDma], g_AudDmaParam[eAioDma].nPeriodSize);

    CamOsSpinInit(&ptPcmData->tLock);
    g_aRuntimeData[eAioDma].pPrivateData = ptPcmData;

    ret |= HalAudDmaSetRate(eAioDma, HalAudApiRateToEnum(g_AudDmaParam[eAioDma].nSampleRate));
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    g_aRuntimeData[eAioDma].nStatus = E_DMA_STATUS_OPEN;

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaPrepare(CHIP_AIO_DMA_e eAioDma)
{
    int ret = AIO_OK;

    ret |= HalAudDmaReset(eAioDma);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }
    ret |= HalAudDmaIntEnable(eAioDma, FALSE, FALSE, FALSE);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    g_aRuntimeData[eAioDma].nStatus = E_DMA_STATUS_PREPARED;

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudDmaStart(CHIP_AIO_DMA_e eAioDma)
{
    int ret  = AIO_OK;
    int line = 0;

    // DBGMSG(AUDIO_DBG_LEVEL_DMA, "eAioDma[%d] level[%u]\n", eAioDma, HalAudDmaGetLevelCnt(eAioDma));

    if (eAioDma == E_CHIP_AIO_DMA_AI_DIRECT_A || eAioDma == E_CHIP_AIO_DMA_AO_DIRECT_A)
    {
        g_aRuntimeData[eAioDma].nStatus = E_DMA_STATUS_RUNNING;
        return AIO_OK;
    }

    // Writer for device (CPU read from device).
    // Flush L3 before dma start.
    if (eAioDma < E_CHIP_AIO_DMA_AO_A)
    {
        CamOsMiuPipeFlush();
    }

    //
    ret |= HalAudDmaClearInt(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    ret |= HalAudDmaEnable(eAioDma, TRUE);
    DBGMSG(AUDIO_DBG_LEVEL_DMA, "eAioDma[%d] level[%u]\n", eAioDma, HalAudDmaGetLevelCnt(eAioDma));
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    g_aRuntimeData[eAioDma].nStatus = E_DMA_STATUS_RUNNING;

    return AIO_OK;
FAIL:
    ERRMSG("Line:%d eAioDma[%d]\n", line, eAioDma);
    return AIO_NG;
}

int HalAudDmaStop(CHIP_AIO_DMA_e eAioDma)
{
    int ret = AIO_OK;

    if (eAioDma == E_CHIP_AIO_DMA_AI_DIRECT_A || eAioDma == E_CHIP_AIO_DMA_AO_DIRECT_A)
    {
        g_aRuntimeData[eAioDma].nStatus = E_DMA_STATUS_OPEN;
        return ret;
    }

    ret |= HalAudDmaEnable(eAioDma, FALSE);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    g_aRuntimeData[eAioDma].nStatus = E_DMA_STATUS_PAUSED;

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudDmaPause(CHIP_AIO_DMA_e eAioDma)
{
    int ret = AIO_OK;

    if (g_aRuntimeData[eAioDma].nStatus != E_DMA_STATUS_RUNNING)
    {
        return AIO_OK;
    }

    ret |= HalAudDmaEnable(eAioDma, FALSE);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    g_aRuntimeData[eAioDma].nStatus = E_DMA_STATUS_PAUSED;

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudDmaResume(CHIP_AIO_DMA_e eAioDma)
{
    int ret = AIO_OK;

    // When str resume, will call resume, so don't return FAIL.
    // if (g_aRuntimeData[eAioDma].nStatus != E_DMA_STATUS_PAUSED)
    //{
    //    return AIO_OK;
    //}

    ret |= HalAudDmaEnable(eAioDma, TRUE);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    g_aRuntimeData[eAioDma].nStatus = E_DMA_STATUS_RUNNING;

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudDmaClose(CHIP_AIO_DMA_e eAioDma)
{
    if (g_aRuntimeData[eAioDma].pPrivateData)
    {
        PcmDmaData_t *ptPcmData = (PcmDmaData_t *)g_aRuntimeData[eAioDma].pPrivateData;
        CamOsSpinDeinit(&ptPcmData->tLock);
        CamOsMemRelease(g_aRuntimeData[eAioDma].pPrivateData);
        g_aRuntimeData[eAioDma].pPrivateData = NULL;
    }
    g_aRuntimeData[eAioDma].nStatus = E_DMA_STATUS_INIT;

    return AIO_OK;
}

static U32 _HalAudDmaTrigLevelCnt(CHIP_AIO_DMA_e eAioDma, U32 nDataSize)
{
    U16 nConfigValue = 0;

    nConfigValue = (U16)((nDataSize / MIU_WORD_BYTE_SIZE) & REG_WR_SIZE_MSK);
    nDataSize    = nConfigValue * MIU_WORD_BYTE_SIZE;
    if (nConfigValue > 0)
    {
        switch (eAioDma)
        {
            case E_CHIP_AIO_DMA_AI_A:
                HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_12_ADDR,
                                AUDIO_BANK1_REG_DMA1_CTRL_12_REG_DMA1_WR_SIZE_MASK, nConfigValue);
                HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_9_ADDR,
                                AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_TRIG_MASK,
                                AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_TRIG_MASK);
                break;

            case E_CHIP_AIO_DMA_AO_A:
                HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_4_ADDR,
                                AUDIO_BANK1_REG_DMA1_CTRL_4_REG_DMA1_RD_SIZE_MASK, nConfigValue);
                HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_1_ADDR,
                                AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_TRIG_MASK,
                                AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_TRIG_MASK);
                break;

            default:
                ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
                return 0;
        }

        return nDataSize;
    }

    return 0;
}

int HalAudDmaTrigPcm(CHIP_AIO_DMA_e eAioDma, U32 nFrames)
{
    PcmDmaData_t *ptDmaData  = (PcmDmaData_t *)(g_aRuntimeData[eAioDma].pPrivateData);
    int           nErr       = 0;
    U32           nTrigCount = 0;

    // DBGMSG(AUDIO_DBG_LEVEL_DMA, "eAioDma[%d] nFrames[%d]", eAioDma, nFrames);

    if (nFrames == 0)
    {
        goto null_data;
    }

    CamOsSpinLockIrqSave(&g_aRuntimeData[eAioDma].tPcmLock);
    switch (g_aRuntimeData[eAioDma].nStatus)
    {
        case E_DMA_STATUS_PREPARED:
        case E_DMA_STATUS_RUNNING:
        case E_DMA_STATUS_PAUSED:
            break;
        default:
            nErr = -EBADFD;
            goto _end_unlock;
    }
    CamOsMiuPipeFlush();
    nTrigCount = _HalAudDmaTrigLevelCnt(ptDmaData->nChnId, (_HwFramesToBytes(&g_aRuntimeData[eAioDma], nFrames)));
_end_unlock:

    CamOsSpinUnlockIrqRestore(&g_aRuntimeData[eAioDma].tPcmLock);
    return nTrigCount; // nSize;

null_data:
    return 0;
}

U32 HalAudDmaGetLevelCnt(CHIP_AIO_DMA_e eAioDma)
{
    U32           nRawMiuUnitLevelCnt;
    PcmDmaData_t *ptDmaData = (PcmDmaData_t *)(g_aRuntimeData[eAioDma].pPrivateData);

    CamOsSpinLockIrqSave(&ptDmaData->tLock);

    nRawMiuUnitLevelCnt = _HalAudDmaGetRawMiuUnitLevelCnt(eAioDma);
    //
    if (eAioDma < E_CHIP_AIO_DMA_AO_A) // dma writer
    {
        nRawMiuUnitLevelCnt = ((nRawMiuUnitLevelCnt > 0) ? (nRawMiuUnitLevelCnt) : 0);
    }
    else // dma reader
    {
        nRawMiuUnitLevelCnt = ((nRawMiuUnitLevelCnt <= 0) ? 0 : nRawMiuUnitLevelCnt);
    }

    CamOsSpinUnlockIrqRestore(&ptDmaData->tLock);

    DBGMSG(AUDIO_DBG_LEVEL_IRQ | AUDIO_DBG_LEVEL_DMA, "eAioDma[%d] level[%u]\n", eAioDma, nRawMiuUnitLevelCnt);
    return (nRawMiuUnitLevelCnt * MIU_WORD_BYTE_SIZE);
}

int HalAudDmaSetChannelMode(CHIP_AIO_DMA_e eAioDma, AudAoChMode_e enChMode)
{
    int ret = AIO_OK;

    if (g_AudDmaParam[eAioDma].bIsOnlyEvenCh)
    {
        ret = _HalAudDmaSelChannelMono(eAioDma, enChMode);
    }
    else
    {
        ret = _HalAudDmaSelChannelStereo(eAioDma, enChMode);
    }

    return ret;
}

void HalAudDmaSetOnlyEvenCh(CHIP_AIO_DMA_e eAioDma, BOOL bIsOnlyEvenCh)
{
    g_AudDmaParam[eAioDma].bIsOnlyEvenCh = bIsOnlyEvenCh;
}

int HalAudDmaReset(CHIP_AIO_DMA_e eAioDma)
{
    _UDelay(4000);
    return _HalAudDmaReInit(eAioDma);
}

int HalAudDmaEnable(CHIP_AIO_DMA_e eAioDma, BOOL bEnable)
{
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            if (bEnable)
            {
                // reader prefetch enable, it should be enabled before reader enable
                HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR,
                                AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_ENABLE_MASK,
                                AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_ENABLE_MASK);
                _UDelay(10);
                HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_1_ADDR,
                                AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_ENABLE_MASK,
                                AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_ENABLE_MASK);
            }
            else
            {
                // reader prefetch enable, it has to be disabled before dma init
                HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_1_ADDR,
                                AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_ENABLE_MASK, 0);
                HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR,
                                AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_ENABLE_MASK, 0);
            }
            break;

        case E_CHIP_AIO_DMA_AI_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_9_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_ENABLE_MASK,
                            (bEnable ? AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_ENABLE_MASK : 0));
            break;

        default:
            ERRMSG("eAioDma[%d] bEnable[%d] Fail !", eAioDma, bEnable);
            goto FAIL;
            break;
    }

    DBGMSG(AUDIO_DBG_LEVEL_DMA, "eAioDma[%d] bEnable[%d]", eAioDma, bEnable);

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaIntLocalEnable(CHIP_AIO_DMA_e eAioDma, BOOL bLocalfifo)
{
    U8            nAddr;
    BachRegBank_e eBank;
    U16           nMask;

    // local empty
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA_RD_LOCAL_BUFF_CTRL_ADDR;
            nMask = AUDIO_BANK1_REG_DMA_RD_LOCAL_BUFF_CTRL_REG_DMA1_RD_LOCAL_BUFF_EMPTY_INT_EN_MASK;
            break;
        case E_CHIP_AIO_DMA_AO_B:
        case E_CHIP_AIO_DMA_AO_C:
        case E_CHIP_AIO_DMA_AO_D:
        case E_CHIP_AIO_DMA_AO_E:
        case E_CHIP_AIO_DMA_AO_DIRECT_A:
        case E_CHIP_AIO_DMA_AO_DIRECT_B:
            return AIO_OK;
        case E_CHIP_AIO_DMA_AI_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA_RD_LOCAL_BUFF_CTRL_ADDR;
            nMask = AUDIO_BANK1_REG_DMA_RD_LOCAL_BUFF_CTRL_REG_DMA1_WR_LOCAL_BUFF_FULL_INT_EN_MASK;
            break;
        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }
    if (bLocalfifo)
    {
        HalBachWriteReg(eBank, nAddr, nMask, nMask);
    }
    else
    {
        HalBachWriteReg(eBank, nAddr, nMask, 0);
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudDmaIntEnable(CHIP_AIO_DMA_e eAioDma, BOOL bDatatrigger, BOOL bDataboundary, BOOL bDataTranslate)
{
    int ret = AIO_OK;

    if (eAioDma < E_CHIP_AIO_DMA_AO_START)
    {
        ret |= _HalAudDmaWrIntEnable(eAioDma, bDatatrigger, bDataboundary, bDataTranslate);
    }
    else
    {
        ret |= _HalAudDmaRdIntEnable(eAioDma, bDatatrigger, bDataboundary, bDataTranslate);
    }

    if (bDataTranslate)
    {
        HalAudDmaClearIrqStatus(eAioDma);
    }

    if (ret != AIO_OK)
    {
        ERRMSG("eAioDma[%d] bDatatrigger[%d] bDataboundary[%d] Fail !\n", eAioDma, bDatatrigger, bDataboundary);
        goto FAIL;
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudDmaClearInt(CHIP_AIO_DMA_e eAioDma)
{
    U16 nRetry = 0;

    HalAudDmaClearIrqStatus(eAioDma);

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            // DMA writer1 full flag clear / DMA writer1 local buffer full flag clear
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_WR_FULL_FLAG_CLR_MASK,
                            AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_WR_FULL_FLAG_CLR_MASK);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_WR_FULL_FLAG_CLR_MASK, 0);
            break;

        case E_CHIP_AIO_DMA_AO_A:
            // DMA reader1 empty flag clear / DMA reader1 local buffer empty flag clear
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_EMPTY_FLAG_CLR_MASK,
                            AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_EMPTY_FLAG_CLR_MASK);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_EMPTY_FLAG_CLR_MASK, 0);
            break;

        default:
            ERRMSG("eAioDma[%d] Fail !\n", eAioDma);
            goto FAIL;
            break;
    }

    /* Be careful!! check clear bit down*/
    while (nRetry < 20)
    {
        if ((HalBachReadReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR)
             & AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_EMPTY_FLAG_CLR_MASK)
            == 0)
            break;

        HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_0_ADDR,
                        AUDIO_BANK1_REG_DMA1_CTRL_0_REG_DMA1_RD_EMPTY_FLAG_CLR_MASK, 0);
        nRetry++;
    }

    if (nRetry == 20)
    {
        ERRMSG("eAioDma = %d, clear interrupt fail ! \n", (int)eAioDma);
        return FALSE;
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudDmaSetPhyAddr(CHIP_AIO_DMA_e eAioDma, U64 nBufAddrOffset, U32 nBufSize)
{
    U16 nMiuAddrLo, nMiuAddrHi, nMiuAddrExtra, nMiuSize;
    if ((nBufSize / MIU_WORD_BYTE_SIZE) > ((1 << 16) - 1))
    {
        ERRMSG("eAioDma %d, BufSize %d BufSize overflow ! Fail !] \n", eAioDma, nBufSize);
        goto FAIL;
    }

    if ((nBufSize % MIU_WORD_BYTE_SIZE) != 0)
    {
        ERRMSG("eAioDma %d, BufSize %d Buf size must be multiple of MIU_WORD_BYTE_SIZE ! Fail !] \n", eAioDma,
               nBufSize);
        goto FAIL;
    }

    if ((nBufAddrOffset % MIU_WORD_BYTE_SIZE) != 0)
    {
        ERRMSG("eAioDma %d, nBufAddrOffset %lld Buf addr must be multiple of MIU_WORD_BYTE_SIZE ! Fail !]\n", eAioDma,
               nBufAddrOffset);
        goto FAIL;
    }

    // flow need check
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            nMiuAddrLo    = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE)
                               & AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_BASE_ADDR_LOW_MASK);
            nMiuAddrHi    = (U16)(((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_WR_BASE_ADDR_HI_OFFSET)
                               & AUDIO_BANK1_REG_DMA1_CTRL_10_REG_DMA1_WR_BASE_ADDR_HIGH_MASK);
            nMiuAddrExtra = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_WR_BASE_ADDR_ADDITION_OFFSET);
            nMiuSize      = (U16)((nBufSize / MIU_WORD_BYTE_SIZE) & REG_WR_BUFF_SIZE_MSK);

            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_9_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_BASE_ADDR_LOW_MASK, nMiuAddrLo);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_10_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_10_REG_DMA1_WR_BASE_ADDR_HIGH_MASK, nMiuAddrHi);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL8_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL8_REG_DMA1_WR_BASE_ADDR_H_MASK, // 0x1502f4
                            nMiuAddrExtra << AUDIO_BANK1_REG_DMA_TEST_CTRL8_REG_DMA1_WR_BASE_ADDR_H_SHIFT);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_11_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_11_REG_DMA1_WR_BUFF_SIZE_MASK, nMiuSize);
            break;

        case E_CHIP_AIO_DMA_AO_A:
            nMiuAddrLo    = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE)
                               & AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_BASE_ADDR_LOW_MASK);
            nMiuAddrHi    = (U16)(((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_RD_BASE_ADDR_HI_OFFSET)
                               & AUDIO_BANK1_REG_DMA1_CTRL_2_REG_DMA1_RD_BASE_ADDR_HIGH_MASK);
            nMiuSize      = (U16)((nBufSize / MIU_WORD_BYTE_SIZE) & REG_RD_BUFF_SIZE_MSK);
            nMiuAddrExtra = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_RD_BASE_ADDR_ADDITION_OFFSET);

            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_1_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_1_REG_DMA1_RD_BASE_ADDR_LOW_MASK, nMiuAddrLo);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_2_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_2_REG_DMA1_RD_BASE_ADDR_HIGH_MASK, nMiuAddrHi);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL8_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL8_REG_DMA1_RD_BASE_ADDR_H_MASK,
                            nMiuAddrExtra << AUDIO_BANK1_REG_DMA_TEST_CTRL8_REG_DMA1_RD_BASE_ADDR_H_SHIFT);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_3_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_3_REG_DMA1_RD_BUFF_SIZE_MASK, nMiuSize);
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaAiSetThreshold(CHIP_AIO_DMA_e eAioDma, U32 nOverrunTh)
{
    U16 nMiuOverrunTh;

    if ((nOverrunTh < MIU_WORD_BYTE_SIZE) || (nOverrunTh > (0xFFFFUL * MIU_WORD_BYTE_SIZE)))
    {
        ERRMSG("eAioDma[%d] nOverrunTh[%d] Fail !\n", eAioDma, nOverrunTh);
        goto FAIL;
    }

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            nMiuOverrunTh =
                (U16)((nOverrunTh / MIU_WORD_BYTE_SIZE) & AUDIO_BANK1_REG_DMA1_CTRL_13_REG_DMA1_WR_OVERRUN_TH_MASK);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_13_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_13_REG_DMA1_WR_OVERRUN_TH_MASK, nMiuOverrunTh);
            break;

        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaAoSetThreshold(CHIP_AIO_DMA_e eAioDma, U32 nUnderrunTh)
{
    U16 nMiuUnderrunTh;

    if ((nUnderrunTh < MIU_WORD_BYTE_SIZE) || (nUnderrunTh > (0xFFFFUL * MIU_WORD_BYTE_SIZE)))
    {
        ERRMSG("eAioDma[%d] nUnderrunTh[%d] Fail !", eAioDma, nUnderrunTh);
        goto FAIL;
    }

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            nMiuUnderrunTh =
                (U16)((nUnderrunTh / MIU_WORD_BYTE_SIZE) & AUDIO_BANK1_REG_DMA1_CTRL_6_REG_DMA1_RD_UNDERRUN_TH_MASK);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_6_ADDR,
                            AUDIO_BANK1_REG_DMA1_CTRL_6_REG_DMA1_RD_UNDERRUN_TH_MASK, nMiuUnderrunTh);
            break;

        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaAoSetTransThreshold(CHIP_AIO_DMA_e eAioDma, U32 nPeriodCnt)
{
    U8            nAddr;
    BachRegBank_e eBank;
    U16           nMask;
    U8            nPos;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA_1_RD_TH_ADDR;
            nMask = AUDIO_BANK1_REG_DMA_1_RD_TH_REG_DMA_1_RD_TH_MASK;
            nPos  = AUDIO_BANK1_REG_DMA_1_RD_TH_REG_DMA_1_RD_TH_SHIFT;
            break;
        case E_CHIP_AIO_DMA_AO_B:
        case E_CHIP_AIO_DMA_AO_C:
        case E_CHIP_AIO_DMA_AO_D:
        case E_CHIP_AIO_DMA_AO_E:
        case E_CHIP_AIO_DMA_AO_DIRECT_A:
        case E_CHIP_AIO_DMA_AO_DIRECT_B:
            return AIO_OK;
        default:
            ERRMSG("eAioDma[%d] Fail !\n", eAioDma);
            goto FAIL;
            break;
    }

    HalBachWriteReg(eBank, nAddr, nMask, nPeriodCnt << nPos);

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudDmaAiSetTransThreshold(CHIP_AIO_DMA_e eAioDma, U32 nPeriodCnt)
{
    U8            nAddr;
    BachRegBank_e eBank;
    U16           nMask;
    U8            nPos;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            // WDMA1 local buffer len is 256, can't support less than 128 to trigger period size interrupt
            if (nPeriodCnt <= 16)
            {
                nPeriodCnt = 16;
            }
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_DMA_1_WR_TH_ADDR;
            nMask = AUDIO_BANK1_REG_DMA_1_WR_TH_REG_DMA_1_WR_TH_MASK;
            nPos  = AUDIO_BANK1_REG_DMA_1_WR_TH_REG_DMA_1_WR_TH_SHIFT;
            break;
        case E_CHIP_AIO_DMA_AI_B:
        case E_CHIP_AIO_DMA_AI_C:
        case E_CHIP_AIO_DMA_AI_D:
        case E_CHIP_AIO_DMA_AI_E:
        case E_CHIP_AIO_DMA_AI_DIRECT_A:
        case E_CHIP_AIO_DMA_AI_DIRECT_B:
            return AIO_OK;

        default:
            ERRMSG("eAioDma[%d] Fail !\n", eAioDma);
            goto FAIL;
            break;
    }

    HalBachWriteReg(eBank, nAddr, nMask, nPeriodCnt << nPos);

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudDmaGetFlags(CHIP_AIO_DMA_e eAioDma, BOOL *pbDatatrigger, BOOL *pbDataboundary, BOOL *pbLocalData,
                      BOOL *pbTransmit)
{
    int ret = AIO_OK;

    if (eAioDma < E_CHIP_AIO_DMA_AO_A)
    {
        ret |= _HalAudDmaWrGetFlags(eAioDma, pbDatatrigger, pbDataboundary, pbLocalData, pbTransmit);
    }
    else
    {
        ret |= _HalAudDmaRdGetFlags(eAioDma, pbDatatrigger, pbDataboundary, pbLocalData, pbTransmit);
    }

    if (ret != AIO_OK)
    {
        ERRMSG("eAioDma[%d] Fail !\n", eAioDma);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaMonoEnable(CHIP_AIO_DMA_e eAioDma, BOOL bEn, BOOL bMonoCopyEn)
{
    U16 nValue = 0;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL7_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL7_REG_DMA1_WR_MONO_MASK,
                            (bEn ? AUDIO_BANK1_REG_DMA_TEST_CTRL7_REG_DMA1_WR_MONO_MASK : 0));
            break;

        case E_CHIP_AIO_DMA_AO_A:
            if (bEn)
            {
                nValue = nValue | AUDIO_BANK1_REG_DMA_TEST_CTRL7_REG_DMA1_RD_MONO_MASK;
            }

            if (bMonoCopyEn)
            {
                nValue = nValue | AUDIO_BANK1_REG_DMA_TEST_CTRL7_REG_DMA1_RD_MONO_COPY_MASK;
            }

            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL7_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL7_REG_DMA1_RD_MONO_MASK
                                | AUDIO_BANK1_REG_DMA_TEST_CTRL7_REG_DMA1_RD_MONO_COPY_MASK,
                            nValue);
            break;

        default:
            ERRMSG("eAioDma[%d] Fail !\n", eAioDma);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaGetInt(CHIP_AIO_DMA_e eAioDma, BOOL *bDatatrigger, BOOL *bDataboundary, BOOL *bDataLocal)
{
    int ret = AIO_OK;

    if (eAioDma < E_CHIP_AIO_DMA_AO_A)
    {
        ret |= _HalAudDmaGetWrInt(eAioDma, bDatatrigger, bDataboundary, bDataLocal);
    }
    else
    {
        ret |= _HalAudDmaGetRdInt(eAioDma, bDatatrigger, bDataboundary, bDataLocal);
    }

    if (ret != AIO_OK)
    {
        ERRMSG("eAioDma[%d] Fail !", eAioDma);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaWrConfigMchCore(EN_CHIP_AI_MCH eMchSet, U16 nChNum, AudMchClkRef_e eMchClkRef, AudMchSel_e *mch_sel)
{
    U16 i                              = 0;
    S16 nClkRefSetting                 = -1;
    U16 nChSels[E_AUD_MCH_CH_BIND_NUM] = {0};
    int ret                            = AIO_OK;

    if (nChNum > 16)
    {
        ERRMSG("nChNum %d Fail !\n", nChNum);
        goto FAIL;
    }

    //
    switch (eMchClkRef)
    {
        case E_AUD_MCH_CLK_REF_DMIC:
        case E_AUD_MCH_CLK_REF_ADC:
        case E_AUD_MCH_CLK_REF_I2S_TDM_RX:
        case E_AUD_MCH_CLK_REF_SRC:
            break;
        default:
            ERRMSG("eMchClkRef %d Fail !\n", eMchClkRef);
            goto FAIL;
            break;
    }

    //
    for (i = E_AUD_MCH_CH_BIND_01; i < ARRAY_SIZE(nChSels); i++)
    {
        ret |= _HalAudDmaGetMchSelConfigValue(mch_sel[i], &nChSels[i], eMchSet);
        DBGMSG(AUDIO_DBG_LEVEL_PATH, "ai[%d] mch_sel[%d] select[%d]\n", eMchSet, i, nChSels[i]);
        // w1_read_vaild sample rate from the first channel that has been set.
        if (nClkRefSetting < 0 && mch_sel[i] != E_AUD_MCH_SEL_NULL)
        {
            nClkRefSetting = i;
        }
    }

    switch (eMchSet)
    {
        case E_CHIP_AI_MCH_A:
            HalBachWriteReg(
                E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_05_ADDR,
                AUDIO_BANK1_REG_DMA_SOURCE_CFG_05_REG_DMA_W1_CHEF_SEL_MASK,
                (nChSels[E_AUD_MCH_CH_BIND_EF] << AUDIO_BANK1_REG_DMA_SOURCE_CFG_05_REG_DMA_W1_CHEF_SEL_SHIFT));
            HalBachWriteReg(
                E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_05_ADDR,
                AUDIO_BANK1_REG_DMA_SOURCE_CFG_05_REG_DMA_W1_CHCD_SEL_MASK,
                (nChSels[E_AUD_MCH_CH_BIND_CD] << AUDIO_BANK1_REG_DMA_SOURCE_CFG_05_REG_DMA_W1_CHCD_SEL_SHIFT));
            HalBachWriteReg(
                E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_04_ADDR,
                AUDIO_BANK1_REG_DMA_SOURCE_CFG_04_REG_DMA_W1_CHAB_SEL_MASK,
                (nChSels[E_AUD_MCH_CH_BIND_AB] << AUDIO_BANK1_REG_DMA_SOURCE_CFG_04_REG_DMA_W1_CHAB_SEL_SHIFT));
            HalBachWriteReg(
                E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_04_ADDR,
                AUDIO_BANK1_REG_DMA_SOURCE_CFG_04_REG_DMA_W1_CH89_SEL_MASK,
                (nChSels[E_AUD_MCH_CH_BIND_89] << AUDIO_BANK1_REG_DMA_SOURCE_CFG_04_REG_DMA_W1_CH89_SEL_SHIFT));
            HalBachWriteReg(
                E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_03_ADDR,
                AUDIO_BANK1_REG_DMA_SOURCE_CFG_03_REG_DMA_W1_CH67_SEL_MASK,
                (nChSels[E_AUD_MCH_CH_BIND_67] << AUDIO_BANK1_REG_DMA_SOURCE_CFG_03_REG_DMA_W1_CH67_SEL_SHIFT));
            HalBachWriteReg(
                E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_03_ADDR,
                AUDIO_BANK1_REG_DMA_SOURCE_CFG_03_REG_DMA_W1_CH45_SEL_MASK,
                (nChSels[E_AUD_MCH_CH_BIND_45] << AUDIO_BANK1_REG_DMA_SOURCE_CFG_03_REG_DMA_W1_CH45_SEL_SHIFT));
            HalBachWriteReg(
                E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_ADDR,
                AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_REG_DMA_W1_CH23_SEL_MASK,
                (nChSels[E_AUD_MCH_CH_BIND_23] << AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_REG_DMA_W1_CH23_SEL_SHIFT));
            HalBachWriteReg(
                E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_ADDR,
                AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_REG_DMA_W1_CH01_SEL_MASK,
                (nChSels[E_AUD_MCH_CH_BIND_01] << AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_REG_DMA_W1_CH01_SEL_SHIFT));
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_ADDR,
                            AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_REG_DMA_W1_VALID_SEL_MASK,
                            (nClkRefSetting << AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_REG_DMA_W1_VALID_SEL_SHIFT));
            break;

        default:
            ERRMSG("eMchSet[%d] Fail !", eMchSet);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaSetRate(CHIP_AIO_DMA_e eAioDma, AudRate_e eRate)
{
    if (E_AUD_RATE_NULL == eRate)
    {
        goto FAIL;
    }

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            HalAudKeepDpgaGainCompatible(eRate);
            break;

        case E_CHIP_AIO_DMA_AO_A:
            g_AudDmaParam[E_CHIP_AIO_DMA_AO_A].nSampleRate = eRate;
            break;

        default:
            ERRMSG("eAioDma %d eRate %d Fail !", eAioDma, eRate);
            goto FAIL;
    }

    DBGMSG(AUDIO_DBG_LEVEL_DMA, "eAioDma %d, eRate %d.", eAioDma, eRate);

    return AIO_OK;

FAIL:
    ERRMSG("eAioDma %d, eRate %d.", eAioDma, eRate);
    return AIO_NG;
}

CHIP_AO_DMA_e HalAudDmaGetAoEnum(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh)
{
    CHIP_AO_DMA_e eAoDma;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            if (nDmaCh == 0)
            {
                eAoDma = E_CHIP_AO_DMA_AO_A_L;
            }
            else
            {
                eAoDma = E_CHIP_AO_DMA_AO_A_R;
            }
            break;

        case E_CHIP_AIO_DMA_AO_DIRECT_A:
            if (nDmaCh == 0)
            {
                eAoDma = E_CHIP_AO_DMA_DIRECT_A_L;
            }
            else
            {
                eAoDma = E_CHIP_AO_DMA_DIRECT_A_R;
            }
            break;

        default:
            eAoDma = E_CHIP_AO_DMA_TOTAL;
            ERRMSG("don't support this DMA[%d] !", eAioDma);
            break;
    }

    return eAoDma;
}

void HalAudDmaClearIrqStatus(CHIP_AIO_DMA_e eAioDma)
{
    U8            nAddr;
    BachRegBank_e eBank;
    U16           nMask;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_FREQ_DIFF_INT_TRIG_ADDR;
            nMask = AUDIO_BANK1_REG_FREQ_DIFF_INT_TRIG_REG_DMA_1_WR_TH_INT_CLR_MASK;
            break;
        case E_CHIP_AIO_DMA_AO_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = AUDIO_BANK1_REG_FREQ_DIFF_INT_TRIG_ADDR;
            nMask = AUDIO_BANK1_REG_FREQ_DIFF_INT_TRIG_REG_DMA_1_RD_TH_INT_CLR_MASK;
            break;
        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            return;
    }

    HalBachWriteReg(eBank, nAddr, nMask, nMask);
    HalBachWriteReg(eBank, nAddr, nMask, 0);
}

U32 HalAudDmaPrepareRestart(CHIP_AIO_DMA_e eAioDma)
{
    int ret = 0;

    ret |= HalAudDmaStop(eAioDma);
    if (ret != AIO_OK)
    {
        return AIO_NG;
    }

    return AIO_OK;
}

U32 HalAudDmaGetHwAddr(CHIP_AIO_DMA_e eAioDma)
{
    U16 nHwPtr;
    U16 nAddr = 0;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
        {
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_FREQ_DIFF_INT_TRIG_ADDR,
                            AUDIO_BANK1_REG_FREQ_DIFF_INT_TRIG_REG_DMA_1_AWADDR_OFFSET_TRIG_MASK,
                            AUDIO_BANK1_REG_FREQ_DIFF_INT_TRIG_REG_DMA_1_AWADDR_OFFSET_TRIG_MASK);
            _UDelay(1000);
            nHwPtr = HalBachReadReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_1_AWADDR_OFFSET_ADDR);
            nAddr  = HalBachReadReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_9_ADDR)
                    & AUDIO_BANK1_REG_DMA1_CTRL_9_REG_DMA1_WR_BASE_ADDR_LOW_MASK;
            nAddr |= (HalBachReadReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA1_CTRL_10_ADDR) & 0xf) << 12;
            nHwPtr -= nAddr;
            break;
        }
        case E_CHIP_AIO_DMA_AO_A:
        {
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_FREQ_DIFF_INT_TRIG_ADDR,
                            AUDIO_BANK1_REG_FREQ_DIFF_INT_TRIG_REG_DMA_1_ARADDR_OFFSET_TRIG_MASK,
                            AUDIO_BANK1_REG_FREQ_DIFF_INT_TRIG_REG_DMA_1_ARADDR_OFFSET_TRIG_MASK);
            _UDelay(1000);
            nHwPtr = HalBachReadReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_1_ARADDR_OFFSET_ADDR);
            break;
        }
        default:
            ERRMSG("dma[%d] Fail !", eAioDma);
            return 0;
    }
    return nHwPtr * MIU_WORD_BYTE_SIZE;
}

int HalAudDmaLocalDebug(CHIP_AIO_DMA_e eAioDma, U32 time)
{
#if SSTAR_FOR_FPGA_TEST
    if (eAioDma == E_CHIP_AIO_DMA_AO_A)
    {
        HalBachWriteReg2Byte(0x165128, 0xffff, 0x0003);
        HalBachWriteReg2Byte(0x165126, 0xffff, 0x0017);
        _UDelay(time);
        printk("_UDelay end\n");
        HalBachWriteReg2Byte(0x165128, 0xffff, 0x0000);
        HalBachWriteReg2Byte(0x165126, 0xffff, 0x0017);
    }
    if (eAioDma == E_CHIP_AIO_DMA_AI_A)
    {
        HalBachWriteReg2Byte(0x165188, 0xffff, 0x0003);
        HalBachWriteReg2Byte(0x165186, 0xffff, 0x0017);
        _UDelay(time);
        printk("_UDelay end\n");
        HalBachWriteReg2Byte(0x165188, 0xffff, 0x0000);
        HalBachWriteReg2Byte(0x165186, 0xffff, 0x0017);
    }
#else
    _UDelay(time);
    printk("_UDelay end\n");
#endif
    return AIO_OK;
}
