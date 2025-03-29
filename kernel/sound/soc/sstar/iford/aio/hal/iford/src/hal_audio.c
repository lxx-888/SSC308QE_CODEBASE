/*
 * hal_audio.c - Sigmastar
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

// linux
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/timer.h>

#include "cam_clkgen.h"
#include <drv_padmux.h>
#include <drv_puse.h>
#include <drv_gpio.h>
#include <drv_clock.h>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
extern DmaPar_t g_AudDmaParam[E_CHIP_AIO_DMA_TOTAL];
//------------------------------------------------------------------------------
static int _HalAudSysInit(void);
static int _HalAudSetAiPathCoreDmaA(U8 nDmaCh, AI_CH_e eAiCh);
static int _HalAudSetAiPathCoreDmaB(U8 nDmaCh, AI_CH_e eAiCh);

//------------------------------------------------------------------------------
void HalAudPllEnable(S8 parent_clk_index, S8 clk_index, BOOL bEn, S64 nFreq)
{
#ifdef __KERNEL__
    struct device_node *pDevNode;
    void *              parent_clk = NULL;
    struct clk *        clk;
    const char *        dtsCompatibleStr = NULL;

    dtsCompatibleStr = "sstar,audio";

    pDevNode = of_find_compatible_node(NULL, NULL, dtsCompatibleStr);

    if (!pDevNode)
    {
        ERRMSG("Get audio pDevNode fail!");
        return;
    }

    clk = of_clk_get(pDevNode, parent_clk_index);
    if (clk == NULL)
    {
        ERRMSG("Get audio clock[%d] fail!", parent_clk_index);
        return;
    }

    if (bEn)
    {
        if (clk_index >= 0)
        {
            parent_clk = sstar_clk_get_parent_by_index(clk, clk_index);
        }
        if (parent_clk != NULL)
        {
            sstar_clk_set_parent(clk, parent_clk);
        }
        if (!sstar_clk_is_enabled(clk))
        {
            sstar_clk_prepare_enable(clk);
        }
        if (nFreq >= 0)
        {
            sstar_clk_set_rate(parent_clk, nFreq);
        }
    }
    else
    {
        if (sstar_clk_is_enabled(clk))
        {
            if (clk_index >= 0)
            {
                parent_clk = sstar_clk_get_parent_by_index(clk, clk_index);
            }

            if (parent_clk != NULL)
            {
                sstar_clk_set_parent(clk, parent_clk);
                if (nFreq >= 0)
                {
                    sstar_clk_set_rate(parent_clk, nFreq);
                }
            }
            sstar_clk_disable_unprepare(clk);
        }
    }

    of_node_put(pDevNode);
    sstar_clk_put(clk);
    DBGMSG(AUDIO_DBG_LEVEL_POWER, "PLL [%d][%d] %s freq %ld\n", parent_clk_index, clk_index, bEn ? "ON" : "OFF", nFreq);
#endif
}

static void _HalAudPllInit(void)
{
    HalBachWriteRegByte(0x001038da, 0xff, 0x00);
    HalBachWriteRegByte(0x001038db, 0xff, 0x00);
#ifdef __KERNEL__
    // HalAudPllEnable(E_CLK_SRC_384M, -1, TRUE, -1);
    // HalAudPllEnable(E_CLK_SRC_48M, 0, TRUE, FREQ_48MHZ);
    // HalAudPllEnable(E_CLK_SRC_432M, 0, TRUE, -1);
#else
    HalBachWriteRegByte(0x00103fd0, 0xff, 0x00);
    HalBachWriteRegByte(0x00103fd1, 0xff, 0x00);
    HalBachWriteRegByte(0x00141d00, 0xff, 0x00);
    HalBachWriteRegByte(0x00141d0e, 0xff, 0x11);

    HalBachWriteRegByte(0x00141d16, 0xff, 0x9a);
    HalBachWriteRegByte(0x00141d17, 0xff, 0x99);
    HalBachWriteRegByte(0x00141d18, 0xff, 0x15);
    HalBachWriteRegByte(0x00141d0e, 0xff, 0x01);
    HalBachWriteRegByte(0x00141d02, 0xff, 0x98);
    HalBachWriteRegByte(0x00141d03, 0xff, 0x03);

    HalBachWriteRegByte(0x00150600, 0xff, 0x80);
    HalBachWriteRegByte(0x00150601, 0xff, 0x10);
    HalBachWriteRegByte(0x00150602, 0xff, 0x88);
    HalBachWriteRegByte(0x00150603, 0xff, 0x06);
    HalBachWriteRegByte(0x0015060e, 0xff, 0x11);
    HalBachWriteRegByte(0x00150610, 0xff, 0xc1);
    HalBachWriteRegByte(0x00150611, 0xff, 0x00);
    HalBachWriteRegByte(0x00150616, 0xff, 0x00);
    HalBachWriteRegByte(0x00150617, 0xff, 0x5e);
    HalBachWriteRegByte(0x00150618, 0xff, 0x1a);
    HalBachWriteRegByte(0x00150619, 0xff, 0x00);
    HalBachWriteRegByte(0x0015060e, 0xff, 0x01);
#endif
    _UDelay(100);
}

/* DMIC Hpf value table:
 *   Register : 0x15039E
 *   Level   | Cut-off Freq | Value |
 *   --------------------------------
 *   Disable |     <20Hz    | 0x30  |
 *      0    |     20Hz     | 0x17  |
 *      1    |     23Hz     | 0x07  |
 *      2    |     26Hz     | 0x16  |
 *      3    |     32Hz     | 0x06  |
 *      4    |     38Hz     | 0x15  |
 *      5    |     52Hz     | 0x05  |
 *      6    |     65Hz     | 0x14  |
 *      7    |     95Hz     | 0x04  |
 *      8    |     140Hz    | 0x13  |
 *      9    |     190Hz    | 0x03  |
 *      A    |     250Hz    | 0x12  |
 *      B    |     370Hz    | 0x02  |
 *      C    |     480Hz    | 0x11  |
 *      D    |     700Hz    | 0x01  |
 *      E    |     950Hz    | 0x10  |
 *      F    |     15000Hz  | 0x00  |
 */
BOOL HalAudSetHpf(AudHpfDev_e eHfpDev, BOOL bEnable, U8 level)
{
    U16 bank, nAddr, nRegMsk, nPos, nVal;

    if (level > 0xf)
    {
        level = 0xf;
    }

    // Set Hpf value
    switch (eHfpDev)
    {
        case E_AUD_HPF_ADC1:
            bank    = E_BACH_REG_BANK4;
            nAddr   = AUDIO_BANK4_REG_MUX1_SEL_ADDR;
            nRegMsk = AUDIO_BANK4_REG_MUX1_SEL_REG_ADC_HPF_N_MASK;
            nPos    = AUDIO_BANK4_REG_MUX1_SEL_REG_ADC_HPF_N_SHIFT;
            break;
        case E_AUD_HPF_DMIC:
            bank    = E_BACH_REG_BANK3;
            nAddr   = AUDIO_BANK3_REG_VREC_HPF_CFG_00_ADDR;
            nRegMsk = AUDIO_BANK3_REG_VREC_HPF_CFG_00_REG_VREC_HPF_N_MASK;
            nPos    = AUDIO_BANK3_REG_VREC_HPF_CFG_00_REG_VREC_HPF_N_SHIFT;
            break;
        default:
            ERRMSG("hpfdev[%d] max[%d] error !", eHfpDev, E_AUD_HPF_DEV_NUM - 1);
            return FALSE;
    }
    if (!bEnable)
    {
        nVal = 0x00;
    }
    else
    {
        nVal = (0xf - level) / 2;
    }
    HalBachWriteReg(bank, nAddr, nRegMsk, nVal << nPos);

    // Enable Hpf
    switch (eHfpDev)
    {
        case E_AUD_HPF_ADC1:
            bank  = E_BACH_REG_BANK4;
            nAddr = AUDIO_BANK4_REG_ENABLE_CTRL_ADDR;
            nRegMsk =
                AUDIO_BANK4_REG_ENABLE_CTRL_REG_BP_ADC_HPF2_MASK | AUDIO_BANK4_REG_ENABLE_CTRL_REG_BP_ADC_HPF1_MASK;
            nVal = AUDIO_BANK4_REG_ENABLE_CTRL_REG_BP_ADC_HPF2_MASK;
            break;
        case E_AUD_HPF_DMIC:
            bank    = E_BACH_REG_BANK3;
            nAddr   = AUDIO_BANK3_REG_VREC_HPF_CFG_00_ADDR;
            nRegMsk = AUDIO_BANK3_REG_VREC_HPF_CFG_00_REG_BP_VREC_HPF2_MASK
                      | AUDIO_BANK3_REG_VREC_HPF_CFG_00_REG_BP_VREC_HPF1_MASK;
            nVal = AUDIO_BANK3_REG_VREC_HPF_CFG_00_REG_BP_VREC_HPF1_MASK;
            break;
        default:
            ERRMSG("hpfdev[%d] max[%d] error !", eHfpDev, E_AUD_HPF_DEV_NUM - 1);
            return FALSE;
    }
    if (!bEnable)
    {
        nVal = nRegMsk;
    }
    else if ((level % 2) != 0)
    {
        nVal = 0;
    }

    HalBachWriteReg(bank, nAddr, nRegMsk, nVal);

    return TRUE;
}

static void _HalAudAuReset(void)
{
    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR1_SEL_ADDR, AUDIO_BANK4_REG_SR1_SEL_REG_RESET_AU_MASK,
                    AUDIO_BANK4_REG_SR1_SEL_REG_RESET_AU_MASK);
    _UDelay(1);
    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR1_SEL_ADDR, AUDIO_BANK4_REG_SR1_SEL_REG_RESET_AU_MASK, 0);
}

static int _HalAudSysInit(void)
{
    HalAudAtopInit();

    // Linear interpolation clock gate force on
    // initial sram, need delay
    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_ENABLE_CTRL_ADDR,
                    AUDIO_BANK4_REG_ENABLE_CTRL_REG_INIT_SRAM_EN_MASK,
                    AUDIO_BANK4_REG_ENABLE_CTRL_REG_INIT_SRAM_EN_MASK);
    _UDelay(20);
    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_ENABLE_CTRL_ADDR,
                    AUDIO_BANK4_REG_ENABLE_CTRL_REG_INIT_SRAM_EN_MASK, 0);

    // AU reset
    _HalAudAuReset();

    // support write 8bit lenth
    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_BYTE_WR_EN_MODE_ADDR,
                    AUDIO_BANK4_REG_BYTE_WR_EN_MODE_BYTE_WR_EN_MODE_MASK,
                    AUDIO_BANK4_REG_BYTE_WR_EN_MODE_BYTE_WR_EN_MODE_MASK);

    // adc hpf number, set from dtsi
    HalAudSetHpf(E_AUD_HPF_ADC1, HalAudApiGetDtsValues(HPF_ADC1_LEVEL, 0) /* enable */,
                 HalAudApiGetDtsValues(HPF_ADC1_LEVEL, 1) /* value */);

    // enable I2S TRX BCK which gate in lpl
    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_CKG_TEST_MD_ADDR,
                    AUDIO_BANK4_REG_CKG_TEST_MD_REG_TEST_CLK_SEL_MASK, 0);

    // adc out sel
    HalBachWriteReg(
        E_BACH_REG_BANK4, AUDIO_BANK4_REG_ADC_OUT_SEL_ADDR, AUDIO_BANK4_REG_ADC_OUT_SEL_REG_ADC_OUT_1_SEL_MASK,
        (HalAudApiGetDtsValues(ADC_OUT_SEL, 0) ?: 0) << AUDIO_BANK4_REG_ADC_OUT_SEL_REG_ADC_OUT_1_SEL_SHIFT);
    HalBachWriteReg(
        E_BACH_REG_BANK4, AUDIO_BANK4_REG_ADC_OUT_SEL_ADDR, AUDIO_BANK4_REG_ADC_OUT_SEL_REG_ADC_OUT_2_SEL_MASK,
        (HalAudApiGetDtsValues(ADC_OUT_SEL, 1) ?: 1) << AUDIO_BANK4_REG_ADC_OUT_SEL_REG_ADC_OUT_2_SEL_SHIFT);

    HalAudI2sSetBckClkSrc(E_CHIP_AIO_I2S_TX_A, E_CLK_SRC_384M);
    HalAudI2sSetBckClkSrc(E_CHIP_AIO_I2S_RX_A, E_CLK_SRC_384M);

    HalAudI2sSetMckClkSrc(E_CHIP_AIO_I2S_TX_A, E_CLK_SRC_384M);

    return AIO_OK;
}

static int _HalAudSetAiPathCoreDmaA(U8 nDmaCh, AI_CH_e eAiCh)
{
    int ret  = AIO_OK;
    int line = 0;

    //
    // nDmaCh may be used in new chip (P4)

    switch (eAiCh)
    {
        case E_AI_CH_ADC_A_0:
        case E_AI_CH_ADC_B_0:
        case E_AI_CH_ADC_C_0:
        case E_AI_CH_ADC_D_0:
        case E_AI_CH_DMIC_A_0:
        case E_AI_CH_DMIC_A_1:
        case E_AI_CH_DMIC_A_2:
        case E_AI_CH_DMIC_A_3:
        case E_AI_CH_DMIC_A_4:
        case E_AI_CH_DMIC_A_5:
        case E_AI_CH_DMIC_A_6:
        case E_AI_CH_DMIC_A_7:
        case E_AI_CH_ECHO_A_0:
        case E_AI_CH_ECHO_A_1:

            ret |= HalAudSetMux(E_AUD_MUX_DMAWR1_MCH, 1);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AI_CH_I2S_RX_A_0:
        case E_AI_CH_I2S_RX_A_1:
        case E_AI_CH_I2S_RX_A_2:
        case E_AI_CH_I2S_RX_A_3:
        case E_AI_CH_I2S_RX_A_4:
        case E_AI_CH_I2S_RX_A_5:
        case E_AI_CH_I2S_RX_A_6:
        case E_AI_CH_I2S_RX_A_7:

            ret |= HalAudSetMux(E_AUD_MUX_DMAWR1_MCH, 1); // 150396[1:0] 1:selec DMA1 mch 0:select ADC
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            ret |= HalAudSetMux(E_AUD_MUX_I2STDM_RX, 1); // 150378[0] 1: I2STDM 0:Ext I2S
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AI_CH_I2S_RX_B_0:
        case E_AI_CH_I2S_RX_B_1:
        case E_AI_CH_I2S_RX_B_2:
        case E_AI_CH_I2S_RX_B_3:
        case E_AI_CH_I2S_RX_B_4:
        case E_AI_CH_I2S_RX_B_5:
        case E_AI_CH_I2S_RX_B_6:
        case E_AI_CH_I2S_RX_B_7:

            ret |= HalAudSetMux(E_AUD_MUX_DMAWR1_MCH, 1); // 150396[1:0] 1:selec DMA1 mch 0:select ADC
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            ret |= HalAudSetMux(E_AUD_MUX_I2STDM1_RX, 1); // 15034c[0] 1: I2STDM 0:Ext I2S
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            break;
        case E_AI_CH_I2S_RX_C_0:
        case E_AI_CH_I2S_RX_C_1:
            ret |= HalAudSetMux(E_AUD_MUX_DMAWR1_MCH, 1); // 150396[1:0] 1:selec DMA1 mch 0:select ADC
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            ret |= HalAudSetMux(E_AUD_MUX_I2STDM2_RX, 1); // 15034e[0] 1: I2STDM 0:Ext I2S
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            break;
        case E_AI_CH_SPDIF_RX_A_0:
        case E_AI_CH_SPDIF_RX_A_1:
            ret |= HalAudSetMux(E_AUD_MUX_DMAWR1_MCH, 1); // 150396[1:0] 1:selec DMA1 mch 0:select ADC
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            HalAudSpdifSetPadmux(E_CHIP_AI_SPDIF_A);
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("eAiCh[%d] Fail !", line, eAiCh);

    return AIO_NG;
}

static int _HalAudSetAiPathCoreDmaB(U8 nDmaCh, AI_CH_e eAiCh)
{
    int ret  = AIO_OK;
    int line = 0;

    switch (eAiCh)
    {
        case E_AI_CH_ADC_A_0:
        case E_AI_CH_ADC_B_0:
        case E_AI_CH_ADC_C_0:
        case E_AI_CH_ADC_D_0:
        case E_AI_CH_DMIC_A_0:
        case E_AI_CH_DMIC_A_1:
        case E_AI_CH_DMIC_A_2:
        case E_AI_CH_DMIC_A_3:
        case E_AI_CH_DMIC_A_4:
        case E_AI_CH_DMIC_A_5:
        case E_AI_CH_DMIC_A_6:
        case E_AI_CH_DMIC_A_7:
        case E_AI_CH_ECHO_A_0:
        case E_AI_CH_ECHO_A_1:
            ret |= HalAudSetMux(E_AUD_MUX_DMAWR2_MCH, 1); // 0x150398[1] 1:selec DMA2 mch 0:selec ADC
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AI_CH_I2S_RX_A_0:
        case E_AI_CH_I2S_RX_A_1:
        case E_AI_CH_I2S_RX_A_2:
        case E_AI_CH_I2S_RX_A_3:
        case E_AI_CH_I2S_RX_A_4:
        case E_AI_CH_I2S_RX_A_5:
        case E_AI_CH_I2S_RX_A_6:
        case E_AI_CH_I2S_RX_A_7:
            ret |= HalAudSetMux(E_AUD_MUX_DMAWR2_MCH, 1); // 0x150398[1] 1:selec DMA2 mch 0:selec ADC 3:HDMI RX
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_I2STDM_RX, 1); // 150378[0] 1: I2STDM 0:Ext I2S
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AI_CH_I2S_RX_B_0:
        case E_AI_CH_I2S_RX_B_1:
        case E_AI_CH_I2S_RX_B_2:
        case E_AI_CH_I2S_RX_B_3:
        case E_AI_CH_I2S_RX_B_4:
        case E_AI_CH_I2S_RX_B_5:
        case E_AI_CH_I2S_RX_B_6:
        case E_AI_CH_I2S_RX_B_7:
            ret |= HalAudSetMux(E_AUD_MUX_DMAWR2_MCH, 1); // 0x150398[1] 1:selec DMA2 mch 0:selec ADC 3:HDMI RX
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_I2STDM1_RX, 1); // 150378[0] 1: I2STDM 0:Ext I2S
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AI_CH_I2S_RX_C_0:
        case E_AI_CH_I2S_RX_C_1:
            ret |= HalAudSetMux(E_AUD_MUX_DMAWR2_MCH, 1); // 0x150398[1] 1:selec DMA2 mch 0:selec ADC 3:HDMI RX
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_I2STDM2_RX, 1); // 150378[0] 1: I2STDM 0:Ext I2S
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("Line:%d eAiCh[%d] Fail !", line, eAiCh);

    return AIO_NG;
}

void HalAudKeepDpgaGainCompatible(AudRate_e eRate)
{
    // SRC:
    // 48K: 0dB
    // Others: +1.5dB
    // after p5 step 0.125
    if (eRate == E_AUD_RATE_48K)
    {
        // SRC1
        HalAudDpgaSetGainOffset(E_CHIP_DPGA_E, 0, 0);
        HalAudDpgaSetGainOffset(E_CHIP_DPGA_E, 0, 1);
    }
    else
    {
        HalAudDpgaSetGainOffset(E_CHIP_DPGA_E, 0, 0);
        HalAudDpgaSetGainOffset(E_CHIP_DPGA_E, 0, 1);
        // SRC1
        // HalAudDpgaSetGainOffset(E_CHIP_DPGA_E, -12, 0);
        // HalAudDpgaSetGainOffset(E_CHIP_DPGA_E, -12, 1);
    }
}

//------------------------------------------------------------------------------
static void _HalHwBitMaskEnable(void)
{
    U16 nConfigValue;

    nConfigValue = HalBachReadReg2Byte(0x10012E);
    if ((nConfigValue & 0x1) == 0)
    {
        nConfigValue |= 0x1;
        HalBachWriteReg2Byte(0x10012E, 0xFFFF, nConfigValue);
    }
}

int HalAudMainInit(void)
{
    int ret = AIO_OK;
    // Get DTS info
    ret |= HalAudApiDtsInit();
    if (ret != AIO_OK)
    {
        goto FAIL;
    }
    HalBachSysSetBankBaseAddr(IO_ADDRESS(0x1F000000));
    _HalAudPllInit();
    _HalAudSysInit();
    _HalHwBitMaskEnable();
    HalAudDmicInit();
    HalAudDmaInit();

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudMainDeInit(void)
{
    HalAudAtopDeInit();
    HalAudPllEnable(E_CLK_SRC_384M, -1, FALSE, -1);
    HalAudPllEnable(E_CLK_SRC_48M, 0, FALSE, 0);
    HalAudPllEnable(E_CLK_SRC_432M, 0, FALSE, -1);
    HalAudDmaDeInit();

    return AIO_OK;
}

int HalAudSetAiPath(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh)
{
    int ret = AIO_OK;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            ret |= _HalAudSetAiPathCoreDmaA(nDmaCh, eAiCh);
            break;

        case E_CHIP_AIO_DMA_AI_B:
            ret |= _HalAudSetAiPathCoreDmaB(nDmaCh, eAiCh);
            break;

        default:
            ERRMSG("eAioDma[%d] Fail !", eAioDma);
            goto FAIL;
            break;
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

int HalAudSetDirectPath(AI_CH_e eAiCh, AO_CH_e eAoCh)
{
    int ret  = AIO_OK;
    int line = 0;

    switch (eAiCh)
    {
        case E_AI_CH_ADC_A_0:
        case E_AI_CH_ADC_B_0:
            switch (eAoCh)
            {
                case E_AO_CH_DAC_A_0:
                case E_AO_CH_DAC_B_0:
                    HalAudSetMux(E_AUD_MUX_MMC1_DIN_L_SEL, MUX_SEL_ADC_0); // AB1L ADC0
                    HalAudSetMux(E_AUD_MUX_MMC1_DIN_R_SEL, MUX_SEL_ADC_1); // AB1R ADC1
                    break;
                default:
                    line = __LINE__;
                    goto FAIL;
                    break;
            }
            break;
        case E_AI_CH_DMIC_A_0:
        case E_AI_CH_DMIC_A_1:
            switch (eAoCh)
            {
                case E_AO_CH_DAC_A_0:
                case E_AO_CH_DAC_B_0:
                    HalAudSetMux(E_AUD_MUX_MMC1_DIN_L_SEL, MUX_SEL_DMIC_0); // AB1L DMIC0
                    break;
                default:
                    line = __LINE__;
                    goto FAIL;
                    break;
            }
            break;
        case E_AI_CH_I2S_RX_A_0:
        case E_AI_CH_I2S_RX_A_1:

            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_ADDR,
                            AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_I2S_TDM_RX_AFIFO_READ_VALID_SEL_MASK,
                            (2 << AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_I2S_TDM_RX_AFIFO_READ_VALID_SEL_SHIFT));
            ret |= HalAudSetMux(E_AUD_MUX_I2STDM_RX, 1); // 150378[0] 1: I2STDM 0:Ext I2S
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            switch (eAoCh)
            {
                case E_AO_CH_DAC_A_0:
                case E_AO_CH_DAC_B_0:
                    HalAudSetMux(E_AUD_MUX_MMC1_DIN_L_SEL, MUX_SEL_I2S0_RX0); // AB1L I2S_RX0
                    HalAudSetMux(E_AUD_MUX_MMC1_DIN_R_SEL, MUX_SEL_I2S0_RX1); // AB1R I2S_RX1
                    break;
                default:
                    line = __LINE__;
                    goto FAIL;
                    break;
            }
            break;
        default:
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("eAiCh %d eAoCh %d Fail !\n", eAiCh, eAoCh);
    return AIO_NG;
}

int HalAudSetMux(AudMux_e eMux, U8 nChoice)
{
    switch (eMux)
    {
        case E_AUD_MUX_MMC1:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_MUX0_SEL_ADDR,
                            AUDIO_BANK4_REG_MUX0_SEL_REG_MMC1_MMC3_SEL_MASK,
                            (nChoice ? AUDIO_BANK4_REG_MUX0_SEL_REG_MMC1_MMC3_SEL_MASK : 0));
            break;

        case E_AUD_MUX_DMAWR1_MCH:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_ADDR,
                            AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_REG_DMA_W1_NEW_MODE_MASK,
                            nChoice << AUDIO_BANK1_REG_DMA_SOURCE_CFG_00_REG_DMA_W1_NEW_MODE_SHIFT);
            break;

        case E_AUD_MUX_I2S_CFG_00:
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_01_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_01_REG_I2S_TDM_TX_DATA_SEL_MASK,
                            nChoice << AUDIO_BANK2_REG_I2S_TDM_CFG_01_REG_I2S_TDM_TX_DATA_SEL_SHIFT);
            break;

        case E_AUD_MUX_I2STDM_RX:
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_CFG_00_ADDR,
                            AUDIO_BANK2_REG_I2S_CFG_00_REG_I2S_PAD_SEL_MASK,
                            (nChoice ? AUDIO_BANK2_REG_I2S_CFG_00_REG_I2S_PAD_SEL_MASK : 0));
            break;

        case E_AUD_MUX_TDM0_TX:
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_CFG_00_ADDR,
                            AUDIO_BANK2_REG_I2S_CFG_00_REG_I2S_TDM_TX_VALID_SEL_MASK,
                            (nChoice ? AUDIO_BANK2_REG_I2S_CFG_00_REG_I2S_TDM_TX_VALID_SEL_MASK : 0));
            break;

        case E_AUD_MUX_I2S_1_CFG_00:
            break;

        case E_AUD_MUX_SDM_DINL:
            break;

        case E_AUD_MUX_SDM_DINR:
            break;

        case E_AUD_MUX_TDM1_TX:
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_CFG_00_ADDR,
                            AUDIO_BANK2_REG_I2S_CFG_00_REG_I2S_TDM_TX_VALID_SEL_MASK,
                            (nChoice ? AUDIO_BANK2_REG_I2S_CFG_00_REG_I2S_TDM_TX_VALID_SEL_MASK : 0));
            break;

        case E_AUD_MUX_I2S_TDM_TX0_DATA_SEL:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_ADDR,
                            AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_DMA1_READ_VALID_I2STX_SEL_MASK,
                            nChoice << AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_DMA1_READ_VALID_I2STX_SEL_SHIFT);
            break;

        case E_AUD_MUX_MMC1_DIN_L_SEL:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_MMC1_MUX_ADDR,
                            AUDIO_BANK4_REG_MMC1_MUX_REG_MMC1_L_MUX_MASK,
                            nChoice << AUDIO_BANK4_REG_MMC1_MUX_REG_MMC1_L_MUX_SHIFT);
            DBGMSG(AUDIO_DBG_LEVEL_PATH, "E_AUD_MUX_MMC1_DIN_L_SEL nChoice[%d]", nChoice);
            break;

        case E_AUD_MUX_MMC1_DIN_R_SEL:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_MMC1_MUX_ADDR,
                            AUDIO_BANK4_REG_MMC1_MUX_REG_MMC1_R_MUX_MASK,
                            nChoice << AUDIO_BANK4_REG_MMC1_MUX_REG_MMC1_R_MUX_SHIFT);
            break;

        default:
            ERRMSG("eMux[%d] Fail !", eMux);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    return AIO_NG;
}

void HalAudDirectDmaRateSet(int nSampleRate)
{
    switch (nSampleRate)
    {
        case 8000:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            0 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case 11000:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            1 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case 12000:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            2 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case 16000:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            3 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case 22000:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            4 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case 24000:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            5 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case 32000:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            6 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case 44000:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            7 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case 48000:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            8 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        default:
            ERRMSG("direct DMA eRate[%d] Fail !", nSampleRate);
            break;
    }
}

int HalAudAiSetClkRefAndMch(AI_ATTACH_t *aiAttach)
{
    int            i     = 0;
    int            ret   = AIO_OK;
    BOOL           valid = FALSE;
    AI_DMA_e       aiDma = 0;
    AI_IF_e        aiIf  = 0;
    EN_CHIP_AI_MCH eMchSet;
    U16            nChNum                              = 0;
    AudMchClkRef_e eMchClkRef                          = E_AUD_MCH_CLK_REF_NONE;
    AudMchSel_e    mchSel[E_MHAL_AI_DMA_CH_SLOT_TOTAL] = {0};

    //
    aiDma = aiAttach->enAiDma;

    //
    switch (aiDma)
    {
        case E_MHAL_AI_DMA_A:
            eMchSet = E_CHIP_AI_MCH_A;
            break;

        case E_MHAL_AI_DMA_B:
            eMchSet = E_CHIP_AI_MCH_B;
            break;

        default:
            ERRMSG("aiDma[%d] Fail !", aiDma);
            goto FAIL;
            break;
    }

    //
    for (i = 0; i < E_MHAL_AI_DMA_CH_SLOT_TOTAL; i++)
    {
        aiIf      = aiAttach->eAiIf[i];
        valid     = TRUE;
        mchSel[i] = E_AUD_MCH_SEL_NULL;

        DBGMSG(AUDIO_DBG_LEVEL_PATH, "aiAttach->eAiIf[%d] = %d\n", i, aiIf);

        if (aiIf != E_MHAL_AI_IF_NONE)
        {
            switch (aiIf)
            {
                case E_MHAL_AI_IF_ADC_A_0_B_0:
                    mchSel[i] = E_AUD_MCH_SEL_AMIC01;
                    if (eMchClkRef
                        != E_AUD_MCH_CLK_REF_I2S_TDM_RX) // If DMA has attached I2S, just using I2S ref clock.
                    {
                        eMchClkRef = E_AUD_MCH_CLK_REF_ADC;
                    }
                    break;

                case E_MHAL_AI_IF_DMIC_A_0_1:
                    mchSel[i] = E_AUD_MCH_SEL_DMIC01;
                    if (eMchClkRef
                        != E_AUD_MCH_CLK_REF_I2S_TDM_RX) // If DMA has attached I2S, just using I2S ref clock.
                    {
                        eMchClkRef = E_AUD_MCH_CLK_REF_DMIC;
                    }
                    break;

                case E_MHAL_AI_IF_ECHO_A_0_1:
                    mchSel[i] = E_AUD_MCH_SEL_SRC01;
                    if (eMchClkRef
                        != E_AUD_MCH_CLK_REF_I2S_TDM_RX) // If DMA has attached I2S, just using I2S ref clock.
                    {
                        eMchClkRef = E_AUD_MCH_CLK_REF_SRC;
                    }
                    break;

                case E_MHAL_AI_IF_I2S_RX_A_0_1:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_A_RX01;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    break;

                case E_MHAL_AI_IF_I2S_RX_A_2_3:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_A_RX23;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    break;

                case E_MHAL_AI_IF_I2S_RX_A_4_5:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_A_RX45;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    break;

                case E_MHAL_AI_IF_I2S_RX_A_6_7:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_A_RX67;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    break;

                default:
                    valid = FALSE;
                    break;
            }

            //
            if (valid == TRUE)
            {
                nChNum = (i + 1) * AI_DMA_CH_NUM_PER_SLOT;
            }
            else
            {
                ERRMSG("aiIf[%d] Fail !", aiIf);
                goto FAIL;
            }
        }
    }

    //
    ret |= HalAudDmaWrConfigMchCore(eMchSet, nChNum, eMchClkRef, mchSel);
    if (ret != AIO_OK)
    {
        ERRMSG("aiDma[%d] Fail !", aiDma);
        goto FAIL;
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudSetAudioDelay(CHIP_AIO_DMA_e eAioDma, BOOL bCtl)
{
    return AIO_OK;
}

int HalAudAoSetClkUnRef(AO_ATTACH_t *aoAttach)
{
    int           i           = 0;
    AO_DMA_e      aoDma       = 0;
    AO_IF_e       aoIf        = 0;
    AudAoClkRef_e eAoClkUnRef = E_AUD_AO_CLK_REF_NONE;

    //
    aoDma = aoAttach->enAoDma;

    // Default
    eAoClkUnRef = E_AUD_AO_CLK_REF_A;

    for (i = 0; i < E_MHAL_AO_DMA_CH_SLOT_TOTAL; i++)
    {
        aoIf = aoAttach->eAoIf[i];
        if ((aoIf & E_MHAL_AO_IF_I2S_TX_A_0) || (aoIf & E_MHAL_AO_IF_I2S_TX_A_1))
        {
            eAoClkUnRef = E_AUD_AO_CLK_REF_I2S_TDM_TX_A;
        }
    }
    // Just I2S Clk ref need to un ref clk
    if (eAoClkUnRef == E_AUD_AO_CLK_REF_I2S_TDM_TX_A)
    {
        switch (aoDma)
        {
            case E_MHAL_AO_DMA_A:
                HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_ADDR,
                                AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_DMA1_READ_VALID_SEL_MASK,
                                (0 ? AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_DMA1_READ_VALID_SEL_MASK : 0));
                break;

            case E_MHAL_AO_DMA_DIRECT_A:
                break;

            case E_MHAL_AO_DMA_DIRECT_B:
                break;

            default:

                ERRMSG("aoDma[%d] Fail !", aoDma);
                goto FAIL;
                break;
        }
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDpgaSetGainOffset(EN_CHIP_DPGA eDpga, S8 gainRegValue, S8 ch)
{
    U8            nAddr = AUDIO_BANK2_REG_MMC1_DPGA1_CFG2_ADDR;
    U8            nGainIdx;
    U16           nRegMsk;
    S8            nGain;
    U8            nPos;
    BachRegBank_e nBank;

    //
    // 5bits: 15 ~ -16 (0xF ~ 0x10)
    // -7.875dB ~ 8dB

    nGain = gainRegValue;
    if (nGain > 63)
    {
        nGain = 63;
    }
    else if (nGain < -64)
    {
        nGain = -64;
    }

    nGainIdx = (U8)nGain;

    switch (eDpga)
    {
        case E_CHIP_DPGA_A:
            nBank = E_BACH_REG_BANK2;
            nAddr = AUDIO_BANK2_REG_MMC1_DPGA1_CFG2_ADDR;
            break;
        case E_CHIP_DPGA_E:
            nBank = E_BACH_REG_BANK2;
            nAddr = AUDIO_BANK2_REG_MMCDEC1_DPGA_CFG3_ADDR;
            break;

        default:
            ERRMSG("eDpga[%d] Fail !", eDpga);
            goto FAIL;
            break;
    }
    if (ch == 0)
    {
        nRegMsk = AUDIO_BANK2_REG_MMC1_DPGA1_CFG2_MMC1_1_REG_OFFSET_L_MASK;
        nPos    = AUDIO_BANK2_REG_MMC1_DPGA1_CFG2_MMC1_1_REG_OFFSET_L_SHIFT;
    }
    else if (ch == 1)
    {
        nRegMsk = AUDIO_BANK2_REG_MMC1_DPGA1_CFG2_MMC1_1_REG_OFFSET_R_MASK;
        nPos    = AUDIO_BANK2_REG_MMC1_DPGA1_CFG2_MMC1_1_REG_OFFSET_R_SHIFT;
    }
    else
    {
        ERRMSG("eDpga[%d] ch[%d] Fail !", eDpga, ch);
        goto FAIL;
    }

    HalBachWriteReg(nBank, nAddr, nRegMsk, nGainIdx << nPos);

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudDpgaSetGainFading(EN_CHIP_DPGA eDpga, U8 nFading, S8 ch)
{
    U8            nAddr = AUDIO_BANK2_REG_MMC1_DPGA1_CFG1_ADDR;
    U16           nRegMsk, nValue;
    U8            nFadingEn = 1, nFadingValue = 0;
    BachRegBank_e nBank;

    // 0:OFF    1~7:reg value 0~6
    if (nFading > 7)
    {
        ERRMSG("eDpga[%d] nFading[%d] ch[%d] Fail !", eDpga, nFading, ch);
        goto FAIL;
    }

    if (nFading == 0) // OFF
    {
        nFadingEn    = 0;
        nFadingValue = 0;
    }
    else
    {
        nFadingEn    = 1;
        nFadingValue = nFading - 1;
    }

    switch (eDpga)
    {
        case E_CHIP_DPGA_A:
            nBank   = E_BACH_REG_BANK2;
            nAddr   = AUDIO_BANK2_REG_MMC1_DPGA1_CFG1_ADDR;
            nRegMsk = (STEP_MSK | AUDIO_BANK2_REG_MMC1_DPGA1_CFG0_MMC1_1_FADING_EN_MASK);

            break;

        case E_CHIP_DPGA_G:
            nBank   = E_BACH_REG_BANK2;
            nAddr   = AUDIO_BANK2_REG_I2S_TDM_DPGA_CFG2_ADDR;
            nRegMsk = (STEP_MSK | AUDIO_BANK2_REG_I2S_TDM_DPGA_CFG1_REG_I2S_TDM_FADING_EN_MASK);

            break;

        case E_CHIP_DPGA_E:
        case E_CHIP_DPGA_F:
            goto SUCCESS;
            break;
        default:
            ERRMSG("eDpga[%d] unable to find this DPGA !", eDpga);
            goto FAIL;
            break;
    }

    nValue = HalBachReadReg(nBank, nAddr);

    if ((ch == 0) || (ch == 1))
    {
        nValue = nValue & (~nRegMsk);
        nValue = nValue | (nFadingValue << STEP_POS) | (nFadingEn << 1);
    }
    else
    {
        ERRMSG("eDpga[%d] nFading[%d] ch[%d] Fail !", eDpga, nFading, ch);
        goto FAIL;
    }

    HalBachWriteReg(nBank, nAddr, nRegMsk, nValue);
SUCCESS:
    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudAiSetGain(AI_CH_e eAiCh, S16 s16Gain)
{
    int line = 0;

    //
    if ((s16Gain >= CHIP_ADC_GAIN_STEP_TOTAL) || (s16Gain < 0))
    {
        ERRMSG("eAiCh[%d] s16Gain[%d] Fail !", eAiCh, s16Gain);
        goto FAIL;
    }

    //
    switch (eAiCh)
    {
        case E_AI_CH_ADC_A_0:
        case E_AI_CH_ADC_B_0:
        case E_AI_CH_ADC_C_0:
        case E_AI_CH_ADC_D_0:
            HalAudAtopAiSetGain(eAiCh, s16Gain);
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("Line[%d] eAiCh[%d] Gain[%d] fail", line, eAiCh, s16Gain);

    return AIO_NG;
}

int HalAudAoSetGain(AO_CH_e eAoCh, S16 s16Gain)
{
    return AIO_OK;
}

int HalAudAoSetIfSampleRate(AO_IF_e eAoIf, u32 nSampleRate)
{
    int       ret   = AIO_OK;
    AudRate_e eRate = 0;

    eRate = HalAudApiRateToEnum(nSampleRate);

    if (nSampleRate > 48000 || eRate == E_AUD_RATE_NULL)
    {
        ERRMSG("eAoIf[%d] nSampleRate[%d] Fail !", eAoIf, nSampleRate);
        goto FAIL;
    }

    //
    if ((eAoIf & E_MHAL_AO_IF_DAC_A_0) || (eAoIf & E_MHAL_AO_IF_DAC_B_0))
    {
        ret |= HalAudDmaSetRate(E_CHIP_AIO_DMA_AO_A, eRate);
        if (ret != AIO_OK)
        {
            ERRMSG("eAoIf[%d] eRate[%d] Fail !", eAoIf, eRate);
            goto FAIL;
        }
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudAoIfMultiAttachAction(AO_CH_e eAoCh, U8 nAttachCount)
{
    int ret = AIO_OK;

    switch (eAoCh)
    {
        case E_AO_CH_DAC_A_0:

            if (nAttachCount == 0)
            {
                // Recover DAC mux to default setting
                ret |= HalAudSetMux(E_AUD_MUX_SDM_DINL, 0);
            }

            break;

        case E_AO_CH_DAC_B_0:

            if (nAttachCount == 0)
            {
                // Recover DAC mux to default setting
                ret |= HalAudSetMux(E_AUD_MUX_SDM_DINR, 0);
            }

            break;

        case E_AO_CH_I2S_TX_A_0:
            break;

        case E_AO_CH_I2S_TX_A_1:
            break;

        default:
            break;
    }

    if (ret != AIO_OK)
    {
        ERRMSG("eAoCh[%d] nAttachCount[%d] Fail !", eAoCh, nAttachCount);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudRuntimePowerCtl(BOOL bEn)
{
    static int power_on_cnt = 0;

    if (bEn)
    {
        power_on_cnt++;
    }
    else
    {
        power_on_cnt--;
    }

    if (bEn && power_on_cnt == 1)
    {
        HalAudPllEnable(E_CLK_SRC_384M, -1, bEn, -1);
        HalAudPllEnable(E_CLK_SRC_48M, -1, bEn, -1);
        HalAudPllEnable(E_CLK_SRC_432M, 0, bEn, -1);

        DBGMSG(AUDIO_DBG_LEVEL_POWER, "power on \n");
    }
    else if (!bEn && power_on_cnt == 0)
    {
        HalAudPllEnable(E_CLK_SRC_384M, -1, bEn, -1);
        HalAudPllEnable(E_CLK_SRC_48M, -1, bEn, -1);
        HalAudPllEnable(E_CLK_SRC_432M, 0, bEn, -1);

        DBGMSG(AUDIO_DBG_LEVEL_POWER, "power down \n");
    }
    DBGMSG(AUDIO_DBG_LEVEL_POWER, "power_on_cnt [%d] bEn[%d]\n", power_on_cnt, bEn);

    return AIO_OK;
}

int HalAudSrcSetAoPath(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh, BOOL nUseSrc)
{
    int ret  = AIO_OK;
    int line = 0;

    switch (eAoCh)
    {
        case E_AO_CH_DAC_A_0:
        case E_AO_CH_ECHO_A_0:
        case E_AO_CH_DAC_B_0:
        case E_AO_CH_ECHO_A_1:
            ret |= HalAudSetMux(E_AUD_MUX_MMC1, 1); // Select RDMA1
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            ret |= HalAudSetMux(E_AUD_MUX_MMC1_DIN_L_SEL, 0); // AB1_L
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            ret |= HalAudSetMux(E_AUD_MUX_MMC1_DIN_R_SEL, 1); // AB1_R
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            break;

        case E_AO_CH_I2S_TX_A_0:
        case E_AO_CH_I2S_TX_A_1:
        {
            ret |= HalAudSetMux(E_AUD_MUX_I2S_CFG_00,
                                1); // 150362[12:10] I2S Tx data sel 1:RDMA1 2:RDMA2 3:SRC1 4:SRC2 5:None
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }

            ret |= HalAudSetMux(E_AUD_MUX_I2STDM_RX, 1); // 150378[0] pad sel 0:ext l2s trx 1:tdm trx
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_TDM1_TX, 1); // 150378[3] pad sel 0:simple i2s 1:dma valid
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_I2S_TDM_TX0_DATA_SEL, 0); // 15039A[6] RDMA1 I2STX Sel 0:TDM0 1:TDM1
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("eAoCh %d Fail !\n", eAoCh);
    return AIO_NG;
}

void HalAudSrcMixStatusUpdate(void) {}

int HalAudAoDetach(CHIP_AIO_DMA_e eAioDma, AO_CH_e aoch)
{
    return 0;
}

int HalAudSrcSetRate(AI_CH_e eAiCh, AudRate_e eRate)
{
    int rateCfg = 0;

    switch (eAiCh)
    {
        case E_AI_CH_ECHO_A_0:
        case E_AI_CH_ECHO_A_1:
            switch (eRate)
            {
                case E_AUD_RATE_8K:
                    rateCfg = 0;
                    break;
                case E_AUD_RATE_16K:
                    rateCfg = 1;
                    break;
                case E_AUD_RATE_32K:
                    rateCfg = 2;
                    break;
                case E_AUD_RATE_48K:
                    rateCfg = 3;
                    break;
                default:
                    ERRMSG("eAiCh %d eRate %d Fail !", eAiCh, eRate);
                    goto FAIL;
            }
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_CIC_1_SEL_MASK,
                            rateCfg << AUDIO_BANK4_REG_SR0_SEL_REG_CIC_1_SEL_SHIFT);
            break;

        default:
            ERRMSG("eAiCh %d Fail !", eAiCh);
            goto FAIL;
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudSrcConfig(AI_CH_e eAiCh, u32 nSampleRate)
{
    int ret = AIO_OK;

    switch (eAiCh)
    {
        case E_AI_CH_ECHO_A_0:
            break;

        case E_AI_CH_ECHO_A_1:
            break;

        default:
            goto FAIL;
            break;
    }

    ret |= HalAudSrcSetRate(eAiCh, HalAudApiRateToEnum(nSampleRate));
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    return AIO_OK;
FAIL:
    ERRMSG("eAiCh %d Fail !", eAiCh);
    return AIO_NG;
}

int HalAudSrcConfigInputClock(u32 nSampleRate)
{
    return 0;
}

int HalAudAoSetSideToneDpga(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, U32 nVolume, U8 nFading)
{
    return 0;
}

int HalAudAoPathSampleSet(AO_IF_e eAoIf, CHIP_AIO_DMA_e eAioDma, u32 eDmaRate, u32 eIfRate)
{
    AudRate_e eRate;
    eRate = HalAudApiRateToEnum(eDmaRate);
    switch (eRate)
    {
        case E_AUD_RATE_8K:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            0 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case E_AUD_RATE_11K:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            1 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case E_AUD_RATE_12K:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            2 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case E_AUD_RATE_16K:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            3 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case E_AUD_RATE_22K:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            4 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case E_AUD_RATE_24K:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            5 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case E_AUD_RATE_32K:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            6 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case E_AUD_RATE_44K:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            7 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        case E_AUD_RATE_48K:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_MASK,
                            8 << AUDIO_BANK4_REG_SR0_SEL_REG_SRC1_SEL_SHIFT);
            break;

        default:
            break;
    }

    return 0;
}

int HalAudAoSetClkRef(AO_ATTACH_t *aoAttach)
{
    int           i         = 0;
    AO_DMA_e      aoDma     = 0;
    AO_IF_e       aoIf      = 0;
    AudAoClkRef_e eAoClkRef = E_AUD_AO_CLK_REF_A;

    //
    aoDma = aoAttach->enAoDma;

    for (i = 0; i < E_MHAL_AO_DMA_CH_SLOT_TOTAL; i++)
    {
        aoIf = aoAttach->eAoIf[i];

        DBGMSG(AUDIO_DBG_LEVEL_PATH, "aoIf[%d]", aoIf);

        if ((aoIf & E_MHAL_AO_IF_I2S_TX_A_0) || (aoIf & E_MHAL_AO_IF_I2S_TX_A_1))
        {
            eAoClkRef = E_AUD_AO_CLK_REF_I2S_TDM_TX_A;
        }
    }

    switch (aoDma)
    {
        case E_MHAL_AO_DMA_A:

            if (eAoClkRef == E_AUD_AO_CLK_REF_I2S_TDM_TX_A)
            {
                HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_ADDR,
                                AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_DMA1_READ_VALID_SEL_MASK,
                                (1 << AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_DMA1_READ_VALID_SEL_SHIFT));
            }
            else if (eAoClkRef == E_AUD_AO_CLK_REF_A)
            {
                HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_ADDR,
                                AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_DMA1_READ_VALID_SEL_MASK,
                                (0 << AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_DMA1_READ_VALID_SEL_SHIFT));
            }
            break;

        case E_MHAL_AO_DMA_DIRECT_A:
        case E_MHAL_AO_DMA_DIRECT_B:
            break;

        default:
            ERRMSG("aoDma %d Fail !", aoDma);
            goto FAIL;
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}
