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
#include "mdrv_gpio.h"
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
#define EN_BYPASSMODE (FALSE) // BYPASS MODE or ADVANCE MODE(SDR/DDR)
#define UNUSED(x)     (x = x)

volatile BusTimingEmType ge_BusTiming[SDMMC_NUM_TOTAL]  = {0};
U8_T                     gu8_Enable_C2[SDMMC_NUM_TOTAL] = {1, 1, 1};
extern U32_T             gu32_SdmmcClk[SDMMC_NUM_TOTAL];
extern struct clk *      gp_clkSlot[SDMMC_NUM_TOTAL];
extern volatile CardType geCardType[SDMMC_NUM_TOTAL];
#if defined(CONFIG_SUPPORT_SD30) || defined(CONFIG_SUPPORT_EMMC50)
extern struct clk_hw *CLK_IP_1x_p[SDMMC_NUM_TOTAL];
extern struct clk_hw *CLK_IP_2x_p[SDMMC_NUM_TOTAL];
#endif
//***********************************************************************************************************
// IP Setting for Card Platform
//***********************************************************************************************************
static U32_T HAL_CARD_GET_CLK_REG(IpOrder eIP)
{
    U32_T IPClkRegArr[SDMMC_NUM_TOTAL] = {A_CLK_SOURCE_IP0_REG, A_CLK_SOURCE_IP1_REG, A_CLK_SOURCE_IP2_REG};
    return IPClkRegArr[eIP];
}

static U32_T HAL_CARD_GET_BOOT_CLK(IpOrder eIP)
{
    U32_T IPClkRegArr[SDMMC_NUM_TOTAL] = {R_CKG_SD_VALUE_IP_0, R_CKG_SD_VALUE_IP_1, R_CKG_SD_VALUE_IP_2};
    return IPClkRegArr[eIP];
}

void Hal_CARD_IPOnceSetting(MMCPadMuxInfo mmc_PMuxInfo)
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;

#if (FORCE_SWITCH_PAD)
    // reg_all_pad_in => Close
    CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x50), BIT15_T);
#endif

    // Clock Source
    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK, 0x25), HAL_CARD_GET_BOOT_CLK(eIpSel));
#if 1
    clk_set_rate(gp_clkSlot[eIpSel], 300000);
#else
    CARD_REG_CLRBIT(HAL_CARD_GET_CLK_REG(eIpSel), BIT05_T | BIT04_T | BIT03_T | BIT02_T | BIT01_T);
    CARD_REG_SETBIT(HAL_CARD_GET_CLK_REG(eIpSel), BIT04_T | BIT03_T | BIT02_T);
#endif
    CARD_REG_SETBIT(HAL_CARD_GET_CLK_REG(eIpSel), BIT06_T);
}

#if (PADMUX_SET == PADMUX_SET_BY_REG)
void Hal_CARD_ConfigSdPad(MMCPadMuxInfo mmc_PMuxInfo) // Hal_CARD_SetPADToPortPath
{
    IpSelect  eIpSel  = (IpSelect)mmc_PMuxInfo.u8_ipOrder;
    PadSelect ePadSel = (PadSelect)mmc_PMuxInfo.u8_padOrder;

    if (eIpSel == IP_SD)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT09_T | BIT08_T);
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x7A), BIT01_T | BIT00_T); // clear rst mode
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), (ePadSel + 1)
                                                                    << 8); //[B8:B10]/[reg_sd0_mode:reg_sd0_cdz_mode]
    }
    else if (eIpSel == IP_SDIO)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT14_T | BIT13_T | BIT12_T);
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT06_T | BIT05_T | BIT04_T); // clear rst mode
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), (ePadSel + 1)
                                                                    << 12); //[B12:B00]/[reg_sd0_mode:reg_sd0_cdz_mode]
    }
    else if (eIpSel == IP_FCIE)
    {
        if (ePadSel == PAD_SD_MD1)
        {
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT08_T | BIT04_T | BIT02_T | BIT00_T);
            if (mmc_PMuxInfo.u8_busWidth == 8)
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT02_T);
            else
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT00_T);
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
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_EMMCPLL_BANK, 0x47),
                            BIT11_T | BIT09_T | BIT07_T | BIT05_T | BIT03_T | BIT01_T);
            // reg_sd0_ps:CDZ=> pull up
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_EMMCPLL_BANK, 0x49), BIT11_T);

            // reg_sd0_drv: CLK, D3, D2, D1, D0, CMD => [DS0/DS1/DS2 : 1 0 0]
            CARD_REG(GET_CARD_REG_ADDR(A_EMMCPLL_BANK, 0x45)) = 0x70;
        }
    }
    else if (eIpSel == IP_FCIE)
    {
        if (ePadSel == PAD_SD_MD1)
        {
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_EMMCPLL_BANK, 0x45),
                            BIT10_T | BIT09_T | BIT08_T | BIT06_T | BIT05_T | BIT04_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_EMMCPLL_BANK, 0x45), BIT02_T | BIT01_T | BIT00_T);
        }
    }

    if (geCardType[mmc_PMuxInfo.u8_ipOrder] != CARD_TYPE_EMMC)
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
        // OFF:x67 [B8:B9]reg_sd0_mode
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T | BIT09_T);
        if (ePadSel == PAD_SD_MD1) // || ePadSel == PAD_SD_MD2)
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
        else if (ePadSel == PAD_SD_MD2)
        {
            if (ePinPull == EV_PULLDOWN)
            {
            }
            else if (ePinPull == EV_PULLUP)
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT09_T);
        }
    }

    // IP_SDIO
    else if (eIpSel == IP_SDIO)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT14_T | BIT13_T | BIT12_T);
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

                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT12_T);
            }
        }

        else if (ePadSel == PAD_SD_MD2)
        {
            if (ePinPull == EV_PULLDOWN)
            {
            }
            else if (ePinPull == EV_PULLUP)
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T);
        }
        else if (ePadSel == PAD_SD_MD3)
        {
            if (ePinPull == EV_PULLDOWN)
            {
            }
            else if (ePinPull == EV_PULLUP)
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T | BIT12_T);
        }
    }
    else if (eIpSel == IP_FCIE)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT02_T | BIT00_T);
        if (ePadSel == PAD_SD_MD1) // || ePadSel == PAD_SD_MD2)
        {
            if (ePinPull == EV_PULLDOWN)
            {
            }
            else if (ePinPull == EV_PULLUP)
            {
                // SD Mode
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT00_T);
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
    U8_T u8_PadClk = 0, u8_ClkDrvMax = DRV_CTRL_7;
    U8_T u8_PadCmd = 0, u8_CmdDrvMax = DRV_CTRL_7;
    U8_T u8_PadData[8] = {0}, u8_DataDrvMax = DRV_CTRL_7, i;

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
            u8_PadClk     = PAD_SD1_CLK;
            u8_PadCmd     = PAD_SD1_CMD;
            u8_PadData[0] = PAD_SD1_D0;
            u8_PadData[1] = PAD_SD1_D1;
            u8_PadData[2] = PAD_SD1_D2;
            u8_PadData[3] = PAD_SD1_D3;
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

    switch (mmc_PMuxInfo.u32_PinPWR)
    {
        case PAD_SD0_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), (BIT02_T | BIT01_T | BIT00_T)); // reg_gpio_oen_120
            break;
        case PAD_SD1_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7D), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7D), (BIT02_T | BIT01_T | BIT00_T)); // reg_gpio_oen_125
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

    if (mmc_PMuxInfo.u32_PinPWR == PAD_UNKNOWN)
    {
        // Maybe we don't use power pin.
        return;
    }

    switch (mmc_PMuxInfo.u32_PinPWR)
    {
        case PAD_SD0_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT02_T); // reg_gpio_oen_120
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT01_T); // reg_gpio_out_120
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

    if (mmc_PMuxInfo.u32_PinPWR == PAD_UNKNOWN)
    {
        // Maybe we don't use power pin.
        return;
    }

    switch (mmc_PMuxInfo.u32_PinPWR)
    {
        case PAD_SD0_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT02_T); // reg_gpio_oen_120
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x08), BIT01_T); // reg_gpio_out_120
            break;
        case PAD_SD1_GPIO0:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7D), BIT03_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7D), BIT02_T); // reg_gpio_oen_120
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7D), BIT01_T); // reg_gpio_out_120
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

    // PADMUX
    switch (mmc_PMuxInfo.u32_PinCdzRst)
    {
        case PAD_SD0_CDZ:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT10_T);
            break;
        case PAD_SD1_CDZ:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT00_T);
            break;
        case PAD_EMMC_RSTN:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT08_T);
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
        case PAD_SD0_CDZ:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x0A)) & BIT00_T;
            break;
        case PAD_SD1_CDZ:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PADGPIO_BANK, 0x7F)) & BIT00_T;
            break;
        case PAD_EMMC_RSTN:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PADGPIO2_BANK, 0x10)) & BIT00_T;
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
#if (EN_BYPASSMODE)
            CARD_REG_SETBIT(A_DDR_MOD_REG(eIP), R_PAD_IN_BYPASS);
#else
            CARD_REG_SETBIT(A_DDR_MOD_REG(eIP), R_FALL_LATCH | R_PAD_IN_SEL | R_PAD_CLK_SEL);
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
            gu32_SdmmcClk[eIP] = 200000000;
            break;

        case EV_BUS_HS400:
            CARD_REG_W(A_DDR_MOD_REG(eIP), R_DDR_MACRO32_EN | R_DDR_EN);
            CARD_REG_L8(GET_CARD_REG_ADDR(A_CLKGEN_BANK, 0x4D)) = 0x14;
            Hal_CARD_PllSetting(mmc_PMuxInfo, eBusTiming);
            gu32_SdmmcClk[eIP] = 200000000;
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
    IpSelect               eIpSel = (IpSelect)mmc_PMuxInfo->u8_ipOrder;
    struct sstar_mmc_priv *priv   = container_of(mmc_PMuxInfo, struct sstar_mmc_priv, mmc_PMuxInfo);

#if defined(CONFIG_SUPPORT_SD30)
    if ((ge_BusTiming[eIpSel] == EV_BUS_SDR50) || (ge_BusTiming[eIpSel] == EV_BUS_SDR104)
        || (ge_BusTiming[eIpSel] == EV_BUS_DDR50))
    {
        // select clk_sd3.0 settting
        _PLTSD_PLL_1XClock_Setting(eIpSel, u32ClkFromIPSet);
        return;
    }
#endif
#if 1
    if (eIpSel == IP_SDIO)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO), BIT13_T | BIT08_T);
        if (u32ClkFromIPSet == CLK_E)
        {
            u32ClkFromIPSet = 108000000;
            if (priv->mmc_clkPha.u8_clkPhaEn)
            {
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO),
                                BIT13_T | BIT08_T | priv->mmc_clkPha.eClkPha_TX << 9
                                    | priv->mmc_clkPha.eClkPha_RX << 11);
                u32ClkFromIPSet = u32ClkFromIPSet * 4;
            }
        }
    }

    clk_set_rate(gp_clkSlot[eIpSel], u32ClkFromIPSet);
#else

    CARD_REG_CLRBIT(HAL_CARD_GET_CLK_REG(eIpSel), BIT05_T | BIT04_T | BIT03_T | BIT02_T | BIT01_T | BIT00_T);
    switch (u32ClkFromIPSet)
    {
        case CLK_D: // 48000KHz
            break;
        case CLK_C:                                                 // 43200KHz
            CARD_REG_SETBIT(HAL_CARD_GET_CLK_REG(eIpSel), BIT02_T); // 1
            break;
        case CLK_B:                                                 // 40000KHz
            CARD_REG_SETBIT(HAL_CARD_GET_CLK_REG(eIpSel), BIT03_T); // 2
            break;
        case CLK_A:                                                           // 36000KHz
            CARD_REG_SETBIT(HAL_CARD_GET_CLK_REG(eIpSel), BIT03_T | BIT02_T); // 3
            break;
        case CLK_9:                                                 // 32000KHz
            CARD_REG_SETBIT(HAL_CARD_GET_CLK_REG(eIpSel), BIT04_T); // 4
            break;
        case CLK_8:                                                           // 20000KHz
            CARD_REG_SETBIT(HAL_CARD_GET_CLK_REG(eIpSel), BIT04_T | BIT02_T); // 5
            break;
        case CLK_7:                                                           // 12000KHz
            CARD_REG_SETBIT(HAL_CARD_GET_CLK_REG(eIpSel), BIT04_T | BIT03_T); // 6
            break;
        case CLK_6:                                                                     // 300KHz
            CARD_REG_SETBIT(HAL_CARD_GET_CLK_REG(eIpSel), BIT04_T | BIT03_T | BIT02_T); // 7
            break;
    }

#if 0 // in fpga test
    if (u32ClkFromIPSet <= CLK_6)
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, 0x43), BIT06_T);
    else
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK, 0x43), BIT06_T);
#endif
#endif
}

U32_T Hal_CARD_FindClockSetting(IpOrder eIP, U32_T u32ReffClk)
{
    U8_T  u8LV          = 0;
    U32_T u32RealClk    = 0;
    U64_T u32ClkArr[16] = {CLK_F, CLK_E, CLK_D, CLK_C, CLK_B, CLK_A, CLK_9, CLK_8,
                           CLK_7, CLK_6, CLK_5, CLK_4, CLK_3, CLK_2, CLK_1, CLK_0};

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
        *pu32PmIPClk = CARD_REG(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO));
    }
    else if (eIpSel == IP_FCIE)
    {
        *pu32PmIPClk = CARD_REG(GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_FCIE));
    }
}

//***********************************************************************************************************
// Set pm clock to Card Platform
//***********************************************************************************************************
void Hal_CARD_devpm_setClock(IpOrder eIP, U32_T u32PmIPClk, U32_T u32PmBlockClk)
{
    IpSelect eIpSel = (IpSelect)eIP;

    Hal_CARD_IPOnceSetting(eIP);

    CARD_REG(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK, 0x25)) = u32PmBlockClk;

    if (eIpSel == IP_SD)
    {
#if defined(CONFIG_SUPPORT_SD30)
        Hal_CARD_SetPADVdd(eIP, 0, EV_NORVOL, 1);
#endif
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
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;

    CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT04_T);
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
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T);
    }
    else if (eIpSel == IP_SDIO)
    {
    }
    else if (eIpSel == IP_FCIE)
    {
        switch (mmc_PMuxInfo.u8_busWidth)
        {
            case 4:
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT00_T);
                break;
            case 8:
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT02_T);
                break;
            default:
                pr_err(">> [emmc] Err: wrong buswidth config: %u!\n", mmc_PMuxInfo.u8_busWidth);
                break;
        }
    }
    CARD_REG_W(A_RD_SBIT_TIMER_REG(mmc_PMuxInfo.u8_ipOrder), 0x8001);
}

void SDMMC_Read_Timeout_Clear(MMCPadMuxInfo mmc_PMuxInfo)
{
    IpSelect eIpSel = (IpSelect)mmc_PMuxInfo.u8_ipOrder;

    if (eIpSel == IP_SD)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T);
    }
    else if (eIpSel == IP_SDIO)
    {
    }
    else if (eIpSel == IP_FCIE)
    {
        switch (mmc_PMuxInfo.u8_busWidth)
        {
            case 4:
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT00_T);
                break;
            case 8:
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x61), BIT02_T);
                break;
            default:
                pr_err(">> [emmc] Err: wrong buswidth config: %u!\n", mmc_PMuxInfo.u8_busWidth);
                break;
        }
    }
    CARD_REG_W(A_RD_SBIT_TIMER_REG(mmc_PMuxInfo.u8_ipOrder), 0x0);
}

void SDMMC_Write_Timeout_Set(IpOrder eIP)
{
    if ((IpSelect)eIP == IP_SD)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T);
    }
    else if ((IpSelect)eIP == IP_SDIO)
    {
    }
    else if ((IpSelect)eIP == IP_FCIE)
    {
        CARD_REG(GET_CARD_REG_ADDR(A_MIU_BANK, 0x14)) = 0x400; // mask fcie read miu
        CARD_REG(GET_CARD_REG_ADDR(A_MIU_BANK, 0x13)) = 0x17;
        CARD_REG(GET_CARD_REG_ADDR(A_MIU_BANK, 0x43)) = 0x17;
        Hal_Timer_mDelay(1);
    }
    CARD_REG_W(A_WR_SBIT_TIMER_REG(eIP), 0x8001);
}

void SDMMC_Write_Timeout_Clear(IpOrder eIP)
{
    if ((IpSelect)eIP == IP_SD)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T);
    }
    else if ((IpSelect)eIP == IP_SDIO)
    {
    }
    else if ((IpSelect)eIP == IP_FCIE)
    {
        CARD_REG(GET_CARD_REG_ADDR(A_MIU_BANK, 0x14)) = 0x00; // PAD_EMMC_D0
        CARD_REG(GET_CARD_REG_ADDR(A_MIU_BANK, 0x13)) = 0x17; // PAD_EMMC_D0
        CARD_REG(GET_CARD_REG_ADDR(A_MIU_BANK, 0x43)) = 0x17; // PAD_EMMC_D3
    }
    CARD_REG_W(A_WR_SBIT_TIMER_REG(eIP), 0x0);
}
#elif (D_OS == D_OS__RTK)
                                                                                ///
#endif
