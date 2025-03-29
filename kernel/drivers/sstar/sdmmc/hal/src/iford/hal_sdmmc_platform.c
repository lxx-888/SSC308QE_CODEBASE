/*
 * hal_sdmmc_platform.c- Sigmastar
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
 * FileName hal_sdmmc_platform.c
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

#include "gpio.h"
#include "hal_sdmmc_base.h"
#if (D_OS == D_OS__LINUX)
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include "drv_gpio.h"
#include "drv_sdmmc_lnx.h"
#elif (D_OS == D_OS__RTK)
#include "cam_os_wrapper.h"
#include "drv_sdmmc_rtk.h"
#endif
#include "hal_sdmmc_timer.h"
#include "hal_sdmmc_regs.h"
#include "hal_sdmmc_platform.h"
#include "hal_sdmmc_platform_regs.h"
#include "hal_sdmmc_platform_pri_config.h"

//***********************************************************************************************************
#if (D_OS == D_OS__LINUX)
#define pr_sd_err(fmt, arg...) printk(fmt, ##arg)
#elif (D_OS == D_OS__RTK)
#define pr_sd_err(fmt, arg...) CamOsPrintf(KERN_EMERG fmt, ##arg)
#endif
#define UNUSED(x) (x = x)

volatile BusTimingEmType ge_BusTiming[SDMMC_NUM_TOTAL]  = {0};
U8_T                     gu8_Enable_C2[SDMMC_NUM_TOTAL] = {0, 0, 0};
extern struct clk *      gp_clkSlot[SDMMC_NUM_TOTAL];
extern struct clk *      gp_clkpmmcuSlot[SDMMC_NUM_TOTAL];
extern volatile CardType geCardType[SDMMC_NUM_TOTAL];
#if defined(CONFIG_SUPPORT_SD30) || defined(CONFIG_SUPPORT_EMMC50)
extern struct clk_hw *CLK_IP_1x_p[SDMMC_NUM_TOTAL];
extern struct clk_hw *CLK_IP_2x_p[SDMMC_NUM_TOTAL];
#endif
//***********************************************************************************************************
// IP Setting for Card Platform
void Hal_CARD_IPOnceSetting(MMCPadMuxInfo mmc_PMuxInfo)
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;

    // MCG OFF
    CARD_REG_SETBIT(A_CLK_EN_REG(eIpSel), R_MCG_DISABLE);
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
    }
    else if (eIpSel == IP_SDIO)
    {
#if (D_OS == D_OS__LINUX)
        if (!IS_ERR_OR_NULL(gp_clkpmmcuSlot[eIpSel]))
            clk_set_rate(gp_clkpmmcuSlot[eIpSel], CLK_G); // miu clk=250MHz, default is 12MHz
#endif
    }
#if (D_OS == D_OS__LINUX)
    clk_set_rate(gp_clkSlot[eIpSel], CLK_6);
#elif (D_OS == D_OS__RTK)
    CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT06_T);
#endif
}

#if (PADMUX_SET == PADMUX_SET_BY_REG)
void Hal_CARD_ConfigSdPad(MMCPadMuxInfo mmc_PMuxInfo) // Hal_CARD_SetPADToPortPath
{
    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;

    if (eIpSel == IP_SD)
    {
        if (ePadSel == PAD_SD_MD1)
        {
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61),
                            BIT12_T | BIT11_T | BIT10_T | BIT09_T | BIT08_T | BIT04_T | BIT05_T | BIT01_T | BIT00_T);
            if (geCardType[mmc_PMuxInfo.u8_ipOrder] == CARD_TYPE_EMMC)
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT10_T | BIT08_T);
            else
            {
                // c2 setting
                gu8_Enable_C2[eIpSel] = 1;
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61),
                                BIT00_T | BIT04_T); //[B0:B4]/[reg_sd0_mode:reg_sd0_cdz_mode]
            }
        }
        else if (ePadSel == PAD_SD_MD2)
        {
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61),
                            BIT12_T | BIT11_T | BIT10_T | BIT09_T | BIT08_T | BIT04_T | BIT05_T | BIT01_T | BIT00_T);
            if (geCardType[mmc_PMuxInfo.u8_ipOrder] == CARD_TYPE_EMMC)
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT12_T);
            else
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61),
                                BIT01_T | BIT05_T); //[B0:B4]/[reg_sd0_mode:reg_sd0_cdz_mode]
        }
    }
    else if (eIpSel == IP_SDIO)
    {
        // SD mode reg_all_pad_in
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x60), BIT00_T);
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52),
                        BIT04_T | BIT05_T | BIT08_T | BIT09_T | BIT13_T | BIT12_T);
        if (ePadSel == PAD_SD_MD1)
        {
            if (geCardType[mmc_PMuxInfo.u8_ipOrder] == CARD_TYPE_EMMC)
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52),
                                BIT04_T | BIT08_T); //[B9]/[reg_sd0_mode+reg_sd0_cdz_mode]
            else
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52),
                                BIT12_T | BIT08_T); //[B9]/[reg_sd0_mode+reg_sd0_cdz_mode]
        }
        else if (ePadSel == PAD_SD_MD2)
        {
            if (geCardType[mmc_PMuxInfo.u8_ipOrder] == CARD_TYPE_EMMC)
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52),
                                BIT05_T | BIT09_T); //[B9]/[reg_sd0_mode+reg_sd0_cdz_mode]
            else
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52),
                                BIT13_T | BIT09_T); //[B9]/[reg_sd0_mode+reg_sd0_cdz_mode]
        }
        else if (ePadSel == PAD_SD_MD3)
        {
            if (geCardType[mmc_PMuxInfo.u8_ipOrder] == CARD_TYPE_EMMC)
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52),
                                BIT05_T | BIT04_T | BIT09_T | BIT08_T); //[B9]/[reg_sd0_mode+reg_sd0_cdz_mode]
            else
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52),
                                BIT13_T | BIT12_T | BIT09_T | BIT08_T); //[B9]/[reg_sd0_mode+reg_sd0_cdz_mode]
        }
    }
}

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
        else if (ePadSel == PAD_SD_MD2)
        {
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2B), BIT06_T | BIT11_T); // CDZ
            // reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2F), BIT04_T); // CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2D), BIT04_T); // D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT04_T); // D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x31), BIT04_T); // D2
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x30), BIT04_T); // D3

            // reg_sd0_drv: CLK, D3, D2, D1, D0, CMD => drv: 1 for vCore 0.9V->0.85V
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2E), BIT07_T); // CLK
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2F), BIT07_T); // CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2D), BIT07_T); // D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT07_T); // D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x31), BIT07_T); // D2
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x30), BIT07_T); // D3
        }
    }
    else if (eIpSel == IP_SDIO)
    {
        if (ePadSel == PAD_SD_MD1)
        {
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
        else if (ePadSel == PAD_SD_MD2)
        {
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x17), BIT04_T | BIT05_T); // CDZ
            // reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT04_T); // CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT04_T); // D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x28), BIT04_T); // D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x29), BIT04_T); // D2
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT04_T); // D3

            // reg_sd0_drv: CLK, D3, D2, D1, D0, CMD => drv: 1 for vCore 0.9V->0.85V
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1F), BIT07_T); // CLK
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT07_T); // CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT07_T); // D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x28), BIT07_T); // D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x29), BIT07_T); // D2
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT07_T); // D3
        }
        else if (ePadSel == PAD_SD_MD3)
        {
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0D), BIT04_T | BIT05_T); // CDZ
            // reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x14), BIT04_T); // CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x15), BIT04_T); // D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x16), BIT04_T); // D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0E), BIT04_T); // D2
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0F), BIT04_T); // D3

            // reg_sd0_drv: CLK, D3, D2, D1, D0, CMD => drv: 1 for vCore 0.9V->0.85V
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x13), BIT07_T); // CLK
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x14), BIT07_T); // CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x15), BIT07_T); // D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x16), BIT07_T); // D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0E), BIT07_T); // D2
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0F), BIT07_T); // D3
        }
    }

    if (geCardType[eIpSel] != CARD_TYPE_EMMC)
        Hal_CARD_PullPADPin(mmc_PMuxInfo, EV_PULLDOWN); // Pull Down
}

void Hal_CARD_PullPADPin(MMCPadMuxInfo mmc_PMuxInfo, PinPullEmType ePinPull)
{
    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;

    if (geCardType[eIpSel] == CARD_TYPE_EMMC)
        return;

    // IP_SD
    if (eIpSel == IP_SD)
    {
        if (ePadSel == PAD_SD_MD1) // || ePadSel == PAD_SD_MD2)
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

                // SD Mode
                if (ePadSel == PAD_SD_MD1)
                {
                    // OFF:x67 [B8:B9]reg_sd0_mode [B10]reg_sd0_cdz_mode
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT00_T);
                }
                else
                {
                    // OFF:x67 [B8:B9]reg_sd0_mode [B10]reg_sd0_cdz_mode
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT09_T);
                }
            }
        }
        else if (ePadSel == PAD_SD_MD2)
        {
            if (ePinPull == EV_PULLDOWN)
            {
                // reg_sd0_pe: D3, D2, D1, D0, CMD=> pull dis
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT04_T); // D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2D), BIT04_T); // D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2F), BIT04_T); // CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x30), BIT04_T); // D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x31), BIT04_T); // D2

                // PAD -> GPIO mode
                // OFF:x67 [B8:B9]reg_sd0_mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT01_T | BIT00_T);

                // Output Low
                // SD_ClK
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2E), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2E), BIT01_T);

                // SD_CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2F), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2F), BIT01_T);

                // SD_D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2D), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2D), BIT01_T);

                // SD_D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT01_T);

                // SD_D2
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x31), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x31), BIT01_T);

                // SD_D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x30), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x30), BIT01_T);
            }
            else if (ePinPull == EV_PULLUP)
            {
                // reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT04_T); // D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2D), BIT04_T); // D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2F), BIT04_T); // CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x30), BIT04_T); // D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x31), BIT04_T); // D2

                // Input
                // SD_CLK
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2E), BIT02_T);

                // SD_CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2F), BIT02_T);

                // SD_D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2D), BIT02_T);

                // SD_D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2C), BIT02_T);

                // SD_D2
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x31), BIT02_T);

                // SD_D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x30), BIT02_T);

                // SD Mode
                if (ePadSel == PAD_SD_MD2)
                {
                    // OFF:x61 [B1:B0]reg_sd0_mode [B5:B4]reg_sd0_cdz_mode
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT01_T);
                }
                else
                {
                    // OFF:x67 [B8:B9]reg_sd0_mode [B10]reg_sd0_cdz_mode
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T);
                }
            }
        }
    }

    // IP_SDIO
    if (eIpSel == IP_SDIO)
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

                // need setting this bit to ensure GPIO mode bit0~5:clk cmd d0~3
                // CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x3a),
                //                BIT00_T | BIT01_T | BIT02_T | BIT03_T | BIT04_T | BIT05_T);
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

                // need setting this bit to ensure GPIO mode bit0~5:clk cmd d0~3
                // CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x3a),
                //                BIT00_T | BIT01_T | BIT02_T | BIT03_T | BIT04_T | BIT05_T);

                // SD Mode
                if (ePadSel == PAD_SD_MD1)
                {
                    // OFF:x67 [B8:B9]reg_sd0_mode [B10]reg_sd0_cdz_mode
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT08_T);
                }
                else
                {
                    // OFF:x67 [B8:B9]reg_sd0_mode [B10]reg_sd0_cdz_mode
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x67), BIT09_T);
                }
            }
        }
        else if (ePadSel == PAD_SD_MD2)
        {
            if (ePinPull == EV_PULLDOWN)
            {
                // reg_sd0_pe: D3, D2, D1, D0, CMD=> pull dis
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x28), BIT04_T); // D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT04_T); // D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT04_T); // CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT04_T); // D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x29), BIT04_T); // D2

                // PAD -> GPIO mode
                // OFF:x67 [B8:B9]reg_sd0_mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT09_T);

                // Output Low
                // SD_ClK
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1F), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1F), BIT01_T);

                // SD_CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT01_T);

                // SD_D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT01_T);

                // SD_D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x28), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x28), BIT01_T);

                // SD_D2
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x29), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x29), BIT01_T);

                // SD_D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT01_T);

                // need setting this bit to ensure GPIO mode bit0~5:clk cmd d0~3
                // CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x3a),
                //                BIT00_T | BIT01_T | BIT02_T | BIT03_T | BIT04_T | BIT05_T);
            }
            else if (ePinPull == EV_PULLUP)
            {
                // reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x28), BIT04_T); // D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT04_T); // D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT04_T); // CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT04_T); // D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x29), BIT04_T); // D2

                // Input
                // SD_CLK
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1F), BIT02_T);

                // SD_CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1E), BIT02_T);

                // SD_D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x20), BIT02_T);

                // SD_D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x28), BIT02_T);

                // SD_D2
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x29), BIT02_T);

                // SD_D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x21), BIT02_T);

                // need setting this bit to ensure GPIO mode bit0~5:clk cmd d0~3
                // CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x3a),
                //                BIT00_T | BIT01_T | BIT02_T | BIT03_T | BIT04_T | BIT05_T);

                // SD Mode
                if (ePadSel == PAD_SD_MD2)
                {
                    // OFF:x67 [B8:B9]reg_sd0_mode [B10]reg_sd0_cdz_mode
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT09_T);
                }
                else
                {
                    // OFF:x67 [B8:B9]reg_sd0_mode [B10]reg_sd0_cdz_mode
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x67), BIT09_T);
                }
            }
        }
        else if (ePadSel == PAD_SD_MD3)
        {
            if (ePinPull == EV_PULLDOWN)
            {
                // reg_sd0_pe: D3, D2, D1, D0, CMD=> pull dis
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x16), BIT04_T); // D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x15), BIT04_T); // D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x14), BIT04_T); // CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0F), BIT04_T); // D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0E), BIT04_T); // D2

                // PAD -> GPIO mode
                // OFF:x67 [B8:B9]reg_sd0_mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT09_T | BIT08_T);

                // Output Low
                // SD_ClK
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x13), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x13), BIT01_T);

                // SD_CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x14), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x14), BIT01_T);

                // SD_D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x15), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x15), BIT01_T);

                // SD_D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x16), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x16), BIT01_T);

                // SD_D2
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0E), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0E), BIT01_T);

                // SD_D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0F), BIT02_T);
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0F), BIT01_T);

                // need setting this bit to ensure GPIO mode bit0~5:clk cmd d0~3
                // CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x3a),
                //                BIT00_T | BIT01_T | BIT02_T | BIT03_T | BIT04_T | BIT05_T);
            }
            else if (ePinPull == EV_PULLUP)
            {
                // reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x16), BIT04_T); // D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x15), BIT04_T); // D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x14), BIT04_T); // CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0F), BIT04_T); // D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0E), BIT04_T); // D2

                // Input
                // SD_CLK
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x13), BIT02_T);

                // SD_CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x14), BIT02_T);

                // SD_D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x15), BIT02_T);

                // SD_D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x16), BIT02_T);

                // SD_D2
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0E), BIT02_T);

                // SD_D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0F), BIT02_T);

                // need setting this bit to ensure GPIO mode bit0~5:clk cmd d0~3
                // CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x3a),
                //                BIT00_T | BIT01_T | BIT02_T | BIT03_T | BIT04_T | BIT05_T);

                // SD Mode
                if (ePadSel == PAD_SD_MD3)
                {
                    // OFF:x67 [B8:B9]reg_sd0_mode [B10]reg_sd0_cdz_mode
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT09_T | BIT08_T);
                }
                else
                {
                    // OFF:x67 [B8:B9]reg_sd0_mode [B10]reg_sd0_cdz_mode
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x67), BIT09_T);
                }
            }
        }
    }
}

//***********************************************************************************************************
// Signal line driving control Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_DrvCtrlPin(MMCPadMuxInfo mmc_PMuxInfo, MMCPinDrv mmc_PinDrv)
{
#if (D_OS == D_OS__LINUX)
    U8_T u8_PadClk = 0, u8_ClkDrvMax = DRV_CTRL_4;
    U8_T u8_PadCmd = 0, u8_CmdDrvMax = DRV_CTRL_4;
    U8_T u8_PadData[8] = {0}, u8_DataDrvMax = DRV_CTRL_4, i;

    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;

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
        if (ePadSel == PAD_SD_MD1)
        {
            u8_PadClk     = PAD_PM_SDIO_CLK;
            u8_PadCmd     = PAD_PM_SDIO_CMD;
            u8_PadData[0] = PAD_PM_SDIO_D0;
            u8_PadData[1] = PAD_PM_SDIO_D1;
            u8_PadData[2] = PAD_PM_SDIO_D2;
            u8_PadData[3] = PAD_PM_SDIO_D3;
        }
    }

    // driving control switch
    if (mmc_PinDrv.eDrvClk != DRV_NOSET)
        sstar_gpio_drv_set(u8_PadClk, mmc_PinDrv.eDrvClk > u8_ClkDrvMax ? u8_ClkDrvMax : mmc_PinDrv.eDrvClk);

    if (mmc_PinDrv.eDrvCmd != DRV_NOSET)
        sstar_gpio_drv_set(u8_PadCmd, mmc_PinDrv.eDrvCmd > u8_CmdDrvMax ? u8_CmdDrvMax : mmc_PinDrv.eDrvCmd);

    if (mmc_PinDrv.eDrvData != DRV_NOSET)
    {
        for (i = 0; i < mmc_PMuxInfo.u8_busWidth; i++)
            sstar_gpio_drv_set(u8_PadData[i],
                               mmc_PinDrv.eDrvData > u8_DataDrvMax ? u8_DataDrvMax : mmc_PinDrv.eDrvData);
    }
#endif
}

//***********************************************************************************************************
// Power and Voltage Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_ConfigPowerPad(MMCPadMuxInfo mmc_PMuxInfo)
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }
    if (geCardType[eIpSel] == CARD_TYPE_EMMC)
        return;

    switch (mmc_PMuxInfo.u32_PinPWR)
    {
        case PAD_PM_GPIO12:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0), (BIT02_T | BIT01_T | BIT00_T));
            break;
        case PAD_PM_GPIO11:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1), (BIT02_T | BIT01_T | BIT00_T));
            break;

        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            return;
            break;
    }

    // Default power off
    Hal_CARD_PowerOff(mmc_PMuxInfo, 0);
}

void Hal_CARD_PowerOn(MMCPadMuxInfo mmc_PMuxInfo, U16_T u16DelayMs)
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }
    if (geCardType[eIpSel] == CARD_TYPE_EMMC)
        return;

    if (mmc_PMuxInfo.u32_PinPWR == PAD_UNKNOWN)
    {
        // Maybe we don't use power pin.
        return;
    }

    switch (mmc_PMuxInfo.u32_PinPWR)
    {
        case PAD_PM_GPIO12:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0), BIT02_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0), BIT01_T);
            break;
        case PAD_PM_GPIO11:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1), BIT02_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1), BIT01_T);
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
    if (geCardType[eIpSel] == CARD_TYPE_EMMC)
        return;

    if (mmc_PMuxInfo.u32_PinPWR == PAD_UNKNOWN)
    {
        // Maybe we don't use power pin.
        return;
    }

    switch (mmc_PMuxInfo.u32_PinPWR)
    {
        case PAD_PM_GPIO12:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0), BIT02_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x0), BIT01_T);
            break;
        case PAD_PM_GPIO11:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1), BIT02_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x1), BIT01_T);
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

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }
    if (geCardType[eIpSel] == CARD_TYPE_EMMC)
        return;

    // PADMUX
    switch (mmc_PMuxInfo.u32_PinCdzRst)
    {
        case PAD_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT05_T);
            break;
        case PAD_PM_SDIO_INT:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT12_T);
            break;
        case PAD_PM_GPIO8:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT12_T | BIT13_T);
            break;
        case PAD_PM_PSPI0_INT:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT13_T);
            break;
        case PAD_SD0_CDZ:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT04_T);
            break;

        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            return;
            break;
    }
}

BOOL_T Hal_CARD_GetCdzState(MMCPadMuxInfo mmc_PMuxInfo) // Hal_CARD_GetGPIOState
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    U8_T     nLv    = 0;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        goto fail;
    }

    if (mmc_PMuxInfo.u32_PinCdzRst == PAD_UNKNOWN)
    {
        // Maybe we don't use CDZ pin.
        goto fail;
    }

    switch (mmc_PMuxInfo.u32_PinCdzRst)
    {
        case PAD_GPIO0:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x2B)) & BIT00_T;
            break;
        case PAD_PM_SDIO_INT:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x11)) & BIT00_T;
            break;
        case PAD_PM_GPIO8:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x08)) & BIT00_T;
            break;
        case PAD_PM_PSPI0_INT:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PM_PADGPIO_BANK, 0x04)) & BIT00_T;
            break;
        case PAD_SD0_CDZ:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x14)) & BIT00_T;
            break;

        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            goto fail;
            break;
    }

    if (!nLv) // Low Active
    {
        return TRUE;
    }

fail:

    return FALSE;
}

U32_T Hal_CARD_CheckCdzMode(MMCPadMuxInfo mmc_PMuxInfo)
{
    return 1;
}
#endif

#if defined(CONFIG_SUPPORT_SD30)
static BOOL_T _PLTSD_PLL_Switch_AVDD(MMCPadMuxInfo mmc_PMuxInfo, PADVddEmType ePADVdd)
{
    if (mmc_PMuxInfo.u8_ipOrder == IP_ORDER_0) // SD3.0
    {
        if (ePADVdd == EV_NORVOL)
        {
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_SDPLL_BANK, 0X2E), BIT01_T); // MS = 0
            Hal_Timer_mSleep(5);
            CARD_REG(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x9)) = 0x0008; // _VCTRL = 0
        }
        else if (ePADVdd == EV_LOWVOL)
        {
            CARD_REG(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x9)) = 0x000A; // _VCTRL = 1
            Hal_Timer_mSleep(5);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SDPLL_BANK, 0X2E), BIT01_T); // MS = 1
            Hal_Timer_mSleep(10);
        }
    }
    else if ((mmc_PMuxInfo.u8_ipOrder == IP_ORDER_1)) // SDIO3.0
    {
        // Do nothing
    }

    return 0;
}

void _PLTSD_PLL_1XClock_Setting(IpSelect eIP, U32_T u32ClkFromIPSet)
{
    CARD_REG_H8(GET_CARD_REG_ADDR(A_CLKGEN_BANK, 0x44)) = 0x14;

    clk_set_rate(CLK_IP_1x_p[eIP]->clk, u32ClkFromIPSet);

    Hal_Timer_uDelay(100);
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_SetPADVdd
 *     @author jeremy.wang (2018/1/29)
 * Desc:
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param ePAD : PAD
 * @param ePADVdd :
 * @param u16DelayMs : Delay ms after set PAD power
 *
 * @return BOOL_T  :
 ----------------------------------------------------------------------------------------------------------*/
BOOL_T Hal_CARD_SetPADVdd(MMCPadMuxInfo mmc_PMuxInfo, PADVddEmType ePADVdd, U16_T u16DelayMs)
{
    BOOL_T bRet = FALSE;

    bRet = _PLTSD_PLL_Switch_AVDD(mmc_PMuxInfo, ePADVdd);

    if (mmc_PMuxInfo.u8_padOrder == PAD_ORDER_0) // Pad SD0
    {
    }
    else if (mmc_PMuxInfo.u8_padOrder == PAD_ORDER_1) // PAD_SD1
    {
    }

    Hal_Timer_mSleep(u16DelayMs);

    return bRet;
}
#endif

//***********************************************************************************************************
// Clock & BusTiming Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_SetBustiming(MMCPadMuxInfo *mmc_PMuxInfo, BusTimingEmType eBusTiming)
{
    IpOrder eIP = mmc_PMuxInfo->u8_ipOrder;
    pr_sd_err("SDMMC%d >> [Hal_CARD_SetBustiming] %s mode. <<\r\n", eIP,
              (eBusTiming == EV_BUS_LOW)      ? "LS"
              : (eBusTiming == EV_BUS_DEF)    ? "DEFS"
              : (eBusTiming == EV_BUS_HS)     ? "HS"
              : (eBusTiming == EV_BUS_SDR50)  ? "SDR50"
              : (eBusTiming == EV_BUS_SDR104) ? "SDR104"
              : (eBusTiming == EV_BUS_DDR50)  ? "DDR50"
              : (eBusTiming == EV_BUS_HS200)  ? "HS200"
              : (eBusTiming == EV_BUS_HS400)  ? "HS400"
                                              : "Unknow");

    //    fcie
    CARD_REG_CLRBIT(A_DDR_MOD_REG(eIP), R_PAD_IN_BYPASS | R_CIFD_MODE_MASK | R_MACRO_MODE_MASK);

    switch (eBusTiming)
    {
        case EV_BUS_LOW:
        case EV_BUS_DEF:
        case EV_BUS_HS:
// fcie
#if defined(CONFIG_SSTAR_SDMMC_BYPASS_MODE)
            CARD_REG_SETBIT(A_DDR_MOD_REG(eIP), R_PAD_IN_BYPASS);
#else
            CARD_REG_SETBIT(A_DDR_MOD_REG(eIP), R_PAD_IN_SEL | (gu8_Enable_C2[eIP] == 1 ? R_PAD_CLK_SEL : 0));
#endif
            Hal_CARD_PllSetting(mmc_PMuxInfo, eBusTiming);
            break;

        case EV_BUS_SDR50:
        case EV_BUS_SDR104:
            CARD_REG_W(A_DDR_MOD_REG(eIP), R_DDR_MACRO32_EN);
            Hal_CARD_PllSetting(mmc_PMuxInfo, eBusTiming);
            break;

        case EV_BUS_DDR50:
            CARD_REG_W(A_DDR_MOD_REG(eIP), R_DDR_MACRO_EN | R_DDR_EN);
            Hal_CARD_PllSetting(mmc_PMuxInfo, eBusTiming);
            break;

        case EV_BUS_HS200:
            CARD_REG_W(A_DDR_MOD_REG(eIP), R_DDR_MACRO32_EN);
            CARD_REG_L8(GET_CARD_REG_ADDR(A_CLKGEN_BANK, 0x4D))  = 0x14;
            CARD_REG(GET_CARD_REG_ADDR(A_SD_PLL_POS(eIP), 0x04)) = 0x6;
            Hal_CARD_PllSetting(mmc_PMuxInfo, eBusTiming);
            break;

        case EV_BUS_HS400:
            CARD_REG_W(A_DDR_MOD_REG(eIP), R_DDR_MACRO32_EN | R_DDR_EN);
            CARD_REG_L8(GET_CARD_REG_ADDR(A_CLKGEN_BANK, 0x4D)) = 0x14;
            Hal_CARD_PllSetting(mmc_PMuxInfo, eBusTiming);
            break;

        default:
            pr_sd_err("SDMMC%d Err: wrong parameter for switch bus timing: %d\n", eIP, eBusTiming);
    }
    ge_BusTiming[eIP] = eBusTiming;
}

void Hal_CARD_PllSetting(MMCPadMuxInfo *mmc_PMuxInfo, BusTimingEmType eBusTiming)
{
#if (D_OS == D_OS__LINUX)
    IpOrder                eIP  = mmc_PMuxInfo->u8_ipOrder;
    struct sstar_mmc_priv *priv = container_of(mmc_PMuxInfo, struct sstar_mmc_priv, mmc_PMuxInfo);

    if ((priv->u8_supportEMMC50 != 1) && (priv->u8_supportSD30 != 1))
        return;

#if defined(CONFIG_SUPPORT_SD30) || defined(CONFIG_SUPPORT_EMMC50)
    switch (eBusTiming)
    {
        case EV_BUS_LOW:
        case EV_BUS_DEF:
        case EV_BUS_HS:
            clk_set_rate(CLK_IP_1x_p[eIP]->clk, EV_BUS_HS);
            break;

        case EV_BUS_SDR50:
        case EV_BUS_SDR104:
            clk_set_parent(gp_clkSlot[eIP], CLK_IP_1x_p[eIP]->clk);
            clk_set_rate(CLK_IP_1x_p[eIP]->clk, EV_BUS_SDR104);
            break;

        case EV_BUS_DDR50:
            clk_set_parent(gp_clkSlot[eIP], CLK_IP_1x_p[eIP]->clk);
            clk_set_rate(CLK_IP_1x_p[eIP]->clk, eBusTiming);
            break;

        case EV_BUS_HS200:
            clk_set_parent(gp_clkSlot[eIP], CLK_IP_1x_p[eIP]->clk);
            clk_set_rate(CLK_IP_1x_p[eIP]->clk, mmc_PMuxInfo->u8_busWidth);
            break;

        case EV_BUS_HS400:
            clk_set_parent(gp_clkSlot[eIP], CLK_IP_2x_p[eIP]->clk);
            clk_set_rate(CLK_IP_2x_p[eIP]->clk, mmc_PMuxInfo->u8_busWidth);
            break;

        default:
            pr_err("SDMMC%d Err: wrong parameter for switch bus timing: %d\n", eIP, eBusTiming);
    }
    Hal_Timer_uDelay(100); // asked by Irwin
#else
    UNUSED(eIP);
#endif
#endif
}

//***********************************************************************************************************
// Clock Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_SetClock(MMCPadMuxInfo *mmc_PMuxInfo, U32_T u32ClkFromIPSet)
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo->u8_ipOrder;

#if defined(CONFIG_SUPPORT_SD30)
    if ((ge_BusTiming[eIpSel] == EV_BUS_SDR50) || (ge_BusTiming[eIpSel] == EV_BUS_SDR104)
        || (ge_BusTiming[eIpSel] == EV_BUS_DDR50))
    {
        // select clk_sd3.0 settting
        _PLTSD_PLL_1XClock_Setting(eIpSel, u32ClkFromIPSet);
        return;
    }
#endif
#if (D_OS == D_OS__LINUX)
    struct sstar_mmc_priv *priv = container_of(mmc_PMuxInfo, struct sstar_mmc_priv, mmc_PMuxInfo);

    if (eIpSel == IP_SDIO)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT05_T | BIT00_T);
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO),
                        BIT10_T | BIT09_T | BIT04_T | BIT03_T | BIT02_T | BIT01_T);
        if (u32ClkFromIPSet == CLK_DU)
        {
            if (priv->mmc_clkPha.u8_clkPhaEn)
            {
                u32ClkFromIPSet = CLK_G; // rtcpll={200MHz, 216MHz, 250MHz}, ccf Align down
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO),
                                BIT05_T | BIT00_T); // rx & tx enable
                if (priv->mmc_clkPha.u8_eightPhaEn)
                {
                    CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT08_T); // 5 phase
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO),
                                    priv->mmc_clkPha.u8_eightPha_TX << 9 | priv->mmc_clkPha.eClkPha_TX << 1
                                        | priv->mmc_clkPha.u8_eightPha_RX << 10 | priv->mmc_clkPha.eClkPha_RX << 3);
                }
                else
                {
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT08_T); // 4 phase
                    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO),
                                    priv->mmc_clkPha.eClkPha_TX << 1 | priv->mmc_clkPha.eClkPha_RX << 3);
                }
            }
        }
    }

    clk_set_rate(gp_clkSlot[eIpSel], u32ClkFromIPSet);
#elif (D_OS == D_OS__RTK)

    if (eIpSel == IP_SD)
    {
        // clr gating
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT00_T);
        // clr bit6 for glitch free selct
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT06_T);
        // clr clk_sel
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT05_T | BIT04_T | BIT03_T | BIT02_T);

        switch (u32ClkFromIPSet)
        {
            case CLK_D: // 48000KHz
                break;
            case CLK_C:                                                                    // 43200KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT02_T); // 1
                break;
            case CLK_B:                                                                    // 40000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT03_T); // 2
                break;
            case CLK_A:                                                                              // 36000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT03_T | BIT02_T); // 3
                break;
            case CLK_9:                                                                    // 32000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT04_T); // 4
                break;
            case CLK_8:                                                                              // 20000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT04_T | BIT02_T); // 5
                break;
            case CLK_7:                                                                              // 12000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD), BIT04_T | BIT03_T); // 6
                break;
            case CLK_6:                                                                                        // 300KHz
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
            case CLK_D: // 48000KHz-real is 49.15MHz
                break;
            case CLK_9: // 32000KHz-real is 32.768MHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT13_T); // 1
                break;
            case CLK_6:                                                                         // 300KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT14_T); // 2
                break;
                // case CLK2_F: // 200000KHz-real is 196.9MHz
                //     CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT14_T | BIT13_T); // 3
                //     break;
        }

        if (u32ClkFromIPSet != CLK_7)
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT15_T); // select sd_clk
    }

#endif
}

U32_T Hal_CARD_FindClockSetting(IpOrder eIP, U32_T u32ReffClk)
{
    U8_T  u8LV          = 0;
    U32_T u32RealClk    = 0;
    U64_T u32ClkArr[16] = {CLK_F, CLK_E, CLK_DU, CLK_D, CLK_C, CLK_B, CLK_A, CLK_9U,
                           CLK_9, CLK_8, CLK_7,  CLK_6, CLK_5, CLK_4, CLK_3, CLK_2};

    for (; u8LV < 16; u8LV++)
    {
        if ((u32ReffClk >= u32ClkArr[u8LV]) || (u8LV == 15) || (u32ClkArr[u8LV + 1] == 0))
        {
            u32RealClk = u32ClkArr[u8LV];
            break;
        }
    }

    return u32RealClk;
}

#ifdef CONFIG_PM_SLEEP
//***********************************************************************************************************
// Get pm clock from Card Platform
//***********************************************************************************************************
void Hal_CARD_devpm_GetClock(IpOrder eIP, U32_T *pu32PmIPClk, U32_T *pu32PmBlockClk)
{
    IpSelect eIpSel = (IpSelect)eIP;

    *pu32PmBlockClk = CARD_REG(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK, 0x25));

    if (eIpSel == IP_SD)
    {
        *pu32PmIPClk = CARD_REG(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD));
    }
    else if (eIpSel == IP_SDIO)
    {
        *pu32PmIPClk = CARD_REG(GET_CARD_REG_ADDR(A_PM_CLKGEN_BANK, REG_CLK_IP_SDIO));
    }
}

//***********************************************************************************************************
// Set pm clock to Card Platform
//***********************************************************************************************************
void Hal_CARD_devpm_setClock(IpOrder eIP, U32_T u32PmIPClk, U32_T u32PmBlockClk)
{
    IpSelect eIpSel = (IpSelect)eIP;

    CARD_REG(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK, 0x25)) = u32PmBlockClk;

    if (eIpSel == IP_SD)
    {
    }
    else if (eIpSel == IP_SDIO)
    {
        // CARD_REG(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO)) = u32PmIPClk;
    }
}
#endif

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

#elif (D_OS == D_OS__RTK)
    return (dma_addr_t)CamOsMemPhysToMiu(CamOsMemVirtToPhys((void *)ptr_Addr)); // Convert to physical address for DM
#else
    u64 U64MiuBase = 0x20000000;
    if (ptr_Addr >= 0X1000000000)
        U64MiuBase = 0x1000000000;
    return ptr_Addr - U64MiuBase;

#endif
}

#if (D_OS == D_OS__LINUX)
//***********************************************************************************************************
// Hard Ware Resetting for eMMC Card
//***********************************************************************************************************
void Hal_eMMC_HardWare_Reset(MMCPadMuxInfo mmc_PMuxInfo)
{
    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;

    if (eIpSel == IP_SD)
    {
        if ((ePadSel == PAD_SD_MD1) || (ePadSel == PAD_SD_MD3))
        {
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT08_T);
        }
        else if (ePadSel == PAD_SD_MD2)
        {
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT09_T);
        }
    }
    if (eIpSel == IP_SDIO)
    {
        if (ePadSel == PAD_SD_MD1)
        {
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT04_T);
        }
        else if (ePadSel == PAD_SD_MD2)
        {
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT05_T);
        }
        else if (ePadSel == PAD_SD_MD3)
        {
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT05_T | BIT04_T);
        }
    }

    // RST low & high
    CARD_REG_SETBIT(A_BOOT_MOD_REG(eIpSel), BIT01_T);
    CARD_REG_CLRBIT(A_BOOT_MOD_REG(eIpSel), BIT00_T);
    Hal_Timer_mDelay(5);
    CARD_REG_SETBIT(A_BOOT_MOD_REG(eIpSel), BIT00_T);
    CARD_REG_CLRBIT(A_BOOT_MOD_REG(eIpSel), BIT01_T);
    Hal_Timer_mDelay(5);
    return;
}

// Check clk and cmd line's driving control is setting by the same reg?
BOOL_T Hal_Check_ClkCmd_Interrelate(IpOrder eIP, PadOrder ePAD)
{
    return FALSE;
}

//***********************************************************************************************************
// Error Status Set or Clear
//***********************************************************************************************************
void SDMMC_Read_Timeout_Set(MMCPadMuxInfo mmc_PMuxInfo)
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;

    if (eIpSel == IP_SD)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT10_T | BIT00_T);
    }
    else if (eIpSel == IP_SDIO)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT09_T | BIT08_T);
    }

    CARD_REG_W(A_RD_SBIT_TIMER_REG(eIpSel), 0x8001);
}

void SDMMC_Read_Timeout_Clear(MMCPadMuxInfo mmc_PMuxInfo)
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;

    if (eIpSel == IP_SD)
    {
        if (geCardType[mmc_PMuxInfo.u8_ipOrder] == CARD_TYPE_EMMC)
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT10_T);
        else
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT00_T);
    }
    else if (eIpSel == IP_SDIO)
    {
        if (geCardType[mmc_PMuxInfo.u8_ipOrder] == CARD_TYPE_EMMC)
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT09_T);
        else
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_PADTOP_BANK, 0x52), BIT08_T);
    }

    CARD_REG_W(A_RD_SBIT_TIMER_REG(eIpSel), 0x0);
}

void SDMMC_Write_Timeout_Set(IpOrder eIP)
{
    if ((IpSelect)eIP == IP_SD)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_MIU_BANK, 0x33), BIT14_T); // mask fcie read miu
    }
    else if ((IpSelect)eIP == IP_SDIO)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_MIU_BANK, 0x33), BIT15_T); // mask fcie read miu
    }

    Hal_Timer_mSleep(1);
    CARD_REG_W(A_WR_SBIT_TIMER_REG(eIP), 0x8001);
}

void SDMMC_Write_Timeout_Clear(IpOrder eIP)
{
    if ((IpSelect)eIP == IP_SD)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_MIU_BANK, 0x33), BIT14_T); // mask fcie read miu
    }
    else if ((IpSelect)eIP == IP_SDIO)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_MIU_BANK, 0x33), BIT15_T); // mask fcie read miu
    }
    CARD_REG_W(A_WR_SBIT_TIMER_REG(eIP), 0x0);
}
#endif
