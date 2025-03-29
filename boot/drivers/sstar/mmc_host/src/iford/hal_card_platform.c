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
//#define A_PM_SLEEP_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0x00700) // 0Eh
//#define A_PM_GPIO_BANK  GET_CARD_REG_ADDR(A_RIU_BASE, 0x00780) // 0Fh
#define A_CHIPTOP_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0x80F00) // 101Eh
//#define A_MCM_SC_GP_BANK    GET_CARD_REG_ADDR(A_RIU_BASE, 0x89900)//1132h
#define A_SC_GP_CTRL_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0x89A80) // 1135h
#define A_SDPLL_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0C80) // Bank: 0x1419
#define A_FCIEPLL_BANK    GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0D00) // Bank: 0x141A
#define A_PADGPIO_BANK    GET_CARD_REG_ADDR(A_RIU_BASE, 0x81F00) // Bank: 0x103E
#define A_PADGPIO2_BANK   GET_CARD_REG_ADDR(A_RIU_BASE, 0x88200) // Bank: 0x1104

#define A_PM_CLKGEN_BANK  GET_CARD_REG_ADDR(A_RIU_BASE, 0x700)  // 0eh
#define A_PM_PADTOP_BANK  GET_CARD_REG_ADDR(A_RIU_BASE, 0x1F80) // 3fh
#define A_PM_PADGPIO_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0x1F80) // 3fh

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

#define CLK2_F 200000000
#define CLK2_E 48000000
#define CLK2_D 32000000
#define CLK2_C 12000000
#define CLK2_B 300000
#define CLK2_A 0
#define CLK2_9 0
#define CLK2_8 0
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

#define REG_CLK_IP_SD        (0x43)
#define REG_CLK_IP_SDIO      (0x24)
#define REG_CLK_IP_FCIE      (0x4b)
#define REG_FCIE_EIGHT_PHASE (0x4e)

// Bonding ID
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
// IP_FCIE or IP_FCIE Register Basic Address
//----------------------------------------------------------------------------------------------------------
#define A_SD_REG_POS(IP)   GET_CARD_BANK(IP, 0)
#define A_BOOT_MOD_REG(IP) GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0E)
#define A_SDIO_DET_ON(IP)  GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x2F)

// fpga verify setting
//----------------------------------------------------------------------------------------------------------
#define pr_sd_err(fmt, arg...) printk(fmt, ##arg)
#define pr_sd_dbg(fmt, arg...) printk(fmt, ##arg)

#define UNUSED(x) (x = x)

extern U8_T Enable_C2;

//***********************************************************************************************************
// IP Setting for Card Platform
//***********************************************************************************************************
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
BOOL_T Hal_Card_CheckIseMMC(MMCPadMuxInfo mmc_PMuxInfo)
{
    return ((mmc_PMuxInfo.u32_CdzRstMode == PINMUX_FOR_EMMC_RST_MODE_1)
            || (mmc_PMuxInfo.u32_CdzRstMode == PINMUX_FOR_EMMC_RST_MODE_2)
            || (mmc_PMuxInfo.u32_CdzRstMode == PINMUX_FOR_PM_SDIO_RSTN_MODE_1)
            || (mmc_PMuxInfo.u32_CdzRstMode == PINMUX_FOR_PM_SDIO_RSTN_MODE_2)
            || (mmc_PMuxInfo.u32_CdzRstMode == PINMUX_FOR_PM_SDIO_RSTN_MODE_3)
            || (mmc_PMuxInfo.u32_CdzRstMode == PINMUX_FOR_PM_SDIO_RSTN_MODE_4));
}
#endif

void Hal_CARD_IPOnceSetting(IpOrder eIP)
{
    IpSelect eIpSel = (IpSelect)eIP;
    CARD_REG_CLRBIT(A_SDIO_DET_ON(eIpSel), R_SDIO_DET_ON | R_SDIO_DET_ON_SEL);
    // pwr pad use pm-domain gpio
    CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x60), BIT00_T);

    // Clock Source
    if (eIpSel == IP_SD)
    {
#if (FORCE_SWITCH_PAD)
        // reg_all_pad_in => Close
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x50), BIT15_T);
#endif
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK, 0x25),
                        BIT07_T); // BK:x1133(sc_gp_ctrl)[B7] [0:1]/[boot_clk(12M):sd_clk]
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD),
                        BIT07_T | BIT06_T | BIT05_T | BIT04_T | BIT03_T | BIT02_T | BIT01_T | BIT00_T);
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD),
                        BIT07_T); // BK:x1038(reg_ckg_sd)[B5] [0:1]/[boot_clk(12M):sd_clk]
    }
    else if (eIpSel == IP_SDIO)
    {
        /* rtcpll forceon */
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, 0x58), BIT00_T);
        /* affect bandwidths */
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, 0x20), BIT04_T);
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, 0x24), BIT15_T); // glitch free
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, 0x24),
                        BIT14_T | BIT13_T | BIT12_T | BIT11_T); // BK:0x0e(pm reg_ckg_sd)[B15] [0:1]/[sd_clk]
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
        {                                                                                // SD0_MODE
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x14), BIT04_T | BIT05_T); // CDZ
            // reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x15), BIT04_T); // D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x16), BIT04_T); // D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x18), BIT04_T); // CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x19), BIT04_T); // D3
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x1A), BIT04_T); // D2

            // reg_sd0_drv: CLK, D3, D2, D1, D0, CMD => drv: 1 for vCore 0.9V->0.85V
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x15), BIT07_T); // D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x16), BIT07_T); // D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x17), BIT07_T); // CLK
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x18), BIT07_T); // CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x19), BIT07_T); // D3
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x1A), BIT07_T); // D2*/
        }
        else if (ePadSel == PAD_SD_MD2 || ePadSel == PAD_SD_MD4)
        {                                                                               // EMMC0_MODE
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0xF), BIT04_T | BIT05_T); // CDZ
            // reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0xC), BIT04_T); // CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0xA), BIT04_T); // D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x9), BIT04_T); // D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0xE), BIT04_T); // D2
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0xD), BIT04_T); // D3

            // reg_sd0_drv: CLK, D3, D2, D1, D0, CMD => drv: 1 for vCore 0.9V->0.85V
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0xB), BIT07_T); // CLK
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0xC), BIT07_T); // CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0xA), BIT07_T); // D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x9), BIT07_T); // D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0xE), BIT07_T); // D2
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0xD), BIT07_T); // D3
        }
    }
    else if (eIpSel == IP_SDIO)
    {
        if (ePadSel == PAD_SD_MD1 || ePadSel == PAD_SD_MD4)
        { // PM_SDIO_MODE0 or PM_EMMC_MODE0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x11), BIT04_T | BIT05_T); // CDZ
            // reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT04_T); // CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT04_T); // D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1D), BIT04_T); // D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x22), BIT04_T); // D2
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT04_T); // D3

            // reg_sd0_drv: CLK, D3, D2, D1, D0, CMD => drv: 1 for vCore 0.9V->0.85V
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1F), BIT07_T); // CLK
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT07_T); // CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT07_T); // D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1D), BIT07_T); // D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x22), BIT07_T); // D2
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT07_T); // D3
        }
    }
}

void Hal_CARD_ConfigSdPad(MMCPadMuxInfo mmc_PMuxInfo) // Hal_CARD_SetPADToPortPath
{
    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;

    //  SD mode
    if (eIpSel == IP_SD)
    {
        if (ePadSel == PAD_SD_MD1)
        {
            Enable_C2 = 1;
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61),
                            BIT12_T | BIT11_T | BIT09_T | BIT08_T | BIT04_T | BIT05_T | BIT01_T | BIT00_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61),
                            BIT00_T | BIT04_T); //[B0:B4]/[reg_sd0_mode:reg_sd0_cdz_mode]
        }
        else if (ePadSel == PAD_SD_MD2)
        {
        }
        else if (ePadSel == PAD_SD_MD4)
        {
            Enable_C2 = 0;
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61),
                            BIT12_T | BIT11_T | BIT09_T | BIT08_T | BIT04_T | BIT05_T | BIT01_T | BIT00_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61),
                            BIT10_T | BIT08_T); //[B10:B8]/[reg_emmc0_mode:reg_emmc0_rst_mode]
        }
    }
    else if (eIpSel == IP_SDIO)
    {
        Enable_C2 = 0;
        // PM SD mode reg_all_pad_in
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x60), BIT00_T);
        if (ePadSel == PAD_SD_MD1)
        {
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52),
                            BIT04_T | BIT05_T | BIT06_T | BIT08_T | BIT09_T | BIT14_T | BIT13_T | BIT12_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52),
                            BIT12_T | BIT08_T); //[B8]/[reg_sd0_mode+reg_sd0_cdz_mode]
        }
        else if (ePadSel == PAD_SD_MD4)
        {
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52),
                            BIT04_T | BIT05_T | BIT06_T | BIT08_T | BIT09_T | BIT14_T | BIT13_T | BIT12_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52),
                            BIT04_T | BIT08_T); //[B8]/[reg_emmc0_mode+reg_emmc0_rst_mode]
        }
    }
}

void Hal_CARD_ConfigPowerPad(MMCPadMuxInfo mmc_PMuxInfo)
{
    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }
    else if (ePadSel > PAD_SD_MD3)
    {
        // emmc no need config power
        return;
    }
    // pr_sd_err("mmc_PMuxInfo.u32_PinPWR: %d, PAD_SD0_GPIO0: %d\n", mmc_PMuxInfo.u32_PinPWR, PAD_SD0_GPIO0);

    switch (mmc_PMuxInfo.u32_PinPWR)
    {
        case PAD_PM_GPIO11:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), (BIT02_T | BIT01_T | BIT00_T));
            break;
        case PAD_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x25), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x25),
                            (BIT02_T | BIT01_T | BIT00_T)); // reg_gpio_oen_125
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
        if (ePadSel == PAD_SD_MD1)
        {
            if (ePinPull == EV_PULLDOWN)
            {
                // reg_sd0_pe: D3, D2, D1, D0, CMD=> pull dis
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x15), BIT04_T); // D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x16), BIT04_T); // D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x18), BIT04_T); // CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x19), BIT04_T); // D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x1A), BIT04_T); // D2

                // PAD -> GPIO mode
                // OFF:x67 [B8:B9]reg_sd0_mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT00_T);

                // Output Low
                // SD_ClK
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x17), BIT02_T); // reg_sd0_gpio_oen_2
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x17), BIT01_T); // reg_sd0_gpio_out_2

                // SD_CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x18), BIT02_T); // reg_sd0_gpio_oen_3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x18), BIT01_T); // reg_sd0_gpio_out_3

                // SD_D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x16), BIT02_T); // reg_sd0_gpio_oen_1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x16), BIT01_T); // reg_sd0_gpio_out_1

                // SD_D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x15), BIT02_T); // reg_sd0_gpio_oen_0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x15), BIT01_T); // reg_sd0_gpio_out_0

                // SD_D2
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x1A), BIT02_T); // reg_sd0_gpio_oen_5
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x1A), BIT01_T); // reg_sd0_gpio_out_5

                // SD_D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x19), BIT02_T); // reg_sd0_gpio_oen_4
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x19), BIT01_T); // reg_sd0_gpio_out_4
            }
            else if (ePinPull == EV_PULLUP)
            {
                // reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x15), BIT04_T); // D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x16), BIT04_T); // D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x18), BIT04_T); // CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x19), BIT04_T); // D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x1A), BIT04_T); // D2

                // Input
                // SD_CLK
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x17), BIT02_T); // reg_sd0_gpio_oen_2

                // SD_CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x18), BIT02_T); // reg_sd0_gpio_oen_3

                // SD_D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x16), BIT02_T); // reg_sd0_gpio_oen_1

                // SD_D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x15), BIT02_T); // reg_sd0_gpio_oen_0

                // SD_D2
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x1A), BIT02_T); // reg_sd0_gpio_oen_5

                // SD_D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x19), BIT02_T); // reg_sd0_gpio_oen_4

                // OFF:x61 [B0:B1]reg_sd0_mode
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT00_T);
            }
        }
        else if (ePadSel == PAD_SD_MD4)
        {
        }
    }
    else if (eIpSel == IP_SDIO)
    {
        if (ePadSel == PAD_SD_MD1)
        {
            if (ePinPull == EV_PULLDOWN)
            {
                // reg_sd0_pe: D3, D2, D1, D0, CMD=> pull dis
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1D), BIT04_T); // D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT04_T); // D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT04_T); // CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT04_T); // D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x22), BIT04_T); // D2

                // PAD -> GPIO mode
                // OFF:x67 [B8:B9]reg_sd0_mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT08_T);

                // Output Low
                // SD_ClK
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1F), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1F), BIT01_T);

                // SD_CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT01_T);

                // SD_D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT01_T);

                // SD_D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1D), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1D), BIT01_T);

                // SD_D2
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x22), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x22), BIT01_T);

                // SD_D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT01_T);
            }
            else if (ePinPull == EV_PULLUP)
            {
                // reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1D), BIT04_T); // D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT04_T); // D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT04_T); // CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT04_T); // D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x22), BIT04_T); // D2

                // Input
                // SD_CLK
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1F), BIT02_T);

                // SD_CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT02_T);

                // SD_D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT02_T);

                // SD_D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1D), BIT02_T);

                // SD_D2
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x22), BIT02_T);

                // SD_D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT02_T);

                // OFF:x67 [B8:B9]reg_sd0_mode [B10]reg_sd0_cdz_mode
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT08_T);
            }
        }
        else if (ePadSel == PAD_SD_MD4)
        {
        }
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
    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }
    else if (ePadSel > PAD_SD_MD3)
    {
        // emmc no need control power
        return;
    }

    switch (mmc_PMuxInfo.u32_PinPWR)
    {
        case PAD_PM_GPIO11:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT02_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT01_T);
            break;
        case PAD_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x25), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x25), BIT02_T); // reg_gpio_oen_120
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x25), BIT01_T); // reg_gpio_out_120
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
    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }
    else if (ePadSel > PAD_SD_MD3)
    {
        // emmc no need control power
        return;
    }

    switch (mmc_PMuxInfo.u32_PinPWR)
    {
        case PAD_PM_GPIO11:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT02_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT01_T);
            break;
        case PAD_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x25), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x25), BIT02_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x25), BIT01_T);
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
    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;
    U16_T     nPadNo  = mmc_PMuxInfo.u32_PinCdzRst;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }
    else if (ePadSel > PAD_SD_MD3)
    {
        // emmc no need config cdz
        return;
    }
    // pr_sd_err("nPadNo: %d, PAD_SD0_CDZ: %d\n", nPadNo, PAD_SD0_CDZ);

    switch (nPadNo)
    {
        case PAD_SD0_CDZ:
            // OFF:x61 [B0]reg_sd0_mode [B4]reg_sd0_cdz_mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT05_T | BIT04_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT04_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x14), BIT02_T);
            break;
        case PAD_GPIO1:
            // PADMUX sdio cdz mode = 1
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT12_T | BIT13_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT12_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x11), BIT02_T);
            break;
        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            break;
    }

    return;
}

BOOL_T Hal_CARD_GetCdzState(MMCPadMuxInfo mmc_PMuxInfo) // Hal_CARD_GetGPIOState
{
    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;
    U8_T      nLv     = 0;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    else if (ePadSel > PAD_SD_MD3)
    {
        // emmc no need detect cdz
        return TRUE;
    }

    switch (mmc_PMuxInfo.u32_PinCdzRst)
    {
        case PAD_SD0_CDZ:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x14)) & BIT00_T;
            break;
        case PAD_GPIO1:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x11)) & BIT00_T;
            break;
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
void Hal_CARD_SetClock(IpOrder eIP, MMCClkPhase *pmmc_clkPha, U32_T u32ClkFromIPSet)
{
    IpSelect eIpSel = (IpSelect)eIP;

    if (eIpSel == IP_SD)
    {
        // clr gating
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT00_T);
        // clr bit6 for glitch free selct
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT06_T);
        // clr clk_sel
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT05_T | BIT04_T | BIT03_T | BIT02_T);
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT12_T | BIT11_T | BIT10_T | BIT09_T);

        switch (u32ClkFromIPSet)
        {
            case CLK1_D: // 48000KHz
                if (pmmc_clkPha->u8_clkPhaEn)
                {
                    if (pmmc_clkPha->u8_eightPhaEn)
                        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD),
                                        BIT05_T | BIT03_T); // 10 384M
                    else
                        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD),
                                        BIT05_T | BIT04_T | BIT02_T); // 13 216M
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD),
                                    BIT13_T | BIT08_T); // rx and tx enable
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD),
                                    pmmc_clkPha->eClkPha_TX << 9 | pmmc_clkPha->eClkPha_RX << 11);
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_FCIE_EIGHT_PHASE),
                                    pmmc_clkPha->u8_eightPhaEn << 0); // eight phase enable
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_FCIE_EIGHT_PHASE),
                                    pmmc_clkPha->u8_eightPha_TX << 1 | pmmc_clkPha->u8_eightPha_RX << 2);
                }
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
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT06_T); // glitch sel
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK, 0x25), BIT07_T);
    }
    else if (eIpSel == IP_SDIO)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO),
                        BIT11_T); // clr gating
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO),
                        BIT15_T); // glitch free
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO),
                        BIT14_T | BIT13_T); // clr sel

        switch (u32ClkFromIPSet)
        {
            case CLK2_E: // 48000KHz-real is 49.15MHz
                if (pmmc_clkPha->u8_clkPhaEn)
                {
                    CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT05_T | BIT00_T);
                    CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO),
                                    BIT10_T | BIT09_T | BIT04_T | BIT03_T | BIT02_T | BIT01_T);
                    // 200/216/250MHz
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT14_T | BIT13_T);
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO),
                                    BIT05_T | BIT00_T); // rx & tx enable
                    if (pmmc_clkPha->u8_eightPhaEn)
                    {
                        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT08_T); // 5 phase
                        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO),
                                        pmmc_clkPha->u8_eightPha_TX << 9 | pmmc_clkPha->eClkPha_TX << 1
                                            | pmmc_clkPha->u8_eightPha_RX << 10 | pmmc_clkPha->eClkPha_RX << 3);
                    }
                    else
                    {
                        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT08_T); // 4 phase
                        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO),
                                        pmmc_clkPha->eClkPha_TX << 1 | pmmc_clkPha->eClkPha_RX << 3);
                    }
                }
                break;
            case CLK2_D: // 32000KHz-real is 32.768MHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT13_T); // 1
                break;
            case CLK2_B:                                                                        // 300KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT14_T); // 2
                break;
                // case CLK2_F: // 200000KHz-real is 196.9MHz
                //     CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT14_T | BIT13_T); // 3
                //     break;
        }
        if (u32ClkFromIPSet != CLK2_C)
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT15_T); // select sd_clk
    }
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
    if (eIP == IP_ORDER_0)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT08_T);
        // RST low & high
        CARD_REG_SETBIT(A_BOOT_MOD_REG(eIP), BIT01_T);
        CARD_REG_CLRBIT(A_BOOT_MOD_REG(eIP), BIT00_T);
        Hal_Timer_mDelay(u16DelayMs);
        CARD_REG_SETBIT(A_BOOT_MOD_REG(eIP), BIT00_T);
        CARD_REG_CLRBIT(A_BOOT_MOD_REG(eIP), BIT01_T);
        Hal_Timer_mDelay(5);
    }
    else if (eIP == IP_ORDER_1)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT04_T);
        // RST low & high
        CARD_REG_SETBIT(A_BOOT_MOD_REG(eIP), BIT01_T);
        CARD_REG_CLRBIT(A_BOOT_MOD_REG(eIP), BIT00_T);
        Hal_Timer_mDelay(u16DelayMs);
        CARD_REG_SETBIT(A_BOOT_MOD_REG(eIP), BIT00_T);
        CARD_REG_CLRBIT(A_BOOT_MOD_REG(eIP), BIT01_T);
        Hal_Timer_mDelay(5);
    }
}
