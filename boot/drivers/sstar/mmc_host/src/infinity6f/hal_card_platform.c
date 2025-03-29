/*
 * hal_card_platform.c- Sigmastar
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
/***************************************************************************************************************
 *
 * FileName hal_card_platform.c
 *     @author jeremy.wang (2016/11/29)
 * Desc:
 *     The platform setting of all cards will run here.
 *     Because register setting that doesn't belong to FCIE/SDIO may have different register setting at different
 *projects. The goal is that we don't need to change "other" HAL_XX.c Level code. (Timing, FCIE/SDIO)
 *
 *     The limitations were listed as below:
 *     (1) Each Project will have XX project name for different hal_card_platform_XX.c files.
 *     (2) IP init, PAD , clock, power and miu setting belong to here.
 *     (4) Timer setting doesn't belong to here, because it will be included by other HAL level.
 *     (5) FCIE/SDIO IP Reg Setting doesn't belong to here.
 *     (6) If we could, we don't need to change any code of hal_card_platform.h
 *
 ***************************************************************************************************************/

#include "hal_card_platform.h"
#include "hal_card_timer.h"
#include "gpio.h"
#include "drv_gpio.h"
#include "padmux.h"
//#include "drv_gpio.h"
#include "drv_padmux.h"
#include "hal_card_platform_pri_config.h"
#include "drv_puse.h"

//***********************************************************************************************************
// Config Setting (Internal)
//***********************************************************************************************************
// Platform Register Basic Address
//------------------------------------------------------------------------------------
#define A_CLKGEN_BANK  GET_CARD_REG_ADDR(A_RIU_BASE, 0x81C00) // 1038h
#define A_PADTOP_BANK  GET_CARD_REG_ADDR(A_RIU_BASE, 0x81E00) // 103Ch
#define A_CLKGEN2_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0x81F80) // 103Fh
//#define A_GPI_INT_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0x81E80)//103Dh
#define A_PM_SLEEP_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0x00700) // 0Eh
#define A_PM_GPIO_BANK  GET_CARD_REG_ADDR(A_RIU_BASE, 0x00780) // 0Fh
#define A_CHIPTOP_BANK  GET_CARD_REG_ADDR(A_RIU_BASE, 0x80F00) // 101Eh
//#define A_MCM_SC_GP_BANK    GET_CARD_REG_ADDR(A_RIU_BASE, 0x89900)//1132h
#define A_SC_GP_CTRL_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0x89A80) // 1135h
#define A_SDPLL_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0C80) // Bank: 0x1419
#define A_FCIEPLL_BANK    GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0D00) // Bank: 0x141A
#define A_PADGPIO_BANK    GET_CARD_REG_ADDR(A_RIU_BASE, 0x81F00) // Bank: 0x103E
#define A_PADGPIO2_BANK   GET_CARD_REG_ADDR(A_RIU_BASE, 0x88200) // Bank: 0x1104
#define A_PM_PAD_TOP_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0x01F80) // 3Fh

// Clock Level Setting (From High Speed to Low Speed)
//-----------------------------------------------------------------------------------------------------------
#define CLK1_F 200000000
#define CLK1_E 100000000
#define CLK1_D 48000000
#define CLK1_C 43200000 // 43200000
#define CLK1_B 40000000
#define CLK1_A 36000000 // 36000000
#define CLK1_9 32000000
#define CLK1_8 20000000
#define CLK1_7 12000000 // 12000000
#define CLK1_6 300000   // alway 400KHz for DTS
#define CLK1_5 0
#define CLK1_4 0
#define CLK1_3 0
#define CLK1_2 0
#define CLK1_1 0
#define CLK1_0 0

#define CLK2_F 48000000
#define CLK2_E 43200000
#define CLK2_D 40000000
#define CLK2_C 36000000
#define CLK2_B 32000000
#define CLK2_A 20000000
#define CLK2_9 12000000
#define CLK2_8 300000
#define CLK2_7 0
#define CLK2_6 0
#define CLK2_5 0
#define CLK2_4 0
#define CLK2_3 0
#define CLK2_2 0
#define CLK2_1 0
#define CLK2_0 0

#define CLK3_F 48000000
#define CLK3_E 43200000
#define CLK3_D 40000000
#define CLK3_C 36000000
#define CLK3_B 32000000
#define CLK3_A 20000000
#define CLK3_9 12000000
#define CLK3_8 300000
#define CLK3_7 0
#define CLK3_6 0
#define CLK3_5 0
#define CLK3_4 0
#define CLK3_3 0
#define CLK3_2 0
#define CLK3_1 0
#define CLK3_0 0

#define REG_CLK_IP_SD   (0x43)
#define REG_CLK_IP_SDIO (0x45)
#define REG_CLK_IP_FCIE (0x4b)

// Bonding ID
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
// IP_FCIE or IP_FCIE Register Basic Address
//----------------------------------------------------------------------------------------------------------
#define A_SD_REG_POS(IP)   GET_CARD_BANK(IP, 0)
#define A_BOOT_MOD_REG(IP) GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0E)

// fpga verify setting
//----------------------------------------------------------------------------------------------------------
#define pr_sd_err(fmt, arg...) printk(fmt, ##arg)
#define pr_sd_dbg(fmt, arg...) printk(fmt, ##arg)

#define UNUSED(x) (x = x)

#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
static U32_T gu32_SDPuse[2][12] = {
    {MDRV_PUSE_SDIO0_PWR, MDRV_PUSE_SDIO0_CDZ, MDRV_PUSE_SDIO0_CLK, MDRV_PUSE_SDIO0_CMD, MDRV_PUSE_SDIO0_D0,
     MDRV_PUSE_SDIO0_D1, MDRV_PUSE_SDIO0_D2, MDRV_PUSE_SDIO0_D3},
    {MDRV_PUSE_EMMC_PWR, MDRV_PUSE_EMMC_RST, MDRV_PUSE_EMMC_CLK, MDRV_PUSE_EMMC_CMD, MDRV_PUSE_EMMC_D0,
     MDRV_PUSE_EMMC_D1, MDRV_PUSE_EMMC_D2, MDRV_PUSE_EMMC_D3, MDRV_PUSE_EMMC_D4, MDRV_PUSE_EMMC_D5, MDRV_PUSE_EMMC_D6,
     MDRV_PUSE_EMMC_D7},
};
#endif
extern U8_T Enable_C2;

U8_T Hal_CARD_PadmuxGetting(MMCPadMuxInfo *mmc_PMuxInfo)
{
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
    U32_T u32_SdPadPin[12];
    U32_T i, j, k;

    if (drv_padmux_active())
    {
        i = mmc_PMuxInfo->u8_ipOrder;

        for (j = 0; j < 12; j++)
        {
            // sd0 pad pin getting
            u32_SdPadPin[j] = drv_padmux_getpad(gu32_SDPuse[i][j]);
            if (u32_SdPadPin[j] == PAD_UNKNOWN)
            {
                pr_err("Fail to get pad(%#x) ip(%#x_%d)  form padmux !\n", gu32_SDPuse[i][j], i, j);
                // may no need to control power by software or no need reset/cdz pin
                if (j == 0 || j == 1)
                    continue;

                break;
            }
        }
        if (j >= 8)
        {
            mmc_PMuxInfo->u32_PinPWR    = u32_SdPadPin[0];
            mmc_PMuxInfo->u32_PinCdzRst = u32_SdPadPin[1];
            mmc_PMuxInfo->u32_PinCLK    = u32_SdPadPin[2];
            mmc_PMuxInfo->u32_PinCMD    = u32_SdPadPin[3];
            for (k = 0; k < 8; k++)
                mmc_PMuxInfo->u32_PinDAT[k] = u32_SdPadPin[4 + k];

            if (j == 12)
                mmc_PMuxInfo->u8_busWidth = 8;
            else
                mmc_PMuxInfo->u8_busWidth = 4;

            mmc_PMuxInfo->u32_Mode = drv_padmux_getmode(gu32_SDPuse[i][2]);

            if (mmc_PMuxInfo->u32_PinPWR != PAD_UNKNOWN)
                mmc_PMuxInfo->u32_PwrMode = drv_padmux_getmode(gu32_SDPuse[i][0]);
            if (mmc_PMuxInfo->u32_PinCdzRst != PAD_UNKNOWN)
                mmc_PMuxInfo->u32_CdzRstMode = drv_padmux_getmode(gu32_SDPuse[i][1]);
        }
    }
    return 0;
#else
    return 0;
#endif
}

//***********************************************************************************************************
// IP Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_IPOnceSetting(IpOrder eIP)
{
    IpSelect eIpSel = (IpSelect)eIP;

#if (FORCE_SWITCH_PAD)
    // reg_all_pad_in => Close
    CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x50), BIT15_T);
#endif

    // Clock Source
    if (eIpSel == IP_SD)
    {
        // BK:x1133(sc_gp_ctrl)[B7] [0:1]/[boot_clk(12M):sd_clk]
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK, 0x25), BIT07_T);
        // BK:x1038(reg_ckg_sd)[B6] [0:1]/[boot_clk(12M):sd_clk]
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT06_T);
    }
    else if (eIpSel == IP_SDIO)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK, 0x25), BIT06_T);
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT06_T);
    }
    else if (eIpSel == IP_FCIE)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK, 0x25), BIT05_T);
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_FCIE), BIT06_T);
    }
}

#if (PADMUX_SET == PADMUX_SET_BY_REG)
//***********************************************************************************************************
// PAD Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_InitPADPin(MMCPadMuxInfo mmc_PMuxInfo)
{
    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;

    if (eIpSel == IP_SD)
    {
        if (ePadSel == PAD_SD_MD1)
        {
            // reg_sd0_pe:D3, D2, D1, D0, CMD, CDZ=> pull en
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SDPLL_BANK, 0x2B),
                            BIT00_T | BIT01_T | BIT02_T | BIT03_T | BIT04_T | BIT06_T);
            // reg_sd0_ps:CDZ=> pull up
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SDPLL_BANK, 0x29), BIT06_T);

            CARD_REG(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x10)) = 0x55; // D2

            // reg_sd0_drv: CLK, D3, D2, D1, D0, CMD => [DS0/DS1/DS2 : 1 0 0]
            CARD_REG(GET_CARD_REG_ADDR(A_SDPLL_BANK, 0x23)) = 0x3F;
            CARD_REG(GET_CARD_REG_ADDR(A_SDPLL_BANK, 0x24)) = 0x0;
            CARD_REG(GET_CARD_REG_ADDR(A_SDPLL_BANK, 0x25)) = 0x0;
        }
    }
    else if (eIpSel == IP_SDIO)
    {
        if (ePadSel == PAD_SD_MD1)
        {
            // D3, D2, D1, D0, CMD=> pull up (PS = 1, PE = 1)
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x04), BIT11_T | BIT06_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x05), BIT11_T | BIT06_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x00), BIT11_T | BIT06_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x01), BIT11_T | BIT06_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x03), BIT11_T | BIT06_T);

            // CLK => pull down (PS=0, PE=1)
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x02), BIT06_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x02), BIT11_T);
        }
        else if (ePadSel == PAD_SD_MD2)
        {
            // PAD_EMMC_CLK/CMD/D[0:3]
            // reg_sd0_pe:D3, D2, D1, D0, CMD, CDZ=> pull en
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_FCIEPLL_BANK, 0x47),
                            BIT11_T | BIT09_T | BIT07_T | BIT05_T | BIT03_T | BIT01_T);
            // reg_sd0_ps:CDZ=> pull up
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_FCIEPLL_BANK, 0x49), BIT11_T);

            // reg_sd0_drv: CLK, D3, D2, D1, D0, CMD => [DS0/DS1/DS2 : 1 0 0]
            CARD_REG(GET_CARD_REG_ADDR(A_FCIEPLL_BANK, 0x45)) = 0x70;
        }
    }
    else if (eIpSel == IP_FCIE)
    {
        if (ePadSel == PAD_SD_MD1)
        {
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_FCIEPLL_BANK, 0x45), BIT10 | BIT9 | BIT8 | BIT6 | BIT5 | BIT4);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_FCIEPLL_BANK, 0x45), BIT2 | BIT1 | BIT0);
        }
    }
}

void Hal_CARD_ConfigSdPad(MMCPadMuxInfo mmc_PMuxInfo) // Hal_CARD_SetPADToPortPath
{
    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;
    Enable_C2         = 1;

    CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x8), BIT00_T);           // clear sd boot mode
    CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x9), BIT00_T | BIT08_T); // clear emmc boot mode
    //  SD mode
    if (eIpSel == IP_SD)
    {
        if (ePadSel == PAD_SD_MD1)
        {
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT11_T | BIT10_T | BIT09_T | BIT08_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T);
        }
    }
    else if (eIpSel == IP_SDIO)
    {
        // SDIO mode
        // OFF:x67 [B8]reg_sd0_mode [B9]reg_sd0_cdz_mode [B13:B12]reg_sd1_mode
        // OFF:x68 [B9:B8]reg_sd1_cdz_mode
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T | BIT12_T);
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), (ePadSel + 1) << 12);
        // if (ePadSel == PAD_SD_MD1)
        //   Enable_C2 = 0;
    }
    else if (eIpSel == IP_FCIE)
    {
        if (ePadSel == PAD_SD_MD1)
        {
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT04_T | BIT02_T | BIT00_T);

            if (mmc_PMuxInfo.u8_busWidth == 8)
            {
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT04_T | BIT02_T);
            }
            else if (mmc_PMuxInfo.u8_busWidth == 4)
            {
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT04_T | BIT00_T);
            }
        }
    }
}

void Hal_CARD_ConfigPowerPad(MMCPadMuxInfo mmc_PMuxInfo)
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }
    else if (eIpSel == IP_FCIE)
    {
        // FCIE no need config power
        return;
    }
    // pr_sd_err("mmc_PMuxInfo.u32_PinPWR: %d, PAD_SD0_GPIO0: %d\n", mmc_PMuxInfo.u32_PinPWR, PAD_SD0_GPIO0);

    switch (mmc_PMuxInfo.u32_PinPWR)
    {
        case PAD_SD0_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), (BIT02_T | BIT01_T | BIT00_T));
            break;
        case PAD_SD1_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7D), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7D), (BIT02_T | BIT01_T | BIT00_T)); // reg_gpio_oen_125
            break;
        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            break;
    }

    return;
}

void Hal_CARD_PullPADPin(MMCPadMuxInfo mmc_PMuxInfo, PinPullEmType ePinPull)
{
    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;

    // IP_SD
    if (eIpSel == IP_SD)
    {
        // OFF:x67 [B8:B9]reg_sd0_mode
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T | BIT09_T);
        if (ePadSel == PAD_SD_MD1)
        {
            if (ePinPull == EV_PULLDOWN)
            {
                // pull pad pin DOWN
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SDPLL_BANK, 0x2B), 0x1F);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_SDPLL_BANK, 0x29), 0x1F);
            }
            else if (ePinPull == EV_PULLUP)
            {
                // pull pad pin up. ex:clk no need
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SDPLL_BANK, 0x2B), 0x1F);
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SDPLL_BANK, 0x29), 0x1F);
                // SD Mode
                // OFF:x67 [B8:B9]reg_sd0_mode [B10]reg_sd0_cdz_mode
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T);
            }
        }
    }
    else if (eIpSel == IP_SDIO)
    {
        //[B13:B12] sd1 mode clear
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T | BIT12_T);

        if (ePadSel == PAD_SD_MD1)
        {
            if (ePinPull == EV_PULLDOWN)
            {
                // D3, D2, D1, D0, CMD=> pull up (PS = 1, PE = 1)
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x04), BIT11_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x05), BIT11_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x00), BIT11_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x01), BIT11_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x03), BIT11_T);
            }
            else if (ePinPull == EV_PULLUP)
            {
                // D3, D2, D1, D0, CMD=> pull up (PS = 1, PE = 1)
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x04), BIT11_T);
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x05), BIT11_T);
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x00), BIT11_T);
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x01), BIT11_T);
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x03), BIT11_T);

                //[B14:B12] set sd1 mode 1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT12_T);
            }
        }
        else if (ePadSel == PAD_SD_MD2)
        {
            if (ePinPull == EV_PULLDOWN)
            {
                // pull pad pin DOWN
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_FCIEPLL_BANK, 0x47), 0xEAA);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_FCIEPLL_BANK, 0x49), 0x6AA);
            }
            else if (ePinPull == EV_PULLUP)
            {
                // pull pad pin up. ex:clk no need
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_FCIEPLL_BANK, 0x47), 0xEAA);
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_FCIEPLL_BANK, 0x49), 0xAAA);
                // SD Mode
                //[B14:B12] set sd1 mode 2
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T);
            }
        }
    }
    else if (eIpSel == IP_FCIE) // EMMC no need this step
    {
    }
}

//***********************************************************************************************************
// Signal line driving control Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_DrvCtrlPin(MMCPadMuxInfo mmc_PMuxInfo, MMCPinDrv mmc_pinDrv)
{
    /* IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
     PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;

     U8_T u8_PadClk = 0, u8_ClkDrvMax = DRV_CTRL_15;
     U8_T u8_PadCmd = 0, u8_CmdDrvMax = DRV_CTRL_15;
     U8_T u8_PadData[4] = {0}, u8_DataDrvMax = DRV_CTRL_15, i;

     if (eIpSel == IP_SD) // sd3.0 padpin specially set by reg.
     {
         if (ePadSel == PAD_SD_MD1)
         {
             u8_PadClk     = PAD_SD0_CLK;
             u8_PadCmd     = PAD_SD0_CMD;
             u8_PadData[0] = PAD_SD0_D0;
             u8_PadData[1] = PAD_SD0_D1;
             u8_PadData[2] = PAD_SD0_D2;
             u8_PadData[3] = PAD_SD0_D3;
         }
     }
     else if (eIpSel == IP_SDIO) // gpio set by function
     {
         // get sd signal line pad
         if (ePadSel == PAD_SD_MD1)
         {
             u8_PadClk     = PAD_KEY10;
             u8_PadCmd     = PAD_KEY11;
             u8_PadData[0] = PAD_KEY9;
             u8_PadData[1] = PAD_KEY8;
             u8_PadData[2] = PAD_KEY13;
             u8_PadData[3] = PAD_KEY12;
         }
         else if (ePadSel == PAD_SD_MD2)
         {
             u8_PadClk     = PAD_EMMC_CLK;
             u8_PadCmd     = PAD_EMMC_CMD;
             u8_PadData[0] = PAD_EMMC_D0;
             u8_PadData[1] = PAD_EMMC_D1;
             u8_PadData[2] = PAD_EMMC_D2;
             u8_PadData[3] = PAD_EMMC_D3;
         }
     }
     else if (eIpSel == IP_FCIE) // gpio set by function
     {
         // get sd signal line pad
         if (ePadSel == PAD_SD_MD1)
         {
             u8_PadClk     = PAD_EMMC_CLK;
             u8_PadCmd     = PAD_EMMC_CMD;
             u8_PadData[0] = PAD_EMMC_D0;
             u8_PadData[1] = PAD_EMMC_D1;
             u8_PadData[2] = PAD_EMMC_D2;
             u8_PadData[3] = PAD_EMMC_D3;
         }
     }

     // driving control switch
     if (mmc_pinDrv.eDrvClk != DRV_NOSET)
         sstar_gpio_drv_set(u8_PadClk, mmc_pinDrv.eDrvClk > u8_ClkDrvMax ? u8_ClkDrvMax : mmc_pinDrv.eDrvClk);

     if (mmc_pinDrv.eDrvCmd != DRV_NOSET)
         sstar_gpio_drv_set(u8_PadCmd, mmc_pinDrv.eDrvCmd > u8_CmdDrvMax ? u8_CmdDrvMax : mmc_pinDrv.eDrvCmd);

     if (mmc_pinDrv.eDrvData != DRV_NOSET)
     {
         for (i = 0; i < 4; i++)
             sstar_gpio_drv_set(u8_PadData[i], mmc_pinDrv.eDrvData > u8_DataDrvMax ? u8_DataDrvMax :
     mmc_pinDrv.eDrvData);
     }*/
}

//***********************************************************************************************************
// Power and Voltage Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_PowerOn(MMCPadMuxInfo mmc_PMuxInfo, U16_T u16DelayMs)
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }
    else if (eIpSel == IP_FCIE)
    {
        // FCIE no need control power
        return;
    }

    switch (mmc_PMuxInfo.u32_PinPWR)
    {
        case PAD_SD0_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT02_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT01_T);
            break;
        case PAD_SD1_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7D), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7D), BIT02_T); // reg_gpio_oen_120
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7D), BIT01_T); // reg_gpio_out_120
            break;
        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            break;
    }

    Hal_Timer_mSleep(u16DelayMs);
    // Hal_Timer_mDelay(u16DelayMs);
}

void Hal_CARD_PowerOff(MMCPadMuxInfo mmc_PMuxInfo, U16_T u16DelayMs)
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }
    else if (eIpSel == IP_FCIE)
    {
        // FCIE no need control power
        return;
    }

    switch (mmc_PMuxInfo.u32_PinPWR)
    {
        case PAD_SD0_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT02_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT01_T);
            break;
        case PAD_SD1_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7D), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7D), BIT02_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7D), BIT01_T);
            break;
        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            break;
    }

    Hal_Timer_mSleep(u16DelayMs);
    // Hal_Timer_mDelay(u16DelayMs);
}

//***********************************************************************************************************
// Card Detect and GPIO Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_ConfigCdzPad(MMCPadMuxInfo mmc_PMuxInfo) // Hal_CARD_InitGPIO
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    U16_T    nPadNo = mmc_PMuxInfo.u32_PinCdzRst;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }
    else if (eIpSel == IP_FCIE)
    {
        // FCIE no need config cdz
        return;
    }
    // pr_sd_err("nPadNo: %d, PAD_SD0_CDZ: %d\n", nPadNo, PAD_SD0_CDZ);

    switch (nPadNo)
    {
        case PAD_SD0_CDZ:
            // OFF:x67 [B8]reg_sd0_mode [B10]reg_sd0_cdz_mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT11_T | BIT10_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT10_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x0A), BIT02_T);
            break;
        case PAD_SD1_CDZ:
            // PADMUX sdio cdz mode = 1
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT12_T | BIT13_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT12_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7F), BIT02_T);
            break;
        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            break;
    }

    return;
}

BOOL_T Hal_CARD_GetCdzState(MMCPadMuxInfo mmc_PMuxInfo) // Hal_CARD_GetGPIOState
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    U8_T     nLv    = 0;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    else if (eIpSel == IP_FCIE)
    {
        // FCIE no need detect cdz
        return TRUE;
    }

    switch (mmc_PMuxInfo.u32_PinCdzRst)
    {
        case PAD_SD0_CDZ:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x0A)) & BIT00_T;
            break;
        case PAD_SD1_CDZ:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7F)) & BIT00_T;
            break;
        case PAD_EMMC_RSTN:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C)) & BIT00_T;
        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            break;
    }

    if (!nLv) // Low Active
    {
        return TRUE;
    }

    return FALSE;
}
#endif

//***********************************************************************************************************
// Clock Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_SetClock(IpOrder eIP, U32_T u32ClkFromIPSet)
{
    IpSelect eIpSel = (IpSelect)eIP;

    if (eIpSel == IP_SD)
    {
        // [5:2]: Clk_Sel
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT05_T | BIT04_T | BIT03_T | BIT02_T);
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT00_T);

        switch (u32ClkFromIPSet)
        {
            case CLK1_D: // 48000KHz
                break;
            case CLK1_C:                                                                   // 43200KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT02_T); // 1
                break;
            case CLK1_B:                                                                   // 40000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT03_T); // 2
                break;
            case CLK1_A:                                                                             // 36000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT03_T | BIT02_T); // 3
                break;
            case CLK1_9:                                                                   // 32000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT04_T); // 4
                break;
            case CLK1_8:                                                                             // 20000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT04_T | BIT02_T); // 5
                break;
            case CLK1_7:                                                                             // 12000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT04_T | BIT03_T); // 6
                break;
            case CLK1_6:                                                                                       // 300KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT04_T | BIT03_T | BIT02_T); // 7
                break;
        }
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT00_T);
    }
    else if (eIpSel == IP_SDIO)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO),
                        BIT05_T | BIT04_T | BIT03_T | BIT02_T | BIT01_T);
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT00_T);
        switch (u32ClkFromIPSet)
        {
            case CLK2_F: // 48000KHz
                break;
            case CLK2_E:                                                                     // 43200KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT02_T); // 1
                break;
            case CLK2_D:                                                                     // 40000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT03_T); // 2
                break;
            case CLK2_C:                                                                               // 36000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT03_T | BIT02_T); // 3
                break;
            case CLK2_B:                                                                     // 32000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT04_T); // 4
                break;
            case CLK2_A:                                                                               // 20000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT04_T | BIT02_T); // 5
                break;
            case CLK2_9:                                                                               // 12000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT04_T | BIT03_T); // 6
                break;
            case CLK2_8: // 300KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT04_T | BIT03_T | BIT02_T); // 7
                break;
        }
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT00_T);
    }
    else if (eIpSel == IP_FCIE)
    {
        // [4:2]: Clk_Sel
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_FCIE), BIT05_T | BIT04_T | BIT03_T | BIT02_T);
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_FCIE), BIT00_T);

        switch (u32ClkFromIPSet)
        {
            case CLK2_F: // 48000KHz
                break;
            case CLK2_E:                                                                     // 43200KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_FCIE), BIT02_T); // 1
                break;
            case CLK2_D:                                                                     // 40000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_FCIE), BIT03_T); // 2
                break;
            case CLK2_C:                                                                               // 36000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_FCIE), BIT03_T | BIT02_T); // 3
                break;
            case CLK2_B:                                                                     // 32000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_FCIE), BIT04_T); // 4
                break;
            case CLK2_A:                                                                               // 20000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_FCIE), BIT04_T | BIT02_T); // 5
                break;
            case CLK2_9:                                                                               // 12000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_FCIE), BIT04_T | BIT03_T); // 6
                break;
            case CLK2_8: // 300KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_FCIE), BIT04_T | BIT03_T | BIT02_T); // 7
                break;
        }
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_FCIE), BIT00_T);
    }
#if defined(CONFIG_SSTAR_VERSION_FPGA) && CONFIG_SSTAR_VERSION_FPGA
    if (u32ClkFromIPSet == CLK2_8)
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT06_T);
    else
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT06_T);
#endif
}

U32_T Hal_CARD_FindClockSetting(IpOrder eIP, U32_T u32ReffClk)
{
    U8_T  u8LV                     = 0;
    U32_T u32RealClk               = 0;
    U32_T u32ClkArr[MMC_TOTAL][16] = {{CLK1_F, CLK1_E, CLK1_D, CLK1_C, CLK1_B, CLK1_A, CLK1_9, CLK1_8, CLK1_7, CLK1_6,
                                       CLK1_5, CLK1_4, CLK1_3, CLK1_2, CLK1_1, CLK1_0},
                                      {CLK2_F, CLK2_E, CLK2_D, CLK2_C, CLK2_B, CLK2_A, CLK2_9, CLK2_8, CLK2_7, CLK2_6,
                                       CLK2_5, CLK2_4, CLK2_3, CLK2_2, CLK2_1, CLK2_0},
                                      {CLK3_F, CLK3_E, CLK3_D, CLK3_C, CLK3_B, CLK3_A, CLK3_9, CLK3_8, CLK3_7, CLK3_6,
                                       CLK3_5, CLK3_4, CLK3_3, CLK3_2, CLK3_1, CLK3_0},
                                      {CLK3_F, CLK3_E, CLK3_D, CLK3_C, CLK3_B, CLK3_A, CLK3_9, CLK3_8, CLK3_7, CLK3_6,
                                       CLK3_5, CLK3_4, CLK3_3, CLK3_2, CLK3_1, CLK3_0}};

    for (; u8LV < 16; u8LV++)
    {
        if ((u32ReffClk >= u32ClkArr[eIP][u8LV]) || (u8LV == 15) || (u32ClkArr[eIP][u8LV + 1] == 0))
        {
            u32RealClk = u32ClkArr[eIP][u8LV];
            break;
        }
    }

    return u32RealClk;
}

void Hal_CARD_DumpPadMux(PadOrder ePAD)
{
    PadSelect ePadSel = (PadSelect)ePAD;

    if (ePadSel == PAD_SD_MD1)
    {
        printk("reg_allpad_in; reg[101EA1]#7=0b\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x50)));
        printk("reg_test_in_mode; reg[101E24]#1 ~ #0=0b\n");
        printk("reg_test_out_mode; reg[101E24]#5 ~ #4=0b\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x12)));
        printk("reg_spi0_mode; reg[101E18]#2 ~ #0=0b\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0C)));
        printk("reg_fuart_mode; reg[101E06]#2 ~ #0=0b !=4,!=6\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x03)));
        printk("reg_sdio_mode; reg[101E11]#0=0b ==0x100\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08)));
        printk("reg_pwm0_mode; reg[101E0E]#2 ~ #0=0b\n");
        printk("reg_pwm2_mode; reg[101E0F]#0 ~ #-2=0b\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x07)));
        printk("reg_i2s_mode; reg[101E1F]#3 ~ #2=0b\n");
        printk("reg_ttl_mode; reg[101E1E]#7 ~ #6=0b\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0F)));
    }
}

//***********************************************************************************************************
// MIU Setting for Card Platform
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_TransMIUAddr
 *     @author jeremy.wang (2015/7/31)
 * Desc: Transfer original address to HW special dma address (MIU0/MIU1)
 *
 * @param u32Addr : Original address
 *
 * @return U32_T  : DMA address
 ----------------------------------------------------------------------------------------------------------*/
dma_addr_t Hal_CARD_TransMIUAddr(dma_addr_t ptr_Addr, U8_T *pu8MIUSel)
{
#if defined(PHYS_TO_MIU_USE_FUNC) && PHYS_TO_MIU_USE_FUNC
    return Chip_Phys_to_MIU(ptr_Addr);

#else
    dma_addr_t U64MiuBase = 0x20000000;
    return ptr_Addr - U64MiuBase;

#endif
}

void eMMC_hardware_reset(IpOrder eIP, PadOrder ePAD, U16_T u16DelayMs)
{
    if (eIP == IP_ORDER_1)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT04_T);
        // RST low & high
        CARD_REG_SETBIT(A_BOOT_MOD_REG(eIP), BIT01_T);
        CARD_REG_CLRBIT(A_BOOT_MOD_REG(eIP), BIT00_T);
        Hal_Timer_mDelay(u16DelayMs);
        CARD_REG_SETBIT(A_BOOT_MOD_REG(eIP), BIT00_T);
        CARD_REG_CLRBIT(A_BOOT_MOD_REG(eIP), BIT01_T);
        Hal_Timer_mDelay(5);
        return;
    }
}
